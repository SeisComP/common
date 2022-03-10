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



#include <seiscomp/processing/magnitudes/mb.h>
#include <seiscomp/seismology/magnitudes.h>
#include <seiscomp/logging/log.h>

#include "iostream"

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm";

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_mb, MagnitudeProcessor, "MagnitudeProcessor_mb");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_mb, "mb");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mb::MagnitudeProcessor_mb()
 : MagnitudeProcessor("mb") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_mb::MagnitudeProcessor_mb::setup(const Settings &settings) {
	MagnitudeProcessor::setup(settings);

	minDistanceDeg = 5.0; // default minimum distance
	maxDistanceDeg = 105.0; // default maximum distance

	// distance range in degree
	try { minDistanceDeg = settings.getDouble("magnitudes.mb.minDist"); }
	catch ( ... ) {}

	try { maxDistanceDeg = settings.getDouble("magnitudes.mb.maxDist"); }
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mb::computeMagnitude(
	double amplitude,
	const std::string &unit,
	double period, double snr,
	double delta, double depth,
	const DataModel::Origin *, const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	double &value) {

	// Clip depth to 0
	if ( depth < 0 ) {
		depth = 0;
	}

	if ( delta < minDistanceDeg || delta > maxDistanceDeg ) {
		return DistanceOutOfRange;
	}

	if ( (depth < 0) || (depth > 700) ) {
		return DepthOutOfRange;
	}

	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	// maximum allowed period is 3 s according to IASPEI standard (pers. comm. Peter Bormann)
	if ( (period < 0.4) || (period > 3.0) ) {
		SEISCOMP_DEBUG("mb: period is %.2f s", period);
		return PeriodOutOfRange;
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	// amplitude is nanometers, whereas compute_mb wants micrometers
	bool valid = Magnitudes::compute_mb(amplitude*1.E-3, period, delta, depth+1, &value);
	value = correctMagnitude(value);
	return valid ? OK : Error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
