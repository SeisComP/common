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


#ifndef __SC_CORE_UNITTESTS_H__
#define __SC_CORE_UNITTESTS_H__


#ifdef SEISCOMP_TEST_MODULE
#define BOOST_TEST_MODULE SEISCOMP_TEST_MODULE
#else
	#error SEISCOMP_TEST_MODULE not defined
#endif

#include <boost/version.hpp>
#include <boost/test/included/unit_test.hpp>
#if BOOST_VERSION >= 107000
#include <boost/test/tools/floating_point_comparison.hpp>
#else
#include <boost/test/floating_point_comparison.hpp>
#endif

#include <seiscomp3/unittest/output.h>

// Compatibility definition for boost version < 1.35.0
#if BOOST_VERSION <= 103500
#define BOOST_TEST_MESSAGE(msg) BOOST_MESSAGE(msg)
#endif



namespace Seiscomp {
namespace Unittest {

#ifndef BOOST_INITIALIZED
#define BOOST_INITIALIZED
#if BOOST_VERSION < 105900
BOOST_GLOBAL_FIXTURE(Output)
#else
BOOST_GLOBAL_FIXTURE(Output);
#endif
#endif

}
}


#endif
