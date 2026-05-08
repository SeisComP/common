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


#include <seiscomp/processing/magnitudes/Mwp.h>
#include <seiscomp/seismology/magnitudes.h>
#include <seiscomp/config/config.h>
#include <seiscomp/logging/log.h>
#include <math.h>

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm*s";

}


REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_Mwp, "Mwp");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_Mwp::MagnitudeProcessor_Mwp()
 : MagnitudeProcessor("Mwp") {
	_minimumDistanceDeg = 5.0;
	_maximumDistanceDeg = 105.0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_Mwp::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) ) {
		return false;
	}

	const Seiscomp::Config::Config *cfg = settings.localConfiguration;
	auto readDouble = [&](const std::string &key, double &val) {
		if ( !cfg ) return;
		cfg->getDouble(val, key);
	};

	readDouble("magnitudes.Mwp.estimateMw.a",        _mwA);
	readDouble("magnitudes.Mwp.estimateMw.b",        _mwB);
	readDouble("magnitudes.Mwp.estimateMw.stdError", _mwStdError);

	SEISCOMP_DEBUG("Mwp estimateMw: a=%.4f  b=%.4f  stdError=%.4f",
	               _mwA, _mwB, _mwStdError);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mwp::computeMagnitude(
	double amplitude, const std::string &unit,
	double, double, double delta, double,
	const DataModel::Origin *,
	const DataModel::SensorLocation *,
	const DataModel::Amplitude *, const Locale *,
	double &value) {

	if ( amplitude <= 0 ) {
		return AmplitudeOutOfRange;
	}

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) ) {
		return InvalidAmplitudeUnit;
	}

	if ( Magnitudes::compute_Mwp(amplitude*1.E-9, delta, value) ) {
		return OK;
	}
	else {
		return DistanceOutOfRange;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_Mwp::estimateMw(
	const Config::Config *,
	double magnitude,
	double &estimation,
	double &stdError)
{
	estimation = _mwA * magnitude + _mwB;
	stdError = _mwStdError;

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
