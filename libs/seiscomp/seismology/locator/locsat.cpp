/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT LOCSAT
#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/system.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/seismology/ttt.h>

#include <stdlib.h>
#include <math.h>
#include <list>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "locsat_private.h"

// IGN additions for OriginUncertainty computation
#include "eigv.h"
#include "chi2.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


using namespace Seiscomp;
using namespace Seiscomp::Seismology;
namespace dm = Seiscomp::DataModel;


//#define LOCSAT_TESTING

#define TRUE  1
#define FALSE 0
#define ARRIVAL_DEFAULT_TIME_ERROR 1.0
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
enum LOCSATParams {
	LP_NUM_DEG_FREEDOM,      /* 9999    - number of degrees of freedom    */
	LP_EST_STD_ERROR,        /* 1.0     - estimate of data std error      */
	LP_CONF_LEVEL,           /* 0.9     - confidence level    		     */
	LP_DAMPING,              /* -1.0    - damping (-1.0 means no damping) */
	LP_MAX_ITERATIONS,       /* 20      - limit iterations to convergence */
	LP_FIX_DEPTH,            /* true    - use fixed depth ?               */
	LP_FIXING_DEPTH,         /* 0.0     - fixing depth value              */
	LP_LAT_INIT,             /* modifiable - initial latitude             */
	LP_LONG_INIT,            /* modifiable - initial longitude            */
	LP_DEPTH_INIT,           /* modifiable - initial depth                */
	LP_USE_LOCATION,         /* true    - use current origin data ?       */
	LP_VERBOSE,              /* true    - verbose output of data ?        */
	LP_COR_LEVEL,            /* 0       - correction table level          */
	LP_OUT_FILENAME,         /* nullptr    - name of file to print data      */
	LP_PREFIX,               /* nullptr    - dir name & prefix of tt tables  */
	LP_MIN_ARRIVAL_WEIGHT,   /* 0.5     - if arr-weight = less than this, locsat will ignore this arrival */
	LP_DEFAULT_TIME_ERROR,   /* 1.0     - the default pick uncertainty */
	LP_USE_PICK_UNCERTAINTY, /* false   - whether to use pick uncertainty or not */
	LP_USE_PICK_BACKAZIMUTH, /* true    - whether to use pick backazimuth or not */
	LP_USE_PICK_SLOWNESS     /* true    - whether to use pick slowness or not */
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float getTimeError(const dm::Pick *pick,
                   double defaultTimeError,
                   bool useUncertainties) {
	if ( useUncertainties ) {
		try {
			return pick->time().uncertainty();
		}
		catch ( ... ) {
			try {
				return 0.5 * (pick->time().lowerUncertainty() + pick->time().upperUncertainty());
			}
			catch ( ... ) {}
		}
	}

	return defaultTimeError;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool atTransitionPtoPKP(const dm::Arrival* arrival) {
	return (arrival->distance() > 106.9 && arrival->distance() < 111.1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Let this be a local hack for the time being. See the same routine in Autoloc
bool travelTimeP(double lat, double lon, double depth, double delta, double azi, TravelTime &tt) {
	static Seiscomp::TravelTimeTable ttt;

	double lat2, lon2;
	Math::Geo::delandaz2coord(delta, azi, lat, lon, &lat2, &lon2);

	Seiscomp::TravelTimeList
		*ttlist = ttt.compute(lat, lon, depth, lat2, lon2, 0);

	if ( ttlist == nullptr || ttlist->empty() )
		return false;

	for (Seiscomp::TravelTimeList::iterator
	     it = ttlist->begin(); it != ttlist->end(); ++it) {
		tt = *it;
		if (delta < 114)
			// for  distances < 114, allways take 1st arrival
			break;
		if (tt.phase.substr(0,2) != "PK")
			// for  distances >= 114, skip Pdiff etc., take first
			// PKP*, PKiKP*
			continue;
		break;
	}
	delete ttlist;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string LOCSAT::_defaultTablePrefix = "iasp91";
const LOCSAT::IDList LOCSAT::_allowedParameters = {
	"VERBOSE",
	"MAX_ITERATIONS",
	"NUM_DEG_FREEDOM",
	"CONF_LEVEL",
	"DEFAULT_TIME_ERROR",
	"USE_PICK_UNCERTAINTY",
	"USE_PICK_BACKAZIMUTH",
	"USE_PICK_SLOWNESS"
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_LOCATOR(LOCSAT, "LOCSAT");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LOCSAT::LOCSAT() {
	_name = "LOCSAT";
	_params.prefix = new char[1024];
	_params.prefix[1023] = '\0';
	_computeConfidenceEllipsoid = false;
	_enableDebugOutput = false;
	_profiles.push_back("iasp91");
	_profiles.push_back("tab");
	reset();
	LOCSAT::setProfile(_defaultTablePrefix);
	setDefaultLocatorParams();
	sc_locsat_init_ttt(&_ttt);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LOCSAT::~LOCSAT() {
	sc_locsat_free_ttt(&_ttt);
	delete[] _params.prefix;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define P(X) _params.X
bool LOCSAT::init(const Config::Config &config) {
	setDefaultLocatorParams();

	try {
		_profiles = config.getStrings("LOCSAT.profiles");
	}
	catch ( ... ) {
		_profiles.clear();
		_profiles.push_back("iasp91");
		_profiles.push_back("tab");
	}

	try {
		_computeConfidenceEllipsoid = config.getBool("LOCSAT.enableConfidenceEllipsoid");
	}
	catch ( ... ) {}

	try {
		_enableDebugOutput = config.getBool("LOCSAT.enableDebugOutput");
		if ( _enableDebugOutput ) {
			setLocatorParams(LP_VERBOSE, "y");
		}
	}
	catch ( ... ) {}

	try {
		P(depth_init) = config.getDouble("LOCSAT.depthInit");
	}
	catch ( ... ) {}

	try {
		_usePickUncertainties = config.getBool("LOCSAT.usePickUncertainties");
	}
	catch ( ... ) {}

	try {
		_usePickBackazimuth = config.getBool("LOCSAT.usePickBackazimuth");
	}
	catch ( ... ) {}

	try {
		_usePickSlowness = config.getBool("LOCSAT.usePickSlowness");
	}
	catch ( ... ) {}

	try {
		_defaultPickUncertainty = config.getDouble("LOCSAT.defaultTimeError");
	}
	catch ( ... ) {}

	try {
		P(num_dof) = config.getInt("LOCSAT.degreesOfFreedom");
	}
	catch ( ... ) {}

	try {
		P(conf_level) = config.getDouble("LOCSAT.confLevel");
		if ( (P(conf_level) < 0.5) || (P(conf_level) > 1) ) {
			SEISCOMP_ERROR("LOCSAT.confLevel: must be >= 0.5 and <= 1");
			return false;
		}
	}
	catch ( ... ) {}

	if ( _enableDebugOutput )
		SEISCOMP_INFO("LOCSAT: enabled locator-specific debug output");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LocatorInterface::IDList LOCSAT::parameters() const {
	return _allowedParameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string LOCSAT::parameter(const std::string &name) const {
	if ( name == "VERBOSE" ) {
		return getLocatorParams(LP_VERBOSE);
	}
	else if ( name == "MAX_ITERATIONS" ) {
		return getLocatorParams(LP_MAX_ITERATIONS);
	}
	else if ( name == "NUM_DEG_FREEDOM" ) {
		return getLocatorParams(LP_NUM_DEG_FREEDOM);
	}
	else if ( name == "CONF_LEVEL" ) {
		return getLocatorParams(LP_CONF_LEVEL);
	}
	else if ( name == "DEFAULT_TIME_ERROR" ) {
		return getLocatorParams(LP_DEFAULT_TIME_ERROR);
	}
	else if ( name == "USE_PICK_UNCERTAINTY" ) {
		return getLocatorParams(LP_USE_PICK_UNCERTAINTY);
	}
	else if ( name == "USE_PICK_BACKAZIMUTH" ) {
		return getLocatorParams(LP_USE_PICK_BACKAZIMUTH);
	}
	else if ( name == "USE_PICK_SLOWNESS" ) {
		return getLocatorParams(LP_USE_PICK_SLOWNESS);
	}

	return std::string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LOCSAT::setParameter(const std::string &name,
                          const std::string &value) {
	if ( name == "VERBOSE" )
		setLocatorParams(LP_VERBOSE, value.c_str());
	else if ( name == "MAX_ITERATIONS" )
		setLocatorParams(LP_MAX_ITERATIONS, value.c_str());
	else if ( name == "NUM_DEG_FREEDOM" )
		setLocatorParams(LP_NUM_DEG_FREEDOM, value.c_str());
	else if ( name == "CONF_LEVEL" )
		setLocatorParams(LP_CONF_LEVEL, value.c_str());
	else if ( name == "DEFAULT_TIME_ERROR" )
		setLocatorParams(LP_DEFAULT_TIME_ERROR, value.c_str());
	else if ( name == "USE_PICK_UNCERTAINTY" )
		setLocatorParams(LP_USE_PICK_UNCERTAINTY, value.c_str());
	else if ( name == "USE_PICK_BACKAZIMUTH" )
		setLocatorParams(LP_USE_PICK_BACKAZIMUTH, value.c_str());
	else if ( name == "USE_PICK_SLOWNESS" )
		setLocatorParams(LP_USE_PICK_SLOWNESS, value.c_str());
	else
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int LOCSAT::capabilities() const {
	return InitialLocation | FixedDepth | IgnoreInitialLocation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
dm::Origin* LOCSAT::locate(PickList &pickList,
                           double initLat, double initLon, double initDepth,
                           const Core::Time &initTime) {
	P(lat_init) = initLat;
	P(lon_init) = initLon;
	P(depth_init) = initDepth;

	reset();
	setOrigin(initLat, initLon, initDepth);
	setOriginTime(initTime.epoch());

	if ( isInitialLocationIgnored() ) {
		setLocatorParams(LP_USE_LOCATION, "n");
	}
	else {
		setLocatorParams(LP_USE_LOCATION, "y");
	}

	return fromPicks(pickList);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
dm::Origin* LOCSAT::locate(PickList &picks) {
	reset();
	setOrigin(0.0, 0.0, 0.0);
	setOriginTime(0.0);
	setLocatorParams(LP_USE_LOCATION, "n");
	return fromPicks(picks);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
dm::Origin* LOCSAT::fromPicks(PickList &picks){
	if ( _usingFixedDepth ) {
		P(fixing_depth) = _fixedDepth;
		P(fix_depth) = 'y';
	}
	else {
		P(fix_depth) = 'n';
	}

	size_t i = 0;

	for ( auto &pickItem : picks ) {
		auto pick = pickItem.pick.get();
		auto sloc = getSensorLocation(pick);

		if ( sloc ) {
			std::string stationID = pick->waveformID().networkCode()+"."+
			                        pick->waveformID().stationCode();

			if ( !pick->waveformID().locationCode().empty() ) {
				stationID +=  ".";
				stationID += pick->waveformID().locationCode();
			}

			addSite(stationID.c_str(),
			        sloc->latitude(), sloc->longitude(),
			        sloc->elevation());

			std::string phase;
			try { phase = pick->phaseHint().code(); }
			catch (...) { phase = "P"; }

			double cor = stationCorrection(stationID, pick->waveformID().stationCode(), phase);
			addArrival(i++, stationID.c_str(), phase.c_str(),
			           pick->time().value().epoch() - cor,
			           getTimeError(pick, _defaultPickUncertainty, _usePickUncertainties),
			           pickItem.flags & F_TIME);

			// Set backazimuth
			if ( _usePickBackazimuth && (pickItem.flags & F_BACKAZIMUTH) ) {
				try {
					double az = pick->backazimuth().value();
					double delaz;
					try {
						delaz = pick->backazimuth().uncertainty();
					}
					// Default delaz
					catch ( ... ) {
						delaz = 1.0;
					}
					setArrivalAzimuth(az, delaz, 1);
				}
				catch ( ... ) {}
			}

			// Set slowness
			if ( _usePickSlowness && (pickItem.flags & F_SLOWNESS) ) {
				try {
					double slo = pick->horizontalSlowness().value();
					double delslo;

					try {
						delslo = pick->horizontalSlowness().uncertainty();
					}
					// Default delaz
					catch ( ... ) {
						delslo = 1.0;
					}

					setArrivalSlowness(slo, delslo, 1);
				}
				catch ( ... ) {}
			}

#ifdef LOCSAT_TESTING
			SEISCOMP_DEBUG("pick station: %s", stationID.c_str());
			SEISCOMP_DEBUG("station lat: %.2f", (double)sloc->latitude());
			SEISCOMP_DEBUG("station lon: %.2f", (double)sloc->longitude());
			SEISCOMP_DEBUG("station elev: %.2f", (double)sloc->elevation());
#endif
		}
		else {
			throw StationNotFoundException("station '" + pick->waveformID().networkCode() +
			                               "." + pick->waveformID().stationCode() +
			                               "." + pick->waveformID().locationCode() + "' not found");
		}
	}

	dm::Origin *origin = locate();

	if ( origin ) {
		std::set<std::string> stationsUsed;
		std::set<std::string> stationsAssociated;

		for ( auto &arrival : _arrivals ) {
			size_t arid = arrival.arid;
			if ( arid >= picks.size() ) {
				continue;
			}
			dm::Pick* p = picks[arid].pick.get();

			if ( i < origin->arrivalCount() ) {
				origin->arrival(i)->setPickID(p->publicID());

				stationsAssociated.insert(p->waveformID().networkCode() + "." + p->waveformID().stationCode());

				try {
					if ( origin->arrival(i)->weight() == 0 ) {
						continue;
					}
				}
				catch ( ... ) {}

				stationsUsed.insert(p->waveformID().networkCode() + "." + p->waveformID().stationCode());
			}
		}

		try {
			origin->quality().setUsedStationCount(stationsUsed.size());
			origin->quality().setAssociatedStationCount(stationsAssociated.size());
		}
		catch ( ... ) {
			dm::OriginQuality oq;
			oq.setUsedStationCount(stationsUsed.size());
			oq.setAssociatedStationCount(stationsAssociated.size());
			origin->setQuality(oq);
		}
	}

	return origin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
dm::Origin *LOCSAT::relocate(const dm::Origin *origin) {
	if ( !origin ) {
		return nullptr;
	}

	reset();

	if ( isInitialLocationIgnored() ) {
		setLocatorParams(LP_USE_LOCATION, "n");
	}
	else {
		setLocatorParams(LP_USE_LOCATION, "y");
	}

	double depth = 0.0;
	try { depth = origin->depth().value(); } catch (...) {}

	P(lat_init) = origin->latitude().value();
	P(lon_init) = origin->longitude().value();
	P(depth_init) = depth;

	setOrigin(origin->latitude().value(), origin->longitude().value(), depth);
	setOriginTime(origin->time().value().epoch());

	if ( !loadArrivals(origin)) {
		return nullptr;
	}

	if ( _usingFixedDepth ) {
		P(fixing_depth) = _fixedDepth;
		P(fix_depth) = 'y';
	}
	else {
		P(fix_depth) = 'n';
	}

	dm::Origin *result = locate();

	if ( result ) {
		std::set<std::string> stationsUsed;
		std::set<std::string> stationsAssociated;

		for ( size_t i = 0; i < _arrivals.size(); ++i ) {
			auto &arrival = _arrivals[i];
			size_t arid = arrival.arid;

			if ( arid >= origin->arrivalCount() ) {
				continue;
			}

			if ( i < result->arrivalCount() ) {
				result->arrival(i)->setPickID(origin->arrival(arid)->pickID());
				auto p = dm::Pick::Find(result->arrival(i)->pickID());

				if ( p ) {
					stationsAssociated.insert(p->waveformID().networkCode() + "." + p->waveformID().stationCode());
				}

				try {
					if ( result->arrival(i)->weight() == 0 ) {
						continue;
					}
				}
				catch ( ... ) {}

				if ( p ) {
					stationsUsed.insert(p->waveformID().networkCode() + "." + p->waveformID().stationCode());
				}
			}
		}

		try {
			result->quality().setUsedStationCount(stationsUsed.size());
			result->quality().setAssociatedStationCount(stationsAssociated.size());
		}
		catch ( ... ) {
			dm::OriginQuality oq;
			oq.setUsedStationCount(stationsUsed.size());
			oq.setAssociatedStationCount(stationsAssociated.size());
			result->setQuality(oq);
		}
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double LOCSAT::stationCorrection(const std::string &staid,
                                 const std::string &stacode,
                                 const std::string &phase) const {
	StationCorrectionMap::const_iterator it = _stationCorrection.find(staid);
	if ( it != _stationCorrection.end() ) {
		PhaseCorrectionMap::const_iterator pit = it->second.find(phase);
		if ( pit != it->second.end() ) {
			SEISCOMP_DEBUG("LOCSAT: stacorr(%s,%s) = %f", staid.c_str(), phase.c_str(), pit->second);
			return pit->second;
		}
	}

	it = _stationCorrection.find(stacode);
	if ( it != _stationCorrection.end() ) {
		PhaseCorrectionMap::const_iterator pit = it->second.find(phase);
		if ( pit != it->second.end() ) {
			SEISCOMP_DEBUG("LOCSAT: stacorr(%s,%s) = %f", stacode.c_str(), phase.c_str(), pit->second);
			return pit->second;
		}
	}

	return 0.0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LOCSAT::loadArrivals(const dm::Origin *origin) {
	if ( !origin )
		return false;

#ifdef LOCSAT_TESTING
	SEISCOMP_DEBUG("Load arrivals:");
#endif
	for (unsigned int i = 0; i < origin->arrivalCount(); i++){
		dm::Arrival *arrival = origin->arrival(i);
		dm::Pick *pick = getPick(arrival);
		if (!pick){
			throw PickNotFoundException("pick '" + arrival->pickID() + "' not found");
		}
		double traveltime = double(pick->time().value() - origin->time().value());

		int defining = 1;

		try{
			// work around problem related to discontinuity in the travel-time tables
			// at the P->PKS transition
			if ( atTransitionPtoPKP(arrival) )
				defining = 0;
		}
		catch (...) {}

		dm::SensorLocation *sloc = getSensorLocation(pick);
		if (!sloc){
			throw StationNotFoundException("station '" + pick->waveformID().networkCode() +
			                               "." + pick->waveformID().stationCode() + "' not found");
		}

		std::string phaseCode;
		try {
			phaseCode = arrival->phase().code();
			// Rename P to PKP where appropriate. A first arrival
			// with a traveltime of > 1000 s observed at distances
			// > 110 deg is definitely not P but PKP, as PKP
			// traveltime is always well above 1000 s and P traveltime
			// is always well below.
			if ((phaseCode=="P" || phaseCode=="P1") && arrival->distance() > 110 && traveltime > 1000) {
				phaseCode="PKP";
			}
		}
		catch (...) {
			try {
				phaseCode = pick->phaseHint().code();
			}
			catch (...) {
				phaseCode = "P";
			}
		}

		/*
		// This is essentially a whitelist for automatic phases. Only
		// P and PKP are currently deemed acceptable automatic phases.
		// Others may be associated but cannot be used because of
		// this:
		if (pick->evaluationMode() == dm::AUTOMATIC &&
		    ! ( phaseCode == "P" || phaseCode == "PKP") ) // TODO make this configurable
			defining=0;
		*/

		std::string stationID = pick->waveformID().networkCode()+"."+
		                        pick->waveformID().stationCode();

		if ( !pick->waveformID().locationCode().empty() ) {
			stationID +=  ".";
			stationID += pick->waveformID().locationCode();
		}

#ifdef LOCSAT_TESTING
		SEISCOMP_DEBUG(" [%s] set phase to %s", stationID.c_str(), phaseCode.c_str());
#endif

		try {
			addSite(stationID.c_str(),
			        sloc->latitude(), sloc->longitude(), sloc->elevation());
		}
		catch ( std::exception &e ) {
			throw LocatorException(sloc->publicID() + "' (" +
			                       stationID + "): " + e.what());
		}

		double cor = stationCorrection(stationID, pick->waveformID().stationCode(), phaseCode);

		bool timeUsed;
		try {
			timeUsed = arrival->timeUsed();
		}
		catch ( ... ) {
			timeUsed = defining ? true : false;
		}

		addArrival(i, stationID.c_str(), phaseCode.c_str(),
		           pick->time().value().epoch() - cor,
		           getTimeError(pick, _defaultPickUncertainty, _usePickUncertainties),
		           timeUsed ? 1 : 0);

		// Set backazimuth
		bool backazimuthUsed = true;
		try {
			backazimuthUsed = arrival->backazimuthUsed();
		}
		catch ( ... ) {}

		if ( _usePickBackazimuth && backazimuthUsed ) {
			try {
				float az = pick->backazimuth().value();
				float delaz;

				try {
					delaz = pick->backazimuth().uncertainty();
				}
				// Default delaz
				catch ( ... ) {
					delaz = 1.0;
				}

				setArrivalAzimuth(az, delaz, 1);
			}
			catch ( ... ) {}
		}

		// Set slowness
		bool horizontalSlownessUsed = true;
		try {
			horizontalSlownessUsed = arrival->horizontalSlownessUsed();
		}
		catch ( ... ) {}

		if ( _usePickSlowness && horizontalSlownessUsed ) {
			try {
				float slo = pick->horizontalSlowness().value();
				float delslo;

				try {
					delslo = pick->horizontalSlowness().uncertainty();
				}
				// Default delaz
				catch ( ... ) {
					delslo = 1.0;
				}

				setArrivalSlowness(slo, delslo, 1);
			}
			catch ( ... ) {}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::setDefaultLocatorParams() {
	P(use_location)   = TRUE;
	P(fix_depth)      = 'n';
	P(fixing_depth)   = 20.0;
	P(verbose)        = 'n';
	P(lat_init)       = 999.9;
	P(lon_init)       = 999.9;
	P(depth_init)     = 20.0;
	P(conf_level)     = 0.90;
	P(damp)           = -1.00;
	P(est_std_error)  = 1.00;
	P(num_dof)        = 9999;
	P(max_iterations) = 100;
	_defaultPickUncertainty = ARRIVAL_DEFAULT_TIME_ERROR;
	_usePickUncertainties   = false;
	_usePickBackazimuth     = true;
	_usePickSlowness        = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LocatorInterface::IDList LOCSAT::profiles() const {
	return _profiles;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::setProfile(const std::string &prefix) {
	if ( prefix.empty() ) {
		return;
	}

	_stationCorrection.clear();
	_tablePrefix = prefix;
	const char *tablePath = getenv("SEISCOMP_LOCSAT_TABLE_DIR");
	if ( tablePath ) {
		SC_FS_DECLARE_PATH(path, tablePath);
		path /= _tablePrefix;
		strncpy(P(prefix), path.string().c_str(), 1023);
	}
	else {
		strncpy(P(prefix), (Environment::Instance()->shareDir() + "/locsat/tables/" + _tablePrefix).c_str(), 1023);
	}

	std::ifstream ifs;
	ifs.open((Environment::Instance()->shareDir() + "/locsat/tables/" + _tablePrefix + ".stacor").c_str());
	if ( !ifs.is_open() ) {
		SEISCOMP_DEBUG("LOCSAT: no station corrections used for profile %s", _tablePrefix.c_str());
	}
	else {
		std::string line;
		int lc = 1;
		int cnt = 0;
		for ( ; std::getline(ifs, line); ++lc ) {
			Core::trim(line);
			if ( line.empty() ) continue;
			if ( line[0] == '#' ) continue;

			std::vector<std::string> toks;

			Core::split(toks, line.c_str(), " \t");

			if ( toks.size() != 5 ) {
				SEISCOMP_WARNING("LOCSAT: invalid station correction in line %d: expected 5 columns", lc);
				continue;
			}

			if ( toks[0] != "LOCDELAY" ) {
				SEISCOMP_WARNING("LOCSAT: invalid station correction in line %d: expected LOCDELAY", lc);
				continue;
			}

			int num_phases;
			double correction;

			if ( !Core::fromString(num_phases, toks[3]) ) {
				SEISCOMP_WARNING("LOCSAT: invalid station correction in line %d: 4th column is not an integer", lc);
				continue;
			}

			if ( !Core::fromString(correction, toks[4]) ) {
				SEISCOMP_WARNING("LOCSAT: invalid station correction in line %d: 5th column is not a double", lc);
				continue;
			}

			_stationCorrection[toks[1]][toks[2]] = correction;
			++cnt;
		}

		SEISCOMP_DEBUG("LOCSAT: loaded %d station corrections from %d configuration lines",
		               cnt, lc);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string LOCSAT::getLocatorParams(int param) const {
	char value[256];

	switch ( param ) {
		case LP_USE_LOCATION:
			if ( P(use_location) == TRUE ) {
				strcpy(value, "y");
			}
			else {
				strcpy(value, "n");
			}
			break;

		case LP_FIX_DEPTH:
			value[0] = P(fix_depth);
			value[1] = '\0';
			break;

		case LP_FIXING_DEPTH:
			sprintf(value, "%7.2f", P(fixing_depth));
			break;

		case LP_VERBOSE:
			value[0] = P(verbose);
			value[1] = '\0';
			break;

		case LP_PREFIX:
			return P(prefix);
			break;

		case LP_MAX_ITERATIONS:
			sprintf(value, "%d", P(max_iterations));
			break;

		case LP_EST_STD_ERROR:
			sprintf(value, "%7.2f", P(est_std_error));
			break;

		case LP_NUM_DEG_FREEDOM:
			sprintf(value, "%d", P(num_dof));
			break;

		case LP_CONF_LEVEL:
			sprintf(value, "%5.3f", P(conf_level));
			break;

		case LP_MIN_ARRIVAL_WEIGHT:
			sprintf(value, "%7.2f", _minArrivalWeight);
			break;

		case LP_DEFAULT_TIME_ERROR:
			sprintf(value, "%f", _defaultPickUncertainty);
			break;

		case LP_USE_PICK_UNCERTAINTY:
			if ( _usePickUncertainties ) {
				strcpy(value, "y");
			}
			else {
				strcpy(value, "n");
			}
			break;

		case LP_USE_PICK_BACKAZIMUTH:
			if ( _usePickBackazimuth ) {
				strcpy(value, "y");
			}
			else {
				strcpy(value, "n");
			}
			break;

		case LP_USE_PICK_SLOWNESS:
			if ( _usePickSlowness ) {
				strcpy(value, "y");
			}
			else {
				strcpy(value, "n");
			}
			break;

		default:
			SEISCOMP_ERROR("getLocatorParam: wrong Parameter: %d", param);
			return "error";
	}

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::setLocatorParams(int param, const char* value){
	switch ( param ) {
		case LP_USE_LOCATION:
			if ( !strcmp(value, "y") ) {
				P(use_location) = TRUE;
			}
			else {
				P(use_location) = FALSE;
			}
			break;

		case LP_FIX_DEPTH:
			P(fix_depth) = value[0];
			break;

		case LP_FIXING_DEPTH:
			P(fixing_depth) = atof(value);
			break;

		case LP_VERBOSE:
			if ( !strcmp(value, "y") ) {
				P(verbose) = 'y';
			}
			else {
				P(verbose) = 'n';
			}
			break;

		case LP_PREFIX:
			strncpy(P(prefix), value, 1024);
			P(prefix)[1023] = '\0';
			break;

		case LP_MAX_ITERATIONS:
			P(max_iterations) = atoi(value);
			break;

		case LP_EST_STD_ERROR:
			P(est_std_error) = atof(value);
			break;

		case LP_NUM_DEG_FREEDOM:
			P(num_dof) = atoi(value);
			break;

		case LP_CONF_LEVEL:
			P(conf_level) = atof(value);
			break;

		case LP_MIN_ARRIVAL_WEIGHT:
			_minArrivalWeight = atof(value);
			break;

		case LP_DEFAULT_TIME_ERROR:
			_defaultPickUncertainty = atof(value);
			break;

		case LP_USE_PICK_UNCERTAINTY:
			if ( !strcmp(value, "y") ) {
				_usePickUncertainties = true;
			}
			else {
				_usePickUncertainties = false;
			}
			break;

		case LP_USE_PICK_BACKAZIMUTH:
			if ( !strcmp(value, "y") ) {
				_usePickBackazimuth = true;
			}
			else {
				_usePickBackazimuth = false;
			}
			break;

		case LP_USE_PICK_SLOWNESS:
			if ( !strcmp(value, "y") ) {
				_usePickSlowness = true;
			}
			else {
				_usePickSlowness = false;
			}
			break;

		default:
			SEISCOMP_ERROR("setLocatorParam: wrong Parameter: %d", param);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define CUSTOM_VERBOSE_OUTPUT 0


#if CUSTOM_VERBOSE_OUTPUT
std::ostream &operator<<(std::ostream &os, const site &s) {
	os << s.sta << ";" << s.ondate << ";" << s.offdate << ";" << s.lat << ";" << s.lon << ";" << s.elev;
	return os;
}

std::ostream &operator<<(std::ostream &os, const arrival &a) {
	os << a.arid << ";" << a.azimuth << ";" << a.delaz << ";" << a.delslo
	   << ";" << a.deltim << ";" << a.iphase << ";" << a.slow << ";" << a.sta
	   << ";" << a.stype << ";" << std::fixed << a.time;
	return os;
}

std::ostream &operator<<(std::ostream &os, const assoc &a) {
	os << a.arid << ";" << a.azdef << ";" << a.azres << ";" << a.belief << ";"
	   << a.delta << ";" << a.delta << ";" << a.emares << ";" << a.esaz << ";"
	   << a.phase << ";" << a.seaz << ";" << a.slodef << ";" << a.slores << ";"
	   << a.sta << ";" << a.timedef << ";" << a.timeres << ";" << a.vmodel << ";"
	   << a.wgt;
	return os;
}

std::ostream &operator<<(std::ostream &os, const LocatorError &err) {
	os << err.arid << ";" << err.az << ";" << err.slow << ";" << std::fixed << err.time;
	return os;
}

std::ostream &operator<<(std::ostream &os, const LocatorParams &params) {
	os << params.conf_level << ";" << params.cor_level << ";" << params.damp
	   << ";" << params.depth_init << ";" << params.est_std_error << ";"
	   << params.fixing_depth << ";" << params.fix_depth << ";"
	   << params.lat_init << ";" << params.lon_init << ";" << params.max_iterations
	   << ";" << params.num_dof << ";" << params.outfile_name << ";"
	   << params.prefix << ";" << params.use_location << ";" << params.verbose;
	return os;
}
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::reset() {
	static const LOCSAT_Origin Na_Origin = Na_Origin_Init;
	static const LOCSAT_Origerr Na_Origerr = Na_Origerr_Init;

	_sites.clear();
	_arrivals.clear();
	_assocs.clear();
	_errors.clear();
	_origin = Na_Origin;
	_origerr = Na_Origerr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Origin *LOCSAT::locate() {
	if ( _arrivals.empty() ) {
		throw LocatorException("error: Too few usable data");
	}

	//! XXX 9999 should be enough!
	//! changed original FORTRAN/C files: maxdata 300 --> 9999
	//! -------------------------------------------------------
	// librdwrt/rdcor.h:14
	// libloc/hypinv0.f:217
	// libloc/locsat0.f:215
	// libloc/hypcut0.f:149
	// libloc/rdcor.h:14
	// libloc/solve_via_svd_.c:122
	//! -------------------------------------------------------
	if ( _arrivals.size() > 9999 ) {
		throw LocatorException("error: Too many picks/stations [9999] - Please raise limits within pre-f2c locsat code!");
	}

#if CUSTOM_VERBOSE_OUTPUT
	int id = 0;
	for ( auto &site : _sites ) {
		std::cerr << "S#" << id++ << "\t" << site << std::endl;
	}
	id = 0;
	for ( auto &arrival : _arrivals ) {
		std::cerr << "P#" << id++ << "\t" << arrival << std::endl;
	}
	id = 0;
	for ( auto &assoc : _assocs ) {
		std::cerr << "A#" << id++ << "\t" << assoc << std::endl;
	}
	id = 0;
	for ( auto &error : _errors ) {
		std::cerr << "E#" << id++ << "\t" << error << std::endl;
	}
	std::cerr << "PARAMS" << std::endl;
	std::cerr << _params << std::endl;
#endif

	int ierr = sc_locsat_locate_event(
		&_ttt, _sites.data(), static_cast<int>(_sites.size()),
		_arrivals.data(), _assocs.data(),
		&_origin, &_origerr, &_params,
		_errors.data(), static_cast<int>(_arrivals.size())
	);

	//std::cerr << "ierr = locate_event: " <<  ierr << std::endl;

	switch ( ierr ) {
		case 0:
			break;
		case LOCSAT_GLerror1:
			throw LocatorException("Exceeded maximum iterations");
		case LOCSAT_GLerror2:
			throw LocatorException("Solution did not converge");
		case LOCSAT_GLerror3:
			throw LocatorException("Too few usable data");
		case LOCSAT_GLerror4:
			throw LocatorException("Too few data to constrain O.T.");
		case LOCSAT_GLerror5:
			throw LocatorException("Insufficient data for a solution");
		case LOCSAT_GLerror6:
			throw LocatorException("SVD routine can't decompose matrix");
		case LOCSAT_GLerror7:
			throw LocatorException("No observations to process");
		case LOCSAT_GLerror8:
			throw LocatorException("Bad assoc data");
		case LOCSAT_GLerror9:
			throw LocatorException("Bad origin pointer");
		case LOCSAT_GLerror10:
			throw LocatorException("Bad origerr pointer");
		case LOCSAT_GLerror11:
			throw LocatorException("Mismatch between arrival/assoc");
		case LOCSAT_TTerror1:
			throw LocatorException("Null phase_type list");
		case LOCSAT_TTerror2:
			throw LocatorException("Opening travel time tables");
		case LOCSAT_TTerror3:
			throw LocatorException("Error reading travel time tables, unexpected EOF");
		case LOCSAT_TTerror4:
			throw LocatorException("Error reading travel time tables, too many distance or depth samples");
		case LOCSAT_TTerror5:
			throw LocatorException("Unknown error reading travel time tables");
		case LOCSAT_TTerror6:
			throw LocatorException("Insufficient memory for travel-time tables");
		default:
		{
			std::stringstream ss;
			ss << "error from locator: code " << ierr;
			throw LocatorException(ss.str());
		}
	}

	auto origin = dm::Origin::Create();

	if ( !origin ) {
		return nullptr;
	}

	dm::CreationInfo ci;
	ci.setCreationTime(Core::Time().gmt());
	origin->setCreationInfo(ci);

	origin->setMethodID(_name);
	origin->setEarthModelID(_tablePrefix);
	origin->setLatitude(
		dm::RealQuantity(
			_origin.lat, sqrt(_origerr.syy),
			Core::None, Core::None, Core::None
		)
	);
	origin->setLongitude(
		dm::RealQuantity(
			_origin.lon, sqrt(_origerr.sxx),
			Core::None, Core::None, Core::None
		)
	);
	origin->setDepth(
		dm::RealQuantity(
			_origin.depth, sqrt(_origerr.szz),
			Core::None, Core::None, Core::None
		)
	);

	origin->setTime(
		dm::TimeQuantity(
			Core::Time(_origin.time),
			sqrt(_origerr.stt),
			Core::None, Core::None, Core::None
		)
	);

	double rms = 0;
	int phaseAssocCount = 0;
	int usedAssocCount = 0;
	std::vector<double> dist;
	std::vector<double> azi;
	int depthPhaseCount = 0;
	int rmsCount = 0;

	for ( size_t i = 0; i < _arrivals.size(); ++i ) {
		auto &arr = _arrivals[i];
		auto &assoc = _assocs[i];
		++phaseAssocCount;

		dm::ArrivalPtr arrival = new dm::Arrival();

		arrival->setTimeUsed(assoc.timedef == 'd' ? true : false);
		arrival->setBackazimuthUsed(assoc.azdef == 'd' ? true : false);
		arrival->setHorizontalSlownessUsed(assoc.slodef == 'd' ? true : false);

		bool isUsed = arrival->timeUsed() || arrival->backazimuthUsed() || arrival->horizontalSlownessUsed();

		// To have different pickID's just generate some based on
		// the index. They become set correctly later on.
		arrival->setPickID(Core::toString(i));

		if ( (_errors[i].arid != 0) || !isUsed ) {
			arrival->setWeight(0.0);
		}
		else {
			arrival->setWeight(1.0);
			dist.push_back(assoc.delta);
			azi.push_back(assoc.esaz);
			++usedAssocCount;
		}

		if ( arrival->timeUsed() ) {
			rms += (assoc.timeres * assoc.timeres);
			++rmsCount;
		}

		arrival->setDistance(assoc.delta);
		arrival->setTimeResidual(assoc.timeres);
		arrival->setAzimuth(assoc.esaz);
		arrival->setPhase(dm::Phase(assoc.phase));
		if ( arrival->phase().code()[0] == 'p' || arrival->phase().code()[0] == 's' ) {
			if ( isUsed ) {
				depthPhaseCount++;
			}
		}

		// This is a workaround for what seems to be a problem with LOCSAT,
		// namely, that in a narrow distance range around 108 degrees
		// sometimes picks suddenly have a residual of > 800 sec after
		// relocation. The reason is not clear.
		if ( arrival->timeResidual() > 800 && \
		   ( arrival->phase().code()=="P" || arrival->phase().code()=="Pdiff" ) && \
		     atTransitionPtoPKP(arrival.get())) {

			TravelTime tt;
			if ( travelTimeP(origin->latitude(), origin->longitude(), origin->depth(), arrival->distance(), arrival->azimuth(), tt) ) {
				double res = arr.time - (origin->time().value() + Core::TimeSpan(tt.time)).epoch();
				arrival->setTimeResidual(res);
			}
		}

		// Populate horizontal slowness residual
		if ( assoc.slores > -990. ) {
			arrival->setHorizontalSlownessResidual(assoc.slores);
		}

		// Populate backazimuth residual
		if ( assoc.azres > -990. ) {
			arrival->setBackazimuthResidual(assoc.azres);
		}

		if ( !origin->add(arrival.get()) ) {
			SEISCOMP_DEBUG("arrival not added for some reason");
		}
	}

	dm::OriginQuality originQuality;

	originQuality.setAssociatedPhaseCount(phaseAssocCount);
	originQuality.setUsedPhaseCount(usedAssocCount);
	originQuality.setDepthPhaseCount(depthPhaseCount);

	if ( rmsCount > 0 )
		originQuality.setStandardError(sqrt(rms / rmsCount));

	if ( !azi.empty() ) {
		std::sort(azi.begin(), azi.end());
		azi.push_back(azi.front()+360.);
		double azGap = 0.;
		if ( azi.size() > 2 ) {
			for ( size_t i = 0; i < azi.size()-1; ++i ) {
				azGap = (azi[i+1]-azi[i]) > azGap ? (azi[i+1]-azi[i]) : azGap;
			}
		}

		if ( 0. < azGap && azGap < 360. ) {
			originQuality.setAzimuthalGap(azGap);
		}
	}

	if ( !dist.empty() ) {
		std::sort(dist.begin(), dist.end());
		originQuality.setMinimumDistance(dist.front());
		originQuality.setMaximumDistance(dist.back());
		originQuality.setMedianDistance(dist[dist.size()/2]);
	}

// #ifdef LOCSAT_TESTING
//	SEISCOMP_DEBUG("--- Confidence region at %4.2f level: ----------------", 0.9);
//	SEISCOMP_DEBUG("Semi-major axis:   %8.2f km", loc->origerr->smajax);
//	SEISCOMP_DEBUG("Semi-minor axis:   %8.2f km", loc->origerr->sminax);
//	SEISCOMP_DEBUG("Major axis strike: %8.2f deg. clockwise from North", loc->origerr->strike);
//	SEISCOMP_DEBUG("Depth error:       %8.2f km", loc->origerr->sdepth);
//	SEISCOMP_DEBUG("Orig. time error:  %8.2f sec", loc->origerr->stime);
//	SEISCOMP_DEBUG("--- OriginQuality ------------------------------------");
//	SEISCOMP_DEBUG("DefiningPhaseCount: %d", originQuality.definingPhaseCount());
//	SEISCOMP_DEBUG("PhaseAssociationCount: %d", originQuality.phaseAssociationCount());
//// 	SEISCOMP_DEBUG("ArrivalCount: %d", originQuality.stationCount());
//	SEISCOMP_DEBUG("Res. RMS: %f sec", originQuality.rms());
//	SEISCOMP_DEBUG("AzimuthalGap: %f deg", originQuality.azimuthalGap());
//	SEISCOMP_DEBUG("originQuality.setMinimumDistance: %f deg", originQuality.minimumDistance());
//	SEISCOMP_DEBUG("originQuality.setMaximumDistance: %f deg", originQuality.maximumDistance());
//	SEISCOMP_DEBUG("originQuality.setMedianDistance:  %f deg", originQuality.medianDistance());
//	SEISCOMP_DEBUG("------------------------------------------------------");
// #endif

	if ( _computeConfidenceEllipsoid ) {

	// IGN additions: OriginUncertainty computation

	// X axis in LOCSAT is W-E and Y axis is S-N. We'll take X axis as S-N and Y axis as W-E
	// LOCSAT covariance matrix is something like s2*inv(GG.T),
	// where G is the proper cov matrix and s2 the variance

	// M4d is the 4D matrix coming from LOCSAT with the axis changed properly
	double M4d[16] = {
		_origerr.syy, _origerr.sxy, _origerr.syz, _origerr.sty,
		_origerr.sxy, _origerr.sxx, _origerr.sxz, _origerr.stx,
		_origerr.syz, _origerr.sxz, _origerr.szz, _origerr.stz,
		_origerr.sty, _origerr.stx, _origerr.stz, _origerr.stt
	};

	// M3d is the matrix in space
	double M3d[9] = {
		M4d[0], M4d[1], M4d[2],
		M4d[4], M4d[5], M4d[6],
		M4d[8], M4d[9], M4d[10]
	};

	// M2d is the matrix in the XY plane
	double M2d[4] = {
		M4d[0], M4d[1],
		M4d[4], M4d[5]
	};

	// Diagonalize 3D and 2D matrixes
	// We use EISPACK code

	// compute 3D and 2D eigenvalues, eigenvectors
	// EISPACK sort eigenvalues from min to max
	int ierr3;
	double eigvec3d[9];
	double eigval3d[3];
	ierr3 = rs(3, M3d, eigval3d, eigvec3d);

	int ierr2;
	double eigvec2d[4];
	double eigval2d[2];
	ierr2 = rs(2, M2d, eigval2d, eigvec2d);


	/*
	* Confidence coefficients for 1D, 2D and 3D
	* LOCSAT assumes complete uncertainty knowledge.
	*
	* We use ASA091 code
	*
	* The following table summarizes confidence coefficients for 0.90 (LOCSAT) and 0.68 (NonLinLoc) confidence levels
	*
	* confidenceLevel      1D             2D              3D
	*      0.90        1.6448536270    2.1459660263    2.5002777108
	*      0.68        0.9944578832    1.5095921855    1.8724001591
	*
	*
	*/

	if ( ierr3 == 0 && ierr2 == 0 ) {
		double kppf[3];
		double g;
		int ifault;
		double dof;

		for ( int i = 0; i < 3; ++i ) {
			dof = i + 1;
			g = alngam(dof/2.0, &ifault);
			kppf[i] = pow(ppchi2(P(conf_level), dof, g, &ifault), 0.5);
		}

		double sx, sy, smajax, sminax, strike;
#ifdef LOCSAT_TESTING
		double stime, sdepth;
#endif

		// 1D confidence intervals
		sx = kppf[0] * pow(M4d[0], 0.5); // sxx
		sy = kppf[0] * pow(M4d[5], 0.5); // syy
#ifdef LOCSAT_TESTING
		// Take into account fixed depth: LOCSAT's szz = -1
		if ( M4d[10] != -1 ) {
			sdepth = kppf[0] * pow(M4d[10], 0.5);
		}
		else {
			sdepth = -1.0;
		}
		stime  = kppf[0] * pow(M4d[15], 0.5); // stt
#endif

		// 1D confidence intervals
		origin->setTime(
			dm::TimeQuantity(
				Core::Time(_origin.time),
				sqrt(_origerr.stt) * kppf[0],
				Core::None, Core::None, P(conf_level) * 100.0
			)
		);
		origin->setLatitude(
			dm::RealQuantity(
				_origin.lat, sqrt(_origerr.syy) * kppf[0],
				Core::None, Core::None, P(conf_level) * 100.0
			)
		);
		origin->setLongitude(
			dm::RealQuantity(
				_origin.lon, sqrt(_origerr.sxx) * kppf[0],
				Core::None, Core::None, P(conf_level) * 100.0
			)
		);
		origin->setDepth(
			dm::RealQuantity(
				_origin.depth, sqrt(_origerr.szz) * kppf[0],
				Core::None, Core::None, P(conf_level) * 100.0
			)
		);

		// 2D confidence intervals
		sminax = kppf[1] * pow(eigval2d[0], 0.5);
		smajax = kppf[1] * pow(eigval2d[1], 0.5);
		strike = rad2deg(atan(eigvec2d[3] / eigvec2d[2]));

		// give the strike in the [0.0, 180.0] interval
		if ( strike < 0.0 ) {
			strike += 180.0;
		}

		if ( strike > 180.0 ) {
			strike -= 180.0;
		}

		// 3D confidence intervals
		double s3dMajAxis, s3dMinAxis, s3dIntAxis, MajAxisPlunge, MajAxisAzimuth, MajAxisRotation;

		s3dMinAxis = kppf[2] * pow(eigval3d[0], 0.5);
		s3dIntAxis = kppf[2] * pow(eigval3d[1], 0.5);
		s3dMajAxis = kppf[2] * pow(eigval3d[2], 0.5);

		MajAxisPlunge   = rad2deg(atan(eigvec3d[8] / pow(pow(eigvec3d[6], 2.0) + pow(eigvec3d[7], 2.0), 0.5)));
		if ( MajAxisPlunge < 0.0 )
			MajAxisPlunge += 180.0;

		if ( MajAxisPlunge > 180.0 )
			MajAxisPlunge -= 180.0;

		MajAxisAzimuth  = rad2deg(atan(eigvec3d[7] / eigvec3d[6]));
		if ( MajAxisAzimuth < 0.0 )
			MajAxisAzimuth += 180.0;

		if ( MajAxisAzimuth > 180.0 )
			MajAxisAzimuth -= 180.0;

		MajAxisRotation = rad2deg(atan(eigvec3d[2] / pow(pow(eigvec3d[0], 2.0) + pow(eigvec3d[1], 2.0), 0.5)));
		if ( _origerr.szz == 0.0 ) {
			MajAxisRotation = 0.0;
		}

		if ( MajAxisRotation < 0.0 ) {
			MajAxisRotation += 180.0;
		}

		if ( MajAxisRotation > 180.0 ) {
			MajAxisRotation -= 180.0;
		}

		dm::ConfidenceEllipsoid confidenceEllipsoid;
		dm::OriginUncertainty originUncertainty;

		confidenceEllipsoid.setSemiMinorAxisLength(s3dMinAxis*1000.0);
		confidenceEllipsoid.setSemiIntermediateAxisLength(s3dIntAxis*1000.0);
		confidenceEllipsoid.setSemiMajorAxisLength(s3dMajAxis*1000.0);
		confidenceEllipsoid.setMajorAxisPlunge(MajAxisPlunge);
		confidenceEllipsoid.setMajorAxisAzimuth(MajAxisAzimuth);
		confidenceEllipsoid.setMajorAxisRotation(MajAxisRotation);

		// QuakeML, horizontalUncertainty: Circular confidence region, given by single value of horizontal uncertainty.
		// Acordingly, 1D horizontal errors quadratic mean is given
		originUncertainty.setHorizontalUncertainty(sqrt(pow(sx, 2) + pow(sy, 2)));
		originUncertainty.setMinHorizontalUncertainty(sminax);
		originUncertainty.setMaxHorizontalUncertainty(smajax);
		originUncertainty.setAzimuthMaxHorizontalUncertainty(strike);
		originUncertainty.setConfidenceEllipsoid(confidenceEllipsoid);
		originUncertainty.setPreferredDescription(dm::OriginUncertaintyDescription(dm::ELLIPSOID));

		origin->setUncertainty(originUncertainty);

#ifdef LOCSAT_TESTING
		SEISCOMP_DEBUG("Origin quality:");
		SEISCOMP_DEBUG("    Orig. time error:       %+17.8f s", loc->origerr->stime);
		SEISCOMP_DEBUG("    Semi-major axis:        %+17.8f km", loc->origerr->smajax);
		SEISCOMP_DEBUG("    Semi-minor axis:        %+17.8f km", loc->origerr->sminax);
		SEISCOMP_DEBUG("    Major axis strike:      %+17.8f deg clockwise from North", loc->origerr->strike);
		SEISCOMP_DEBUG("    Depth error:            %+17.8f km", loc->origerr->sdepth);
		SEISCOMP_DEBUG("    AzimuthalGap:           %+17.8f deg", originQuality.azimuthalGap());
		SEISCOMP_DEBUG("    setMinimumDistance:     %+17.8f deg", originQuality.minimumDistance());
		SEISCOMP_DEBUG("    setMaximumDistance:     %+17.8f deg", originQuality.maximumDistance());
		SEISCOMP_DEBUG("    setMedianDistance:      %+17.8f deg", originQuality.medianDistance());
		SEISCOMP_DEBUG("IGN's origin uncertainty computation:");
		SEISCOMP_DEBUG("  LOCSAT uncentainties:");
		SEISCOMP_DEBUG("    Semi-Major axis:        %+17.8f m", loc->origerr->smajax * 1000.0);
		SEISCOMP_DEBUG("    Semi-minor axis:        %+17.8f m", loc->origerr->sminax * 1000.0);
		SEISCOMP_DEBUG("    Major axis strike:      %+17.8f deg clockwise from North", loc->origerr->strike);
		SEISCOMP_DEBUG("    Depth error:            %+17.8f m", loc->origerr->sdepth * 1000.0);
		SEISCOMP_DEBUG("    Orig. time error:       %+17.8f s", loc->origerr->stime);
		SEISCOMP_DEBUG("  Confidence level: %4.2f %%", _locator_params->conf_level * 100.0);
		SEISCOMP_DEBUG("  Covariance matrix:");
		SEISCOMP_DEBUG("    1D uncertainties:");
		SEISCOMP_DEBUG("       Orig. time error:    %+17.8f s", stime);
		SEISCOMP_DEBUG("       X axis (S-N):        %+17.8f m", sx * 1000.0);
		SEISCOMP_DEBUG("       Y axis (W-E):        %+17.8f m", sy * 1000.0);
		SEISCOMP_DEBUG("       Depth error:         %+17.8f m", sdepth * 1000.0);
		SEISCOMP_DEBUG("    2D uncertainties:");
		SEISCOMP_DEBUG("       Semi-major axis:     %+17.8f m", smajax * 1000.0);
		SEISCOMP_DEBUG("       Semi-minor axis:     %+17.8f m", sminax * 1000.0);
		SEISCOMP_DEBUG("       Major axis strike:   %+17.8f deg clockwise from North", strike);
		SEISCOMP_DEBUG("    3D uncertainties / QuakeML:");
		SEISCOMP_DEBUG("       semiMinorAxisLength: %+17.8f m", s3dMinAxis * 1000.0);
		SEISCOMP_DEBUG("       semiMajorAxisLength: %+17.8f m", s3dMajAxis * 1000.0);
		SEISCOMP_DEBUG("       semiInterAxisLength: %+17.8f m", s3dIntAxis * 1000.0);
		SEISCOMP_DEBUG("       majorAxisPlunge:     %+17.8f deg", MajAxisPlunge);
		SEISCOMP_DEBUG("       majorAxisAzimuth:    %+17.8f deg", MajAxisAzimuth);
		SEISCOMP_DEBUG("       majorAxisRotation:   %+17.8f deg", MajAxisRotation);
		SEISCOMP_DEBUG("DEBUG INFO:");
		SEISCOMP_DEBUG("  LOCSAT values:");
		SEISCOMP_DEBUG("    stx: %+17.8f", loc->origerr->stx);
		SEISCOMP_DEBUG("    sty: %+17.8f", loc->origerr->sty);
		SEISCOMP_DEBUG("    stz: %+17.8f", loc->origerr->stz);
		SEISCOMP_DEBUG("    stt: %+17.8f", loc->origerr->stt);
		SEISCOMP_DEBUG("    sxx: %+17.8f", loc->origerr->sxx);
		SEISCOMP_DEBUG("    sxy: %+17.8f", loc->origerr->sxy);
		SEISCOMP_DEBUG("    sxz: %+17.8f", loc->origerr->sxz);
		SEISCOMP_DEBUG("    syy: %+17.8f", loc->origerr->syy);
		SEISCOMP_DEBUG("    syz: %+17.8f", loc->origerr->syz);
		SEISCOMP_DEBUG("    szz: %+17.8f", loc->origerr->szz);
		SEISCOMP_DEBUG("  4D matrix:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f %+17.8f", M4d[0], M4d[1], M4d[2], M4d[3]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f %+17.8f", M4d[4], M4d[5], M4d[6], M4d[7]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f %+17.8f", M4d[8], M4d[9], M4d[10], M4d[11]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f %+17.8f", M4d[12], M4d[13], M4d[14], M4d[15]);
		SEISCOMP_DEBUG("  3D matrix:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", M3d[0], M3d[1], M3d[2]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", M3d[3], M3d[4], M3d[5]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", M3d[6], M3d[7], M3d[8]);
		SEISCOMP_DEBUG("  2D matrix:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", M2d[0], M2d[1]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", M2d[2], M2d[3]);
		SEISCOMP_DEBUG("  3D eigenvalues:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", eigval3d[0], eigval3d[1],eigval3d[2]);
		SEISCOMP_DEBUG("  3D eigenvectors:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", eigvec3d[0], eigvec3d[3],eigvec3d[6]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", eigvec3d[1], eigvec3d[4],eigvec3d[7]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f %+17.8f", eigvec3d[2], eigvec3d[5],eigvec3d[8]);
		SEISCOMP_DEBUG("  2D eigenvalues:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", eigval2d[0], eigval2d[1]);
		SEISCOMP_DEBUG("  2D eigenvectors:");
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", eigvec2d[0], eigvec2d[2]);
		SEISCOMP_DEBUG("    %+17.8f %+17.8f", eigvec2d[1], eigvec2d[3]);
		SEISCOMP_DEBUG("  Chi2 Percent point function:");
		SEISCOMP_DEBUG("    1D: %+10.5f", kppf[0]);
		SEISCOMP_DEBUG("    2D: %+10.5f", kppf[1]);
		SEISCOMP_DEBUG("    3D: %+10.5f", kppf[2]);
#endif
	}
	else {
		SEISCOMP_DEBUG("Unable to calculate eigenvalues/eigenvectors. No Confidence ellipsoid will be computed");

		dm::OriginUncertainty originUncertainty;

		// Convert to m
		originUncertainty.setMinHorizontalUncertainty(_origerr.sminax);
		originUncertainty.setMaxHorizontalUncertainty(_origerr.smajax);
		originUncertainty.setAzimuthMaxHorizontalUncertainty(_origerr.strike);
		originUncertainty.setPreferredDescription(
			dm::OriginUncertaintyDescription(dm::HORIZONTAL)
		);

		origin->setUncertainty(originUncertainty);
	}

	} // Closing bracket for _computeConfidenceEllipsoid == true
	else {
		dm::OriginUncertainty originUncertainty;

		// Convert to m
		originUncertainty.setMinHorizontalUncertainty(_origerr.sminax);
		originUncertainty.setMaxHorizontalUncertainty(_origerr.smajax);
		originUncertainty.setAzimuthMaxHorizontalUncertainty(_origerr.strike);
		originUncertainty.setPreferredDescription(
			dm::OriginUncertaintyDescription(dm::HORIZONTAL)
		);

		origin->setUncertainty(originUncertainty);
	}

	origin->setQuality(originQuality);

	return origin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::addSite(const char *station, float lat, float lon, float elev) {
	for ( auto &site : _sites ) {
		if ( !strcmp(station, site.sta) ) {
			return;
		}
	}

	_sites.push_back(LOCSAT_Site());

	_sites.back().sta[sizeof(LOCSAT_Site::sta) - 1] = '\0';
	strncpy(_sites.back().sta, station, sizeof(LOCSAT_Site::sta) - 1);
	_sites.back().lat = lat;
	_sites.back().lon = lon;
	_sites.back().elev = elev * 0.001;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::addArrival(long arrival_id, const char *station, const char *phase,
                        double time, float deltim, int defining) {
	static const LOCSAT_Arrival Na_Arrival = Na_Arrival_Init;
	static const LOCSAT_Assoc Na_Assoc = Na_Assoc_Init;

	_arrivals.push_back(Na_Arrival);
	_assocs.push_back(Na_Assoc);
	_errors.push_back(LOCSAT_Errors());

	_arrivals.back().time = time;
	_arrivals.back().deltim = deltim;

	_assocs.back().timedef = defining > 0 ? 'd' : 'n';
	_assocs.back().azdef = 'n';
	_assocs.back().slodef = 'n';

	_arrivals.back().arid = arrival_id;
	_assocs.back().arid = arrival_id;
	strncpy(_arrivals.back().sta, station, sizeof(LOCSAT_Arrival::sta) - 1);
	_arrivals.back().sta[sizeof(LOCSAT_Arrival::sta) - 1] = '\0';
	strncpy(_assocs.back().phase, phase, sizeof(LOCSAT_Assoc::phase) - 1);
	_assocs.back().phase[sizeof(LOCSAT_Assoc::phase) - 1] = '\0';

	_errors.back().arid = 0;
	_errors.back().time = 0;
	_errors.back().slow = 0;
	_errors.back().az = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::setArrivalAzimuth(float azimuth, float delaz, int defining) {
	_arrivals.back().azimuth = azimuth;
	_arrivals.back().delaz = delaz;

	if ( defining > 0 ) {
		_assocs.back().azdef = 'd';
	}
	else {
		_assocs.back().azdef = 'n';
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::setArrivalSlowness(float slow, float delslo, int defining) {
	_arrivals.back().slow = slow;
	_arrivals.back().delslo = delslo;

	if ( defining > 0 ) {
		_assocs.back().slodef = 'd';
	}
	else {
		_assocs.back().slodef = 'n';
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::setOrigin(float lat_init, float lon_init, float depth_init) {
	_origin.lat = lat_init;
	_origin.lon = lon_init;
	_origin.depth = depth_init;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LOCSAT::setOriginTime(double epoch) {
	_origin.time = epoch;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
