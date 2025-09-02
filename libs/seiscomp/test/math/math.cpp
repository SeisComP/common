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


#define SEISCOMP_TEST_MODULE SeisComP


#include <unistd.h>

#include <seiscomp/math/math.h>
#include <seiscomp/unittest/unittests.h>

#include <tuple>
#include <vector>


using namespace Seiscomp::Math;



BOOST_AUTO_TEST_SUITE(seiscomp_utils_timer)


BOOST_AUTO_TEST_CASE(Double2Frac) {

	double exact = 0.0;
	double approx = 1e-06;
    double sloppy = 1e-02;
	std::vector<std::tuple<double, double> > data = {
		{ 1.0, 0.0},
		{ 1.0, exact },
		{ 42.0, exact },
		{ 62.5, exact },
		{ M_PI, approx },
		{ M_E, approx },
		{ 10.1, exact },
		{ 100.1, exact },
		{ 1'000.1, exact },
		{ 10'000.1, exact },
		{ 100'000.1, exact },
		{ 1'000'000.1, exact },
		{ 10'000'000.1, exact },
		{ 10'000'000.01, sloppy },
		{ 10'000'000.001, sloppy },
		{ 10'000'000.0001, sloppy },
		{ 10'000'000.00001, sloppy },
		{ 10.9, exact },
		{ 100.9, exact },
		{ 1'000.9, exact },
		{ 10'000.9, exact },
		{ 100'000.9, exact },
		{ 1'000'000.9, exact },
		{ 10'000'000.9, exact },
		{ 10'000'000.09, sloppy },
		{ 10'000'000.009, sloppy },
		{ 10'000'000.0009, sloppy },
		{ 10'000'000.00009, sloppy },
		{ 9.9, exact },
		{ 99.9, exact },
		{ 999.9, exact },
		{ 99'999.1, exact },
		{ 999'999.9, exact },
		{ 9'999'999.9, exact },
		{ 9'999'999.09, exact },
		{ 9'999'999.009, sloppy },
		{ 9'999'999.0009, sloppy },
		{ 9'999'999.00009, sloppy },
		{ 0.1, exact },
		{ 0.01, exact },
		{ 0.001, exact },
		{ 0.0001, exact },
		{ 0.00001, exact },
		{ 0.000001, exact },
		{ 0.9, exact },
		{ 0.09, exact },
		{ 0.009, exact },
		{ 0.0009, exact },
		{ 0.00009, exact },
		{ 0.000009, exact }
	};

	// test positive and negative numbers alike
	for ( auto sign : {1, -1} ) {
		for ( const auto &[abs_value, requiredPrecision] : data ) {
			auto value = sign * abs_value;
			auto frac = double2frac(value);
			auto df = static_cast<double>(frac.first) / frac.second;
			auto precision = fabs(df-value);
			BOOST_CHECK_MESSAGE(precision <= requiredPrecision,
			                   "Value: " << value << " -> "
			                   << frac.first << "/" << frac.second
			                   << " (precision/expected: " << precision << "/"
			                   << requiredPrecision << ")");
		}
	}
}


BOOST_AUTO_TEST_SUITE_END()
