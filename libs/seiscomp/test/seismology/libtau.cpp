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
#include <seiscomp/seismology/ttt.h>
#include <seiscomp/seismology/ttt/libtau.h>

#include <thread>


using namespace std;
using namespace Seiscomp;


#define STR(X) #X
#define STR2(X) STR(X)


BOOST_AUTO_TEST_SUITE(seiscomp_core_seismology_libtau)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(MultiThreaded) {
	vector<thread> threads;
	threads.resize(4);

	setenv("SEISCOMP_LIBTAU_TABLE_DIR", STR2(BUILD_DIR) "/libs/3rd-party/tau/data/", 1);

	for ( auto &thrd : threads ) {
		thrd = thread([]() {
			TravelTimeTableInterfacePtr ttt = TravelTimeTableInterfaceFactory::Create("libtau");
			BOOST_REQUIRE(ttt);

			double receiverLat = 0.0;
			double receiverLon = 0.0;

			const int nY = 180;
			const int nX = 360;

			ttt->setModel("iasp91");

			for ( int y = 0; y < nY; ++y ) {
				for ( int x = 0; x < nX; ++x ) {
					double sourceLat = (y * 180.0) / nY - 90.0;
					double sourceLon = (x * 360.0) / nX - 180.0;
					auto tt = ttt->compute("P", sourceLat, sourceLon, 10.0, receiverLat, receiverLon, 0.0);
				}
			}
		});
	}

	for ( auto &thrd : threads ) {
		thrd.join();
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
