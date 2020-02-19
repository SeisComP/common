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

#define SEISCOMP_TEST_MODULE TestMSEEDRecord

#include <iostream>
#include <stdexcept>
#include <stdio.h>

#include <seiscomp3/unittest/unittests.h>

#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/io/records/mseedrecord.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct TestData {
	TestData() {
		Time startTime;
		startTime.set(2019, 1, 1, 0, 0, 0, 8543);
		filledRec.setStartTime(startTime);
		filledRec.setSamplingFrequency(20);
		filledRec.setNetworkCode("XX");
		filledRec.setStationCode("STA");
		filledRec.setLocationCode("");
		filledRec.setChannelCode("BHZ");
		filledRec.setTimingQuality(30);
		filledRec.setDataType(Array::INT);

		IntArrayPtr samples = new IntArray(100);
		for ( int i = 0; i < 100; ++i ) {
			samples->set(i, i);
		}

		filledRec.setData(samples.get());
		filledRec.dataUpdated();
	}

	GenericRecord filledRec;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_FIXTURE_TEST_SUITE(test_suite1, TestData)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(WRITE_READ) {
	MSeedRecord mseed(filledRec);

	stringbuf buf;
	iostream ios(&buf);

	BOOST_CHECK_NO_THROW(mseed.write(ios));
	MSeedRecord rec;
	rec.setHint(Record::DATA_ONLY);
	rec.setDataType(Array::INT);
	BOOST_CHECK_NO_THROW(rec.read(ios));

	BOOST_CHECK_EQUAL(rec.streamID(), filledRec.streamID());
	BOOST_CHECK(rec.startTime() == filledRec.startTime());
	BOOST_CHECK_EQUAL(rec.samplingFrequency(), filledRec.samplingFrequency());
	BOOST_CHECK_EQUAL(rec.timingQuality(), filledRec.timingQuality());
	BOOST_CHECK_EQUAL(rec.dataType(), filledRec.dataType());
	BOOST_CHECK_EQUAL(rec.sampleCount(), filledRec.sampleCount());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE_END()
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
