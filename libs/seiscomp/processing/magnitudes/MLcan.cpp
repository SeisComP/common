/* ################################################################################
* #    Copyright (C) 2024 by IGN Spain                                           #
* #                                                                              #
* #    author: J. Barco, E. Suarez                                               #
* #    email:  jbarco@transportes.gob.es   ,  eadiaz@transportes.gob.es          #
* #    last modified: 2024-03-20                                                 #
* #                                                                              #
* #    This program is free software; you can redistribute it and/or modify      #
* #    it under the terms of the GNU General Public License as published by      #
* #    the Free Software Foundation; either version 2 of the License, or         #
* #    (at your option) any later version.                                       #
* #                                                                              #
* #    This program is distributed in the hope that it will be useful,           #
* #    but WITHOUT ANY WARRANTY; without even the implied warranty of            #
* #    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
* #    GNU General Public License for more details.                              #
* #                                                                              #
* #    You should have received a copy of the GNU General Public License         #
* #    along with this program; if not, write to the                             #
* #    Free Software Foundation, Inc.,                                           #
* #    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 #
* ################################################################################ */

#define SEISCOMP_COMPONENT MLcan

#include <seiscomp/math/geo.h>
#include <seiscomp/processing/magnitudes/MLcan.h>
#include <seiscomp/config/config.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/datamodel/amplitude.h>

using namespace std;


namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "mm";

DEFINE_SMARTPOINTER(ExtraLocale);
class ExtraLocale : public Core::BaseObject {
	public:
		// general magnitude parameters
		OPT(string) distanceMode;
		OPT(string) calibrationType;
		// parametric coefficients
		OPT(double) c0_first;
		OPT(double) c0_second;
		OPT(double) c1;
		OPT(double) c2;
		OPT(double) c3;
		OPT(double) c4;
		OPT(double) c5;
		// A0, non-parametric coefficients
		OPT(LogA0)  logA0;
};

}


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_MLcan, "MLcan");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_MLcan::MagnitudeProcessor_MLcan()
: Processing::MagnitudeProcessor("MLcan") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MagnitudeProcessor_MLcan::amplitudeType() const {
	return MagnitudeProcessor::amplitudeType();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_MLcan::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) ) {
		return false;
	}

	try {_maxDepth = settings.getDouble("magnitudes." + _type + ".maxDepth"); }
	catch ( ... ) {}

	// distance constraints
	try {_distanceMode = settings.getString("magnitudes." + _type + ".distMode"); }
	catch ( ... ) {}
	try {_minDistanceKm = Math::Geo::deg2km(settings.getDouble("magnitudes." + _type + ".minDist")); }
	catch ( ... ) {}
	try {_maxDistanceKm = Math::Geo::deg2km(settings.getDouble("magnitudes." + _type + ".maxDist")); }
	catch ( ... ) {}

	// calibration function
	try {_calibrationType = settings.getString("magnitudes." + _type + ".calibrationType"); }
	catch ( ... ) {}

	if ( (_calibrationType != "A0") && (_calibrationType != "parametric") ) {
		SEISCOMP_ERROR("%s: unrecognized calibration type: %s", _type.c_str(),
		               _calibrationType.c_str());
		return false;
	}

	// parametric calibration function
	try { _c0_first = settings.getDouble("magnitudes." + _type + ".parametric.c0_first"); }
	catch ( ... ) {}
	try { _c0_second = settings.getDouble("magnitudes." + _type + ".parametric.c0_second"); }
	catch ( ... ) {}
	try { _c1 = settings.getDouble("magnitudes." + _type + ".parametric.c1"); }
	catch ( ... ) {}
	try { _c2 = settings.getDouble("magnitudes." + _type + ".parametric.c2"); }
	catch ( ... ) {}
	try { _c3 = settings.getDouble("magnitudes." + _type + ".parametric.c3"); }
	catch ( ... ) {}
	try { _c4 = settings.getDouble("magnitudes." + _type + ".parametric.c4"); }
	catch ( ... ) {}
	try { _c5 = settings.getDouble("magnitudes." + _type + ".parametric.c5"); }
	catch ( ... ) {}

	// A0, non-parametric calibration function
	std::string defLogA0 = "0:-1.3,60:-2.8,100:-3.0,400:-4.5,1000:-5.85";
	bool logA0Default = true;

	try {
		defLogA0 = settings.getString("magnitudes." + _type + ".A0.logA0");
		logA0Default = false;
	}
	catch ( ... ) {}

	// set distance-dependent intervals
	if ( !_logA0.set(defLogA0) ) {
		SEISCOMP_ERROR("%s: incorrect correction term log(A0): %s", _type.c_str(),
		               defLogA0.c_str());
		return false;
	}

	if ( !logA0Default ) {
		SEISCOMP_DEBUG("%s.%s: %s: logA0 from bindings = %s",
		               settings.networkCode, settings.stationCode,
		               type(), Core::toString(_logA0));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_MLcan::initLocale(Locale *locale,
                                       const Settings &settings,
                                       const string &configPrefix) {
	const Seiscomp::Config::Config *cfg = settings.localConfiguration;
	ExtraLocalePtr extra = new ExtraLocale;

	// general
	try {
		extra->distanceMode = cfg->getString(configPrefix + "distMode");
	}
	catch ( ... ) {}
	try {
		extra->calibrationType = cfg->getString(configPrefix + "calibrationType");
	}
	catch ( ... ) {}

	// parametric
    try {
        extra->c0_first = cfg->getDouble(configPrefix + ".parametric.c0_first");
    }
    catch ( ... ){}
    	try {
		extra->c0_second = cfg->getDouble(configPrefix + ".parametric.c0_second");
	}
	catch ( ... ) {}{}
	try {
		extra->c1 = cfg->getDouble(configPrefix + "parametric.c1");
	}
	catch ( ... ) {}
	try {
		extra->c2 = cfg->getDouble(configPrefix + "parametric.c2");
	}
	catch ( ... ) {}
	try {
		extra->c3 = cfg->getDouble(configPrefix + "parametric.c3");
	}
	catch ( ... ) {}
	try {
		extra->c4 = cfg->getDouble(configPrefix + "parametric.c4");
	}
	catch ( ... ) {}
	try {
		extra->c5 = cfg->getDouble(configPrefix + "parametric.c5");
	}
	catch ( ... ) {}

	// A0, non-parametric
	try {
		auto logA0 = cfg->getStrings(configPrefix + "A0.logA0");
		if ( !logA0.empty() ) {
			// If the first item contains a comma, then maybe it has been configured
			// with double quotes. Raise an error.
			if ( logA0[0].find(',') != string::npos ) {
				SEISCOMP_ERROR("%sA0.logA0[0] contains a comma. Are the coefficients "
				               "enclosed with double quotes in the configuration?",
				               configPrefix);
				return false;
			}

			if ( logA0[0].find(';') != string::npos ) {
				SEISCOMP_ERROR("%sA0.logA0 = %s contains semicolon. Supported format:"
				               " distance1:correction1,distance2:correction2, ...",
				               configPrefix, Core::toString(logA0).c_str());
				return false;
			}

			extra->logA0 = LogA0();
			if ( !extra->logA0->set(logA0) ) {
				SEISCOMP_ERROR("%s@%s: incorrect correction term log(A0)",
				               _type.c_str(), locale->name.c_str());
				return false;
			}
		}
	}
	catch ( ... ) {}

	locale->extra = extra;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

double MagnitudeProcessor_MLcan::getChanCorr(const double c0_first, const double c0_second, std::string ChanelCode){
        double chanCorr = 0;
		string component = "";
		component = ChanelCode.back();
		if ((component == "1") || (component == "N")){
			chanCorr = c0_first;
		}
		if ((component == "2") || (component == "E")){
			chanCorr = c0_second;
		}
		else{
			chanCorr = (c0_first + c0_second)/2;
		}
        return chanCorr;

    };



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_MLcan::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double, double delta, double depth,
	const DataModel::Origin *, 
	const DataModel::SensorLocation *receiver,
	const DataModel::Amplitude *new_amplitude,
	const Locale *locale,
	double &value) {

	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	auto distanceMode = _distanceMode;
	auto calibrationType = _calibrationType;
	auto minimumDistanceKm = _minDistanceKm;
	auto maximumDistanceKm = _maxDistanceKm;
	auto maximumDepth = _maxDepth;

	ExtraLocale *extra = nullptr;
	if ( locale ) {
		extra = static_cast<ExtraLocale*>(locale->extra.get());
		if ( extra ) {
			if ( extra->distanceMode ) {
				distanceMode = *extra->distanceMode;
			}

			if ( extra->calibrationType ) {
				calibrationType = *extra->calibrationType;
			}
		}

		if ( locale->minimumDistance ) {
			minimumDistanceKm = Math::Geo::deg2km(*locale->minimumDistance);
		}

		if ( locale->maximumDistance ) {
			maximumDistanceKm = Math::Geo::deg2km(*locale->maximumDistance);
		}
	}
	else {

		SEISCOMP_DEBUG("  + maximum depth: %.3f km", maximumDepth);
		if ( depth > maximumDepth ) {
			return DepthOutOfRange;
		}
		SEISCOMP_DEBUG("  + minimum distance: %.3f km", minimumDistanceKm);
		SEISCOMP_DEBUG("  + maximum distance: %.3f km", maximumDistanceKm);
	}

	SEISCOMP_DEBUG("  + distance type: %s", distanceMode.c_str());

	double hDistanceKm = Math::Geo::deg2km(delta);
	double vDistanceKm = 0;

	if ( !receiver && distanceMode == "hypocentral" ) {
		return MetaDataRequired;
	}

	if ( receiver ) {
		vDistanceKm = receiver->elevation() / 1000. + depth;
	}

	double distanceKm;
	if ( distanceMode == "hypocentral" ) {
		distanceKm = sqrt(pow(hDistanceKm, 2) + pow(vDistanceKm, 2));
	}
	else {
		distanceKm = hDistanceKm;
	}

	SEISCOMP_DEBUG("  + considered distance to station: %.3f km", distanceKm);

	if ( minimumDistanceKm >= 0 && distanceKm < minimumDistanceKm ) {
		return DistanceOutOfRange;
	}

	if ( maximumDistanceKm >= 0 && distanceKm > maximumDistanceKm ) {
		return DistanceOutOfRange;
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	SEISCOMP_DEBUG("  + %s distance: %.3f km (horiz. %.3f vert. %.3f)",
	               distanceMode.c_str(), distanceKm, hDistanceKm, vDistanceKm);

	double correction;
	SEISCOMP_DEBUG("  + calibration type: %s", calibrationType.c_str());

	if ( calibrationType == "parametric" ) {
		// parametric calibration function
		double c0 = 0;
		// parametric calibration function
		auto c0_first = (extra and extra->c0_first) ? *extra->c0_first : _c0_first;
    	auto c0_second = (extra and extra->c0_second) ? *extra->c0_second : _c0_second;
		auto c1 = (extra and extra->c1) ? *extra->c1 : _c1;
		auto c2 = (extra and extra->c2) ? *extra->c2 : _c2;
		auto c3 = (extra and extra->c3) ? *extra->c3 : _c3;
		auto c4 = (extra and extra->c4) ? *extra->c4 : _c4;
		auto c5 = (extra and extra->c5) ? *extra->c5 : _c5;



		// extract the channel code and the location code from the amplitude
		// as strings 
		std::string channelCode = new_amplitude->waveformID().channelCode();
		std::string locationCode = new_amplitude->waveformID().locationCode();

    	c0 = getChanCorr(c0_first, c0_second, channelCode);
		SEISCOMP_DEBUG("  + Location: %s", locationCode);
		SEISCOMP_DEBUG("  + Channel: %s", channelCode);
		

		SEISCOMP_DEBUG("  + c0 - c5: %.5f %.5f %.5f %.5f %.5f %.5f",
		               c0, c1, c2, c3, c4, c5);
		correction = c3 * log10(distanceKm / c5)
		             + c2 * (distanceKm + c4)
		             + c1
		             + c0;
		value = log10(amplitude) + correction;
	}
	else if ( calibrationType == "A0" ) {
		// A0, non-parametric calibration function
		try {
			correction = -1.0 * (extra and extra->logA0 ? extra->logA0->at(distanceKm) : _logA0.at(distanceKm));

			SEISCOMP_DEBUG("  + -log10(A0) at %.3f km: %.5f", distanceKm, correction);
			value = log10(amplitude) + correction;
		}
		catch ( std::out_of_range & ) {
			return DistanceOutOfRange;
		}
	}

	else {
		return IncompleteConfiguration;
	}

	SEISCOMP_DEBUG("  + amplitude: %.5f, magnitude: %.3f", amplitude, value);

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
