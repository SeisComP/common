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


#include "./math.h"

#include <cmath>
#include <limits>
#include <numeric>


namespace {


template <typename T>
int countr_zero(T n) {
	static_assert(std::is_integral<T>::value, "type must be integer");
	int bits = sizeof(T) * 8;
	int num_zeros = 0;
	T test = 1;
	while ( bits ) {
		if ( n & test ) {
			break;
		}
		test <<= 1;
		++num_zeros;
		--bits;
	}

	return num_zeros;
}



template <typename T>
int gcd(T m, T n) {
	static_assert(std::is_integral<T>::value, "type must be integer");

	if ( m == 0 ) {
		return n;
	}

	if ( n == 0 ) {
		return m;
	}

	const int i = countr_zero(m);
	m >>= i;
	const int j = countr_zero(n);
	n >>= j;
	const int k = i < j ? i : j;

	while ( true ) {
		if ( m > n ) {
			T tmp = m;
			m = n;
			n = tmp;
		}

		n -= m;

		if ( n == 0 ) {
			return m << k;
		}

		n >>= countr_zero(n);
	}
}


}


namespace Seiscomp {
namespace Math {


Fraction double2frac(double value) {
	// Check numeric limits
	if ( value > std::numeric_limits<int>::max() ) {
		return {std::numeric_limits<int>::max(), 1};
	}

	if ( value < std::numeric_limits<int>::min() ) {
		return {std::numeric_limits<int>::min(), 1};
	}

	// Operate on positive numbers
	int sign = 1;
	if ( value < 0 ) {
		sign = -1;
		value = -value;
	}

	// Calculatate the largest possible power of 10 giving numeric integer
	// limits and the current input number
	static auto max_exp = floor(log10(std::numeric_limits<int>::max())) - 1.0;
	auto exp = max_exp;
	if ( value >= 10 ) {
		exp -= floor(log10(value));
	}

	// Expand input number with power of 10
	auto denominator = static_cast<int>(pow(10, exp));
	auto numerator = static_cast<int>(round(value * denominator));

	// Simplify the fraction by calculating the greatest common divisor
	int d = gcd(numerator, denominator);

	return {sign * numerator / d, denominator / d};
}


} // namespace Math
} // namespace Seiscomp
