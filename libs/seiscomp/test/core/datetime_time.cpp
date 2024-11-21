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
BOOST_AUTO_TEST_CASE(addition) {
	sc::TimeSpan k{5, 0}, l{7, 0};
	BOOST_CHECK(k + l == sc::TimeSpan(12, 0));

	sc::TimeSpan m = 320.5, n = 60.2;
	BOOST_CHECK(m + n == sc::TimeSpan(380.7));

	sc::TimeSpan g{55, 0}, d{-50, 0};
	BOOST_CHECK(d + g  == sc::TimeSpan(5, 0));

	sc::TimeSpan t = -80.0053, s = -70.0044;
	sc::TimeSpan result = t + s;
	long sec = t.seconds() + s.seconds();
	long micro = t.microseconds() + s.microseconds();
	BOOST_CHECK_EQUAL(isClose(result, sec,micro),true);

	sc::TimeSpan u = -5.035, v = -60.044;
	result = u + v;
	sec = u.seconds() + v.seconds();
	micro = u.microseconds() + v.microseconds();
	BOOST_CHECK_EQUAL(isClose(result,sec, micro), true);

	sc::TimeSpan w = -5.0885, x = -6.01111;
	result = w + x;
	sec = w.seconds() + x.seconds();
	micro = w.microseconds() + x.microseconds();
	BOOST_CHECK_EQUAL(isClose(result,sec, micro), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(subtraction) {
	sc::TimeSpan k{5, 0}, l{6, 0};
	BOOST_CHECK(k - l == sc::TimeSpan(-1, 0));

	sc::TimeSpan t{58, 0}, i = 68.05;
	sc::TimeSpan result = t - i;
	long sec = t.seconds() - i.seconds();
	long micro = t.microseconds() - i.microseconds();
	BOOST_CHECK_EQUAL(isClose(result,sec, micro), true);

	sc::TimeSpan e(30,4);
	sc::TimeSpan o(45,3);
	result = e - o;
	sec = e.seconds() - o.seconds();
	micro = e.microseconds() - o.microseconds();
	BOOST_CHECK_EQUAL(isClose(result,sec, micro), true);

	sc::TimeSpan f = 30.00004, g = -45.00003;
	result = f - g;
	sec = f.seconds() - g.seconds();
	micro = f.microseconds() - g.microseconds();
	BOOST_CHECK_EQUAL(isClose(result,sec, micro), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(setSec) {
	sc::TimeSpan h, k, l;
	BOOST_CHECK_EQUAL(h.set(4), sc::TimeSpan(4, 0));
	BOOST_CHECK_EQUAL(k.set(2), sc::TimeSpan(2, 0));
	BOOST_CHECK_EQUAL(l.set(1), sc::TimeSpan(1, 0));
	BOOST_CHECK_EQUAL(l.set(-10), sc::TimeSpan(-10, 0));
	BOOST_CHECK_EQUAL(k.set(-9876), sc::TimeSpan(-9876, 0));
	BOOST_CHECK_EQUAL(l.set(0), sc::TimeSpan(0.));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(setMicro) {
	sc::TimeSpan h, k, l, ts;
	BOOST_CHECK_EQUAL(h.setUSecs(9), sc::TimeSpan(0.000009));
	BOOST_CHECK_EQUAL(k.setUSecs(2), sc::TimeSpan(0.0000020));
	BOOST_CHECK_EQUAL(l.setUSecs(3), sc::TimeSpan(0.000003));
	BOOST_CHECK_EQUAL(ts.setUSecs(4), sc::TimeSpan(0.000004));
	BOOST_CHECK_EQUAL(l.setUSecs(0), sc::TimeSpan(0.0));
	BOOST_CHECK_EQUAL(h.setUSecs(2000000), sc::TimeSpan(2.00));
	BOOST_CHECK_EQUAL(k.setUSecs(-3000000), sc::TimeSpan(-3.0));

	bu::unit_test_log.set_threshold_level(bu::log_warnings);

	sc::TimeSpan test = l.setUSecs(-7262);
	BOOST_WARN_EQUAL(test.microseconds(), -7262);

	sc::TimeSpan test2 = l.setUSecs(-98744);
	BOOST_WARN_EQUAL(test2.microseconds(), -98744);

	sc::TimeSpan test3 = l.setUSecs(-98);
	BOOST_WARN_EQUAL(isClose(test3,0, -98), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(secAndMicro) {
	sc::TimeSpan ts(2.000002);
	BOOST_CHECK(ts.seconds() == 2 && ts.microseconds() == 2);

	sc::TimeSpan h(4.000009);
	BOOST_CHECK(h.seconds() == 4 && h.microseconds()  == 9);

	sc::TimeSpan t(0.000004);
	BOOST_CHECK(t.seconds() == 0 && t.microseconds()  == 4);

	sc::TimeSpan k(0.000000);
	BOOST_CHECK(k.seconds() == 0 && k.microseconds()  == 0);

	sc::TimeSpan m(-8.123456);
	long sec =  -8;
	long micro = -123456;
	BOOST_WARN_EQUAL(isClose(m,sec, micro,20), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(absolute) {
	sc::TimeSpan k(-2.567);
	sc::TimeSpan result = k.abs();
	long sec = result.seconds();
	long micro = result.microseconds();
	BOOST_CHECK_EQUAL(isClose(result,2, 567000,20), true);

	sc::TimeSpan m(-2, -5);
	sc::TimeSpan n(2, 5);
	BOOST_CHECK_EQUAL(m.abs(), n.abs());
	BOOST_CHECK_EQUAL(m.abs(), n);

	sc::TimeSpan i(600, -700000);
	result = i.abs();
	BOOST_CHECK_EQUAL(result.seconds(), 599);
	BOOST_CHECK_EQUAL(result.microseconds(),300000);

	sc::TimeSpan r(-200, -5);
	sc::TimeSpan s(200.000005);
	sc::TimeSpan absR = r.abs();
	BOOST_CHECK_EQUAL(r.abs(), s.abs());
	BOOST_CHECK_EQUAL(absR.seconds(), s.seconds());
	BOOST_CHECK_EQUAL(absR.microseconds(), s.microseconds());

	sc::TimeSpan l = (double)-1.000678;
	result = l.abs();
	sec = 1;
	micro = 678;
	BOOST_CHECK_EQUAL(isClose(result,sec, micro), true);

	sc::TimeSpan h{-4, 0};
	BOOST_CHECK_EQUAL(h.abs(), sc::TimeSpan(4, 0));

	sc::TimeSpan ts;
	BOOST_CHECK_EQUAL(ts.abs(), sc::TimeSpan(0.0));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(length, *bu::tolerance(1E-7)) {
	sc::TimeSpan k = 2.000002;
	BOOST_TEST(k.length() == 2.000002);

	sc::TimeSpan l = 1.000003;
	BOOST_TEST(l.length() == 1.000003);

	sc::TimeSpan h = 4.000009;
	BOOST_TEST(h.length() == 4.000009);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(notEqual) {
	sc::TimeSpan k = 2.000002, l = 1.000003;
	BOOST_CHECK(k != l);

	sc::TimeSpan ts, t = 0.000007;
	BOOST_CHECK(ts != t);

	sc::TimeSpan h = 4.000009, j = 2845687.000004;
	BOOST_CHECK(h != j);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(assign) {
	sc::TimeSpan k = 2.000002;
	BOOST_CHECK(k == sc::TimeSpan(2.000002));

	sc::TimeSpan t;
	BOOST_CHECK(t  == sc::TimeSpan(0.0));

	sc::TimeSpan h = 4.000009;
	BOOST_CHECK(h  == sc::TimeSpan(4.000009));

	sc::TimeSpan ts = 0.000004;
	BOOST_CHECK(ts == sc::TimeSpan(0.000004));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(plus) {
	sc::TimeSpan k = 2.000002, l = 1.000003;
	BOOST_CHECK_EQUAL(k += l, sc::TimeSpan(3.000005));

	sc::TimeSpan h = 4.000009, t;
	BOOST_CHECK_EQUAL(t += h, sc::TimeSpan(4.000009));

	sc::TimeSpan ts = 0.000004, j = 80005.000004;
	BOOST_CHECK_EQUAL(ts += j, sc::TimeSpan(80005.000008));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(minus) {
	sc::TimeSpan k = 2.000002, l = 1.000003;
	BOOST_CHECK_EQUAL(k -= l, sc::TimeSpan(0.999999));

	sc::TimeSpan t, j = 6897.098772;
	BOOST_CHECK_EQUAL(j -= t, sc::TimeSpan(6897.098772));

	sc::TimeSpan h = 4.000009, ts = 0.000004;
	BOOST_CHECK_EQUAL(h -= ts, sc::TimeSpan(4.000005));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(lower) {
	sc::TimeSpan k = 2.000002, t;
	BOOST_CHECK_EQUAL(t < k, true);

	sc::TimeSpan h = 4.000009, j = 2.897665;
	BOOST_CHECK_EQUAL(j < h, true);

	sc::TimeSpan ts = 0.000004, z = 7893648.987645;
	BOOST_CHECK_EQUAL(z < ts, false);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(greater) {
	sc::TimeSpan k = 2.000002, t;
	BOOST_CHECK_EQUAL(k > t, true);

	sc::TimeSpan h = 4.000009, j = 3.909888;
	BOOST_CHECK_EQUAL(h > j, true);

	sc::TimeSpan ts = 0.000004, g = 0.000001;
	BOOST_CHECK_EQUAL(g > ts, false);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(lowerEqual) {
	sc::TimeSpan k = 2.000002;
	BOOST_CHECK_EQUAL(k <= sc::TimeSpan(2.000002), true);

	sc::TimeSpan t;
	BOOST_CHECK_EQUAL(t <= sc::TimeSpan(2, 0), true);

	sc::TimeSpan h = 4.000009;
	BOOST_CHECK_EQUAL(h <= sc::TimeSpan(2, 0), false);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(greaterEqual) {
	sc::TimeSpan h = 4.000009;
	BOOST_CHECK_EQUAL(h >= sc::TimeSpan(2, 0), true);

	sc::TimeSpan ts = 0.000004;
	BOOST_CHECK_EQUAL(ts >= sc::TimeSpan(0.000001), true);

	sc::TimeSpan k = 2.000002;
	BOOST_CHECK_EQUAL(k >= sc::TimeSpan(2.000002), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(inDouble, *bu::tolerance(1E-7)) {
	sc::TimeSpan k = 6.000003;
	double kd = k.operator double();
	BOOST_TEST(kd == 6.000003);

	sc::TimeSpan t = 0.000008;
	double td = t.operator double();
	BOOST_TEST(td == 0.000008);

	sc::TimeSpan ts = 2.000004;
	double tsd = ts.operator double();
	BOOST_TEST(tsd == 2.000004);
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

	auto offset = utc.timeZoneOffset("CET");
	BOOST_CHECK_EQUAL(utc.toZonedString("%FT%T.%f", "CET"), (utc + offset).toString("%FT%T.%f"));

	BOOST_CHECK_THROW(utc.timeZoneOffset("XXX"), std::runtime_error);
	BOOST_CHECK_THROW(utc.toZonedString("%FT%T.%f", "XXX"), std::runtime_error);
}


BOOST_AUTO_TEST_SUITE_END()
