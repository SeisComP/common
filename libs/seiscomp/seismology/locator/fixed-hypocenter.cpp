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


#define SEISCOMP_COMPONENT FixedHypocenter


#include <math.h>
#include <stdio.h>


namespace {


#include "cephes/mconf.h"
#include "cephes/const.c"
#include "cephes/mtherr.c"
#include "cephes/polevl.c"
#include "cephes/gamma.c"
#include "cephes/ndtri.c"
#include "cephes/incbet.c"
#include "cephes/incbi.c"
#include "cephes/fdtr.c"


}


#include <seiscomp/logging/log.h>
#include <seiscomp/math/math.h>
#include <seiscomp/math/mean.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/seismology/locator/utils.h>
#include <iostream>

#include "fixed-hypocenter_private.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double getTimeError(const Pick *pick, double defaultTimeError,
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * @brief Compute confidence coefficient as per Jordan & Sverdrup (1981).
 * @param label Output label
 * @param m Number of degrees of freedom in solution
 * @param n Number of input data (arrivals)
 * @param k Number of degrees of freedom assigned to prior estimate
 * @param rw2 Squared sum of residuals. The default is 1.
 * @param p Target probability. The default is 0.9.
 * @param sk2 Prior estimate of variance scale factor. The default is 1.
 * @return
 */
double confidenceCoefficient(string &label,
                             int m, size_t n, int k, double rw2, double p = 0.9,
                             double sk2 = 1.0) {
	if ( k == 0 )
		label = "a posteriori";
	else
		label = Core::stringify("K-weighted ($K$=%d, $s_K$=%g s)", k, sqrt(sk2));

	return m * (k*sk2 + rw2) / (k + n - m) * fdtri(m, k + n - m, p);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_LOCATOR(FixedHypocenter, "FixedHypocenter");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FixedHypocenter::init(const Config::Config &config) {
	try {
		_profiles = config.getStrings("FixedHypocenter.profiles");
	}
	catch ( ... ) {
		try {
			_profiles = config.getStrings("LOCSAT.profiles");
			for ( size_t i = 0; i < _profiles.size(); ++i ) {
				_profiles[i] = "LOCSAT/" + _profiles[i];
			}
		}
		catch ( ... ) {
			_profiles.push_back("LOCSAT/iasp91");
			_profiles.push_back("LOCSAT/tab");
		}
	}

	try {
		if ( config.getBool("FixedHypocenter.usePickUncertainties") ) {
			_flags |= UsePickUncertainties;
		}
	}
	catch ( ... ) {}

	try {
		_defaultTimeError = config.getDouble("FixedHypocenter.defaultTimeError");
	}
	catch ( ... ) {}

	try {
		_degreesOfFreedom = config.getInt("FixedHypocenter.degreesOfFreedom");
	}
	catch ( ... ) {}

	try {
		_confidenceLevel = config.getDouble("FixedHypocenter.confLevel");
		if ( (_confidenceLevel < 0.5) || (_confidenceLevel > 1) ) {
			SEISCOMP_ERROR("FixedHypocenter.confLevel: must be >= 0.5 and <= 1");
			return false;
		}
	}
	catch ( ... ) {}

	_initLat = Core::None;
	_initLon = Core::None;
	_initDepth = Core::None;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FixedHypocenter::IDList FixedHypocenter::parameters() const {
	static IDList allowedParameters;

	if ( allowedParameters.empty() ) {
		allowedParameters.push_back("VERBOSE");
		allowedParameters.push_back("LATITUDE");
		allowedParameters.push_back("LONGITUDE");
		allowedParameters.push_back("DEPTH");
		allowedParameters.push_back("USE_PICK_UNCERTAINTIES");
		allowedParameters.push_back("DEFAULT_TIME_ERROR");
		allowedParameters.push_back("NUM_DEG_FREEDOM");
		allowedParameters.push_back("CONF_LEVEL");
	}

	return allowedParameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string FixedHypocenter::parameter(const string &name) const {
	if ( name == "USE_PICK_UNCERTAINTIES" )
		return (_flags & UsePickUncertainties) ? "y" : "n";
	else if ( name == "DEFAULT_TIME_ERROR" )
		return Core::toString(_defaultTimeError);
	else if ( name == "VERBOSE" )
		return _verbose ? "y" : "n";
	else if ( name == "NUM_DEG_FREEDOM" )
		return Core::toString(_degreesOfFreedom);
	else if ( name == "CONF_LEVEL" )
		return Core::toString(_confidenceLevel);
	else if ( name == "LATITUDE" && _initLat )
		return Core::toString(*_initLat);
	else if ( name == "LONGITUDE" && _initLon )
		return Core::toString(*_initLon);
	else if ( name == "DEPTH" && _initDepth )
		return Core::toString(*_initDepth);
	return string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FixedHypocenter::setParameter(const string &name, const string &value) {
	if ( name == "USE_PICK_UNCERTAINTIES" ) {
		if ( value == "y" ) {
			_flags |= UsePickUncertainties;
		}
		else {
			_flags &= ~UsePickUncertainties;
		}
	}
	else if ( name == "DEFAULT_TIME_ERROR" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) )
			return false;
		_defaultTimeError = tmp;
	}
	else if ( name == "VERBOSE" ) {
		_verbose = value == "y";
	}
	else if ( name == "NUM_DEG_FREEDOM" ) {
		int tmp;
		if ( !Core::fromString(tmp, value) )
			return false;
		_degreesOfFreedom = tmp;
	}
	else if ( name == "CONF_LEVEL" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) )
			return false;
		if ( (tmp < 0.5) || (tmp > 1.0) )
			return false;
		_confidenceLevel = tmp;
	}
	else if ( name == "LATITUDE" ) {
		if ( value.empty() )
			_initLat = Core::None;
		else {
			double tmp;
			if ( !Core::fromString(tmp, value) )
				return false;
			_initLat = tmp;
			_flags &= ~UseOriginUncertainties;
		}
	}
	else if ( name == "LONGITUDE" ) {
		if ( value.empty() )
			_initLon = Core::None;
		else {
			double tmp;
			if ( !Core::fromString(tmp, value) )
				return false;
			_initLon = tmp;
			_flags &= ~UseOriginUncertainties;
		}
	}
	else if ( name == "DEPTH" ) {
		if ( value.empty() )
			_initDepth = Core::None;
		else {
			double tmp;
			if ( !Core::fromString(tmp, value) )
				return false;
			_initDepth = tmp;
			_flags &= ~UseOriginUncertainties;
		}
	}
	else
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FixedHypocenter::IDList FixedHypocenter::profiles() const {
	return _profiles;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FixedHypocenter::setProfile(const string &name) {
	_ttt.reset();

	size_t pos = name.find('/');
	if ( pos == string::npos ) {
		_lastError = "Invalid profile, missing '/' separator";
		SEISCOMP_ERROR("%s", _lastError.c_str());
		return;
	}

	string type = name.substr(0, pos);
	string model = name.substr(pos + 1);

	_ttt = TravelTimeTableInterface::Create(type.c_str());
	if ( !_ttt ) {
		_lastError = "Failed to create interface '" + type + "'";
		SEISCOMP_ERROR("%s", _lastError.c_str());
		return;
	}

	if ( !_ttt->setModel(model) ) {
		_lastError = "Failed to set model '" + model + "' for '" + type + "'";
		SEISCOMP_ERROR("%s", _lastError.c_str());
		return;
	}

	_lastError.clear();
	SEISCOMP_DEBUG("Switched profile to %s/%s", type.c_str(), model.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FixedHypocenter::capabilities() const {
	return InitialLocation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *FixedHypocenter::locate(PickList &pickList) {
	SEISCOMP_ERROR("This call is not supported, an origin is required");
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *FixedHypocenter::locate(PickList &pickList,
                                double initLat, double initLon, double initDepth,
                                const Core::Time &initTime) {
	SEISCOMP_ERROR("This call is not supported, an origin is required");
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *FixedHypocenter::relocate(const Origin *origin) {
	if ( !_ttt )
		throw LocatorException("No travel time table active: " + _lastError);

	vector<double> originTimes;
	vector<double> weights;
	vector<double> travelTimes;
	vector<double> pickTimes;
	vector<double> arrivalWeights;
	vector<double> distances;
	vector<double> azimuths;

	double slat, slon, sdepth;
	double rlat, rlon, relev;

	bool recomputeDistance = false;

	try {
		slat = origin->latitude().value();
	}
	catch ( ... ) {
		throw LocatorException("incomplete origin, latitude is not set");
	}

	try {
		slon = origin->longitude().value();
	}
	catch ( ... ) {
		throw LocatorException("incomplete origin, longitude is not set");
	}

	if ( _initLat ) {
		if ( slat != *_initLat )
			recomputeDistance = true;
		slat = *_initLat;
	}

	if ( _initLon ) {
		if ( slon != *_initLon )
			recomputeDistance = true;
		slon = *_initLon;
	}

	if ( _initDepth )
		sdepth = *_initDepth;
	else {
		try {
			sdepth = origin->depth().value();
		}
		catch ( ... ) {
			throw LocatorException("incomplete origin, depth is not set");
		}
	}

	pickTimes.resize(origin->arrivalCount());
	travelTimes.resize(origin->arrivalCount());
	arrivalWeights.resize(origin->arrivalCount());

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		try {
			origin->arrival(i)->distance();
			origin->arrival(i)->azimuth();
		}
		catch ( ... ) {
			// If either of the attributes is not set, recompute them
			recomputeDistance = true;
		}
	}

	if ( recomputeDistance ) {
		SEISCOMP_DEBUG("Recalculation of distances and aizmuths is required");
		distances.resize(origin->arrivalCount());
		azimuths.resize(origin->arrivalCount());
	}

	int activeArrivals = 0;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		DataModel::Arrival *arrival = origin->arrival(i);

		// Reset travel time
		travelTimes[i] = -1;

		Pick *pick = Pick::Find(arrival->pickID());
		if ( !pick ) {
			throw PickNotFoundException("pick '" + arrival->pickID() + "' not found");
		}

		SensorLocation *sloc = getSensorLocation(pick);
		if (!sloc){
			throw StationNotFoundException("sensor location '" + pick->waveformID().networkCode() +
			                               "." + pick->waveformID().stationCode() + "." +
			                               pick->waveformID().locationCode() + "' not found");
		}

		try {
			rlat = sloc->latitude();
			rlon = sloc->longitude();
			relev = 0;
		}
		catch ( ... ) {
			throw LocatorException("sensor location '" + pick->waveformID().networkCode() +
			                       "." + pick->waveformID().stationCode() + "." +
			                       pick->waveformID().locationCode() + "' is incomplete w.r.t. lat/lon");
		}

		try { relev = sloc->elevation(); }
		catch ( ... ) {}

		double pickTime = pick->time().value().epoch();
		double travelTime;

		try {
			if ( arrival->weight() > 0 )
				++activeArrivals;
		}
		catch ( ... ) {}

		try {
			travelTime = _ttt->computeTime(arrival->phase().code().c_str(),
			                                  slat, slon, sdepth, rlat, rlon, relev);
			if ( travelTime < 0 ) {
				if ( _verbose ) {
					cerr << "Could not get travel time for "
					     << pick->waveformID().networkCode() << "."
					     << pick->waveformID().stationCode() << "."
					     << pick->waveformID().locationCode()
					     << ": ignoring arrival #" << i << endl;
				}
				continue;
			}
		}
		catch ( exception &e ) {
			if ( _verbose ) {
				cerr << "Could not get travel time for "
				     << pick->waveformID().networkCode() << "."
				     << pick->waveformID().stationCode() << "."
				     << pick->waveformID().locationCode() << ": "
				     << e.what() << ": ignoring arrival #" << i << endl;
			}
			continue;
		}

		arrivalWeights[i] = 1;
		pickTimes[i] = pickTime;
		travelTimes[i] = travelTime;

		if ( recomputeDistance ) {
			double baz;
			Math::Geo::delazi_wgs84(slat, slon, rlat, rlon, &distances[i], &azimuths[i], &baz);
		}

		try {
			if ( arrival->weight() == 0 ) {
				if ( _verbose )
					cerr << "Omitting arrival #" << i << " with weight 0" << endl;

				arrivalWeights[i] = 0;
			}
		}
		catch ( ... ) {}

		if ( arrivalWeights[i] > 0 ) {
			double uncertainty = getTimeError(pick, _defaultTimeError, (_flags & UsePickUncertainties) ? true : false);
			if ( Math::isNaN(uncertainty) || uncertainty <= 0.0 )
				uncertainty = _defaultTimeError;
			arrivalWeights[i] = 1.0 / uncertainty;
		}

		originTimes.push_back(pickTime - travelTime);
		weights.push_back(arrivalWeights[i]);
	}

	if ( originTimes.empty() ) {
		if ( !activeArrivals )
			throw LocatorException("Empty set of active arrivals");
		else
			throw LocatorException("Could not compute travel times of active arrivals");
	}

	double originTime, originTimeError;
	Math::Statistics::average(originTimes, weights, originTime, originTimeError);

	Origin *newOrigin = Origin::Create();
	*newOrigin = *origin;
	if ( !newOrigin )
		throw LocatorException("Could not create origin");

	newOrigin->setDepthType(DataModel::OriginDepthType(DataModel::OPERATOR_ASSIGNED));

	// Copy arrivals

	double sumSquaredResiduals = 0.0, sumSquaredWeights = 0.0;
	double sumWeights = 0.0;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		newOrigin->add(static_cast<Arrival*>(origin->arrival(i)->clone()));
		if ( travelTimes[i] >= 0 ) {
			double residual = pickTimes[i] - (originTime + travelTimes[i]);
			newOrigin->arrival(i)->setTimeResidual(residual);

			if ( arrivalWeights[i] <= 0 ) {
				// Not used
				newOrigin->arrival(i)->setTimeUsed(false);
				newOrigin->arrival(i)->setWeight(0.0);
			}
			else {
				sumWeights += arrivalWeights[i];
				sumSquaredResiduals += (residual * arrivalWeights[i]) * (residual * arrivalWeights[i]);
				sumSquaredWeights += arrivalWeights[i] * arrivalWeights[i];
				newOrigin->arrival(i)->setTimeUsed(true);
				newOrigin->arrival(i)->setWeight(arrivalWeights[i]);
			}
		}
		else {
			newOrigin->arrival(i)->setTimeResidual(Core::None);
			newOrigin->arrival(i)->setTimeUsed(false);
			newOrigin->arrival(i)->setWeight(0.0);
		}

		if ( recomputeDistance ) {
			newOrigin->arrival(i)->setDistance(distances[i]);
			newOrigin->arrival(i)->setAzimuth(azimuths[i]);
		}
	}

	if ( sumSquaredWeights <= 0 ) {
		throw LocatorException("At least one active and valid arrival is required");
	}

	double effectiveSampleSize =  sumWeights * sumWeights / sumSquaredWeights;
	string label;
	double kappa_p = confidenceCoefficient(label, 1,
	                                       originTimes.size(), _degreesOfFreedom,
	                                       sumSquaredResiduals, _confidenceLevel,
	                                       1.0);

	string description = Core::stringify("Confidence coefficient: %s, $\\kappa_p$ = %0.1f, $n_{eff}$ = %.1f",
	                                     label.c_str(), kappa_p, effectiveSampleSize);

	if ( _initLat )	newOrigin->setLatitude(*_initLat);
	if ( _initLon )	newOrigin->setLongitude(*_initLon);
	if ( _initDepth ) newOrigin->setDepth(RealQuantity(*_initDepth));

	newOrigin->setTime(TimeQuantity(Core::Time(originTime)));
	newOrigin->time().setConfidenceLevel(_confidenceLevel * 100.0);
	newOrigin->time().setUncertainty(sqrt(kappa_p / sumSquaredWeights));
	newOrigin->setMethodID("FixedHypocenter");
	newOrigin->setEarthModelID(_ttt->model());
	newOrigin->setEpicenterFixed(true);
	newOrigin->setTimeFixed(false);

	OriginQuality qual;

	qual.setStandardError(sqrt(sumSquaredResiduals/sumSquaredWeights));
	qual.setGroundTruthLevel("GT1");
	compile(qual, newOrigin);
	newOrigin->setQuality(qual);

	if ( !(_flags & UseOriginUncertainties) ) {
		// set uncertainties to zero in case any value was set manually
		SEISCOMP_DEBUG("Coordinates set manually: using uncertainties of 0 km");
		newOrigin->setUncertainty(Core::None);
		newOrigin->latitude().setUncertainty(0.0);
		newOrigin->longitude().setUncertainty(0.0);
		newOrigin->depth().setUncertainty(0.0);
	}

	CreationInfo ci;
	ci.setCreationTime(Core::Time::UTC());
	newOrigin->setCreationInfo(ci);

	if ( !description.empty() ) {
		CommentPtr comment = new Comment;
		comment->setId("confidence/description");
		comment->setText(description);
		newOrigin->add(comment.get());
	}

	return newOrigin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
