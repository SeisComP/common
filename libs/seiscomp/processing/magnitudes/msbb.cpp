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



#include <seiscomp/processing/magnitudes/msbb.h>
#include <seiscomp/seismology/magnitudes.h>


#define DELTA_MIN 2.
#define DELTA_MAX 160.

#define DEPTH_MAX 100


namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "m/s";

}


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_msbb, "Ms(BB)");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_msbb::MagnitudeProcessor_msbb()
 : MagnitudeProcessor("Ms(BB)") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_msbb::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double,
	double delta, double depth,
	const DataModel::Origin *,
	const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	const Locale *,
	double &value) {
	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	if ( (delta < DELTA_MIN) || (delta > DELTA_MAX) ) {
		return DistanceOutOfRange;
	}

	// Clip depth to 0
	if ( depth < 0 ) {
		depth = 0;
	}

	if ( depth > DEPTH_MAX ) {
		return DepthOutOfRange; // strictly speaking it would be 60 km
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	// Convert amplitude unit from meters to micrometers
	value = log10((amplitude * 1E06) / (2 * M_PI)) + 1.66 * log10(delta) + 3.3;

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
