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
#define SEISCOMP_COMPONENT TestArchive

#include "eventxml.h"

#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/datamodel/inventory_package.h>
#include <seiscomp/datamodel/journaling.h>
#include <seiscomp/io/archive/binarchive.h>
#include <seiscomp/io/archive/jsonarchive.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/logging/output/fd.h>

#include <seiscomp/unittest/unittests.h>

#include <sstream>


using namespace std;
using namespace Seiscomp;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define ASSERT_MSG(cond, msg) do \
{ if (!(cond)) { \
	ostringstream oss; \
	oss << __FILE__ << "(" << __LINE__ << "): "<< msg << endl; cerr << oss.str(); \
	abort(); } \
} while(0)
#define ASSERT_EQUAL_MSG(obj1, obj2, msg) \
ASSERT_MSG(obj1 == obj2, msg << ": " << #obj1 " == " #obj2 \
           " has failed [" << obj1 << " != " << obj2 << "]");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct TestData {
	TestData() {
		DataModel::PublicObject::SetRegistrationEnabled(false);

		logger.setUTCEnabled(true);
		logger.subscribe(Logging::getGlobalChannel("debug"));
		logger.logContext(true);

		// Convert test event to current datamodel schema
		stringbuf xmlOrigBuf(XML_gempa2021ijvk, ios_base::in);
		IO::XMLArchive ar(&xmlOrigBuf, true);

		ar >> referenceEP;
		ar.close();

		stringbuf xmlCurrentBuf(ios_base::out);
		IO::XMLArchive archiveCurrent;
		archiveCurrent.create(&xmlCurrentBuf);
		archiveCurrent.setFormattedOutput(true);
		archiveCurrent << referenceEP;
		archiveCurrent.close();

		referenceXML = xmlCurrentBuf.str();
	}

	string referenceXML;
	DataModel::EventParametersPtr referenceEP;
	Logging::FdOutput logger;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_FIXTURE_TEST_SUITE(seiscomp_io_archive, TestData)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(json_archive) {
	ASSERT_MSG(referenceXML.length() > 1000,
	           "Reference XML string to small: " << referenceXML.length());

	// Convert EventParameters to JSON
	stringstream jsonOut;
	IO::JSONArchive jsonArchive;
	ASSERT_MSG(jsonArchive.create(&jsonOut),
	           "Could not create JSON archive for output buffer");
	jsonArchive.setFormattedOutput(true);
	jsonArchive << referenceEP;
	jsonArchive.close();

	string json = jsonOut.str();
	BOOST_CHECK_MESSAGE(json.length() > 1000, "JSON output string to small");

	// Read JSON into EventParameters
	stringbuf jsonInBuf(json, ios_base::in);
	DataModel::EventParametersPtr ep;
	ASSERT_MSG(jsonArchive.open(&jsonInBuf),
	           "Could not open JSON archive for input buffer of length: "
	           << json.length());
	jsonArchive >> ep;
	jsonArchive.close();

	ASSERT_MSG(ep, "EventParameters not initialized");
	ASSERT_EQUAL_MSG(ep->eventCount(), 1, "Invalid number of events");

	// Convert EventParameters back to XML
	stringbuf xmlBuf(ios_base::out);
	IO::XMLArchive xmlArchive(&xmlBuf, false);
	xmlArchive.setFormattedOutput(true);
	xmlArchive << ep;
	xmlArchive.close();

	string xml = xmlBuf.str();
	BOOST_CHECK_EQUAL(xml, referenceXML);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(xmlEp) {
	stringbuf xmlBuf(XML_ep, ios_base::in);
	IO::XMLArchive ar(&xmlBuf, true);

	DataModel::EventParametersPtr ep;
	DataModel::JournalingPtr ej;

	ar >> ep >> ej;
	ar.close();

	BOOST_CHECK(ep != nullptr);
	BOOST_CHECK(ej == nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(xmlMixed) {
	stringbuf xmlBuf(XML_mixed, ios_base::in);
	IO::XMLArchive ar(&xmlBuf, true);

	DataModel::EventParametersPtr ep;
	DataModel::JournalingPtr ej;

	ar >> ep >> ej;
	ar.close();

	BOOST_CHECK(ep != nullptr);
	BOOST_CHECK(ej != nullptr);

	BOOST_CHECK_EQUAL(ep->className(), DataModel::EventParameters::ClassName());
	BOOST_CHECK_EQUAL(ej->className(), DataModel::Journaling::ClassName());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(binEmptyArrays) {
	DataModel::ResponsePAZPtr polesAndZeros = DataModel::ResponsePAZ::Create("RESP");
	polesAndZeros->setPoles(DataModel::ComplexArray());
	polesAndZeros->setZeros(DataModel::ComplexArray());

	IO::VBinaryArchive ar;
	stringbuf storage(ios_base::out);
	ar.create(&storage);
	ar << polesAndZeros;
	ar.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE_END()
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
