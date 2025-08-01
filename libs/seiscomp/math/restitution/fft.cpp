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



#include <seiscomp/math/restitution/fft.h>
#include <seiscomp/math/restitution/transferfunction.h>
#include <seiscomp/math/fft.h>
#include <seiscomp/math/filter.h>

#include <vector>
#include <iostream>


#define TWO_PI (M_PI*2)


using namespace std;


namespace Seiscomp {
namespace Math {
namespace Restitution {


namespace {


template <typename T>
void costaper(int n, T *inout, int istart, int iend, int estart, int eend) {
	int taperLength = iend - istart;

	for ( int i = 0; i < istart; ++i )
		inout[i] = 0;

	for ( int i = 0; i < taperLength; ++i ) {
		double frac = double(i)/taperLength;
		inout[istart+i] *= 0.5*(1-cos(M_PI*frac));
	}

	taperLength = eend - estart;

	for ( int i = 0; i < taperLength; ++i ) {
		double frac = double(i)/taperLength;
		inout[estart+i] *= 0.5*(1+cos(M_PI*frac));
	}

	for ( int i = eend; i < n; ++i )
		inout[i] = 0;
}


}


template <typename T>
bool transformFFT(int n, T *inout, double fsamp,
                  const FFT::TransferFunction *tf, double cutoff,
                  double min_freq, double max_freq) {
	if ( n <= 0 ) return false;

	int fftn = n;
	if ( fftn <= 0 )
		return false;

	if ( fsamp <= 0 )
		return false;

	// Demean time series
	T mean = 0;

	for ( int i = 0; i < n; ++i )
		mean += inout[i];

	mean /= (T)n;

	for ( int i = 0; i < n; ++i )
		inout[i] -= mean;

	// Time series taper
	int iTaperStart, iTaperEnd;
	int eTaperStart, eTaperEnd;
	int taperLength;

	if ( cutoff > 0 ) {
		taperLength = (int)(cutoff * fsamp);
		if ( taperLength > n ) taperLength = n;

		iTaperStart = 0;
		iTaperEnd = taperLength;

		eTaperStart = n - taperLength;
		eTaperEnd = n;

		if ( iTaperEnd > eTaperStart )
			eTaperStart = iTaperEnd;

		costaper(n, inout, iTaperStart, iTaperEnd, eTaperStart, eTaperEnd);
	}

	double nyquist_freq = fsamp * 0.5;

	vector<Complex> data_coeff;
	// len(data_coeff) = fftn/2+1
	fft(data_coeff, n, inout);

	int fftn2 = data_coeff.size();

	// Initial spectra taper
	double df = nyquist_freq / (fftn2-1);

	if ( min_freq > 0 ) {
		iTaperEnd = (int)(min_freq / df);
		if ( iTaperEnd > fftn2 ) iTaperEnd = fftn2;

		iTaperStart = iTaperEnd / 2;
	}
	else {
		iTaperStart = 0;
		iTaperEnd = 0;
	}

	// End spectra taper
	if ( max_freq > 0 ) {
		eTaperStart = (int)(max_freq / df);
		eTaperEnd = (int)((max_freq * 2) / df);

		if ( eTaperStart > fftn2 ) eTaperStart = fftn2;
		if ( eTaperEnd > fftn2 ) eTaperEnd = fftn2;
		if ( eTaperStart < iTaperEnd ) eTaperStart = iTaperEnd;
	}
	else {
		eTaperStart = data_coeff.size();
		eTaperEnd = eTaperStart;
	}

	// Multiply by freqs
	df = nyquist_freq/(fftn2-1);

	tf->deconvolve(data_coeff.size()-1, &data_coeff[1], df, df);

	costaper(data_coeff.size(), data_coeff.data(), iTaperStart, iTaperEnd, eTaperStart, eTaperEnd);

	// do the inverse FFT
	ifft(n, inout, data_coeff);

	return true;
}


// Explicit template instantiation for float and double types
template SC_SYSTEM_CORE_API
bool transformFFT<float>(int n, float *inout, double fsamp,
                         const FFT::TransferFunction *tf,
                         double cutoff, double min_freq, double max_freq);

template SC_SYSTEM_CORE_API
bool transformFFT<double>(int n, double *inout, double fsamp,
                          const FFT::TransferFunction *tf,
                          double cutoff, double min_freq, double max_freq);

}
}
}
