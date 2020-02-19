/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

#define SEISCOMP_TEST_MODULE TestUtilsMisc

#include <iostream>
#include <stdexcept>
#include <stdio.h>

#include <seiscomp3/unittest/unittests.h>

#include <seiscomp3/core/strings.h>
#include <seiscomp3/utils/misc.h>


using namespace Seiscomp;


BOOST_AUTO_TEST_CASE(tohex) {
	std::string out;
	unsigned int v = 0x12345678;
	Util::toHex(out, v);
	BOOST_CHECK_EQUAL(out, "12345678");

	out.clear();
	Util::toHex(out, (char)0x47);
	BOOST_CHECK_EQUAL(out, "47");

	out.clear();
	Util::toHex(out, (unsigned int)0xffff8040);
	BOOST_CHECK_EQUAL(out, "ffff8040");
}


