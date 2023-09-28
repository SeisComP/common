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


#include <seiscomp/processing/amplitudes/Mwp.h>
#include <seiscomp/math/filter/iirintegrate.h>
#include<seiscomp/math/filter/butterworth.h>

#include <seiscomp/math/geo.h>

#include <limits>


using namespace Seiscomp::Math::Filtering::IIR;


namespace Seiscomp {
namespace Processing {


namespace {


void Mwp_demean(int n, double *f, int i0) {
	int i;
	double sum = 0, mean;

	for (i=0; i<i0; i++)
			sum += f[i];
	mean = sum/i0;

	for (i=0; i<n; i++)
			f[i] -= mean;
}

void Mwp_taper(int n, double *f, int i0) {
	int i, nn=i0/2;
	double q = M_PI/nn;
	for (i=0; i<nn; i++)
			f[i] *= 0.5*(1-cos(i*q));
}

void Mwp_integr(int n, double *f, int i0) {
	int i;
	double sum = 0;

	for (i=0; i<n; i++) {
		sum += f[i];
		f[i] = sum;
	}
}


void Mwp_scale(int n, double *f, double factor) {
	int i;

	for (i=0; i<n; i++) {
		f[i] *= factor;
	}
}


double Mwp_SNR(int n, double *f, int i0) {
	int i;
	double smax = 0, nmax = 0;

	for (i=0; i<i0; i++) {
		double n = fabs(f[i]);
		if (n > nmax)
			nmax = n;
	}
	for (i=i0; i<n; i++) {
		double s = fabs(f[i]);
		if (s > smax)
			smax = s;
	}

	return smax/nmax;
}


double Mwp_amplitude(int n, double *f, int i0, int *pos) {
	int i;
	double smax = 0;
	*pos = i0;

	for (i=i0; i<n; i++) {
		double s = fabs(f[i]);
		if (s > smax) {
			*pos = i;
			smax = s;
		}
	}

	return smax;
}


void Mwp_double_integration(int n, double *f, int i0, double fsamp) {
	Mwp_integr(n, f, i0);
	Mwp_integr(n, f, i0);
	Mwp_scale (n, f, 1/(fsamp*fsamp));
}


}


REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_Mwp, "Mwp");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_Mwp::AmplitudeProcessor_Mwp()
	: AmplitudeProcessor("Mwp") {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Mwp::init() {
	setSignalEnd("min(D * 11.5, 95)");
	setNoiseStart(-240.);
	setMinDist(5);
	setMaxDist(105);
	setMinSNR(3);
	computeTimeWindow();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



/*
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Mwp::initFilter(double fsamp) {
	AmplitudeProcessor::setFilter(
		new Math::Filtering::IIR::ButterworthHighpass<double>(3,.01, fsamp)
	);

	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
*/



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_Mwp::computeAmplitude(const DoubleArray &data,
                                              size_t i1, size_t i2,
                                              size_t si1, size_t si2, double offset,
                                              AmplitudeIndex *dt,
                                              AmplitudeValue *amplitude,
                                              double *period, double *snr) {
	size_t imax = find_absmax(data.size(), (const double*)data.data(), si1, si2, offset);
	double amax = fabs(data[imax] - offset);

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

/*
	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		_processedData = continuousData();
		return false;
	}
*/
	int onset = i1, n=i2; // XXX

	_processedData.resize(n);

	for ( int i = 0; i < n; ++i )
		_processedData[i] = (data[i] - offset) / _streamConfig[_usedComponent].gain;

	// Apply mild highpass to take care of long-period noise.
	// This is required unless the stations are exceptionally good.
	ButterworthHighpass<double> *hp = new ButterworthHighpass<double>(2,.008, _stream.fsamp);

	Mwp_demean(n, _processedData.typedData(), onset);
	Mwp_taper (n, _processedData.typedData(), onset);
	hp->apply(n, _processedData.typedData());
	Mwp_double_integration(n, _processedData.typedData(), onset, _stream.fsamp);
	// apply high pass a second time
	hp->reset();
	hp->apply(n, _processedData.typedData());
	delete hp;

	// Amplitude in nanometers
	amplitude->value = 1.E9*Mwp_amplitude(si2, _processedData.typedData(), si1, &onset);

	dt->index = onset; // FIXME
	*period = 0.0;


	// Now check the SNR of the doubly integrated trace.
	// Perhaps we can skip the initial SNR test completely!
	*snr = Mwp_SNR(n, _processedData.typedData(), i1);
	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DoubleArray *AmplitudeProcessor_Mwp::processedData(Component comp) const {
	if ( comp != (Component)_usedComponent ) return nullptr;
	return &_processedData;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
