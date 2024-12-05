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
#include <seiscomp/unittest/unittests.h>

#include <seiscomp/core/datetime.h>
#include <seiscomp/core/exceptions.h>


namespace sc = Seiscomp::Core;
namespace bu = boost::unit_test;


bool isClose(sc::TimeSpan time, long sec, long micro, int offset = 1) {
	long microSeconds = time.microseconds();

	long secDiff = time.seconds() - sec;

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


BOOST_AUTO_TEST_SUITE(seiscomp_core_timespan)


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

BOOST_AUTO_TEST_CASE(construction) {
	bu::unit_test_log.set_threshold_level(bu::log_warnings);
	bu::unit_test_log.set_threshold_level(bu::log_messages);

	// copy
	sc::TimeSpan copyPositive(sc::TimeSpan(79743.123456));
	BOOST_CHECK(copyPositive.seconds() == 79743);
	BOOST_CHECK(copyPositive.microseconds() == 123456);

	sc::TimeSpan copyNegative(sc::TimeSpan(-98765.123456));
	long sec = -98765;
	long micro = -123456;
	BOOST_CHECK(isClose(copyNegative, sec, micro, 20) == true);

	sc::TimeSpan copyNegativeTest(sc::TimeSpan(-98765.000070));
	sec = -98765;
	micro = -70;
	BOOST_CHECK_EQUAL(isClose(copyNegativeTest, sec, micro, 20), true);

	// long
	sc::TimeSpan longPositive(765432, 456789);
	BOOST_CHECK(longPositive.seconds() == 765432);
	BOOST_CHECK(longPositive.microseconds() == 456789);

	sc::TimeSpan longNegativeUsec(200, -732);
	BOOST_CHECK_EQUAL(longNegativeUsec.seconds(), 199);
	BOOST_CHECK_EQUAL(longNegativeUsec.microseconds(), 999268);

	sc::TimeSpan longNegativeSec(-800, 73265);
	BOOST_CHECK_EQUAL(longNegativeSec.seconds(), -799);
	BOOST_CHECK_EQUAL(longNegativeSec.microseconds(), -926735);

	sc::TimeSpan longNegative(-500, -732650);
	BOOST_CHECK_EQUAL(longNegative.seconds(), -500);
	BOOST_CHECK_EQUAL(longNegative.microseconds(), -732650);

	// double
	double number = 123456.98765;
	sc::TimeSpan doublePositive(number);
	BOOST_CHECK_EQUAL(doublePositive.seconds(), 123456);
	BOOST_CHECK_EQUAL(doublePositive.microseconds(), 987650);

	number = -98765.123470;
	sc::TimeSpan doubleNegative(number);
	BOOST_CHECK_EQUAL(doubleNegative.seconds(), -98765);
	BOOST_CHECK_CLOSE((double)doubleNegative.microseconds(), -123470, 0.01);

	number = -98765.000080;
	sc::TimeSpan doubleNegativeTest(number);
	sec = -98765;
	micro = 80;
	BOOST_CHECK_EQUAL(isClose(doubleNegativeTest,sec, micro,500), true);

	sc::TimeSpan ts;
	BOOST_CHECK(ts == sc::TimeSpan(0.0));

	// long
	sc::TimeSpan tPositive(200, 600);
	BOOST_CHECK(tPositive == sc::TimeSpan(200.000600));

	sc::TimeSpan tNegativeUsec(3000, -789);
	BOOST_CHECK_EQUAL(tNegativeUsec.seconds(), 2999);
	BOOST_CHECK_EQUAL(tNegativeUsec.microseconds(), 999211);

	sc::TimeSpan tNegativeSec(-12, 12345);
	BOOST_CHECK_EQUAL(tNegativeSec.seconds(), -11);
	BOOST_CHECK_EQUAL(tNegativeSec.microseconds(), -987655);

	sc::TimeSpan tNegative(-15, -9876);
	BOOST_CHECK_EQUAL(tNegative.seconds(), -15);
	BOOST_CHECK_EQUAL(tNegative.microseconds(), -9876);

	// TimeSpan
	sc::TimeSpan tsPositive(sc::TimeSpan(5.345));
	BOOST_CHECK_EQUAL(tsPositive.seconds(), 5);
	BOOST_CHECK_EQUAL(tsPositive.microseconds(), 345000);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(assignment) {
	sc::TimeSpan ts;
	ts = 12;
	BOOST_CHECK(ts == sc::TimeSpan(12, 0));
	ts = 0.25;
	BOOST_CHECK(ts == sc::TimeSpan(0, 250000));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(conversion) {
	sc::TimeSpan a(1.2);
	BOOST_CHECK_EQUAL(a.seconds(), 1);
	BOOST_CHECK_EQUAL(a.microseconds(), 200000);
	BOOST_CHECK_EQUAL(a.length(), 1.2);

	a = sc::TimeSpan(-1.2);
	BOOST_CHECK_EQUAL(a.seconds(), -1);
	BOOST_CHECK_EQUAL(a.microseconds(), -200000);
	BOOST_CHECK_EQUAL(a.length(), -1.2);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
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




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
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
BOOST_AUTO_TEST_CASE(addition) {
	sc::TimeSpan a(7,5);
	sc::TimeSpan b = 9.000004;
	sc::TimeSpan result = a + b;
	BOOST_CHECK_EQUAL(result.microseconds(), 9);
	BOOST_CHECK_EQUAL(result.seconds(), 16);

	sc::TimeSpan c(7,5);
	sc::TimeSpan d = -3.000004;
	result = c + d;
	BOOST_CHECK_EQUAL(result.microseconds(), 1);
	BOOST_CHECK_EQUAL(result.seconds(), 4);

	sc::TimeSpan e(-7,5);
	sc::TimeSpan f = 9.000004;
	result = e + f;
	BOOST_CHECK_EQUAL(result.microseconds(),9);
	BOOST_CHECK_EQUAL(result.seconds(), 2);

	sc::TimeSpan g(900,789);
	sc::TimeSpan h;
	result = h += g;
	BOOST_CHECK_EQUAL(result.microseconds(),789);
	BOOST_CHECK_EQUAL(result.seconds(), 900);

	sc::TimeSpan i(455, -355);
	sc::TimeSpan j = 80.000444;
	i += j;
	BOOST_CHECK_EQUAL(i.microseconds(),89);
	BOOST_CHECK_EQUAL(i.seconds(), 535);

	sc::TimeSpan k(-899, 22255);
	sc::TimeSpan l = 773.992;
	l += k;
	BOOST_CHECK_EQUAL(l.seconds(), -124);
	BOOST_CHECK_EQUAL(l.microseconds(), -985745);

	sc::TimeSpan m(500, 987);
	sc::TimeSpan n(30, 876);
	auto result2 = m.microseconds() + n.microseconds();
	auto result3 = m.seconds() + n.seconds();
	m += n;
	BOOST_CHECK_EQUAL(m.microseconds(), result2);
	BOOST_CHECK_EQUAL(m.seconds(), result3);

	sc::TimeSpan o(-60, -47);
	sc::TimeSpan p(-44, -5);
	auto sec = o.seconds() + p.seconds();
	auto micro = o.microseconds() + p.microseconds();
	o += p;
	BOOST_CHECK_EQUAL(isClose(o, sec, micro), true);

	sc::TimeSpan q(9876, -6748);
	sc::TimeSpan r = -876.987;
	q += r;
	BOOST_CHECK_EQUAL(q.microseconds(), 6252);
	BOOST_CHECK_EQUAL(q.seconds(), 8999);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(subtraction) {
	sc::TimeSpan a(7,5);
	sc::TimeSpan b(9,000004);
	sc::TimeSpan result = a - b;
	long sec = a.seconds() - b.seconds();
	long micro = a.microseconds() - b.microseconds();
	BOOST_CHECK_EQUAL(isClose(result, sec, micro), true);

	sc::TimeSpan c(7,5);
	sc::TimeSpan d = -3.000004;
	result = c - d;
	BOOST_CHECK_EQUAL(result.microseconds(), 9);
	BOOST_CHECK_EQUAL(result.seconds(), 10);

	sc::TimeSpan e(-7,5);
	sc::TimeSpan f(9,000004);
	result = e - f;
	sec = e.seconds() - f.seconds();
	micro = e.microseconds() -f.microseconds();
	BOOST_WARN_EQUAL(isClose(result, sec, micro),true);

	sc::TimeSpan g(900,789);
	sc::TimeSpan h;
	sec = h.seconds() - g.seconds();
	micro = h.microseconds() - g.microseconds();
	h -= g;
	BOOST_CHECK_EQUAL(isClose(h, sec, micro), true);

	sc::TimeSpan i(455, -355);
	sc::TimeSpan j(80, 444);
	sec = i.seconds() - j.seconds();
	micro = i.microseconds() - j.microseconds();
	i -= j;
	BOOST_CHECK_EQUAL(isClose(i, sec, micro), true);

	sc::TimeSpan k(-899, 22255);
	sc::TimeSpan l(773, 992);
	sec = l.seconds() - k.seconds();
	micro = l.microseconds() - k.microseconds();
	l -= k;
	BOOST_CHECK_EQUAL(isClose(l, sec, micro), true);

	sc::TimeSpan m(500,987);
	sc::TimeSpan n = -30.876;
	m -= n;
	BOOST_CHECK_EQUAL(m.microseconds(), 876987);
	BOOST_CHECK_EQUAL(m.seconds(), 530);

	sc::TimeSpan o(-60, 47);
	sc::TimeSpan p = -44.05;
	sec = o.seconds() - p.seconds();
	micro = o.microseconds() - p.microseconds();
	o -= p;
	BOOST_CHECK_EQUAL(isClose(o, sec, micro), true);

	sc::TimeSpan q(9876, -6748);
	sc::TimeSpan r = -876.987;
	sec = q.seconds() -r.seconds();
	micro = q.microseconds() - r.microseconds();
	q -= r;
	BOOST_CHECK_EQUAL(isClose(q, sec, micro), true);

	sc::TimeSpan s(50, 778), t(4, 221);
	result = s - t;
	BOOST_CHECK_EQUAL(result.microseconds(), 557);
	BOOST_CHECK_EQUAL(result.seconds(), 46);

	sc::TimeSpan u(-30,0),v(60,66);
	result = u - v;
	sec = u.seconds() -v.seconds();
	micro = u.microseconds() - v.microseconds();
	BOOST_CHECK_EQUAL(isClose(result, sec, micro), true);

	sc::TimeSpan w(798, -444),x(6, 0321);
	sec = w.seconds() - x.seconds();
	micro = w.microseconds() - x.microseconds();
	result = w - x;
	BOOST_CHECK_EQUAL(isClose(result, sec, micro), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(exception) {
	sc::TimeSpan k;
	BOOST_CHECK_THROW(k = sc::Time::MinTime-1, sc::OverflowException);

	sc::TimeSpan l;
	BOOST_CHECK_THROW(l = sc::Time::MaxTime+1, sc::OverflowException);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(setAndGet) {
	sc::Time date;
	sc::TimeSpan oneDay (86400, 0); // one day in seconds
	sc::TimeSpan oneYear (31536000, 0); // one year in seconds
	sc::TimeSpan toNextYear (26524800, 0); // seconds to the next year
	int year = 1970, month = 8, day = 5,h  = 7, min = 50, sec = 33, uSec= 80;
	date.set(year,month,day,h,min,sec,uSec);
	BOOST_CHECK(date.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK(year == 1970);
	BOOST_CHECK(month == 8);
	BOOST_CHECK(day == 5);
	BOOST_CHECK(h == 7);
	BOOST_CHECK(min = 50);
	BOOST_CHECK(sec = 33);
	BOOST_CHECK(uSec = 80);

	date -= oneYear;
	BOOST_CHECK(date.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK(year == 1969);
	BOOST_CHECK(month == 8);
	BOOST_CHECK_EQUAL(day , 5);

	year = 2017, month = 2, day = 28;
	date.set(year,month,day,h,min,sec,uSec);
	BOOST_CHECK(date.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK(year == 2017);
	BOOST_CHECK(month == 2);
	BOOST_CHECK(day == 28);

	date += oneDay;
	BOOST_CHECK(date.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK_EQUAL(month , 3);
	BOOST_CHECK_EQUAL(day , 1);

	year = 2018, month = 2, day = 28;
	date.set(year,month,day,h,min,sec,uSec);
	date += oneDay;
	BOOST_CHECK(date.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK_EQUAL(month , 3);
	BOOST_CHECK_EQUAL(day, 1);

	date += oneYear;
	BOOST_CHECK(date.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK_EQUAL(year , 2019);
	BOOST_CHECK_EQUAL(day, 1);
	BOOST_CHECK_EQUAL(month , 3);

	sc::Time leapYear;
	year = 1956, month = 2, day = 28;
	leapYear.set(year,month,day,h,min,sec,uSec);
	BOOST_CHECK(leapYear.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK(year == 1956);
	BOOST_CHECK(month == 2);
	BOOST_CHECK(day == 28);

	leapYear += oneDay;
	BOOST_CHECK(leapYear.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK(month == 2);
	BOOST_CHECK(day == 29);

	leapYear += oneDay;
	BOOST_CHECK(leapYear.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK(month == 3);
	BOOST_CHECK(day == 1);

	sc::Time time;
	year = 2011, month = 2, day = 28;
	int yday ;
	time.set(year,month,day,h,min,sec,uSec);
	BOOST_CHECK(time.get2(&year,&yday,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK(year == 2011);
	BOOST_CHECK_EQUAL(yday , 58);

	time += toNextYear;
	BOOST_CHECK(time.get2(&year,&yday,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK_EQUAL(year , 2012);
	BOOST_CHECK_EQUAL(yday , 0);

	year = 1964, month = 2, day = 29;
	leapYear.set(year,month,day,h,min,sec,uSec);
	BOOST_CHECK(leapYear.get2(&year,&yday,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK_EQUAL(yday , 59);

	leapYear += toNextYear;
	BOOST_CHECK(leapYear.get2(&year,&yday,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK_EQUAL(year, 1965);
	BOOST_CHECK_EQUAL(yday , 0);

	/* Currently not compatible with 32bit systems
	sc::Time before1900;
	day = 28, month = 2, year = 1900;
	before1900.set(year,month,day,h,min,sec,uSec);
	BOOST_CHECK(before1900.get(&year,&month,&day,&h,&min,&sec,&uSec) == true);
	BOOST_CHECK_EQUAL(year , 1900);
	BOOST_CHECK_EQUAL(day , 28);
	BOOST_CHECK_EQUAL(month, 2);
	*/

	sc::Time pure{0, 0};
	pure.get(&year, &month, &day, &h, &min, &sec, &uSec);
	BOOST_CHECK_EQUAL(year, 1970);

	pure -= oneYear;
	pure.get(&year, &month, &day, &h, &min, &sec, &uSec);
	BOOST_CHECK_EQUAL(year, 1969);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(suffix) {
	using namespace sc::Literals;

	auto ts = 5_minutes;
	BOOST_CHECK_EQUAL(ts, sc::TimeSpan(5 * 60, 0));
	ts = 30_seconds;
	BOOST_CHECK_EQUAL(ts, sc::TimeSpan(30, 0));
	ts = 500_milliseconds;
	BOOST_CHECK_EQUAL(ts, sc::TimeSpan(0, 500000));
	ts = 2_hours;
	BOOST_CHECK_EQUAL(ts, sc::TimeSpan(60 * 60 * 2, 0));
	ts = 3_days;
	BOOST_CHECK_EQUAL(ts, sc::TimeSpan(24 * 60 * 60 * 3, 0));
	ts = 5_weeks;
	BOOST_CHECK_EQUAL(ts, sc::TimeSpan(24 * 60 * 60 * 7 * 5, 0));
}


BOOST_AUTO_TEST_SUITE_END()
