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
#include <cstdlib>


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
namespace bu = boost::unit_test;


BOOST_AUTO_TEST_SUITE(seiscomp_core_xml)
/****************************************************************/

const char *JSON2 = R"({
"code":"BHZ",
"start":"2007-11-19T00:00:00.0000Z",
"datalogger":"Datalogger#20120509134106.694537.39279",
"dataloggerSerialNumber":"xxxx",
"dataloggerChannel":0,
"sensor":"Sensor#20110428113337.619967.1975",
"sensorSerialNumber":"yyyy",
"sensorChannel":"wrong",
"sampleRateNumerator":40,
"sampleRateDenominator":1,
"depth":3,
"azimuth":0,
"dip":-90,
"gain":2506580000,
"gainFrequency":5,
"gainUnit":"M/S",
"format":"Steim2",
"flags":"G",
"restricted":false,
"shared":true
})";

const char *JSON1 = R"({
"code":"BHZ",
"start":"2007-11-19T00:00:00.0000Z",
"datalogger":"Datalogger#20120509134106.694537.39279",
"dataloggerSerialNumber":"xxxx",
"dataloggerChannel":0,
"sensor":"Sensor#20110428113337.619967.1975",
"sensorSerialNumber":"yyyy",
"sensorChannel":0,
"sampleRateNumerator":40,
"sampleRateDenominator":1,
"depth":3,
"azimuth":0,
"dip":-90,
"gain":2506580000,
"gainFrequency":5,
"gainUnit":"M/S",
"format":"Steim2",
"flags":"G",
"restricted":false,
"shared":true
})";


/****************************************************************/

const char* getContent(int a) {

	const char *XML2 = R"(<?xml version="1.0" encoding="UTF-8"?>
<seiscomp xmlns="http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.7" version="0.7">
    <EventParameters>
        <origin publicID="Origin#20150212124718.250769.756608">
            <time>
                <value>2000-02-12T12:09:48Z</value>
            </time>
            <latitude>
                <value>45.678</value>
            </latitude>
            <longitude>
                <value>-123.456</value>
            </longitude>
            <depth>
                <value>98.765</value>
            </depth>
            <depthType>from location</depthType>
            <methodID>LOCSAT</methodID>
            <earthModelID>iasp91</earthModelID>
        </origin>
    </EventParameters>
</seiscomp>
)";

	const char *XML1 = R"(<?xml version="1.0" encoding="UTF-8"?>
<seiscomp xmlns="http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.7" version="0.7">
    <EventParameters>
        <origin publicID="Origin#20150212124718.250769.756608">
            <time>
                <value>2000-02-12T12:09:48.17871Z</value>
                <uncertainty>0.5</uncertainty>
            </time>
            <latitude>
                <value>45.678</value>
            </latitude>
            <longitude>
                <value>-123.456</value>
            </longitude>
            <depth>
                <value>98.765</value>
            </depth>
            <depthType>from location</depthType>
            <methodID>LOCSAT</methodID>
            <earthModelID>iasp91</earthModelID>
        </origin>
    </EventParameters>
</seiscomp>
)";

	return a == 1 ? XML1 : XML2;
}

/****************************************************************/


class StringBuf : public std::streambuf {
	public:
		StringBuf(const char *s) {
			setbuf(const_cast<char*>(s), strlen(s));
		}

		std::streambuf *setbuf(char *s, std::streamsize n) override {
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

	t1.set(2008, 4, 12, 20, 44, 3, 567445);
	s = toString(t1);
	check = "2008-04-12T20:44:03.567445Z";
	match = boost::iequals(s, check);
	BOOST_CHECK_EQUAL(match, true);
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
	BOOST_CHECK_EQUAL(ar.success(), true);

	/************* Test JSON1 **************/
	if ( doc.HasMember("start") ) {
		string getStart = string(doc["start"].GetString());
		string check = "2007-11-19T00:00:00.0000Z";
		bool equal = boost::iequals(check, getStart);
		BOOST_CHECK_EQUAL(equal, true);
	}

	if ( doc.HasMember("code") ) {
		string getCode = doc["code"].GetString();
		string check = "BHZ";
		bool equal = boost::iequals(check, getCode);
		BOOST_CHECK_EQUAL(equal, true);
	}

	if ( doc.HasMember("datalogger") ) {
		string getDatalogger = string(doc["datalogger"].GetString());
		string check = "Datalogger#20120509134106.694537.39279";
		bool equal = boost::iequals(check, getDatalogger);
		BOOST_CHECK_EQUAL(equal, true);
	}

	if ( doc.HasMember("depth") ) {
		int getDepth = doc["depth"].GetInt();
		BOOST_CHECK_EQUAL(getDepth, 3);
	}

	if ( doc.HasMember("dip") ) {
		int getDip = doc["dip"].GetInt();
		BOOST_CHECK_EQUAL(getDip, -90);
	}

	if ( doc.HasMember("restricted") ) {
		bool getRest = doc["restricted"].GetBool();
		BOOST_CHECK_EQUAL(getRest, false);
	}

	if ( doc.HasMember("sensorChannel") ) {
		int getSenChannel = doc["sensorChannel"].GetInt();
		BOOST_CHECK_EQUAL(getSenChannel, 0);
	}

	if ( doc.HasMember("gain") ) {
		rapidjson::Value::ConstMemberIterator itr = doc.FindMember("gain");
		if ( itr->value.IsInt() ) {
			int getGain = doc["gain"].GetInt();
			BOOST_CHECK_EQUAL(getGain, 2506580000);
		}

		if ( itr->value.IsString() ) {
			string getGain = doc["gain"].GetString();
			string check = "2506580000";
			bool equal = boost::iequals(check, getGain);
			BOOST_CHECK_EQUAL(equal, true);
		}
	}
	/************* Test JSON2 **************/
	doc.Parse(JSON2);
	BOOST_CHECK_EQUAL(doc.HasParseError(), false);

	if ( doc.HasMember("start") ) {
		string getStart = string(doc["start"].GetString());
		string check = "2007-11-19T00:00:00.0000Z";
		bool equal = boost::iequals(check, getStart);
		BOOST_CHECK_EQUAL(equal, true);
	}

	if ( doc.HasMember("code") ) {
		string getCode = string(doc["code"].GetString());
		string check = "BHZ";
		bool equal = boost::iequals(check, getCode);
		BOOST_CHECK_EQUAL(equal, true);
	}

	if ( doc.HasMember("datalogger") ) {
		string getDatalogger = string(doc["datalogger"].GetString());
		string check = "Datalogger#20120509134106.694537.39279";
		bool equal = boost::iequals(check, getDatalogger);
		BOOST_CHECK_EQUAL(equal, true);
	}

	if ( doc.HasMember("depth") ) {
		int getDepth = doc["depth"].GetInt();
		BOOST_CHECK_EQUAL(getDepth, 3);
	}

	if ( doc.HasMember("dip") ) {
		int getDip = doc["dip"].GetInt();
		BOOST_CHECK_EQUAL(getDip, -90);
	}

	if ( doc.HasMember("shared") ) {
		bool getRest = doc["shared"].GetBool();
		BOOST_CHECK_EQUAL(getRest, true);
	}

	if ( doc.HasMember("sensorChannel") ) {
		rapidjson::Value::ConstMemberIterator itr = doc.FindMember("sensorChannel");
		if(itr->value.IsInt()) {
			int getSenChannel1 = doc["sensorChannel"].GetInt();
			BOOST_CHECK_MESSAGE(itr->value.IsInt(), "Value of sensorChannel is:"
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
	const auto *XML1 = getContent(1);
	StringBuf buf(XML1);
	Seiscomp::IO::XMLArchive x;
	EventParametersPtr ep;
	BaseObjectPtr obj;
	Origin *org;

	/******** test XML1 *******/
	x.open(&buf);
	x >> ep;
	x.close();

	BOOST_CHECK((bool)ep);
	bool oriCount = ep->originCount();
	BOOST_CHECK(oriCount == 1);

	org = ep->origin(0);
	string getString = org->time().value().iso();
	string check = "2000-02-12T12:09:48.17871Z";
	BOOST_CHECK_EQUAL(check, getString);
	BOOST_CHECK_EQUAL(org->time().uncertainty(), 0.5);

	getString = org->publicID();
	check = "Origin#20150212124718.250769.756608";
	BOOST_CHECK_EQUAL(getString, check);

	double getDouble = org->longitude();
	BOOST_CHECK_EQUAL(getDouble, -123.456);

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

	BOOST_CHECK((bool)ep);
	oriCount = ep->originCount();
	BOOST_CHECK(oriCount);

	org = ep->origin(0);
	getDouble = org->latitude().value();
	BOOST_CHECK_EQUAL(getDouble, 45.678);

	getString = org->time().value().iso();
	check = "2000-02-12T12:09:48.0000z";
	BOOST_CHECK_EQUAL(getString, "2000-02-12T12:09:48.0000Z");
	bool equal = boost::iequals(getString, check);
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
