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

#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/publicobjectcache.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE(seiscomp_datamodel_cache)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(TIMESPAN) {
	PublicObjectTimeSpanBuffer buffer(nullptr, TimeSpan(1, 0));
	PickPtr pick = Pick::Create();
	BOOST_CHECK(buffer.feed(pick.get()));
	string publicID = pick->publicID();

	BOOST_CHECK_EQUAL(buffer.size(), 1);
	BOOST_CHECK_EQUAL(pick->referenceCount(), 2);
	pick.reset();

	// Retrieving the pick must succeed as nothing else has been added
	// to the cache.
	pick = buffer.get<Pick>(publicID);
	BOOST_CHECK(pick);

	// Sleep for two seconds
	sleep(2);

	// and feed another pick. Since the buffer length is just one second
	// the old pick should be removed.
	BOOST_CHECK(buffer.feed(Pick::Create()));

	BOOST_CHECK_EQUAL(buffer.size(), 1);
	BOOST_CHECK_EQUAL(pick->referenceCount(), 1);

	// The pick must be found in PublicObject registry because it is
	// still being held by the smartpointer in pick. And the cache will feed
	// again any valid object which has been found.
	pick = buffer.get<Pick>(publicID);
	BOOST_CHECK(pick);
	BOOST_CHECK_EQUAL(buffer.size(), 2);
	BOOST_CHECK_EQUAL(pick->publicID(), publicID);
	BOOST_CHECK_EQUAL(pick->referenceCount(), 2);

	// Sleep for two seconds
	sleep(2);

	BOOST_CHECK(buffer.feed(Pick::Create()));
	BOOST_CHECK_EQUAL(buffer.size(), 1);

	// Release the smart pointer and check again. Now it must fail.
	pick.reset();
	pick = buffer.get<Pick>(publicID);
	BOOST_CHECK(!pick);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE_END()
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
