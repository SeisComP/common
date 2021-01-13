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
#include <stdio.h>

#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/utils/misc.h>


using namespace Seiscomp;


BOOST_AUTO_TEST_SUITE(seiscomp_utils_misc)


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


BOOST_AUTO_TEST_SUITE_END()
