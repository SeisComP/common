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

#include <seiscomp/gui/core/utils.h>

namespace bu = boost::unit_test;
using namespace std;


BOOST_AUTO_TEST_SUITE(seiscomp_gui_strings)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(index) {
	QColor c;

	BOOST_CHECK(Seiscomp::Gui::fromString(c, "rgba(1,2,3,4)"));
	BOOST_CHECK_EQUAL(c.red(), 1);
	BOOST_CHECK_EQUAL(c.green(), 2);
	BOOST_CHECK_EQUAL(c.blue(), 3);
	BOOST_CHECK_EQUAL(c.alpha(), 4);

	BOOST_CHECK(Seiscomp::Gui::fromString(c, "rgba(5, 6, 7, 8)"));
	BOOST_CHECK_EQUAL(c.red(), 5);
	BOOST_CHECK_EQUAL(c.green(), 6);
	BOOST_CHECK_EQUAL(c.blue(), 7);
	BOOST_CHECK_EQUAL(c.alpha(), 8);

	BOOST_CHECK(Seiscomp::Gui::fromString(c, "rgb(10,11,12)"));
	BOOST_CHECK_EQUAL(c.red(), 10);
	BOOST_CHECK_EQUAL(c.green(), 11);
	BOOST_CHECK_EQUAL(c.blue(), 12);
	BOOST_CHECK_EQUAL(c.alpha(), 255);

	BOOST_CHECK(Seiscomp::Gui::fromString(c, "rgb(13, 14, 15)"));
	BOOST_CHECK_EQUAL(c.red(), 13);
	BOOST_CHECK_EQUAL(c.green(), 14);
	BOOST_CHECK_EQUAL(c.blue(), 15);
	BOOST_CHECK_EQUAL(c.alpha(), 255);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
