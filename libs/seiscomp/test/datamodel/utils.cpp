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

#include <seiscomp/datamodel/inventory_package.h>
#include <seiscomp/datamodel/utils.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE(seiscomp_datamodel_utils)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(ID) {
	NetworkPtr net = Network::Create();
	StationPtr sta = Station::Create();
	SensorLocationPtr loc = SensorLocation::Create();
	StreamPtr stream = Stream::Create();

	BOOST_CHECK_EQUAL(id(static_cast<Network*>(nullptr)), "");
	BOOST_CHECK_EQUAL(id(static_cast<Station*>(nullptr)), ".");
	BOOST_CHECK_EQUAL(id(static_cast<SensorLocation*>(nullptr)), "..");
	BOOST_CHECK_EQUAL(id(static_cast<SensorLocation*>(nullptr), "--"), "..--");
	BOOST_CHECK_EQUAL(id(static_cast<Stream*>(nullptr)), "...");
	BOOST_CHECK_EQUAL(id(static_cast<Stream*>(nullptr), "--"), "..--.");

	BOOST_CHECK_EQUAL(id(net.get()), "");
	BOOST_CHECK_EQUAL(id(sta.get()), ".");
	BOOST_CHECK_EQUAL(id(loc.get()), "..");
	BOOST_CHECK_EQUAL(id(loc.get(), "--"), "..--");
	BOOST_CHECK_EQUAL(id(stream.get()), "...");
	BOOST_CHECK_EQUAL(id(stream.get(), "--"), "..--.");

	net->setCode("11");
	sta->setCode("22222");
	loc->setCode("33");
	stream->setCode("HHZ");

	BOOST_CHECK_EQUAL(id(net.get()), "11");
	BOOST_CHECK_EQUAL(id(sta.get()), ".22222");
	BOOST_CHECK_EQUAL(id(loc.get()), "..33");
	BOOST_CHECK_EQUAL(id(stream.get(), "--"), "..--.HHZ");
	BOOST_CHECK_EQUAL(id(stream.get()), "...HHZ");

	net->add(sta.get());
	BOOST_CHECK_EQUAL(id(sta.get()), "11.22222");

	sta->add(loc.get());
	BOOST_CHECK_EQUAL(id(loc.get()), "11.22222.33");

	loc->add(stream.get());
	BOOST_CHECK_EQUAL(id(stream.get()), "11.22222.33.HHZ");
	BOOST_CHECK_EQUAL(id(stream.get(), "--", false), "11.22222.33.HH");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE_END()
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
