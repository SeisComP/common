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

#include <seiscomp/client/inventory.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/seismology/locatorinterface.h>

#include <boost/algorithm/string.hpp>

#include <filesystem>
#include <thread>

namespace fs = std::filesystem;
namespace sc = Seiscomp::Core;
namespace sio = Seiscomp::IO;
namespace sd = Seiscomp::DataModel;

bool readEventParameters(Seiscomp::DataModel::EventParameters &ep,
                         const std::string &filename) {
	Seiscomp::IO::XMLArchive ar;

	if ( !ar.open(filename.c_str()) ) {
		std::cerr << "failed to open file" << std::endl;
		return false;
	}

	ar >> NAMED_OBJECT("EventParameters", ep);
	return ar.success();
}



template<typename T>
void checkUncertainties(const T &first, const T &second) {
	OPT(double) firstLower, secondLower;
	try { firstLower = first.lowerUncertainty(); } catch ( ...) {}
	try { secondLower = second.lowerUncertainty(); } catch ( ...) {}

	BOOST_CHECK(firstLower == secondLower);

	OPT(double) firstUpper, secondUpper;
	try { firstUpper = first.upperUncertainty(); } catch ( ...) {}
	try { secondUpper = second.upperUncertainty(); } catch ( ...) {}

	BOOST_CHECK(firstUpper == secondUpper);
}

void checkRealQuantity(const sd::RealQuantity &first, const sd::RealQuantity &second, double frac) {
	BOOST_CHECK_CLOSE_FRACTION(first.value(), second.value(), frac);
	checkUncertainties(first, second);
}

void checkTimeQuantity(const sd::TimeQuantity &first, const sd::TimeQuantity &second) {
	BOOST_CHECK_EQUAL(first.value(), second.value());
	checkUncertainties(first, second);
}

template<typename T> bool cmpOptDouble(const T &first, const T &second, double frac) {
	if ( first ) {
		if ( !second ) {
			BOOST_CHECK(0);
		}

		BOOST_CHECK_CLOSE_FRACTION(*first, *second, frac);
	}
	else {
		if ( second ) {
			BOOST_CHECK(0);
		}
	}

	return true;
}

void cmpIntMember(const sd::OriginQuality &first,
                  const sd::OriginQuality &second,
                  int (sd::OriginQuality::*func)() const) {
	OPT(int) firstValue, secondValue;
	try { firstValue = (first.*func)(); } catch ( ... ) {};
	try { secondValue = (second.*func)(); } catch ( ... ) {};

	BOOST_CHECK(firstValue == secondValue);
}

void cmpDoubleMember(const sd::OriginQuality &first,
                     const sd::OriginQuality &second,
                     double epsilon,
                     double (sd::OriginQuality::*func)() const) {
	OPT(double) firstValue, secondValue;
	try { firstValue = (first.*func)(); } catch ( ... ) {};
	try { secondValue = (second.*func)(); } catch ( ... ) {};

	cmpOptDouble(firstValue, secondValue, epsilon);
}

void checkQuality(const sd::Origin* firstOrigin, const sd::Origin *secondOrigin,
                  double epsilon) {
	OPT(sd::OriginQuality) first, second;
	try { first = firstOrigin->quality();} catch ( ... ) {}
	try { second = secondOrigin->quality();} catch ( ... ) {}

	if ( !first ) {
		if ( second ) {
			BOOST_CHECK(0);
		}

		return;
	}

	if ( !second ) {
		BOOST_CHECK(0);
	}

	cmpIntMember(*first, *second, &sd::OriginQuality::associatedPhaseCount);
	cmpIntMember(*first, *second, &sd::OriginQuality::usedPhaseCount);
	cmpIntMember(*first, *second, &sd::OriginQuality::associatedStationCount);
	cmpIntMember(*first, *second, &sd::OriginQuality::usedStationCount);
	cmpIntMember(*first, *second, &sd::OriginQuality::depthPhaseCount);

	BOOST_CHECK_EQUAL(first->groundTruthLevel(), second->groundTruthLevel());

	cmpDoubleMember(*first, *second, epsilon, &sd::OriginQuality::standardError);
	cmpDoubleMember(*first, *second, epsilon, &sd::OriginQuality::azimuthalGap);
	cmpDoubleMember(*first, *second, epsilon, &sd::OriginQuality::secondaryAzimuthalGap);
	cmpDoubleMember(*first, *second, epsilon, &sd::OriginQuality::maximumDistance);
	cmpDoubleMember(*first, *second, epsilon, &sd::OriginQuality::minimumDistance);
	cmpDoubleMember(*first, *second, epsilon, &sd::OriginQuality::medianDistance);
}

using RefData = std::map<std::string, sd::OriginPtr>;

struct TestInstance {
	TestInstance() {
		if ( !Seiscomp::Client::Inventory::Instance()->inventory() ) {
			Seiscomp::Client::Inventory::Instance()->load("data/inventory.xml");
		}
		locator = Seiscomp::Seismology::LocatorInterface::Create("LOCSAT");
		locator->init(Seiscomp::Config::Config());
		locator->setProfile("iasp91");

		for ( const auto &entry : fs::directory_iterator("data/locsat-refdata") ) {
			std::string publicID = entry.path().filename().stem().string();
			if ( fs::file_size(entry.path()) == 0 ) {
				refData[publicID] = nullptr;
				continue;
			}

			sio::XMLArchive ar;
			if ( !ar.open(entry.path().c_str()) ) {
				std::cerr << "Could not open file " << entry.path().string() << std::endl;
				exit(-1);
			}


			sd::OriginPtr origin;
			ar >> origin;

			refData[publicID] = ar.success() ? origin : nullptr;
		}
	}
	Seiscomp::Seismology::LocatorInterfacePtr locator;
	RefData                                   refData;
};


struct TestInstanceIII {
	TestInstanceIII() {
		if ( !Seiscomp::Client::Inventory::Instance()->inventory() ) {
			Seiscomp::Client::Inventory::Instance()->load("data/inventory.xml");
		}
		locator = Seiscomp::Seismology::LocatorInterface::Create("LOCSAT");
		locator->init(Seiscomp::Config::Config());
		locator->setProfile("iasp91");
		locator->setIgnoreInitialLocation(true);

		for ( const auto &entry : fs::directory_iterator("data/locsat-refdata-iil") ) {
			std::string publicID = entry.path().filename().stem().string();
			if ( fs::file_size(entry.path()) == 0 ) {
				refData[publicID] = nullptr;
				continue;
			}

			sio::XMLArchive ar;
			if ( !ar.open(entry.path().c_str()) ) {
				std::cerr << "Could not open file " << entry.path().string() << std::endl;
				exit(-1);
			}


			sd::OriginPtr origin;
			ar >> origin;

			refData[publicID] = ar.success() ? origin : nullptr;
		}
	}
	Seiscomp::Seismology::LocatorInterfacePtr locator;
	RefData                                   refData;
};

//std::mutex m;

void process(const std::vector<sd::OriginPtr> &origins, int start, int end,
             const RefData &refData, Seiscomp::Seismology::LocatorInterface *locator) {
	for (int i = start; i < end; ++i) {
		auto origin = origins[i];

		auto publicID = origin->publicID();
		boost::replace_all(publicID, "/", "-");
		auto it = refData.find(publicID);

		try {
			//std::scoped_lock lock(m);
			sd::OriginPtr relocatedOrigin = locator->relocate(origin.get());
			BOOST_CHECK(relocatedOrigin);
			checkRealQuantity(it->second->latitude(), relocatedOrigin->latitude(), 0.0000001);
			checkRealQuantity(it->second->longitude(), relocatedOrigin->longitude(), 0.0000001);
			checkRealQuantity(it->second->depth(), relocatedOrigin->depth(), 0.0000001);
			checkTimeQuantity(it->second->time(), relocatedOrigin->time());
			checkQuality(it->second.get(), relocatedOrigin.get(), 0.0000001);
		}
		catch ( const std::exception &exc ) {
			BOOST_CHECK(!it->second);
		}
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_FIXTURE_TEST_SUITE(seiscomp_core_locsat, TestInstance)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(NoPicks) {
	Seiscomp::Seismology::LocatorInterface::PickList picks;
	BOOST_CHECK_THROW(locator->locate(picks), Seiscomp::Seismology::LocatorException);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// BOOST_AUTO_TEST_CASE(CreateRefData) {
// 	fs::path baseDir("/tmp/locsat-refdata");
// 	fs::create_directories(baseDir);

// 	for (const auto &entry : fs::directory_iterator("/tmp/events") ) {
// 		sd::EventParameters ep;
// 		readEventParameters(ep, entry.path());

// 		auto *origin = ep.origin(0);
// 		auto publicID = origin->publicID();
// 		boost::replace_all(publicID, "/", "-");

// 		auto filename = (baseDir / (publicID + ".xml")).string();

// 		try {
// 			auto relocatedOrigin = locator->relocate(origin);
// 			relocatedOrigin->creationInfo().setCreationTime(sc::Time(2025, 0, 1));

// 			sio::XMLArchive ar;
// 			ar.create(filename.c_str());
// 			ar << relocatedOrigin;
// 			ar.close();
// 		}
// 		catch ( const std::exception &exc ) {
// 			std::ofstream ofs(filename.c_str());
// 			ofs.close();
// 			continue;
// 		}
// 	}
// }
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(Relocate) {
	double epsilon = 0.0000001;

	for (const auto &entry : fs::directory_iterator("data/events") ) {
		sd::EventParameters ep;
		readEventParameters(ep, entry.path());

		auto origin = ep.origin(0);

		auto publicID = origin->publicID();
		boost::replace_all(publicID, "/", "-");

		auto it = refData.find(publicID);

		sd::OriginPtr relocatedOrigin;
		try {
			sd::OriginPtr relocatedOrigin = locator->relocate(origin);
			BOOST_CHECK(relocatedOrigin);
			checkRealQuantity(it->second->latitude(), relocatedOrigin->latitude(), epsilon);
			checkRealQuantity(it->second->longitude(), relocatedOrigin->longitude(), epsilon);
			checkRealQuantity(it->second->depth(), relocatedOrigin->depth(), epsilon);
			checkTimeQuantity(it->second->time(), relocatedOrigin->time());
			checkQuality(it->second.get(), relocatedOrigin.get(), epsilon);
			BOOST_CHECK_EQUAL(it->second->arrivalCount(), relocatedOrigin->arrivalCount());
			for ( size_t i = 0; i < it->second->arrivalCount(); ++i ) {
				auto arr1 = it->second->arrival(i);
				auto arr2 = relocatedOrigin->arrival(i);
				BOOST_CHECK_EQUAL(arr1->phase().code(), arr2->phase().code());
				BOOST_CHECK_CLOSE_FRACTION(arr1->distance(), arr2->distance(), epsilon);
				BOOST_CHECK_CLOSE_FRACTION(arr1->azimuth(), arr2->azimuth(), epsilon);
				BOOST_CHECK_CLOSE_FRACTION(arr1->timeResidual(), arr2->timeResidual(), epsilon);
			}
		}
		catch ( const std::exception &exc ) {
			BOOST_CHECK(!it->second);
			continue;
		}
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(RelocateMultiThreaded) {
	std::vector<sd::OriginPtr> origins;
	std::vector<sd::PickPtr> picks;
	for (const auto &entry : fs::directory_iterator("data/events") ) {
		sd::EventParameters ep;
		readEventParameters(ep, entry.path());

		origins.push_back(ep.origin(0));
		for ( size_t i = 0; i < ep.pickCount(); ++i ) {
			picks.push_back(ep.pick(i));
		}
	}

	int numThreads = 3;
	std::vector<Seiscomp::Seismology::LocatorInterfacePtr> locators;
	for ( int i = 0; i < numThreads; ++i ) {
		auto *loc = Seiscomp::Seismology::LocatorInterface::Create("LOCSAT");
		loc->init(Seiscomp::Config::Config());
		loc->setProfile("iasp91");
		locators.push_back(loc);
	}

	std::vector<std::thread> threads;
	int segmentSize = origins.size() / numThreads;

	for (int i = 0; i < numThreads; ++i) {
		int start = i * segmentSize;
		int end = (i == numThreads - 1) ? origins.size() : start + segmentSize;
		threads.emplace_back(process, origins, start, end, refData, locators[i].get());
	}

	for (auto& thread : threads) {
		thread.join();
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_FIXTURE_TEST_SUITE(seiscomp_core_locsat_iil, TestInstanceIII)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// BOOST_AUTO_TEST_CASE(CreateRefDataIII) {
// 	fs::path baseDir("/tmp/locsat-refdata-iil");
// 	fs::create_directories(baseDir);

// 	for (const auto &entry : fs::directory_iterator("/tmp/events") ) {
// 		sd::EventParameters ep;
// 		readEventParameters(ep, entry.path());

// 		auto *origin = ep.origin(0);
// 		auto publicID = origin->publicID();
// 		boost::replace_all(publicID, "/", "-");

// 		auto filename = (baseDir / (publicID + ".xml")).string();

// 		try {
// 			auto relocatedOrigin = locator->relocate(origin);
// 			relocatedOrigin->creationInfo().setCreationTime(sc::Time(2025, 0, 1));

// 			sio::XMLArchive ar;
// 			ar.create(filename.c_str());
// 			ar << relocatedOrigin;
// 			ar.close();
// 		}
// 		catch ( const std::exception &exc ) {
// 			std::ofstream ofs(filename.c_str());
// 			ofs.close();
// 			continue;
// 		}
// 	}
// }
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(Relocate) {
	double epsilon = 0.0000001;

	for (const auto &entry : fs::directory_iterator("data/events") ) {
		sd::EventParameters ep;
		readEventParameters(ep, entry.path());

		auto origin = ep.origin(0);

		auto publicID = origin->publicID();
		boost::replace_all(publicID, "/", "-");

		auto it = refData.find(publicID);

		sd::OriginPtr relocatedOrigin;
		try {
			sd::OriginPtr relocatedOrigin = locator->relocate(origin);
			BOOST_CHECK(relocatedOrigin);
			checkRealQuantity(it->second->latitude(), relocatedOrigin->latitude(), epsilon);
			checkRealQuantity(it->second->longitude(), relocatedOrigin->longitude(), epsilon);
			checkRealQuantity(it->second->depth(), relocatedOrigin->depth(), epsilon);
			checkTimeQuantity(it->second->time(), relocatedOrigin->time());
			checkQuality(it->second.get(), relocatedOrigin.get(), epsilon);
			BOOST_CHECK_EQUAL(it->second->arrivalCount(), relocatedOrigin->arrivalCount());
			for ( size_t i = 0; i < it->second->arrivalCount(); ++i ) {
				auto arr1 = it->second->arrival(i);
				auto arr2 = relocatedOrigin->arrival(i);
				BOOST_CHECK_EQUAL(arr1->phase().code(), arr2->phase().code());
				BOOST_CHECK_CLOSE_FRACTION(arr1->distance(), arr2->distance(), epsilon);
				BOOST_CHECK_CLOSE_FRACTION(arr1->azimuth(), arr2->azimuth(), epsilon);
				BOOST_CHECK_CLOSE_FRACTION(arr1->timeResidual(), arr2->timeResidual(), epsilon);
			}
		}
		catch ( const std::exception &exc ) {
			BOOST_CHECK(!it->second);
		}
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
