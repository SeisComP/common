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


#define SEISCOMP_COMPONENT MLc

#include <seiscomp/math/geo.h>
#include <seiscomp/processing/magnitudes/MLc.h>
#include <seiscomp/config/config.h>
#include <seiscomp/logging/log.h>


using namespace std;


namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "mm";

}


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_MLc, "MLc");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_MLc::MagnitudeProcessor_MLc()
: Processing::MagnitudeProcessor("MLc") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MagnitudeProcessor_MLc::amplitudeType() const {
	return "MLc";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_MLc::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) )
		return false;

	// calibration function
	try { _c0 = settings.getDouble("magnitudes." + _type + ".c0"); }
	catch ( ... ) {}
	try { _c1 = settings.getDouble("magnitudes." + _type + ".c1"); }
	catch ( ... ) {}
	try { _c2 = settings.getDouble("magnitudes." + _type + ".c2"); }
	catch ( ... ) {}
	try { _c3 = settings.getDouble("magnitudes." + _type + ".c3"); }
	catch ( ... ) {}
	try { _c4 = settings.getDouble("magnitudes." + _type + ".c4"); }
	catch ( ... ) {}
	try { _c5 = settings.getDouble("magnitudes." + _type + ".c5"); }
	catch ( ... ) {}

	// depth constraint
	try {_maxDepth = settings.getDouble("magnitudes." + _type + ".maxDepth"); }
	catch ( ... ) {}

	// distance constraints
	try {_distanceMode = settings.getString("magnitudes." + _type + ".distMode"); }
	catch ( ... ) {}
	try {_maxDistanceKm = Math::Geo::deg2km(settings.getDouble("magnitudes." + _type + ".maxDist")); }
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_MLc::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double, double delta, double depth,
	const DataModel::Origin *, const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	const Locale *locale,
	double &value) {
	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	// Check depth, clip negative depth to 0
	if ( depth > _maxDepth ) {
		return DepthOutOfRange;
	}
	if ( depth < 0 ) depth = 0;

	double distanceKm;
	if ( _distanceMode == "hypocentral" ) {
		distanceKm = sqrt(pow(Math::Geo::deg2km(delta),2) + pow(depth,2));
	}
	else {
		distanceKm = Math::Geo::deg2km(delta);
	}
	if ( _maxDistanceKm > 0 and distanceKm > _maxDistanceKm ) {
		return DistanceOutOfRange;
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	try {
		value = log10(amplitude)
		        + _c3 * log10(distanceKm / _c5)
		        + _c2 * (distanceKm + _c4)
		        + _c1
		        + _c0;
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
