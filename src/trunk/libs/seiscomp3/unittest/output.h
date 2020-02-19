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


#ifndef __SEISCOMP_UNITTEST_OUTPUT_H__
#define __SEISCOMP_UNITTEST_OUTPUT_H__


#include <boost/algorithm/string.hpp>
#if BOOST_VERSION <= 106000
#include <boost/test/test_observer.hpp>
#else
#include <boost/test/tree/observer.hpp>
#include <boost/test/tree/visitor.hpp>
#include <boost/test/tree/traverse.hpp>
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
		virtual void test_start(boost::unit_test::counter_t);
		virtual void test_aborted();

		virtual void test_unit_start( boost::unit_test::test_unit const& unit );

		virtual void test_unit_finish( boost::unit_test::test_unit const&, unsigned long /* elapsed */ );

		virtual void test_unit_skipped( boost::unit_test::test_unit const& );
		virtual void test_unit_aborted( boost::unit_test::test_unit const& );

		virtual void assertion_result( bool passed );
		virtual void test_finish();

		virtual void exception_caught( boost::execution_exception const& );

		virtual int priority();


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

	bool test_suite_start( boost::unit_test::test_suite const& suite ) {return true;}

	void test_suite_finish( boost::unit_test::test_suite const& suite ) {level--;}
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
