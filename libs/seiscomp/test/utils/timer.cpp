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
#include <stdexcept>
#include <unistd.h>

#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/utils/timer.h>


using namespace Seiscomp;


BOOST_AUTO_TEST_SUITE(seiscomp_utils_timer)


BOOST_AUTO_TEST_CASE(timer) {
	Util::StopWatch stopWatch;
	usleep(100000);
	auto ns = stopWatch.nanoseconds();
	auto us = stopWatch.microseconds();
	auto ts = stopWatch.elapsed();
	BOOST_CHECK(ns >= 100000000 && ns < 110000000);
	BOOST_CHECK(us >= 100000 && us < 110000);
	BOOST_CHECK(ts.seconds() == 0 && ts.microseconds() >= 100000 && ts.microseconds() < 110000);

	stopWatch.restart();
	sleep(1);
	ns = stopWatch.nanoseconds();
	us = stopWatch.microseconds();
	ts = stopWatch.elapsed();
	BOOST_CHECK(ns >= 1000000000 && ns < 1100000000);
	BOOST_CHECK(us >= 1000000 && us < 1100000);
	BOOST_CHECK(ts.seconds() == 1 && ts.microseconds() < 100000);
}


BOOST_AUTO_TEST_SUITE_END()
