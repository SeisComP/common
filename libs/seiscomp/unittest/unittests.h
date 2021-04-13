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


#ifndef SEISCOMP_CORE_UNITTESTS_H
#define SEISCOMP_CORE_UNITTESTS_H


#ifdef SEISCOMP_TEST_MODULE
#define BOOST_TEST_MODULE SEISCOMP_TEST_MODULE
#else
	#error SEISCOMP_TEST_MODULE not defined
#endif

#include <boost/version.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/test/included/unit_test.hpp>
#if BOOST_VERSION >= 107000
#include <boost/test/tools/floating_point_comparison.hpp>
#else
#include <boost/test/floating_point_comparison.hpp>
#endif

// Compatibility definition for boost version < 1.35.0
#if BOOST_VERSION <= 103500
#define BOOST_TEST_MESSAGE(msg) BOOST_MESSAGE(msg)
#endif


#endif
