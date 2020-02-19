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

#define SEISCOMP_TEST_MODULE TestUnits

#include <iostream>
#include <stdexcept>
#include <stdio.h>

#include <seiscomp3/unittest/unittests.h>

#include <seiscomp3/core/strings.h>
#include <seiscomp3/utils/units.h>


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


BOOST_AUTO_TEST_CASE(units) {
	EXPECT_SI_UNIT("m*s", "M*S", 1.0);
	EXPECT_QML_UNIT("m*s", "m*s", 1.0);
	EXPECT_SI_UNIT("M*S", "M*S", 1.0);
	EXPECT_QML_UNIT("M*S", "m*s", 1.0);
	EXPECT_SI_UNIT("M", "M", 1.0);
	EXPECT_QML_UNIT("M", "m", 1.0);
	EXPECT_SI_UNIT("nm", "M", 1E-9);
	EXPECT_QML_UNIT("nm", "m", 1E-9);
	EXPECT_SI_UNIT("cm", "M", 1E-2);
	EXPECT_QML_UNIT("cm", "m", 1E-2);
	EXPECT_SI_UNIT("m/s", "M/S", 1.0);
	EXPECT_QML_UNIT("m/s", "m/s", 1.0);
	EXPECT_SI_UNIT("um/s", "M/S", 1E-6);
	EXPECT_QML_UNIT("um/s", "m/s", 1E-6);
	EXPECT_SI_UNIT("m/s/s", "M/S**2", 1.0);
	EXPECT_QML_UNIT("m/s**2", "m/(s*s)", 1.0);
}


