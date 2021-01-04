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


#ifndef SEISCOMP_UNITTEST_OUTPUT_H
#define SEISCOMP_UNITTEST_OUTPUT_H


#include <boost/algorithm/string.hpp>
#if BOOST_VERSION >= 105900
#include <boost/test/tree/observer.hpp>
#include <boost/test/tree/visitor.hpp>
#include <boost/test/tree/traverse.hpp>
#else
#include <boost/test/test_observer.hpp>
#endif


namespace Seiscomp {
namespace Unittest {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** @brief Class that change the output of Boosttest files
 */
class OutputObserver : public boost::unit_test::test_observer {
	// --------------------------------------------------------------------------
	// X'truction
	// --------------------------------------------------------------------------
	public:
		OutputObserver();
		virtual ~OutputObserver();


	// ---------------------------------------------------------------------------
	// Public Interface
	// ---------------------------------------------------------------------------
	public:
#if BOOST_VERSION < 107300
		virtual void test_start(boost::unit_test::counter_t) override;
#else
		virtual void test_start(boost::unit_test::counter_t, boost::unit_test::test_unit_id) override;
#endif
		virtual void test_aborted() override;

		virtual void test_unit_start(boost::unit_test::test_unit const &unit) override;

		virtual void test_unit_finish(boost::unit_test::test_unit const &,
		                              unsigned long /* elapsed */ ) override;

		virtual void test_unit_skipped(boost::unit_test::test_unit const&) override;
		virtual void test_unit_aborted(boost::unit_test::test_unit const&) override;

#if BOOST_VERSION >= 105900
		virtual void assertion_result(boost::unit_test::assertion_result result) override;
#else
		virtual void assertion_result(bool passed) override;
#endif

		virtual void test_finish() override;

		virtual void exception_caught(boost::execution_exception const&) override;

		virtual int priority() override;


	// ---------------------------------------------------------------------------
	// private members
	// ---------------------------------------------------------------------------
	private:
		size_t countTests;
		size_t countTrue;
		size_t countFalse;
		size_t temp;
		size_t countTotal;
		size_t caseNumber;
};

/****************************************************************/

struct Visitor : boost::unit_test::test_tree_visitor {
	static size_t level;

	bool test_suite_start(boost::unit_test::test_suite const &suite) override {
		return true;
	}

	void test_suite_finish(boost::unit_test::test_suite const &suite) override {
		level--;
	}
};


/****************************************************************/

struct Output {
	Output();
	~Output();

	OutputObserver observer;
};



}
}


#endif
