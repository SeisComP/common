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



#ifndef SEISCOMP_MATH_RESTITUTION_FFT_H
#define SEISCOMP_MATH_RESTITUTION_FFT_H


#include <complex>
#include <vector>
#include <seiscomp/math/math.h>
#include <seiscomp/math/restitution/types.h>
#include <seiscomp/math/restitution/transferfunction.h>


namespace Seiscomp {
namespace Math {
namespace Restitution {


// Transforms a time series into the spectra, deconvolves it with the
// transfer function and transforms the spectra back into the time series.
// The spectra is tapered before min_freq and after max_freq. min_freq or
// max_freq has to be greater than 0 otherwise the tapering on the
// corresponding end is disabled.
template <typename T>
bool transformFFT(int n, T *inout, double fsamp, const FFT::TransferFunction *tf,
                  double cutoff, double min_freq, double max_freq);

template <typename T>
bool transformFFT(std::vector<T> &inout, double fsamp, const FFT::TransferFunction *tf,
                  double cutoff, double min_freq, double max_freq) {
	return transformFFT(inout.size(), &inout[0], fsamp, tf, cutoff, min_freq, max_freq);
}


template <typename T>
bool transformFFT(int n, T *inout, double fsamp, int n_poles, SeismometerResponse::Pole *poles,
                  int n_zeros, SeismometerResponse::Zero *zeros, double norm,
                  double cutoff, double min_freq, double max_freq) {
	FFT::PolesAndZeros paz(n_poles, poles, n_zeros, zeros, norm);
	return transformFFT(n, inout, fsamp, &paz, cutoff, min_freq, max_freq);
}

template <typename T>
bool transformFFT(std::vector<T> &inout, double fsamp, const Poles &poles,
                  const Zeros &zeros, double norm, double cutoff,
                  double min_freq, double max_freq) {
	return transformFFT(inout.size(), &inout[0], poles.size(), &poles[0],
	                    zeros.size(), &zeros[0], norm, cutoff, min_freq, max_freq);
}


}
}
}


#endif
