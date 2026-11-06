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


#include <seiscomp/math/filter.h>
#include <seiscomp/unittest/unittests.h>

#include <memory>
#include <string>


using namespace Seiscomp::Math::Filtering;


namespace {


std::string createError(const std::string &filter) {
	std::string error;
	std::unique_ptr<InPlaceFilter<double>> f(InPlaceFilter<double>::Create(filter, &error));
	BOOST_CHECK(!f);
	return error;
}


}


BOOST_AUTO_TEST_SUITE(seiscomp_math_filter)


BOOST_AUTO_TEST_CASE(MissingBracket) {
	std::string error = "expected ')' after the parameters of filter 'BW'";
	BOOST_CHECK_EQUAL(createError("BW(4,1"), error);
	BOOST_CHECK_EQUAL(createError("BW("), error);
	BOOST_CHECK_EQUAL(createError("BW(4,,5)"), error);
	BOOST_CHECK_EQUAL(createError("ITAPER(10)>>BW(4,1"), error);
}


BOOST_AUTO_TEST_CASE(WrongParameterCount) {
	std::string error = "filter 'BW' takes 3 parameters";
	BOOST_CHECK_EQUAL(createError("BW"), error);
	BOOST_CHECK_EQUAL(createError("BW()"), error);
	BOOST_CHECK_EQUAL(createError("BW(4,1)"), error);
}


BOOST_AUTO_TEST_CASE(UnknownFilter) {
	BOOST_CHECK_EQUAL(createError("NOSUCH(1)"), "unknown filter 'NOSUCH'");
	BOOST_CHECK_EQUAL(createError("NOSUCH(1"), "unknown filter 'NOSUCH'");
}


BOOST_AUTO_TEST_SUITE_END()
