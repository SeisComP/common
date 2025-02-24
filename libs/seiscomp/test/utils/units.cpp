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
#include <stdexcept>
#include <stdio.h>

#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/utils/units.h>


using namespace Seiscomp;

#define GET_CONVERSION(fromUnit) const Util::UnitConversion *uc = Util::UnitConverter::get(fromUnit)

#define EXPECT_SI_UNIT(fUnit, tUnit, s)\
	do {\
		GET_CONVERSION(fUnit);\
		BOOST_CHECK_EQUAL(uc->scale, s);\
		BOOST_CHECK_EQUAL(uc->toUnit, tUnit);\
	}\
	while (0)


#define EXPECT_QML_UNIT(fUnit, tUnit, s)\
	do {\
		GET_CONVERSION(fUnit);\
		BOOST_CHECK_EQUAL(uc->scale, s);\
		BOOST_CHECK_EQUAL(uc->toQMLUnit, tUnit );\
	}\
	while (0)


BOOST_AUTO_TEST_SUITE(seiscomp_utils_units)


BOOST_AUTO_TEST_CASE(units) {
	EXPECT_SI_UNIT("m*s", "m*s", 1.0);
	EXPECT_QML_UNIT("m*s", "m*s", 1.0);
	EXPECT_SI_UNIT("M*S", "m*s", 1.0);
	EXPECT_QML_UNIT("M*S", "m*s", 1.0);
	EXPECT_SI_UNIT("M", "m", 1.0);
	EXPECT_QML_UNIT("M", "m", 1.0);
	EXPECT_SI_UNIT("nm", "m", 1E-9);
	EXPECT_QML_UNIT("nm", "m", 1E-9);
	EXPECT_SI_UNIT("cm", "m", 1E-2);
	EXPECT_QML_UNIT("cm", "m", 1E-2);
	EXPECT_SI_UNIT("m/s", "m/s", 1.0);
	EXPECT_QML_UNIT("m/s", "m/s", 1.0);
	EXPECT_SI_UNIT("um/s", "m/s", 1E-6);
	EXPECT_QML_UNIT("um/s", "m/s", 1E-6);
	EXPECT_SI_UNIT("m/s/s", "m/s**2", 1.0);
	EXPECT_QML_UNIT("m/s**2", "m/(s*s)", 1.0);
}


BOOST_AUTO_TEST_CASE(strings) {
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("1.23", "deg"), 1.23);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("1234 m", "km"), 1.234);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("1234m", "km"), 1.234);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("1.234 km", "m"), 1234);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("1.234km", "m"), 1234);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("2 cm", "km"), 0.00002);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("2cm", "km"), 0.00002);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("2 km", "cm"), 200000);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("2km", "cm"), 200000);

	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("111.195079734632 km", "deg"), 1);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("1 deg", "km"), 111.195079734632);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("1°", "km"), 111.195079734632);
	BOOST_CHECK_EQUAL(Util::UnitConverter::parse<double>("1°", "deg"), 1);

	BOOST_CHECK_THROW(Util::UnitConverter::parse<double>("1°", "m/s"), std::invalid_argument);
	BOOST_CHECK_THROW(Util::UnitConverter::parse<double>("1°", "ABC"), std::invalid_argument);
	BOOST_CHECK_THROW(Util::UnitConverter::parse<double>("1 m/s", "m/s**2"), std::invalid_argument);
}


BOOST_AUTO_TEST_SUITE_END()
