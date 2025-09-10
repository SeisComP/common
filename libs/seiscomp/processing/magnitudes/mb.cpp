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

#include <seiscomp/processing/magnitudes/mb.h>
#include <seiscomp/seismology/magnitudes.h>
#include <seiscomp/logging/log.h>

#include "iostream"

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm";

}


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_mb, "mb");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mb::MagnitudeProcessor_mb()
 : MagnitudeProcessor("mb") {
	MagnitudeProcessor_mb::setDefaults();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeProcessor_mb::setDefaults() {
	_minimumDepthKm     = 0.0;
	_maximumDepthKm     = 700.0;
	_minimumDistanceDeg = 5.0;   // default minimum distance
	_maximumDistanceDeg = 105.0; // default maximum distance

	// maximum allowed period is 3 s according to IASPEI standard (pers. comm. Peter Bormann)
	_minimumPeriod = 0.4;
	_maximumPeriod = 3.0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mb::computeMagnitude(
	double amplitude,
	const std::string &unit,
	double period, double,
	double delta, double depth,
	const DataModel::Origin *, const DataModel::SensorLocation *,
	const DataModel::Amplitude *,
	const Locale *,
	double &value) {

	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	// amplitude is nanometers, whereas compute_mb wants micrometers
	bool valid = Magnitudes::compute_mb(amplitude * 1.E-3, period, delta, depth, &value);
	return valid ? OK : Error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
