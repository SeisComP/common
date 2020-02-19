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


#define SEISCOMP_TEST_MODULE TestLZ4
#define SEISCOMP_COMPONENT TestLZ4

#include <iostream>
#include <stdexcept>
#include <stdio.h>

#include <seiscomp/logging/log.h>
#include <seiscomp/unittest/unittests.h>
#include <seiscomp/io/streams/filter/lz4.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>


using namespace std;
using namespace boost::iostreams;
using namespace Seiscomp;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct Init {
	Init() {
		if ( !initialized ) {
			Logging::enableConsoleLogging(Logging::getAll());
			initialized = true;
		}
	}

	static bool initialized;
};

bool Init::initialized = false;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_FIXTURE_TEST_SUITE(test_lz4, Init)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(WRITE1) {
	string store;

	{
		stream_buffer<back_insert_device<string> > buf(store);
		filtering_ostreambuf filtered_buf;
		filtered_buf.push(ext::boost::iostreams::lz4_compressor());
		filtered_buf.push(buf);

		filtered_buf.sputn("0123456789", 10);
		filtered_buf.sputn("ABCDEF", 6);
	}

	SEISCOMP_DEBUG("16 -> %d", int(store.size()));
	BOOST_REQUIRE_EQUAL(store.size(), 31);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(WRITE2) {
	string store;

	{
		stream_buffer<back_insert_device<string> > buf(store);
		filtering_ostreambuf filtered_buf;
		filtered_buf.push(ext::boost::iostreams::lz4_compressor());
		filtered_buf.push(buf);

		filtered_buf.sputn("ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCD", 40);
	}

	SEISCOMP_DEBUG("40 -> %d", int(store.size()));
	BOOST_REQUIRE_EQUAL(store.size(), 29);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(WRITEREAD1) {
	string store;

	{
		stream_buffer<back_insert_device<string> > buf(store);
		filtering_ostreambuf filtered_buf;
		filtered_buf.push(ext::boost::iostreams::lz4_compressor());
		filtered_buf.push(buf);

		filtered_buf.sputn("ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCD", 40);
	}

	char sink[1024];

	stream_buffer<array_source> buf(store.c_str(), store.size());
	filtering_istreambuf filtered_buf;
	filtered_buf.push(ext::boost::iostreams::lz4_decompressor());
	filtered_buf.push(buf);

	BOOST_REQUIRE_EQUAL(filtered_buf.sgetn(sink, 40), 40);
	sink[40] = '\0';

	BOOST_REQUIRE_EQUAL(sink, "ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCD");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(WRITEREAD2) {
	string store;
	string data;

	for ( int i = 0; i < 400; ++i )
		data += rand() % 26 + 65;

	{
		stream_buffer<back_insert_device<string> > buf(store);
		filtering_ostreambuf filtered_buf;
		filtered_buf.push(ext::boost::iostreams::lz4_compressor());
		filtered_buf.push(buf);

		filtered_buf.sputn(data.c_str(), data.size());
	}

	SEISCOMP_DEBUG("400 -> %d", int(store.size()));

	char sink[1024];

	stream_buffer<array_source> buf(store.c_str(), store.size());
	filtering_istreambuf filtered_buf;
	filtered_buf.push(ext::boost::iostreams::lz4_decompressor());
	filtered_buf.push(buf);

	BOOST_REQUIRE_EQUAL(filtered_buf.sgetn(sink, 400), 400);
	sink[400] = '\0';

	BOOST_REQUIRE_EQUAL(data, sink);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE_END()
