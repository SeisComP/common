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
#include <numeric>


namespace Seiscomp::Math {


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
	int gcd = std::gcd(numerator, denominator);

	return {sign * numerator / gcd, denominator / gcd};
}


} // namespace Seiscomp::Math
