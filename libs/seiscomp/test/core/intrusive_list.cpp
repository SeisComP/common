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

#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/list.h>

#include <iostream>


namespace bu = boost::unit_test;
using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;


DEFINE_SMARTPOINTER(Dummy);
struct Dummy : BaseObject,
               Generic::IntrusiveListItem<DummyPtr> {
	Dummy(int a = 0) : a(a) {
		++Count;
	}

	~Dummy() {
		--Count;
	}

	int a;

	static int Count;
};


int Dummy::Count = 0;


BOOST_AUTO_TEST_SUITE(seiscomp_core_instrusive_list)
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(insertAndSize) {
	Generic::IntrusiveList<DummyPtr> dummies;

	int len = 100;
	DummyPtr elements[len];

	for ( int i = 0; i < len; ++i ) {
		elements[i] = new Dummy(i+1);
		dummies.push_back(elements[i]);
	}
	BOOST_CHECK_EQUAL(dummies.size(), len);
	BOOST_CHECK_EQUAL(Dummy::Count, dummies.size());

	BOOST_CHECK_EQUAL(dummies.front()->referenceCount(), 2);
	BOOST_CHECK_EQUAL(dummies.back()->referenceCount(), 2);

	int count = 1;
	for ( auto item : dummies ) {
		BOOST_CHECK_EQUAL(item->a, count++);
		BOOST_CHECK_EQUAL(item->referenceCount(), 2);
	}

	dummies.clear();
	BOOST_CHECK_EQUAL(dummies.size(), 0);
	BOOST_CHECK_EQUAL(Dummy::Count, len);

	for ( int i = 0; i < len; ++i ) {
		BOOST_CHECK_EQUAL(elements[i]->referenceCount(), 1);
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(modify) {
	Generic::IntrusiveList<DummyPtr> dummies;

	int len = 100;
	DummyPtr elements[len];

	for ( int i = 0; i < len; ++i ) {
		elements[i] = new Dummy(i+1);
		dummies.push_back(elements[i]);
	}

	auto elem = elements[len / 2].get();
	for ( int i = 0; i < len; ++i ) {
		elements[i] = nullptr;
	}

	for ( auto item : dummies ) {
		BOOST_CHECK_EQUAL(item->referenceCount(), 1);
	}

	dummies.replace(elem, new Dummy(-1));
	BOOST_CHECK_EQUAL(Dummy::Count, len);

	int count = 0;
	for ( auto item : dummies ) {
		if ( count == len / 2 ) {
			BOOST_CHECK_EQUAL(item->a, -1);
			break;
		}
		++count;
	}

	while ( !dummies.empty() ) {
		dummies.pop_front();
		BOOST_CHECK_EQUAL(Dummy::Count, dummies.size());
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(move) {
	Generic::IntrusiveList<DummyPtr> dummies;

	int len = 100;

	for ( int i = 0; i < len; ++i ) {
		dummies.push_back(new Dummy(i+1));
	}

	Generic::IntrusiveList<DummyPtr> dummies2(std::move(dummies));

	BOOST_CHECK_EQUAL(Dummy::Count, len);
	BOOST_CHECK_EQUAL(dummies.size(), 0);
	BOOST_CHECK_EQUAL(dummies2.size(), len);
	BOOST_CHECK(dummies.front() == nullptr);
	BOOST_CHECK(dummies.back() == nullptr);
	BOOST_CHECK(dummies2.front() != nullptr);
	BOOST_CHECK(dummies2.back() != nullptr);

	dummies = std::move(dummies2);

	BOOST_CHECK_EQUAL(Dummy::Count, len);
	BOOST_CHECK_EQUAL(dummies2.size(), 0);
	BOOST_CHECK_EQUAL(dummies.size(), len);
	BOOST_CHECK(dummies2.front() == nullptr);
	BOOST_CHECK(dummies2.back() == nullptr);
	BOOST_CHECK(dummies.front() != nullptr);
	BOOST_CHECK(dummies.back() != nullptr);

	for ( int i = 0; i < len; ++i ) {
		dummies2.push_back(new Dummy(len + i+1));
	}

	BOOST_CHECK_EQUAL(Dummy::Count, dummies.size() + dummies2.size());

	dummies = std::move(dummies2);

	BOOST_CHECK_EQUAL(Dummy::Count, dummies.size());
	BOOST_CHECK_EQUAL(dummies2.size(), 0);
	BOOST_CHECK_EQUAL(dummies.size(), len);
	BOOST_CHECK(dummies2.front() == nullptr);
	BOOST_CHECK(dummies2.back() == nullptr);
	BOOST_CHECK(dummies.front() != nullptr);
	BOOST_CHECK(dummies.back() != nullptr);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(constIt) {
	Generic::IntrusiveList<DummyPtr> dummies;

	int len = 100;

	for ( int i = 0; i < len; ++i ) {
		dummies.push_back(new Dummy(i+1));
	}

	// This simply checks if const iteration works
	const Generic::IntrusiveList<DummyPtr> &constDmmies = dummies;
	for ( const auto &ptr : constDmmies ) {
		BOOST_CHECK(ptr->a > 0);
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_SUITE_END()
