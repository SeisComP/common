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

#define SEISCOMP_TEST_MODULE test_version


#include <seiscomp/unittest/unittests.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/version.h>


#define STR(X) toString(int(X))

using namespace std;
using namespace Seiscomp::Core;
namespace bu = boost::unit_test;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(TestVersion) {
	Version v1(SC_API_VERSION);
	string apiVersion = STR(SC_API_VERSION_MAJOR(SC_API_VERSION)) + "." + STR(SC_API_VERSION_MINOR(SC_API_VERSION)) + "." + STR(SC_API_VERSION_PATCH(SC_API_VERSION));
	BOOST_CHECK(v1.majorTag() == SC_API_VERSION_MAJOR(SC_API_VERSION));
	BOOST_CHECK(v1.minorTag() == SC_API_VERSION_MINOR(SC_API_VERSION));
	BOOST_CHECK(v1.patchTag() == SC_API_VERSION_PATCH(SC_API_VERSION));
	BOOST_CHECK(v1.toString() == apiVersion);
	BOOST_CHECK(Version(SC_API_VERSION).toString() == apiVersion);

	Version v2 = v1;
	BOOST_CHECK(v1 == v2);

	v2 = Version();
	v2.fromString(apiVersion);
	BOOST_CHECK(v1 == v2);

	BOOST_CHECK(v1.fromString("1.2.3"));
	BOOST_CHECK_EQUAL(v1.majorTag(), 1);
	BOOST_CHECK_EQUAL(v1.minorTag(), 2);
	BOOST_CHECK_EQUAL(v1.patchTag(), 3);

	BOOST_CHECK(v2.fromString("4.5"));
	BOOST_CHECK_EQUAL(v2.majorTag(), 4);
	BOOST_CHECK_EQUAL(v2.minorTag(), 5);
	BOOST_CHECK_EQUAL(v2.patchTag(), 0);

	BOOST_CHECK(v1 < v2);
}
