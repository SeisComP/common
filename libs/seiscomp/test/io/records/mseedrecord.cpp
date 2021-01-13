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
#include <seiscomp/core/genericrecord.h>
#include <seiscomp/io/records/mseedrecord.h>


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
BOOST_FIXTURE_TEST_SUITE(seiscomp_io_records_mseed, TestData)
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
