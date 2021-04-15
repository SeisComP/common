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

#include <seiscomp/core/datamessage.h>
#include <seiscomp/io/archive/binarchive.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/archive/jsonarchive.h>
#include <seiscomp/datamodel/eventparameters_package.h>
#include <seiscomp/datamodel/inventory_package.h>
#include <seiscomp/utils/timer.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
namespace bu = boost::unit_test;


BOOST_AUTO_TEST_SUITE(seiscomp_core_xml)
/****************************************************************/

const char *JSON2 =
"{"
"\"code\":\"BHZ\","
"\"start\":\"2007-11-19T00:00:00.0000Z\","
"\"datalogger\":\"Datalogger#20120509134106.694537.39279\","
"\"dataloggerSerialNumber\":\"xxxx\","
"\"dataloggerChannel\":0,"
"\"sensor\":\"Sensor#20110428113337.619967.1975\","
"\"sensorSerialNumber\":\"yyyy\","
"\"sensorChannel\":\"wrong\","
"\"sampleRateNumerator\":40,"
"\"sampleRateDenominator\":1,"
"\"depth\":3,"
"\"azimuth\":0,"
"\"dip\":-90,"
"\"gain\":2506580000,"
"\"gainFrequency\":5,"
"\"gainUnit\":\"M/S\","
"\"format\":\"Steim2\","
"\"flags\":\"G\","
"\"restricted\":false,"
"\"shared\":true"
"}";

const char *JSON1 =
"{"
"\"code\":\"BHZ\","
"\"start\":\"2007-11-19T00:00:00.0000Z\","
"\"datalogger\":\"Datalogger#20120509134106.694537.39279\","
"\"dataloggerSerialNumber\":\"xxxx\","
"\"dataloggerChannel\":0,"
"\"sensor\":\"Sensor#20110428113337.619967.1975\","
"\"sensorSerialNumber\":\"yyyy\","
"\"sensorChannel\":0,"
"\"sampleRateNumerator\":40,"
"\"sampleRateDenominator\":1,"
"\"depth\":3,"
"\"azimuth\":0,"
"\"dip\":-90,"
"\"gain\":2506580000,"
"\"gainFrequency\":5,"
"\"gainUnit\":\"M/S\","
"\"format\":\"Steim2\","
"\"flags\":\"G\","
"\"restricted\":false,"
"\"shared\":true"
"}";

/****************************************************************/

const char* getContent(int a) {

	const char *XML2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
	<seiscomp xmlns=\"http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.7\" version=\"0.7\">\n\
	<EventParameters>\n\
	    <origin publicID=\"Origin#20150212124718.250769.756608\">\n\
	        <time>\n\
	            <value>2000-02-12T12:09:48Z</value>\n\
	        </time>\n\
	        <latitude>\n\
	            <value>45.678</value>\n\
	        </latitude>\n\
	        <longitude>\n\
	            <value>-123.456</value>\n\
	        </longitude>\n\
	        <depth>\n\
	            <value>98.765</value>\n\
	        </depth>\n\
	        <depthType>from location</depthType>\n\
	        <methodID>LOCSAT</methodID>\n\
	        <earthModelID>iasp91</earthModelID>\n\
	    </origin>\n\
	</EventParameters>\n\
	</seiscomp>\
	";

	const char *XML1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
	<seiscomp xmlns=\"http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.7\" version=\"0.7\">\n\
	    <EventParameters>\n\
	        <origin publicID=\"Origin#20150212124718.250769.756608\">\n\
	            <time>\n\
	                <value>2000-02-12T12:09:48.17871Z</value>\n\
	                <uncertainty>0.5</uncertainty>\n\
	            </time>\n\
	            <latitude>\n\
	                <value>45.678</value>\n\
	            </latitude>\n\
	            <longitude>\n\
	                <value>-123.456</value>\n\
	            </longitude>\n\
	            <depth>\n\
	                <value>98.765</value>\n\
	            </depth>\n\
	            <depthType>from location</depthType>\n\
	            <methodID>LOCSAT</methodID>\n\
	            <earthModelID>iasp91</earthModelID>\n\
	        </origin>\n\
	    </EventParameters>\n\
	</seiscomp>\
	";
	if(a == 1)
	    return (XML1);
	else
	    return (XML2);
}

/****************************************************************/


class StringBuf : public std::streambuf {
	public:
		StringBuf(const char *s) : std::streambuf() {
			setbuf(const_cast<char*>(s), strlen(s));
		}

		std::streambuf *setbuf(char *s, std::streamsize n) {
			setp(nullptr, nullptr);
			setg(s, s, s + n);
			return this;
		}
};

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

BOOST_AUTO_TEST_CASE(TimeToString) {
	bu::unit_test_log.set_threshold_level(bu::log_warnings);
	bu::unit_test_log.set_threshold_level(bu::log_messages);

	Time t1;
	string check;
	bool match;
	string s;

	t1.set(1945, 5, 8, 16, 34, 51, 2123456);
	s = toString(t1);
	check = "1945-05-08T16:34:53.123456Z";
	match = boost::iequals(s, check);
	BOOST_CHECK_EQUAL(match, true);

	t1.set(2008,4,12,20,44,3,567445);
	s = toString(t1);
	check = "2008-04-12T20:44:03.567445Z";
	match = boost::iequals(s,check);
	BOOST_CHECK_EQUAL(match, true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(comparisonSign) {
	Time t1,t2;

	// check equal
	t1.set(2007,8,9,17,48,41,0);
	t2.set(2007,8,9,17,48,41,0);
	BOOST_CHECK_EQUAL(t1.seconds(), t2.seconds());
	string s1 = toString(t1);
	string s2 = toString(t2);
	bool match = boost::iequals(s1,s2);
	BOOST_CHECK_EQUAL(match, true);

	// check greater than
	t1.set(2007,4,3,14,44,23,0);
	t2.set(2007,4,3,4,44,23,0);
	s1 = toString(t1);
	s2 = toString(t2);
	BOOST_CHECK_GT((double)t1, (double)t2);
	match =  boost::iequals(s1,s2);
	BOOST_CHECK_EQUAL(match, false);

	// check greater equal
	BOOST_CHECK_GE((double)t1, (double)t2);
	match = boost::iequals(s1,s2);
	BOOST_CHECK_EQUAL(match, false);
	t1.set(2007,6,26,8,46,40,0);
	t2.set(2007,6,26,8,46,40,0);
	s1 = toString(t1);
	s2 = toString(t2);
	BOOST_CHECK_GE((double)t1,(double)t2);
	match = boost::iequals(s1,s2);
	BOOST_CHECK_EQUAL(match, true);

	// check lower equal
	t1.set(2007,12,30,5,30,22,0);
	t2.set(2008,12,30,5,30,22,0);
	BOOST_CHECK_LE((double)t1, (double)t2);
	s1 = toString(t1);
	s2 = toString(t2);
	match = boost::iequals(s1,s2);
	BOOST_CHECK_EQUAL(match, false);

	// check lower than
	BOOST_CHECK_LT((double)t1, (double)t2);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(jsonEqualString) {

	// parse JSON file
	rapidjson::Document doc;
	doc.Parse(JSON1);
	BOOST_CHECK_EQUAL(doc.HasParseError(), false);

	Seiscomp::IO::JSONArchive ar;
	ar.from(&doc);
	Seiscomp::DataModel::StreamPtr stream;
	ar >> stream;
	BOOST_CHECK_EQUAL(ar.success(),true);

	/************* Test JSON1 **************/
	bool start = doc.HasMember("start");
	if( start == true ) {
		string getStart = string(doc["start"].GetString());
		string check = "2007-11-19T00:00:00.0000Z";
		bool equal = boost::iequals(check,getStart);
		BOOST_CHECK_EQUAL(equal,true);
	}

	start = doc.HasMember("code");
	if( start == true ) {
		string getCode = string(doc["code"].GetString());
		string check = "BHZ";
		bool equal = boost::iequals(check,getCode);
		BOOST_CHECK_EQUAL(equal,true);
	}

	start = doc.HasMember("datalogger");
	if( start == true ) {
		string getDatalogger = string(doc["datalogger"].GetString());
		string check = "Datalogger#20120509134106.694537.39279";
		bool equal = boost::iequals(check,getDatalogger);
		BOOST_CHECK_EQUAL(equal,true);
	}

	start = doc.HasMember("depth");
	if( start == true ) {
		int getDepth = int(doc["depth"].GetInt());
		BOOST_CHECK_EQUAL(getDepth,3);
	}

	start = doc.HasMember("dip");
	if( start == true ) {
		int getDip = int(doc["dip"].GetInt());
		BOOST_CHECK_EQUAL(getDip, -90);
	}

	start = doc.HasMember("restricted");
	if( start == true ) {
		bool getRest = bool(doc["restricted"].GetBool());
		BOOST_CHECK_EQUAL(getRest, false);
	}

	start = doc.HasMember("sensorChannel");
	if( start == true ) {
		int getSenChannel = int(doc["sensorChannel"].GetInt());
		BOOST_CHECK_EQUAL(getSenChannel,0);
	}

	start = doc.HasMember("gain");
	if( start == true ) {
		rapidjson::Value::ConstMemberIterator itr = doc.FindMember("gain");
		if( itr->value.IsInt() ) {
			int getGain = int(doc["gain"].GetInt());
			BOOST_CHECK_EQUAL(getGain,2506580000);
		}
		if( itr->value.IsString() ) {
			string getGain = string(doc["gain"].GetString());
			string check = "2506580000";
			bool equal = boost::iequals(check,getGain);
			BOOST_CHECK_EQUAL(equal, true);
		}
	}
	/************* Test JSON2 **************/
	doc.Parse(JSON2);
	BOOST_CHECK_EQUAL(doc.HasParseError(), false);

	start = doc.HasMember("start");
	if( start == true ) {
		string getStart = string(doc["start"].GetString());
		string check = "2007-11-19T00:00:00.0000Z";
		bool equal = boost::iequals(check,getStart);
		BOOST_CHECK_EQUAL(equal,true);
	}

	start = doc.HasMember("code");
	if( start == true ) {
		string getCode = string(doc["code"].GetString());
		string check = "BHZ";
		bool equal = boost::iequals(check,getCode);
		BOOST_CHECK_EQUAL(equal,true);
	}

	start = doc.HasMember("datalogger");
	if( start == true ) {
		string getDatalogger = string(doc["datalogger"].GetString());
		string check = "Datalogger#20120509134106.694537.39279";
		bool equal = boost::iequals(check,getDatalogger);
		BOOST_CHECK_EQUAL(equal,true);
	}

	start = doc.HasMember("depth");
	if( start == true ) {
		int getDepth = int(doc["depth"].GetInt());
		BOOST_CHECK_EQUAL(getDepth,3);
	}

	start = doc.HasMember("dip");
	if( start == true ) {
		int getDip = int(doc["dip"].GetInt());
		BOOST_CHECK_EQUAL(getDip, -90);
	}

	start = doc.HasMember("shared");
	if( start == true ) {
		bool getRest = bool(doc["shared"].GetBool());
		BOOST_CHECK_EQUAL(getRest, true);
	}

	start = doc.HasMember("sensorChannel");
	if( start == true ) {
		rapidjson::Value::ConstMemberIterator itr = doc.FindMember("sensorChannel");
		if(itr->value.IsInt()) {
			int getSenChannel1 = int(doc["sensorChannel"].GetInt());
			BOOST_CHECK_MESSAGE(itr->value.IsInt(),"Value of sensorChannel is:"
			                    << getSenChannel1<< ".");
		}
		if( itr->value.IsString() ) {
			std::string getSenChannel2 = string(doc["sensorChannel"].GetString());
			string check = "wrong";
			BOOST_CHECK_EQUAL(check, getSenChannel2);
		}
	}


}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(xmlEqualString) {

	const char* XML1 = getContent(1);
	StringBuf buf(XML1);
	Seiscomp::IO::XMLArchive x;
	EventParametersPtr ep;
	BaseObjectPtr obj;
	Origin *org;

	/******** test XML1 *******/
	x.open(&buf);
	x >> ep;
	x.close();

	BOOST_CHECK((bool)ep == true);
	bool oriCount = ep->originCount();
	BOOST_CHECK( oriCount == 1);

	org = ep->origin(0);
	string getString = org->time().value().iso();
	string check = "2000-02-12T12:09:48.17871Z";
	BOOST_CHECK_EQUAL(check, getString);
	BOOST_CHECK_EQUAL(org->time().uncertainty(),0.5);

	getString = org->publicID();
	check = "Origin#20150212124718.250769.756608";
	BOOST_CHECK_EQUAL(getString, check);

	double getDouble = org->longitude();
	BOOST_CHECK_EQUAL(getDouble,-123.456);

	getString = org->methodID();
	check = "LOCSAT";
	BOOST_CHECK_EQUAL(getString, check);

	ep = nullptr;

	/*********** testXML2 **********/
	const char* XML2 = getContent(2);
	StringBuf buf2(XML2);
	x.open(&buf2);
	x >> ep;
	x.close();

	BOOST_CHECK_EQUAL((bool)ep , true);
	oriCount = ep->originCount();
	BOOST_CHECK(oriCount == true);

	org = ep->origin(0);
	getDouble = org->latitude().value();
	BOOST_CHECK_EQUAL(getDouble,45.678);

	getString = org->time().value().iso();
	check = "2000-02-12T12:09:48.0000z";
	bool equal = boost::iequals(getString,check);
	BOOST_CHECK_EQUAL(equal, true);

	getDouble = org->depth().value();
	BOOST_CHECK_EQUAL(getDouble, 98.765);

	getString = org->publicID();
	check = "Origin#20150212124718.250769.756608";
	BOOST_CHECK(getString == check);

	getString = org->depthType().toString();
	check = "from location";
	BOOST_CHECK_EQUAL(check, getString);
}


BOOST_AUTO_TEST_SUITE_END()
