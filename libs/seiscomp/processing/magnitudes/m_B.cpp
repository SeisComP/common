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



#include <seiscomp/processing/magnitudes/m_B.h>
#include <seiscomp/seismology/magnitudes.h>
#include <math.h>

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm/s";

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_mB, MagnitudeProcessor, "MagnitudeProcessor_mB");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_mB, "mB");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mB::MagnitudeProcessor_mB()
 : MagnitudeProcessor("mB") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

MagnitudeProcessor_mB::MagnitudeProcessor_mB(const std::string& type)
 : MagnitudeProcessor(type) {}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mB::computeMagnitude(
        double amplitude, const std::string &unit,
        double, double,
        double delta, double depth,
        const DataModel::Origin *, const DataModel::SensorLocation *,
        const DataModel::Amplitude *, double &value) {
	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) )
		return InvalidAmplitudeUnit;

	bool status = Magnitudes::compute_mb(amplitude*1.E-3, 2*M_PI, delta, depth+1, &value);
	value -= 0.14; // HACK until we have an optimal calibration function
	value = correctMagnitude(value);
	return status ? OK : Error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mB::estimateMw(
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
