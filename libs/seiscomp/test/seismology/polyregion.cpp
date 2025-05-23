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
#include <seiscomp/seismology/regions/polygon.h>


using namespace std;
using namespace Seiscomp;


#define STR(X) #X
#define STR2(X) STR(X)

#define ASSERT_MSG(cond, msg) do \
{ if (!(cond)) { \
	ostringstream oss; \
	oss << __FILE__ << "(" << __LINE__ << "): "<< msg << endl; cerr << oss.str(); \
	abort(); } \
} while(0)

BOOST_AUTO_TEST_SUITE(seiscomp_core_seismology_polygon)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(FEP) {
	Geo::PolyRegions regions("data/regions/fep");
	ASSERT_MSG(regions.regionCount() == 3,
	           "Invalid number of polygons in: " << regions.dataDir());
	BOOST_CHECK_EQUAL(regions.findRegionName(52.387549, 13.068868), "Potsdam 😎");
	BOOST_CHECK_EQUAL(regions.region(1)->name(), "NoSize");
	BOOST_CHECK_EQUAL(regions.region(2)->name(), "Comment");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
