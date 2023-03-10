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


#define SEISCOMP_COMPONENT ML

#include <seiscomp/math/geo.h>
#include <seiscomp/processing/magnitudes/ML.h>
#include <seiscomp/config/config.h>
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


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_ML, MagnitudeProcessor, "MagnitudeProcessor_ML");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_ML, "ML");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_ML::MagnitudeProcessor_ML() : Processing::MagnitudeProcessor("ML") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MagnitudeProcessor_ML::amplitudeType() const {
	return "ML";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_ML::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) )
		return false;

	std::string defLogA0;

	// This is the default
	defLogA0 = "0:-1.3,60:-2.8,100:-3.0,400:-4.5,1000:-5.85";
	_maxDistanceKm = -1; // distance according to the logA0 range

	try { defLogA0 = settings.getString("magnitudes." + type() + ".logA0"); }
	catch ( ... ) {}
	try {
		defLogA0 = settings.getString(type() + ".logA0");
		SEISCOMP_WARNING("%s.logA0 has been deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove the parameter from bindings and use magnitudes.%s.logA0", type().c_str());
	}
	catch ( ... ) {}

	if ( !_logA0.set(defLogA0) ) {
		SEISCOMP_ERROR("%s: incorrect correction term log(A0): %s", _type.c_str(),
		               defLogA0.c_str());
		return false;
	}

	try {
		_maxDistanceKm = settings.getDouble("magnitudes." + type() + ".maxDistanceKm");
	}
	catch ( ... ) {	}

	try {
		_maxDistanceKm = settings.getDouble(type() + ".maxDistanceKm");
		SEISCOMP_WARNING("%s.maxDistanceKm has been deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove the parameter from bindings and use magnitudes.%s.maxDistanceKm", type().c_str());
	}
	catch ( ... ) {	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_ML::initLocale(Locale *locale,
                                       const Settings &settings,
                                       const string &configPrefix) {
	const Seiscomp::Config::Config *cfg = settings.localConfiguration;
	try {
		string logA0 = cfg->getString(configPrefix + "logA0");
		if ( !logA0.empty() ) {
			ExtraLocalePtr extra = new ExtraLocale;
			extra->logA0 = LogA0();
			if ( !extra->logA0->set(logA0) ) {
				SEISCOMP_ERROR("%s@%s: incorrect correction term log(A0): %s",
				               _type.c_str(), locale->name.c_str(),
				               logA0.c_str());
				return false;
			}

			locale->extra = extra;
		}
	}
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_ML::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double, double delta, double depth,
	const DataModel::Origin *, const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	const Locale *locale,
	double &value) {
	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	double distanceKm = Math::Geo::deg2km(delta);
	if ( _maxDistanceKm > 0 and distanceKm > _maxDistanceKm )
		return DistanceOutOfRange;

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) )
		return InvalidAmplitudeUnit;

	ExtraLocale *extra = nullptr;
	if ( locale )
		extra = static_cast<ExtraLocale*>(locale->extra.get());

	try {
		value = log10(amplitude) - (extra and extra->logA0 ? extra->logA0->at(distanceKm) : _logA0.at(distanceKm));
	}
	catch ( Core::ValueException & ) {
		return DistanceOutOfRange;
	}

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
