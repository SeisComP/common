/* ################################################################################
* #    Copyright (C) 2024 by IGN Spain                                           #
* #                                                                              #
* #    author: J. Barco, E. Suarez                                               #
* #    email:  jbarco@transportes.gob.es   ,  eadiaz@transportes.gob.es          #
* #    last modified: 2024-03-20                                                 #
* #                                                                              #
* #    This program is free software; you can redistribute it and/or modify      #
* #    it under the terms of the GNU General Public License as published by      #
* #    the Free Software Foundation; either version 2 of the License, or         #
* #    (at your option) any later version.                                       #
* #                                                                              #
* #    This program is distributed in the hope that it will be useful,           #
* #    but WITHOUT ANY WARRANTY; without even the implied warranty of            #
* #    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
* #    GNU General Public License for more details.                              #
* #                                                                              #
* #    You should have received a copy of the GNU General Public License         #
* #    along with this program; if not, write to the                             #
* #    Free Software Foundation, Inc.,                                           #
* #    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 #
* ################################################################################ */


#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/processing/amplitudes/mb_VC.h>

#include <cmath>
#include <limits>


namespace Seiscomp {
namespace Processing {


REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_mB_VC, "mB_VC");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mB_VC::AmplitudeProcessor_mB_VC()
: AmplitudeProcessor("mB_VC") {
	// 60 s should be OK for rupture durations up to ~100 s.
	// This default value MUST NOT be increased because if the amplitudes are
	// computed in the stream picker, which doesn't know of origins, it also
	// doesn't know the S-P time. For distances of 5 degrees the S wave would
	// leak into the time window thus contaminating the measurement.
	setSignalEnd("min(D * 11.5, 60)");
	setMinDist(5);
	setMaxDist(105);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_mB_VC::finalizeAmplitude(DataModel::Amplitude *amplitude) const {
	if ( !amplitude )
		return;

	try {
		DataModel::TimeQuantity time(amplitude->timeWindow().reference());
		amplitude->setScalingTime(time);
	}
	catch ( ... ) {
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_mB_VC::computeAmplitude(const DoubleArray &data,
                                             size_t i1, size_t i2,
                                             size_t si1, size_t si2,
                                             double offset,AmplitudeIndex *dt,
                                             AmplitudeValue *amplitude,
                                             double *period, double *snr) {
	/*
	* Low-level signal amplitude computation. This is magnitude specific.
	*
	* Input:
	*      f           double array of length n
	*      i1,i2       indices defining the measurement window,
	*                  0 <= i1 < i2 <= n
	*      offset      this is subtracted from the samples in f before
	*                  computation
	*
	* Output:
	*      dt          Point at which the measurement was mad/completed. May
	*                  be the peak time or end of integration.
	*      amplitude   amplitude. This may be a peak amplitude as well as a
	*                  sum or integral.
	*      period      dominant period of the signal. Optional. If the period
	*                  is not computed, set it to -1.
	*/
	int    imax = find_absmax(data.size(), data.typedData(), si1, si2, offset);
	double amax = fabs(data[imax] - offset);
	double pmax = -1;

	// Bei Mwp bestimmt man amax zur Berechnung des SNR. Die eigentliche
	// Amplitude ist aber was anderes! Daher ist SNR-Bestimmung auch
	// magnitudenspezifisch!

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

	// SNR check
	if (*snr < _config.snrMin) {
		setStatus(LowSNR, *snr);
		return false;
	}

	dt->index = imax;
	// Amplitudes are send in nanometers
	*period = pmax;

	amplitude->value = amax;

	if ( _streamConfig[_usedComponent].gain != 0.0 )
		amplitude->value /= _streamConfig[_usedComponent].gain;
	else {
		setStatus(MissingGain, 0.0);
		return false;
	}

	// Convert m/s to nm/s
	amplitude->value *= 1.E09;

	amplitude->value = std::abs(amplitude->value);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
