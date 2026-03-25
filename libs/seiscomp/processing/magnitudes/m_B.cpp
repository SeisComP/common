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

#include <seiscomp/processing/magnitudes/m_B.h>
#include <seiscomp/seismology/magnitudes.h>
#include <seiscomp/logging/log.h>
#include <math.h>


namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm/s";

}


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_mB, "mB");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mB::MagnitudeProcessor_mB()
 : MagnitudeProcessor("mB") {
	MagnitudeProcessor_mB::setDefaults();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mB::MagnitudeProcessor_mB(const std::string &type)
 : MagnitudeProcessor(type) {
	MagnitudeProcessor_mB::setDefaults();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeProcessor_mB::setDefaults() {
	_minimumDistanceDeg = 5.0;   // default minimum distance
	_maximumDistanceDeg = 105.0; // default maximum distance
	_minimumDepthKm = 0.0;   // default minimum depth
	_maximumDepthKm = 700.0; // default maximum depth
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_mB::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) ) {
		return false;
	}

	if ( (_minimumDepthKm && (*_minimumDepthKm < 0.)) ||
	     (_maximumDepthKm && (*_maximumDepthKm > 700.)) ) {
		SEISCOMP_WARNING("%s: configured minimum/maximum depth is out of allowed range [0, 700]km", type());
		return false;
	}

	if ( (_minimumDistanceDeg && (*_minimumDistanceDeg < 5)) ||
	     (_maximumDistanceDeg && (*_maximumDistanceDeg > 105)) ) {
		SEISCOMP_WARNING("%s: configured minimum/maximum distance is out of allowed range [5, 105]°", type());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mB::computeMagnitude(
        double amplitude, const std::string &unit,
        double, double,
        double delta, double depth,
        const DataModel::Origin *, const DataModel::SensorLocation *,
        const DataModel::Amplitude *, const Locale *, double &value) {
	// Clip depth to 0
	if ( depth < 0 ) {
		depth = 0;
	}

	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	bool status = Magnitudes::compute_mb(amplitude * 1.E-3, 2 * M_PI, delta, depth + 1, &value);
	value -= 0.14; // HACK until we have an optimal calibration function
	return status ? OK : Error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mB::estimateMw(
	const Config::Config *,
	double magnitude,
	double &Mw_estimate,
	double &Mw_stdError)
{
	const double a=1.30, b=-2.18;
//	if ( magnitude>=6 ) {
		Mw_estimate = a * magnitude + b;
//	}
//	else {
//		// This is to limit the difference between mB and Mw(mB)
//		// FIXME hack to be revised...
//		estimation = a * 6. + b + 0.7*(magnitude-6);
//	}

	Mw_stdError = 0.4;

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
