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


#define SEISCOMP_COMPONENT MLv

#include <seiscomp/math/geo.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/sensorlocation.h>
#include <seiscomp/processing/regions.h>
#include <seiscomp/processing/magnitudes/MLv.h>
#include <seiscomp/geo/featureset.h>
#include <seiscomp/config/config.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/logging/log.h>


using namespace std;


namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "mm";

DEFINE_SMARTPOINTER(ExtraLocale);
class ExtraLocale : public Core::BaseObject {
	public:
		OPT(LogA0) logA0;
};

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_MLv, MagnitudeProcessor, "MagnitudeProcessor_MLv");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_MLv, "MLv");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_MLv::MagnitudeProcessor_MLv()
 : MagnitudeProcessor("MLv") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_MLv::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) ) {
		return false;
	}

	std::string defLogA0;

	// This is the default
	defLogA0 = "0:-1.3,60:-2.8,100:-3.0,400:-4.5,1000:-5.85";
	_maxDistanceKm = -1; // distance according to the logA0 range

	try { defLogA0 = settings.getString("magnitudes." + type() + ".logA0"); }
	catch ( ... ) {}
	try {
		defLogA0 = settings.getString(type() + ".logA0");
		SEISCOMP_WARNING("%s.logA0 is deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.logA0", type().c_str());
	}
	catch ( ... ) {}

	if ( !_logA0.set(defLogA0) ) {
		SEISCOMP_ERROR("%s: incorrect correction term log(A0): %s", _type.c_str(),
		               defLogA0.c_str());
		return false;
	}

	SEISCOMP_DEBUG("Parameters for magnitude %s:", type().c_str());
	SEISCOMP_DEBUG("  + logA0: %s", Core::toString(_logA0).c_str());

	try { _maxDistanceKm = settings.getDouble("magnitudes." + type() + ".maxDistanceKm"); }
	catch ( ... ) {}

	try {
		_maxDistanceKm = settings.getDouble(type() + ".maxDistanceKm");
		SEISCOMP_WARNING("%s.maxDistanceKm is deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.maxDistanceKm", type().c_str());
	}
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_MLv::initLocale(Locale *locale,
                                        const Settings &settings,
                                        const string &configPrefix) {
	const Seiscomp::Config::Config *cfg = settings.localConfiguration;
	try {
		auto logA0 = cfg->getStrings(configPrefix + "logA0");
		if ( !logA0.empty() ) {
			// If the first item contains a comma then maybe it has been configured
			// with double quotes. Raise an error.
			if ( logA0[0].find(',') != string::npos ) {
				SEISCOMP_ERROR("%slogA0[0] contains a comma. Are the coefficients "
				               "enclosed with double quotes in the configuration?",
				               configPrefix);
				return false;
			}

			if ( logA0[0].find(';') != string::npos ) {
				SEISCOMP_ERROR("%slogA0 = %s contains semicolon. Supported format:"
				               " distance1:correction1,distance2:correction2, ...",
				               configPrefix, Core::toString(logA0).c_str());
				return false;
			}

			ExtraLocalePtr extra = new ExtraLocale;
			extra->logA0 = LogA0();
			if ( !extra->logA0->set(logA0) ) {
				SEISCOMP_ERROR("%s@%s: incorrect correction term log(A0)",
				               _type.c_str(), locale->name.c_str());
				return false;
			}

			SEISCOMP_DEBUG("%s (locale) %s logA0: %s", _type.c_str(),
			               locale->name.c_str(),
			               Core::toString(*extra->logA0).c_str());
			locale->extra = extra;
		}
	}
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_MLv::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double,
	double delta, double depth,
	const DataModel::Origin *,
	const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	const Locale *locale,
	double &value) {

	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	// Clip depth to 0
	if ( depth < 0 ) {
		depth = 0;
	}
	double distanceKm = Math::Geo::deg2km(delta);
	if ( _maxDistanceKm > 0 and distanceKm > _maxDistanceKm )
		return DistanceOutOfRange;

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	ExtraLocale *extra = nullptr;
	if ( locale ) {
		extra = static_cast<ExtraLocale*>(locale->extra.get());
	}

	try {
		double correction = -1.0 * (extra and extra->logA0 ? extra->logA0->at(distanceKm) : _logA0.at(distanceKm));
		SEISCOMP_DEBUG("  + distance: %.5f deg, logA0 correction: %.3f", delta, correction);
		value = log10(amplitude) + correction;
		SEISCOMP_DEBUG("  + amplitude: %.5f, magnitude: %.3f", amplitude, value);
	}
	catch ( std::out_of_range & ) {
		return DistanceOutOfRange;
	}

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
