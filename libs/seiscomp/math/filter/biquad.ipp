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


// This file is included by "biquad.cpp"

template<typename TYPE>
Biquad<TYPE>::Biquad(double b0, double b1, double b2,
                     double a0, double a1, double a2)
: InPlaceFilter<TYPE>()
, coefficients(b0, b1, b2, a0, a1, a2) {
	v1 = v2 = 0.;
}

template<typename TYPE>
Biquad<TYPE>::Biquad(const BiquadCoefficients &coeff)
: InPlaceFilter<TYPE>()
, coefficients(coeff) {
	v1 = v2 = 0.;
}

template<typename TYPE>
Biquad<TYPE>::Biquad(const Biquad<TYPE> &other)
: InPlaceFilter<TYPE>()
, coefficients(other.coefficients) {
	v1 = v2 = 0.;
}

template<typename TYPE>
void Biquad<TYPE>::apply(int n, TYPE *inout) {
	// This is the direct form 2 implementation according to
	// https://en.wikipedia.org/wiki/Digital_biquad_filter#Direct_form_2
	TYPE *ff = inout;
	for ( int i = 0; i < n;  ++i ) {

		// a0 is assumed to be 1
		double v0 = ff[i] - coefficients.a1*v1 - coefficients.a2*v2;
		ff[i]     = TYPE(coefficients.b0*v0 + coefficients.b1*v1 + coefficients.b2*v2);
		v2 = v1; v1 = v0;
	}
}

template<typename TYPE>
InPlaceFilter<TYPE>* Biquad<TYPE>::clone() const {
	return new Biquad<TYPE>(coefficients);
}

template<typename TYPE>
void Biquad<TYPE>::reset() {
	v1 = v2 = 0.;
}

template<typename TYPE>
void Biquad<TYPE>::setSamplingFrequency(double fsamp) {}

template<typename TYPE>
int Biquad<TYPE>::setParameters(int n, const double *params) {
	if ( n != 6 ) return 6;

	reset();
	coefficients.set(params[0], params[1], params[2],
	                 params[3], params[4], params[5]);

	return n;
}

template<typename TYPE>
BiquadCascade<TYPE>::BiquadCascade() {}

template<typename TYPE>
BiquadCascade<TYPE>::BiquadCascade(const BiquadCascade &other) {
	_biq = other._biq;
	reset();
}

template<typename TYPE>
int BiquadCascade<TYPE>::size() const { return _biq.size(); }

template<typename TYPE>
void BiquadCascade<TYPE>::apply(int n, TYPE *inout) {
	for ( Biquad<TYPE> &biq : _biq )
		biq.apply(n, inout);
}

template<typename TYPE>
int BiquadCascade<TYPE>::setParameters(int /*n*/, const double* /*params*/) {
	return 0;
}

template<typename TYPE>
InPlaceFilter<TYPE>* BiquadCascade<TYPE>::clone() const {
	return new BiquadCascade<TYPE>(*this);
}

template<typename TYPE>
void BiquadCascade<TYPE>::reset() {
	for ( Biquad<TYPE> &biq : _biq )
		biq.reset();
}

template<typename TYPE>
void BiquadCascade<TYPE>::append(Biquad<TYPE> const &biq) {
	_biq.push_back(biq);
}

template<typename TYPE>
void BiquadCascade<TYPE>::set(const Biquads &biquads) {
	_biq.clear();
	for ( const BiquadCoefficients &biq : biquads )
		_biq.push_back(biq);
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const BiquadCascade<T> &b) {
	int i = 0;
	for ( const Biquad<T> &biq : b._biq )
		os << "Biquad #" << ++i << std::endl << biq.coefficients;
	return os;
}

