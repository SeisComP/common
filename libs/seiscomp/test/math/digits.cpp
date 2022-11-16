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


#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <random>
#include <vector>
#include <stdio.h>

#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/math/math.h>
#include <seiscomp/utils/timer.h>


using namespace Seiscomp;


template <typename T>
std::string toSignificantString(T val) {
	std::ostringstream ss;
	ss << std::setprecision(
		ss.flags() & std::ios_base::fixed ?
		Math::significantFixedDigits10(val) :
		Math::significantScientificDigits10(val)
	) << val;
	return ss.str();
}


BOOST_AUTO_TEST_SUITE(seiscomp_math_digits)


BOOST_AUTO_TEST_CASE(digits) {
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.5f), 1);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.5), 1);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.52f), 2);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.52), 2);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.5000000002f), 1);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.5000000002), 1);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.50002f), 8);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.50002), 5);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(0.0000000002), 10);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(321.0000000002), 0);
	BOOST_CHECK_EQUAL(Math::significantScientificDigits10(321.0000000002), 3);

	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(123.456), 3);
	BOOST_CHECK_EQUAL(Math::significantScientificDigits10(123.456), 6);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(123.45678), 5);
	BOOST_CHECK_EQUAL(Math::significantScientificDigits10(123.45678), 8);
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(123.45678900000001), 6);
	BOOST_CHECK_EQUAL(Math::significantScientificDigits10(123.45678900000001), 9);
	BOOST_CHECK_EQUAL(toSignificantString(123.45678900000001), "123.456789");
	BOOST_CHECK_EQUAL(Math::significantFixedDigits10(123.45678999999991), 5);
	BOOST_CHECK_EQUAL(Math::significantScientificDigits10(123.45678999999991), 8);
	BOOST_CHECK_EQUAL(toSignificantString(123.45678999999991), "123.45679");

	BOOST_CHECK_EQUAL(toSignificantString(0.10879999999999999), "0.1088");
	BOOST_CHECK_EQUAL(toSignificantString(9.3750000000000002e-05), "9.375e-05");
}


BOOST_AUTO_TEST_CASE(performance) {
	const size_t N = 1000000;
	std::vector<double> numbers(N*2);

	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<double> rand(100, 1000000);

	for ( size_t i = 0; i < N; ++i ) {
		numbers[i] = rand(eng);
	}

	std::uniform_real_distribution<double> rand2(0, 0.1);

	for ( size_t i = 0; i < N; ++i ) {
		numbers[N+i] = rand2(eng);
	}

	Util::StopWatch stopWatch;
	int generatedCharacters = 0;

	for ( size_t i = 0; i < numbers.size(); ++i ) {
		auto out = Core::toString(numbers[i]);
		generatedCharacters += out.size();
	}

	auto elapsed = stopWatch.elapsed();

	std::cerr << "Default: " << elapsed << "s, " << generatedCharacters << " chars" << std::endl;

	stopWatch.restart();
	int generatedCharacters2 = 0;

	for ( size_t i = 0; i < numbers.size(); ++i ) {
		auto out = toSignificantString(numbers[i]);
		generatedCharacters2 += out.size();
	}

	auto elapsed2 = stopWatch.elapsed();

	std::cerr << "Significant: " << elapsed2 << "s, " << generatedCharacters2 << " chars" << std::endl;

	std::cerr << "Difference time: " << (elapsed2 - elapsed) << "s, "
	          << (double(elapsed2 - elapsed) / elapsed)*100 << "%"
	          << std::endl;

	std::cerr << "Difference chars: " << (generatedCharacters2 - generatedCharacters) << " ch, "
	          << (double(generatedCharacters2 - generatedCharacters) / generatedCharacters)*100 << "%"
	          << std::endl;
}


BOOST_AUTO_TEST_SUITE_END()
