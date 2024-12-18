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

#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core/recordsequence.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/utils/timer.h>


using namespace std;
using namespace Seiscomp;


namespace {


/*
const Record *findRecord1(const RecordSequence &seq, const Core::Time &ts) {
	for ( auto &rec : seq ) {
		if ( rec->endTime() <= ts ) {
			continue;
		}

		return rec.get();
	}

	return nullptr;
}


const Record *findRecord2(const RecordSequence &seq, const Core::Time &ts) {
	auto it = seq.lowerBound(ts);
	return it != seq.end() ? it->get() : nullptr;
}
*/


}


BOOST_AUTO_TEST_SUITE(seiscomp_core_recordsequence)


/*
BOOST_AUTO_TEST_CASE(speedLowerBound) {
	Core::Time start(2024, 12, 18, 0, 0, 0);
	float zero = 0.0;

	RingBuffer seq(0);

	for ( int i = 0; i < 86400; ++i ) {
		GenericRecordPtr rec = new GenericRecord("XX", "ABCD", "", "XYZ", start + Core::TimeSpan(i, 0), 1);
		rec->setData(new FloatArray(1, &zero));
		seq.feed(rec.get());
	}

	Core::Time ts = start + Core::TimeSpan(20 * 3600 + 30 * 60, 0);
	const int runs = 1000;

	Util::StopWatch timer;

	for ( int i = 0; i < runs; ++i ) {
		findRecord1(seq, ts);
	}

	auto elapsed1 = timer.elapsed();
	std::cerr << "Find method 1: " << elapsed1 << std::endl;

	timer.restart();

	for ( int i = 0; i < runs; ++i ) {
		findRecord2(seq, ts);
	}

	auto elapsed2 = timer.elapsed();
	std::cerr << "Find method 2: " << elapsed2 << std::endl;

	std::cerr << "Speed up ratio: " << static_cast<double>(elapsed1) / static_cast<double>(elapsed2) << std::endl;
}
*/


BOOST_AUTO_TEST_CASE(bounds) {
	Core::Time start(2024, 12, 18, 0, 0, 0);
	float zero = 0.0;

	RingBuffer seq(0);

	for ( int i = 0; i < 86400; ++i ) {
		GenericRecordPtr rec = new GenericRecord("XX", "ABCD", "", "XYZ", start + Core::TimeSpan(i, 0), 1);
		rec->setData(new FloatArray(1, &zero));
		seq.feed(rec.get());
	}

	for ( double secs = 0.0; secs < 86400; secs += 0.5 ) {
		auto ts = start + secs;
		auto it = seq.lowerBound(ts);
		BOOST_REQUIRE(it != seq.end());
		BOOST_CHECK((*it)->startTime() <= ts);
		BOOST_CHECK((*it)->endTime() > ts);
	}

	for ( double secs = 0.0; secs < 86399; secs += 0.5 ) {
		auto ts = start + secs;
		auto it = seq.upperBound(ts);
		BOOST_REQUIRE(it != seq.end());
		BOOST_CHECK((*it)->startTime() >= ts);
	}
}


BOOST_AUTO_TEST_SUITE_END()
