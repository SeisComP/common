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


#include <seiscomp/processing/amplitudes/mBc.h>
#include <limits>
#include "mBc_measure.h"


namespace Seiscomp {
namespace Processing {

// #define M_CAPITAL_B_DEFAULT_WINDOW_LENGTH 60 // Same as for mB since it is only a default

REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_mBc, "mBc");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mBc::AmplitudeProcessor_mBc() {
	_type = "mBc";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mBc::AmplitudeProcessor_mBc(const Core::Time& trigger)
: AmplitudeProcessor_mB(trigger) {
	_type = "mBc";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_mBc::computeAmplitude(const DoubleArray &data,
                                              size_t, size_t,
                                              size_t si1, size_t si2,
                                              double offset, AmplitudeIndex *dt,
                                              AmplitudeValue *amplitude,
                                              double *period, double *snr) {
	// see also amplitudeprocessor_m_B.cpp
	size_t n = si2 - si1;
	const double *v = data.typedData() + si1;
	Measurement_mBc measurement(n);
	measurement.setOffset(offset);
	measurement.feed(n, v);

	double pmax = -1;

	// Bei Mwp bestimmt man amax zur Berechnung des SNR. Die eigentliche
	// Amplitude ist aber was anderes! Daher ist SNR-Bestimmung auch
	// magnitudenspezifisch!

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else    // measurement.vmax is the same as used in mB
		*snr = measurement.vmax / *_noiseAmplitude;

	// SNR check
	if (*snr < _config.snrMin) {
		setStatus(LowSNR, *snr);
		return false;
	}

	dt->index = si1 + measurement.icum;
	*period = pmax;
	amplitude->value = measurement.vcum;
	if ( _streamConfig[_usedComponent].gain != 0.0 )
		amplitude->value /= _streamConfig[_usedComponent].gain;
	else {
		setStatus(MissingGain, 0.0);
		return false;
	}

	// Convert m/s to nm/s
	amplitude->value *= 1.E09;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
