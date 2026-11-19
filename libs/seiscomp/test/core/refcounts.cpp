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
#include <string>
#include <cstdio>
#include <atomic>

#include <boost/intrusive_ptr.hpp>

#include <seiscomp/core/typedarray.h>
#include <seiscomp/utils/timer.h>
#include <seiscomp/unittest/unittests.h>


using namespace std;
using namespace Seiscomp;


class X {
	public:
		using Pointer = boost::intrusive_ptr<X>;
		X() = default;

	private:
		mutable int _refcount{0};
		string data;

		friend void intrusive_ptr_add_ref(const X *x) {
			++x->_refcount;
		}

		friend void intrusive_ptr_release(const X *x) {
			if ( --x->_refcount == 0 ) {
				delete x;
			}
		}
};


class ThreadSafeX {
	public:
		using Pointer = boost::intrusive_ptr<ThreadSafeX>;
		ThreadSafeX() : _refcount(0) {}

	private:
		mutable atomic<int> _refcount;
		string data;

		friend void intrusive_ptr_add_ref(const ThreadSafeX *x) {
			x->_refcount.fetch_add(1, memory_order_relaxed);
		}

		friend void intrusive_ptr_release(const ThreadSafeX *x) {
			if ( x->_refcount.fetch_sub(1, memory_order_release) == 1 ) {
				atomic_thread_fence(memory_order_acquire);
				delete x;
			}
		}
};


BOOST_AUTO_TEST_SUITE(seiscomp_core_referencecounts)


BOOST_AUTO_TEST_CASE(measureperformance) {
	enum {
		LOOPN = 100000000
	};
	double elapsed1;
	double elapsed2;
	double elapsed3;

	{
		X::Pointer ptr1 = new X;
		X::Pointer ptr2;

		Util::StopWatch stopWatch;

		for ( size_t i = 0; i < LOOPN; ++i ) {
			ptr2 = nullptr;
			ptr2 = ptr1;
		}

		elapsed1 = stopWatch.elapsed().length();
	}

	{
		ThreadSafeX::Pointer ptr1 = new ThreadSafeX;
		ThreadSafeX::Pointer ptr2;

		Util::StopWatch stopWatch;

		for ( size_t i = 0; i < LOOPN; ++i ) {
			ptr2 = nullptr;
			ptr2 = ptr1;
		}

		elapsed2 = stopWatch.elapsed().length();
	}

	{
		Core::BaseObjectPtr ptr1 = new DoubleArray;
		Core::BaseObjectPtr ptr2;

		Util::StopWatch stopWatch;

		for ( size_t i = 0; i < LOOPN; ++i ) {
			ptr2 = nullptr;
			ptr2 = ptr1;
		}

		elapsed3 = stopWatch.elapsed().length();
	}

	cerr << "basic version: " << elapsed1 << "s\n"
	     << "thread-safe version: " << elapsed2 << "s\n"
	     << "BaseObject version: " << elapsed3 << "s\n";
}


BOOST_AUTO_TEST_SUITE_END()
