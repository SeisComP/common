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


#define SEISCOMP_COMPONENT MAGNITUDES

#include <seiscomp/processing/magnitudes/Ms20.h>
#include <seiscomp/seismology/magnitudes.h>
#include <seiscomp/logging/log.h>

#include<iostream>

#define DELTA_MIN 20.
#define DELTA_MAX 160.

#define DEPTH_MAX 100


using namespace std;

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm";

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_ms20, MagnitudeProcessor, "MagnitudeProcessor_ms20");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_ms20, "Ms_20");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_ms20::MagnitudeProcessor_ms20::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) )
		return false;

	// This is the default
	lowPer = 18;
	upPer = 22;
	minDistanceDeg = 20.0; // default minimum distance
	maxDistanceDeg = 160.0; // default maximum distance
	maxDepthKm = 100.0; // default maximum depth

	try { lowPer = settings.getDouble("magnitudes." + type() + ".lowerPeriod"); }
	catch ( ... ) {}

	try { upPer = settings.getDouble("magnitudes." + type() + ".upperPeriod"); }
	catch ( ... ) {}

	// distance range in degree
	try { minDistanceDeg = settings.getDouble("magnitudes." + type() + ".minDist"); }
	catch ( ... ) {}

	try { maxDistanceDeg = settings.getDouble("magnitudes." + type() + ".maxDist"); }
	catch ( ... ) {}

	// depth range in km
	try { maxDepthKm = settings.getDouble("magnitudes." + type() + ".maxDepth"); }
	catch ( ... ) {}

	// deprecated parameters
	// period range in seconds
	try {
		lowPer = settings.getDouble(type() + ".lowerPeriod");
		SEISCOMP_WARNING("%s.lowerPeriod has been deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.lowerPeriod", type().c_str());
	}
	catch ( ... ) {}

	try {
		upPer = settings.getDouble(type() + ".upperPeriod");
		SEISCOMP_WARNING("%s.upperPeriod has been deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.upperPeriod", type().c_str());
	}
	catch ( ... ) {}

	// distance range in degree
	try {
		minDistanceDeg = settings.getDouble(type() + ".minimumDistance");
		SEISCOMP_WARNING("%s.minimumDistance has been deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.maxDist", type().c_str());
	}
	catch ( ... ) {}

	try {
		maxDistanceDeg = settings.getDouble(type() + ".maximumDistance");
		SEISCOMP_WARNING("%s.maximumDistance has been deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.maxDist", type().c_str());
	}
	catch ( ... ) {}

	// depth range in km
	try {
		maxDepthKm = settings.getDouble(type() + ".maximumDepth");
		SEISCOMP_WARNING("%s.maximumDepth has been deprecated", type().c_str());
		SEISCOMP_WARNING("  + remove parameter from bindings and use magnitudes.%s.maxDist", type().c_str());
	}
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_ms20::MagnitudeProcessor_ms20()
 : MagnitudeProcessor("Ms_20") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_ms20::computeMagnitude(
	double amplitude, const std::string &unit,
	double period, double,
	double delta, double depth,
	const DataModel::Origin *,
	const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	const Locale *,
	double &value) {
	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	// allowed periods are 18 - 22 s acocrding to IASPEI standard (IASPEI recommendations of magnitude working group, 2013)
	if ( period < lowPer || period > upPer ) {
		SEISCOMP_DEBUG("%s: period is %.2f s", type().c_str(), period);
		return PeriodOutOfRange;
	}

	if ( delta < minDistanceDeg || delta > maxDistanceDeg ) {
		return DistanceOutOfRange;
	}

	// Clip depth to 0
	if ( depth < 0 ) {
		depth = 0;
	}

	if ( depth > maxDepthKm ) {
		return DepthOutOfRange; // strictly speaking it would be 60 km
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	// Use amplitude in nm
	value = log10((amplitude)/(period)) + 1.66*log10(delta) + 0.3;

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
