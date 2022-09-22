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



#ifndef SEISCOMP_MATH_FFT_H
#define SEISCOMP_MATH_FFT_H


#include <complex>
#include <vector>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/math/math.h>


namespace Seiscomp {
namespace Math {


typedef std::complex<double> Complex;
typedef std::vector<Math::Complex> ComplexArray;


template <typename T>
void fft(ComplexArray &spec, int n, const T *data);

template <typename T>
void fft(ComplexArray &spec, const std::vector<T> &data) {
	fft(spec, static_cast<int>(data.size()), &data[0]);
}

template <typename T>
void fft(ComplexArray &spec, const TypedArray<T> &data) {
	fft(spec, data.impl());
}

template <typename T>
void fft(Seiscomp::ComplexDoubleArray &spec, const TypedArray<T> &data) {
	fft(spec.impl(), data.impl());
}

inline void fft(Seiscomp::ComplexDoubleArray &spec, const DoubleArray &data) {
	fft(spec.impl(), data.impl());
}


template <typename T>
void ifft(int n, T *out, ComplexArray &spec);

template <typename T>
void ifft(std::vector<T> &out, ComplexArray &spec) {
	ifft(static_cast<int>(out.size()), &out[0], spec);
}

template <typename T>
void ifft(TypedArray<T> &out, ComplexArray &spec) {
	ifft(out.impl(), &out[0], spec);
}

template <typename T>
void ifft(TypedArray<T> &out, Seiscomp::ComplexDoubleArray &spec) {
	ifft(out.impl(), &out[0], spec.impl());
}

inline void ifft(DoubleArray &out, Seiscomp::ComplexDoubleArray &spec) {
	ifft(out.impl(), spec.impl());
}


}
}


#endif
