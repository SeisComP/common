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

#define SEISCOMP_TEST_MODULE TestSDSArchive
#define SEISCOMP_COMPONENT TestSDSArchive

#include <seiscomp3/unittest/unittests.h>

#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/recordstream/sdsarchive.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(READ_FR_SALF) {
	Logging::enableConsoleLogging(Logging::getAll());
	RecordStream::SDSArchive sds("archive");
	Time startTime(2018,06,30,16,18,38,943300);
	Time endTime(2018,06,30,16,21,58,943300);
	sds.addStream("FR", "SALF", "00", "HHN", startTime, endTime);

	RingBuffer buffer(0);
	RecordPtr rec;

	while ( (rec = sds.next()) ) {
		buffer.push_back(rec);
	}

	RecordPtr crec = buffer.contiguousRecord<double>();
	BOOST_REQUIRE(crec);
	SEISCOMP_DEBUG("%s ~ %s", crec->startTime().iso().c_str(), crec->endTime().iso().c_str());

	BOOST_CHECK(crec->startTime() <= startTime && endTime <= crec->endTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(READ_FR_SALF_MULTIPLE) {
	RecordStream::SDSArchive sds(
		"archive-day1/BHE,"
		"archive-day2/BHE,"
		"archive-day1/BHN,"
		"archive-day2/BHN,"
		"archive-day1/BHZ,"
		"archive-day2/BHZ"
	);

	Time startTime(2019,5,1,23,59,10,0);
	Time endTime(2019,5,2,0,0,50,0);
	sds.addStream("GE", "MORC", "", "BHE", startTime, endTime);
	sds.addStream("GE", "MORC", "", "BHN", startTime, endTime);
	sds.addStream("GE", "MORC", "", "BHZ", startTime, endTime);

	RingBuffer buffer1(0), buffer2(0), buffer3(0);
	RecordPtr rec;

	while ( (rec = sds.next()) ) {
		if ( rec->channelCode() == "BHE" )
			buffer1.push_back(rec);
		else if ( rec->channelCode() == "BHN" )
			buffer2.push_back(rec);
		else if ( rec->channelCode() == "BHZ" )
			buffer3.push_back(rec);
	}

	RecordPtr crec = buffer1.contiguousRecord<double>();
	BOOST_REQUIRE(crec);
	SEISCOMP_DEBUG("%s ~ %s", crec->startTime().iso().c_str(), crec->endTime().iso().c_str());
	BOOST_CHECK(crec->startTime() <= startTime && endTime <= crec->endTime());

	crec = buffer2.contiguousRecord<double>();
	BOOST_REQUIRE(crec);
	SEISCOMP_DEBUG("%s ~ %s", crec->startTime().iso().c_str(), crec->endTime().iso().c_str());
	BOOST_CHECK(crec->startTime() <= startTime && endTime <= crec->endTime());

	crec = buffer3.contiguousRecord<double>();
	BOOST_REQUIRE(crec);
	SEISCOMP_DEBUG("%s ~ %s", crec->startTime().iso().c_str(), crec->endTime().iso().c_str());
	BOOST_CHECK(crec->startTime() <= startTime && endTime <= crec->endTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
