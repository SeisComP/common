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
#define SEISCOMP_COMPONENT TEST_TIME


#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/datetime.h>
#include <seiscomp/core/exceptions.h>
#include <seiscomp/core/optional.h>
#include <seiscomp/logging/log.h>


namespace sc = Seiscomp::Core;
namespace bu = boost::unit_test;


BOOST_AUTO_TEST_SUITE(seiscomp_core_time)


bool isClose(sc::TimeSpan time, long sec, long micro, int offset = 1) {
	long microSeconds = time.microseconds();

	long secDiff = time.seconds() - sec;
	if ( secDiff > 0 )
		microSeconds += secDiff * 1000000;
	else if ( secDiff < 0 )
		micro += abs(secDiff) * 1000000;

	if ( abs(microSeconds - micro) <= offset )
		return true;
	return false;
}


bool isClose(sc::Time time, long sec, long micro, int offset = 1) {
	long microSeconds = time.microseconds();

	long secDiff = time.epochSeconds() - sec;

	if ( secDiff > 0 ) {
		microSeconds += secDiff * 1000000;
	}
	else if ( secDiff < 0 ) {
		micro += abs(secDiff) * 1000000;
	}

	if ( abs(microSeconds - micro) <= offset ) {
		return true;
	}

	return false;
}


BOOST_AUTO_TEST_CASE(construction) {
	// double
	double val = 5678.9864;
	sc::Time tdPositive(val);
	BOOST_CHECK_EQUAL(tdPositive.epochSeconds(), 5678);
	BOOST_CHECK_EQUAL(tdPositive.microseconds(), 986400);

	val = -89765.745377;
	sc::Time tdNegative(val);
	BOOST_CHECK_EQUAL(isClose(tdNegative, -89765, -745377), true);

	// copy
	sc::Time copyPositive(sc::Time(758.9975));
	BOOST_CHECK_EQUAL(copyPositive.epochSeconds(), 758);
	BOOST_CHECK_EQUAL(copyPositive.microseconds(), 997500);

	sc::Time copyNegative(sc::Time(-877.963));
	BOOST_CHECK_EQUAL(isClose(copyNegative, -877, -963000), true);

	// date
	sc::Time date(1971,1,3,1,1,4,6544);
	double secondsPerDay = 86400;
	double secondsPerYear = 31536000;
	BOOST_CHECK_CLOSE(date.epochSeconds(), secondsPerYear + secondsPerDay * 2, 0.3);

	BOOST_CHECK_EQUAL(sc::Time().valid(), false);
	BOOST_CHECK_EQUAL(sc::Time(), sc::Time(1970,1,1,0,0,0));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(inDouble, *bu::tolerance(1E-7)) {
	sc::Time time = sc::Time(1970, 1, 1, 0, 0, 5, 500000);
	BOOST_TEST(time.seconds() + time.microseconds() * 1E-6 == static_cast<double>(time));

	time = sc::Time(1970, 1, 1, 0, 0, 5, 123456);
	BOOST_TEST(time.seconds() + time.microseconds() * 1E-6 == static_cast<double>(time));

	time = sc::Time(1970, 1, 1, 0, 0, 5, 7279289);
	BOOST_TEST(time.seconds() + time.microseconds() * 1E-6 == static_cast<double>(time));

	time = sc::Time(1970, 1, 1, 0, 0, 5, 7279291);
	BOOST_TEST(time.seconds() + time.microseconds() * 1E-6 == static_cast<double>(time));

	time = sc::Time(1733300000, 7279289);
	BOOST_TEST(time.seconds() + time.microseconds() * 1E-6 == static_cast<double>(time));

	time = sc::Time(1733300000, 7279291);
	BOOST_TEST(time.seconds() + time.microseconds() * 1E-6 == static_cast<double>(time));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(fromString) {
	Seiscomp::Logging::enableConsoleLogging(Seiscomp::Logging::getAll());

	sc::Time time = sc::Time::FromString("2019-01-01 10:01:59", "%F %T");

	BOOST_CHECK(time.valid());

	int year, month, day, hour, min, sec;
	BOOST_CHECK(time.get(&year, &month, &day, &hour, &min, &sec));
	BOOST_CHECK_EQUAL(year, 2019);
	BOOST_CHECK_EQUAL(month, 1);
	BOOST_CHECK_EQUAL(day, 1);
	BOOST_CHECK_EQUAL(hour, 10);
	BOOST_CHECK_EQUAL(min, 1);
	BOOST_CHECK_EQUAL(sec, 59);

	BOOST_CHECK(!time.fromString("2024-04-10T12:00:00Z", "%FT%T"));
	BOOST_CHECK(!time.fromString("2024-04-10T12:00:00", "%FT%TZ"));
	BOOST_CHECK(!time.fromString("2024-04-10 12:00:00", "%F %T abc"));

	// Buffer overflow test
	std::string str;
	for ( int i = 0; i < 500; ++i )
		str.push_back(' ');
	str.append(".123456");
	BOOST_CHECK_THROW(sc::Time::FromString(str.c_str(), "%F %T.%f"), std::runtime_error);

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00+00:00", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 12, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00+0000", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 12, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00+03:30", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 8, 30, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00+0330", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 8, 30, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00+03", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 9, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00+3", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 9, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00-03:30", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 15, 30, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00-0330", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 15, 30, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00-03", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 15, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-04-10T12:00:00-3", "%FT%T%z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 4, 10, 15, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024", "%Y"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 1, 1, 0, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("2024-07", "%Y-%m"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(2024, 7, 1, 0, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("12:34:56", "%T"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(1970, 1, 1, 12, 34, 56, 0).iso());

	BOOST_CHECK(time.fromString("12:34:56.789", "%T.%f"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(1970, 1, 1, 12, 34, 56, 789000).iso());

	BOOST_CHECK(time.fromString("1970-01-01T00:00:00Z"));
	BOOST_CHECK_EQUAL(time.iso(), sc::Time(1970, 1, 1, 0, 0, 0, 0).iso());

	BOOST_CHECK(time.fromString("201712", "%Y%m"));
	time.get(&year, &month, &day);
	BOOST_CHECK_EQUAL(year, 2017); BOOST_CHECK_EQUAL(month, 12); BOOST_CHECK_EQUAL(day, 1);
	BOOST_CHECK(time.fromString("20171207", "%Y%m%d"));
	time.get(&year, &month, &day);
	time.get(&year, &month, &day);
	BOOST_CHECK_EQUAL(year, 2017); BOOST_CHECK_EQUAL(month, 12); BOOST_CHECK_EQUAL(day, 7);
	BOOST_CHECK(time.fromString("20171207.142931.50", "%Y%m%d.%H%M%S.%f"));

	BOOST_CHECK(time.fromString("12345-04-25", "%F"));
	time.get(&year, &month, &day);
	BOOST_CHECK_EQUAL(year, 12345); BOOST_CHECK_EQUAL(month, 4); BOOST_CHECK_EQUAL(day, 25);

	BOOST_CHECK(time.fromString("123-06-26", "%F"));
	time.get(&year, &month, &day);
	BOOST_CHECK_EQUAL(year, 123); BOOST_CHECK_EQUAL(month, 6); BOOST_CHECK_EQUAL(day, 26);

	BOOST_CHECK(time.fromString("201712", "%4Y%m"));
	BOOST_CHECK(time.fromString("20171207", "%4Y%m%d"));
	BOOST_CHECK(time.fromString("20171207.142931.50", "%4Y%m%d.%H%M%S.%f"));
	BOOST_CHECK(time.fromString("201712", "%4Y%m"));

	BOOST_CHECK(time.fromString("2017-23"));
	BOOST_CHECK_EQUAL(time.iso(), "2017-01-23T00:00:00.0000Z");

	BOOST_CHECK(time.fromString("2017"));
	BOOST_CHECK_EQUAL(time.iso(), "2017-01-01T00:00:00.0000Z");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(dayOfYear) {
	Seiscomp::Logging::enableConsoleLogging(Seiscomp::Logging::getAll());

	sc::Time time = sc::Time::FromString("2019-01-01 11:23:42.1234", "%F %T.%f");

	BOOST_CHECK(time.valid());

	int year, yday, hour, min, sec, usec;
	BOOST_CHECK(time.get2(&year, &yday, &hour, &min, &sec, &usec));
	BOOST_CHECK_EQUAL(year, 2019);
	BOOST_CHECK_EQUAL(yday, 0);
	BOOST_CHECK_EQUAL(hour, 11);
	BOOST_CHECK_EQUAL(min, 23);
	BOOST_CHECK_EQUAL(sec, 42);
	BOOST_CHECK_EQUAL(usec, 123400);

	// Set new time to noon on March 1st 2020. Since 2020 is a leap year it is
	// the 60th day after January 1st.
	BOOST_CHECK(time.set2(2020, 31 + 29, 12, 3, 4, 1));
	BOOST_CHECK_EQUAL(time.toString("%F %T.%f"), "2020-03-01 12:03:04.000001");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(localTime) {
	sc::Time local;
	local.set(1970,3,14,5,30,3,39);
	sc::Time time(local);
	BOOST_CHECK_EQUAL(local.epoch(), time.epoch());
	std::string check1 = local.toString("%FT%T.%fZ");
	std::string check2 = "1970-03-14T05:30:03.000039Z";
	bool equal = boost::iequals(check1,check2);
	BOOST_CHECK_EQUAL(equal, true);
	sc::Time localtest = local.Now();
	local = local.Now();
	localtest.setUSecs(0);
	local.setUSecs(0);
	check1 = local.iso();
	check2 = localtest.iso();
	BOOST_CHECK_EQUAL(check1, check2);

	local.set(1970,3,14,5,30,3,39);
	check1 = "1970-03-14T05:30:03.000039Z";
	check2 = local.toString("%FT%T.%fZ");
	BOOST_CHECK_EQUAL(check1, check2);

	local.set(1981,9,14,5,30,3,39);
	check1 = "1981-09-14T05:30:03.000039Z";
	check2 = local.toString("%FT%T.%fZ");
	BOOST_CHECK_EQUAL(check1, check2);

	local.set(2014,3,14,5,30,3,39);
	check1 = "2014-03-14T05:30:03.000039Z";
	check2 = local.toString("%FT%T.%fZ");
	BOOST_CHECK_EQUAL(check1, check2);

	local.set(2000,8,14,5,30,3,39);
	check1 = local.toString("%FT%T.%fZ");
	check2 = "2000-08-14T05:30:03.000039Z";
	BOOST_CHECK_EQUAL(check1, check2);

	// before 1970
	sc::Time before1970;
	before1970.set(1950,6,4,15,8,66,11);
	sc::Time t(before1970);
	sc::Time time1 = local.Now();
	time1.setUSecs(0);
	sc::Time time2 = before1970.Now();
	time2.setUSecs(0);
	check1 = time1.toString("%FT%T.%fZ");
	check2 = time2.toString("%FT%T.%fZ");
	BOOST_CHECK_EQUAL(check1, check2);

	before1970.set(1914,9,4,7,8,66,11);
	check1 = "1914-09-04T07:09:06.000011Z";
	check2 = before1970.toString("%FT%T.%fZ");
	BOOST_CHECK_EQUAL(check1, check2);

	sc::Time lastYear(2016,8,26,15,44,9,644);
	t = lastYear.toUTC();
	check1 = lastYear.toString("%FT%T.%fZ");
	check2 = t.toString("%FT%T.%fZ");
	equal = check1 == check2;
	if ( !equal ) {
		t += t.localTimeZoneOffset();
		check2 = t.toString("%FT%T.%fZ");
		equal = check1 == check2;
		BOOST_CHECK_EQUAL(equal, true);
	}

	sc::Time yearDay = yearDay.FromYearDay(1971, 3);
	double secondsPerDay = 86400;
	double secondsPerYear = 31536000;
	BOOST_CHECK_EQUAL(yearDay.epochSeconds(), secondsPerYear + secondsPerDay * 2);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(yearDay) {
	sc::Time t1 = sc::Time::Now(), t2;
	int y, m, d, H, M, S, US;

	t1.get(&y, &m, &d, &H, &M, &S, &US);
	t2.set(y, m, d, H, M, S, US);

	BOOST_CHECK_EQUAL(t1, t2);

	t1.get2(&y, &d, &H, &M, &S, &US);
	t2.set2(y, d, H, M, S, US);

	BOOST_CHECK_EQUAL(t1, t2);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(individuals) {
	sc::Time t1;
	int y, m, d, H, M, S, US;

	t1 = sc::Time::FromString("2024-01-01", "%F");
	t1.get(&y, &m, &d, &H, &M, &S, &US);
	BOOST_CHECK_EQUAL(H, 0);
	BOOST_CHECK_EQUAL(M, 0);
	BOOST_CHECK_EQUAL(S, 0);
	BOOST_CHECK_EQUAL(US, 0);

	BOOST_CHECK_EQUAL(y, 2024);
	BOOST_CHECK_EQUAL(m, 1);
	BOOST_CHECK_EQUAL(d, 1);

	t1.get2(&y, &d, &H, &M, &S, &US);
	BOOST_CHECK_EQUAL(H, 0);
	BOOST_CHECK_EQUAL(M, 0);
	BOOST_CHECK_EQUAL(S, 0);
	BOOST_CHECK_EQUAL(US, 0);

	BOOST_CHECK_EQUAL(y, 2024);
	BOOST_CHECK_EQUAL(d, 0);

	t1 = sc::Time::FromString("2024-03-20", "%F");
	t1.get(&y, &m, &d, &H, &M, &S, &US);
	BOOST_CHECK_EQUAL(H, 0);
	BOOST_CHECK_EQUAL(M, 0);
	BOOST_CHECK_EQUAL(S, 0);
	BOOST_CHECK_EQUAL(US, 0);

	BOOST_CHECK_EQUAL(y, 2024);
	BOOST_CHECK_EQUAL(m, 3);
	BOOST_CHECK_EQUAL(d, 20);

	t1.get2(&y, &d, &H, &M, &S, &US);
	BOOST_CHECK_EQUAL(H, 0);
	BOOST_CHECK_EQUAL(M, 0);
	BOOST_CHECK_EQUAL(S, 0);
	BOOST_CHECK_EQUAL(US, 0);

	BOOST_CHECK_EQUAL(y, 2024);
	BOOST_CHECK_EQUAL(d, 79);

	t1 = sc::Time::FromString("2024-12-24", "%F");
	t1.get(&y, &m, &d, &H, &M, &S, &US);
	BOOST_CHECK_EQUAL(H, 0);
	BOOST_CHECK_EQUAL(M, 0);
	BOOST_CHECK_EQUAL(S, 0);
	BOOST_CHECK_EQUAL(US, 0);

	BOOST_CHECK_EQUAL(y, 2024);
	BOOST_CHECK_EQUAL(m, 12);
	BOOST_CHECK_EQUAL(d, 24);

	t1.get2(&y, &d, &H, &M, &S, &US);
	BOOST_CHECK_EQUAL(H, 0);
	BOOST_CHECK_EQUAL(M, 0);
	BOOST_CHECK_EQUAL(S, 0);
	BOOST_CHECK_EQUAL(US, 0);

	BOOST_CHECK_EQUAL(y, 2024);
	BOOST_CHECK_EQUAL(d, 358);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(validStrings) {
	sc::Time date(2016,8,26,15,44,9,644);
	std::string test = date.toString("%FT%T.%fZ");
	std::string check = "2016-08-26T15:44:09.000644Z";
	bool equal = test == check;
	BOOST_CHECK_EQUAL(equal, true);
	BOOST_CHECK(date.FromString(test.c_str(),"%FT%T.%fZ") == date);

	BOOST_CHECK(test == date.iso());

	BOOST_CHECK(date.fromString(test.c_str(),"%FT%T.%fZ") == true);

	BOOST_CHECK_EQUAL(date.valid(), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(toString) {
	// Buffer overflow test
	//sc::Time time = sc::Time::UTC();
	//time.toString("%F %T.f %F %T %F %T %F %T %F %T.f %F %T %F %T %F %T %F %T %F.f %T.f %F.f %T.f");

	sc::Time time;
	BOOST_CHECK_EQUAL(time.iso(), "1970-01-01T00:00:00.0000Z");

	BOOST_CHECK_EQUAL(sc::Time(2024, 1, 1, 3, 4, 5).iso(), "2024-01-01T03:04:05.0000Z");
	BOOST_CHECK_EQUAL(sc::Time(2019, 8, 25, 8, 51, 2, 992597).toString("%Y-%m-%dT%H:%M:%S.%f"), "2019-08-25T08:51:02.992597");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(timeZone) {
	sc::Time time;
	BOOST_CHECK(!sc::Time::LocalTimeZone().empty());
	BOOST_CHECK_NO_THROW(time.localTimeZoneOffset());

	auto utc = sc::Time::UTC();
	auto local = utc.toLocalTime();

	BOOST_CHECK_EQUAL(local - utc, local.localTimeZoneOffset());
	BOOST_CHECK_EQUAL(local - utc, utc.localTimeZoneOffset());

	BOOST_CHECK_EQUAL(local, utc.toLocalTime());
	BOOST_CHECK_EQUAL(local.toUTC(), utc);

	BOOST_CHECK_EQUAL(local.localTimeZoneOffset(), local.timeZoneOffset(sc::Time::LocalTimeZone()));

	BOOST_CHECK_EQUAL(local.toString("%FT%T.%f"), utc.toLocalString("%FT%T.%f"));

	try {
		auto offset = utc.timeZoneOffset("CET");
		BOOST_CHECK_EQUAL(utc.toZonedString("%FT%T.%f", "CET"), (utc + offset).toString("%FT%T.%f"));
	}
	catch ( ... ) {
		// Throwing an exception because of the missing time zone is fine here.
	}

	BOOST_CHECK_THROW(utc.timeZoneOffset("XXX"), std::runtime_error);
	BOOST_CHECK_THROW(utc.toZonedString("%FT%T.%f", "XXX"), std::runtime_error);

	BOOST_CHECK(!time.fromString("2026-02-19T10:00:00 ThisIsNotATimeZone", "%FT%T %Z"));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(opt) {
	OPT(sc::Time) t1, t2;

	// Both are None
	BOOST_CHECK(  t1 == t2);
	BOOST_CHECK(!(t1 != t2));
	BOOST_CHECK(!(t1 <  t2));
	BOOST_CHECK(  t1 <= t2);
	BOOST_CHECK(!(t1 >  t2));
	BOOST_CHECK(  t1 >= t2);

	// t1 is set and t2 is None
	t1 = sc::Time::Now();
	BOOST_CHECK(!(t1 == t2));
	BOOST_CHECK(  t1 != t2);
	BOOST_CHECK(!(t1 <  t2));
	BOOST_CHECK(!(t1 <= t2));
	BOOST_CHECK(  t1 >  t2);
	BOOST_CHECK(  t1 >= t2);

	// t1 is None and t2 is set
	t1 = sc::None;
	t2 = sc::Time::Now();
	BOOST_CHECK(!(t1 == t2));
	BOOST_CHECK(  t1 != t2);
	BOOST_CHECK(  t1 <  t2);
	BOOST_CHECK(  t1 <= t2);
	BOOST_CHECK(!(t1 >  t2));
	BOOST_CHECK(!(t1 >= t2));

	// Both are set and t1 < t2
	t1 = *t2 - sc::TimeSpan(1, 0);
	BOOST_CHECK(!(t1 == t2));
	BOOST_CHECK(  t1 != t2);
	BOOST_CHECK(  t1 <  t2);
	BOOST_CHECK(  t1 <= t2);
	BOOST_CHECK(!(t1 >  t2));
	BOOST_CHECK(!(t1 >= t2));
}


BOOST_AUTO_TEST_SUITE_END()
