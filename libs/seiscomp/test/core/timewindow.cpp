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


#include <seiscomp/unittest/unittests.h>
#include <seiscomp/core/timewindow.h>


using namespace std;
using namespace Seiscomp::Core;
namespace bu = boost::unit_test;


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE(seiscomp_core_timewindow)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(Construction) {
	Time start(2024, 12, 1, 10, 38, 42, 0);
	Time end(2024, 12, 1, 11, 38, 42, 0);

	TimeWindow tw0;
	TimeWindow tw1(start, end);
	TimeWindow tw2(start, end - start);
	TimeWindow tw3(start, static_cast<double>(end - start));

	BOOST_CHECK_EQUAL(tw0.startTime(), Time());
	BOOST_CHECK_EQUAL(tw0.endTime(), Time());

	BOOST_CHECK_EQUAL(tw1.startTime(), tw2.startTime());
	BOOST_CHECK_EQUAL(tw2.startTime(), tw3.startTime());

	BOOST_CHECK_EQUAL(tw1.endTime(), tw2.endTime());
	BOOST_CHECK_EQUAL(tw2.endTime(), tw3.endTime());

	BOOST_CHECK_EQUAL(tw1.length(), tw2.length());
	BOOST_CHECK_EQUAL(tw2.length(), tw3.length());

	BOOST_CHECK(!tw0);
	BOOST_CHECK(TimeWindow(start, start));
	BOOST_CHECK(tw1);
	BOOST_CHECK(tw2);
	BOOST_CHECK(tw3);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(Or) {
	TimeWindow tw1(Time(2024, 12, 1, 10, 38, 42, 0), Time(2024, 12, 1, 11, 38, 42, 0));
	TimeWindow tw2(Time(2024, 12, 1, 11, 38, 42, 0), Time(2024, 12, 1, 12, 0, 0, 0));

	auto tw = tw1 | tw2;

	BOOST_CHECK_EQUAL(tw.startTime(), tw1.startTime());
	BOOST_CHECK_EQUAL(tw.endTime(), tw2.endTime());

	tw = TimeWindow() | tw1;

	BOOST_CHECK_EQUAL(tw.startTime(), tw1.startTime());
	BOOST_CHECK_EQUAL(tw.endTime(), tw1.endTime());
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
