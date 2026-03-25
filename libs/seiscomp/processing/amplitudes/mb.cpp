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


#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/processing/amplitudes/mb.h>
#include <seiscomp/processing/amplitudes/iaspei.h>
#include <seiscomp/math/filter/seismometers.h>

#include <cmath>
#include <limits>

using namespace Seiscomp::Math;


namespace {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool measurePeriod(
	int n, const double *f, int i0, double offset,
	double *per, double *std)
{

	// Measures the period of an approximately sinusoidal signal f about
	// the sample with index i0. It does so by measuring the zero
	// crossings of the signal as well as the position of its extrema.
	int ip1, ip2, in1, in2;

	double f0 = f[i0] - offset;

	// Find zero crossings

	// first previous
	for ( ip1 = i0;   ip1 >= 0 && (f[ip1] - offset)*f0 >= 0;  ip1-- );
	// second previous
	for ( ip2 = ip1;  ip2 >= 0 && (f[ip2] - offset)*f0 <  0;  ip2-- );

	// first next
	for ( in1 = i0;   in1 < n  && (f[in1] - offset)*f0 >= 0;  in1++ );
	// second next
	for ( in2 = in1;  in2 < n  && (f[in2] - offset)*f0 <  0;  in2++ );

	double wt = 0, pp = 0;

	// for computing the standard deviation, we need:
	double m[5];
	int nm=0;

	if ( ip2 >= 0 ) {
		wt += 0.5;
		pp += 0.5*(ip1 - ip2);
		m[nm++] = ip1 - ip2;

		int imax = find_absmax(n, f, ip2, ip1, offset);
		wt += 1;
		pp += i0 -imax;
		m[nm++] = i0 -imax;
	}
	if ( ip1 >= 0 && in1 < n ) {
		wt += 1;
		pp += in1 -ip1;
		m[nm++] = in1 -ip1;
	}
	if ( in2 < n ) {
		wt += 0.5;
		pp += 0.5*(in2 -in1);
		m[nm++] = in2 -in1;

		int imax = find_absmax(n, f, in1, in2, offset);
		wt += 1;
		pp += imax - i0;
		m[nm++] = imax - i0;

	}

	// compute standard deviation of period
	if ( nm >= 3 ) {
		double avg = 0, sum = 0;
		for ( int i = 0; i < nm; i++ )
			avg += m[i];
		avg /= nm;
		for ( int i = 0; i < nm; i++ )
			sum += (m[i]-avg)*(m[i]-avg);
		*std = 2*sqrt(sum/(nm-1));
	}
	else {
		*std = 0;
	}

	if (wt < 0.9)
		return false;

	*per = 2*pp/wt;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}

namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(AmplitudeProcessor_mb, AmplitudeProcessor, "AmplitudeProcessor_mb");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_mb, "mb");




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_mb::AmplitudeProcessor_mb()
: AmplitudeProcessor("mb") {
	setSignalEnd(30);
	setMinSNR(0);
	setMinDist(5);
	setMaxDist(105);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_mb::initFilter(double fsamp) {
	AmplitudeProcessor::setFilter(
		new Filtering::IIR::WWSSN_SP_Filter<double>(Velocity)
	);
	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_mb::computeAmplitude(
	const DoubleArray &data,
	size_t, size_t,
	size_t si1, size_t si2,
	double offset,
	AmplitudeIndex *dt,
	AmplitudeValue *amplitude,
	double *period, double *snr)
{
	const int n = data.size();
	const double *f = static_cast<const double*>(data.data());

	double amax, pmax;
	int imax;

	if ( _config.iaspeiAmplitudes ) {
		IASPEI::AmplitudePeriodMeasurement m;

		bool OK = IASPEI::measureAmplitudePeriod(n, f, offset, si1, si2, m);
		if ( ! OK )
			return false;

		amax = (m.ap2p2 + m.ap2p1) / 2;
		imax = (m.ip2p2 + m.ip2p1) / 2;
		pmax = (m.ip2p2 - m.ip2p1) * 2;
		// We don't determine the standard error of the period.
	}
	else {
		std::vector<double> d(n);

		// Locate the max. of the derivative to find (A/T)_max
		for ( int i = 1; i < n - 1; ++i ) {
			d[i] = 0.5 * (f[i + 1] - f[i - 1]);
		}

		d[0] = d[n - 1] = 0;

		// Find the max. amplitude in the *derivative*
		imax = find_absmax(n, d.data(), si1, si2, offset);
		pmax = -1; // dominant period around maximum
		double pstd =  0; // standard error of period

		// Measure period in the original trace but at the position
		// of the max. amplitude of its *derivative*
		if ( !measurePeriod(si2 - si1, f + si1, imax - si1, offset, &pmax, &pstd) ) {
			pmax = -1;
		}
		else {
			// Finally relocate the max. amplitude in the original trace
			// at the position of the max. amplitude of its *derivative*
			imax = find_absmax(si2 - si1, f + si1, imax - si1 - (int)pmax, imax - si1 + (int)pmax, offset) + si1;
		}

		amax = std::abs(f[imax] - offset);
	}

	if ( *_noiseAmplitude == 0. )
		*snr = 1E6;
	else
		*snr = amax / *_noiseAmplitude;

	// SNR check
	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	dt->index = imax;

	if ( pmax > 0 )
		*period = pmax;

	amplitude->value = amax;

	if ( _streamConfig[targetComponent()].gain != 0.0 ) {
		amplitude->value /= _streamConfig[targetComponent()].gain;
	}
	else {
		setStatus(MissingGain, 0.0);
		return false;
	}

	// Convert meters to nanometers
	amplitude->value *= 1.E9;

	amplitude->value = std::abs(amplitude->value);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_mb::finalizeAmplitude(DataModel::Amplitude *amplitude) const {
	if ( amplitude == NULL )
		return;

	try {
		DataModel::TimeQuantity time(amplitude->timeWindow().reference());
		amplitude->setScalingTime(time);
	}
	catch ( ... ) {
	}

	try {
		DataModel::RealQuantity A = amplitude->amplitude();
		double f = 1. / amplitude->period().value();
		double c = 1. / IASPEI::wwssnspAmplitudeResponse(f);
		A.setValue(c*A.value());
		amplitude->setAmplitude(A);
	}
	catch ( ... ) {
	}

	if (_config.iaspeiAmplitudes) {
		amplitude->setMethodID("IASPEI mb amplitude");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
