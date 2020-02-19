/***************************************************************************
 *   Copyright (C) by gempa GmbH                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT TEST_TIME
#define SEISCOMP_TEST_MODULE test_datetime_time


#include <seiscomp3/unittest/unittests.h>

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/logging/log.h>


namespace sc = Seiscomp::Core;
namespace bu = boost::unit_test;


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

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


BOOST_AUTO_TEST_CASE(construction) {
	struct timeval tvPositive;
	tvPositive.tv_sec = 60000;
	tvPositive.tv_usec = 123456;
	sc::TimeSpan k(tvPositive);
	BOOST_CHECK(k.seconds() == tvPositive.tv_sec);
	BOOST_CHECK(k.microseconds() == tvPositive.tv_usec);

	struct timeval tvNegativeUsec;
	tvNegativeUsec.tv_sec = 300;
	tvNegativeUsec.tv_usec = -13456;
	sc::TimeSpan ki(tvNegativeUsec);
	BOOST_CHECK_EQUAL(ki.seconds() , tvNegativeUsec.tv_sec);
	BOOST_CHECK_EQUAL(ki.microseconds() , tvNegativeUsec.tv_usec);

	struct timeval tvNegativeSec;
	tvNegativeSec.tv_sec = -300;
	tvNegativeSec.tv_usec = 13456;

	sc::TimeSpan kj(tvNegativeSec);
	BOOST_CHECK_EQUAL(kj.seconds() , tvNegativeSec.tv_sec);
	BOOST_CHECK_EQUAL(kj.microseconds() , tvNegativeSec.tv_usec);

	struct timeval tvNegative;
	tvNegative.tv_sec = -3000;
	tvNegative.tv_usec = -123456;
	sc::TimeSpan kk(tvNegative);
	BOOST_CHECK_EQUAL(kk.seconds() , tvNegative.tv_sec);
	BOOST_CHECK_EQUAL(kk.microseconds() , tvNegative.tv_usec);

	struct timeval tvNull;
	sc::TimeSpan kl(tvNull);
	BOOST_CHECK_EQUAL(kl.seconds() , tvNull.tv_sec);
	BOOST_CHECK_EQUAL(kl.microseconds() , tvNull.tv_usec);

	// copy
	sc::TimeSpan copyPositive(sc::TimeSpan(79743.123456));
	BOOST_CHECK(copyPositive.seconds() == 79743);
	BOOST_CHECK(copyPositive.microseconds() == 123456);

	sc::TimeSpan copyNegative(sc::TimeSpan(-98765.123456));
	long sec = -98765;
	long micro = -123456;
	BOOST_CHECK(isClose(copyNegative, sec, micro,20) == true);

	sc::TimeSpan copyNegativeTest(sc::TimeSpan(-98765.000070));
	sec = -98765 ;
	micro = 70;
	BOOST_CHECK_EQUAL(isClose(copyNegativeTest,sec, micro,500), true);

	// long
	sc::TimeSpan longPositive(765432, 456789);
	BOOST_CHECK(longPositive.seconds() == 765432);
	BOOST_CHECK(longPositive.microseconds() == 456789);

	sc::TimeSpan longNegativeUsec(200, -732);
	BOOST_CHECK_EQUAL(longNegativeUsec.seconds(), 200);
	BOOST_CHECK_EQUAL(longNegativeUsec.microseconds(), -732);

	sc::TimeSpan longNegativeSec(-800, 73265);
	BOOST_CHECK_EQUAL(longNegativeSec.seconds(), -800);
	BOOST_CHECK_EQUAL(longNegativeSec.microseconds(), 73265);

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

	// pointer
	timeval n;
	n.tv_sec = 123;
	n.tv_usec = 123456;

	sc::TimeSpan pointerPositive(&n);
	BOOST_CHECK_EQUAL(pointerPositive.seconds(), 123);
	BOOST_CHECK_EQUAL(pointerPositive.microseconds(), 123456);

	n.tv_sec = -123;
	n.tv_usec = 123456;

	sc::TimeSpan pointerNegativeSec(&n);
	BOOST_CHECK_EQUAL(pointerNegativeSec.seconds(), -123);
	BOOST_CHECK_EQUAL(pointerNegativeSec.microseconds(), 123456);

	n.tv_sec = 123;
	n.tv_usec = -123456;

	sc::TimeSpan pointerNegativeUsec(&n);
	BOOST_CHECK_EQUAL(pointerNegativeUsec.seconds(), 123);
	BOOST_CHECK_EQUAL(pointerNegativeUsec.microseconds(), -123456);

	n.tv_sec = -123;
	n.tv_usec = -123456;

	sc::TimeSpan pointerNegative(&n);
	BOOST_CHECK_EQUAL(pointerNegative.seconds(), -123);
	BOOST_CHECK_EQUAL(pointerNegative.microseconds(), -123456);

	timeval *nullPointer = NULL;
	sc::TimeSpan pointerNull(nullPointer);
	BOOST_CHECK_EQUAL(pointerNull.seconds(), 0);
	BOOST_WARN_MESSAGE(pointerNull.microseconds(), "pointer is a NULL pointer");
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(addition) {
	sc::TimeSpan k = 5, l = 7;
	BOOST_CHECK(k + l  == sc::TimeSpan(12));

	sc::TimeSpan m = 320.5, n = 60.2;
	BOOST_CHECK(m + n  == sc::TimeSpan(380.7));

	sc::TimeSpan g = 55, d = -50;
	BOOST_CHECK(d + g  == sc::TimeSpan(5));

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
	sc::TimeSpan k = 5, l = 6;
	BOOST_CHECK(k - l == sc::TimeSpan(-1));

	sc::TimeSpan t = 58, i = 68.05;
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
	BOOST_CHECK_EQUAL(h.set(4), sc::TimeSpan(4));
	BOOST_CHECK_EQUAL(k.set(2), sc::TimeSpan(2));
	BOOST_CHECK_EQUAL(l.set(1), sc::TimeSpan(1));
	BOOST_CHECK_EQUAL(l.set(-10), sc::TimeSpan(-10));
	BOOST_CHECK_EQUAL(k.set(-9876), sc::TimeSpan(-9876));
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
	BOOST_CHECK_EQUAL(result.seconds(), 600);
	BOOST_CHECK_EQUAL(result.microseconds(),700000);

	sc::TimeSpan r(200, -5);
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

	sc::TimeSpan h = -4;
	BOOST_CHECK_EQUAL(h.abs(), sc::TimeSpan(4));

	sc::TimeSpan ts;
	BOOST_CHECK_EQUAL(ts.abs(), sc::TimeSpan(0.0));
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(length) {
	sc::TimeSpan k = 2.000002;
	BOOST_CHECK(k.length() == 2.000002);

	sc::TimeSpan l = 1.000003;
	BOOST_CHECK(l.length() == 1.000003);

	sc::TimeSpan h = 4.000009;
	BOOST_CHECK(h.length() == 4.000009);
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
	BOOST_CHECK_EQUAL(t <= sc::TimeSpan(2), true);

	sc::TimeSpan h = 4.000009;
	BOOST_CHECK_EQUAL(h <= sc::TimeSpan(2), false);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(greaterEqual) {
	sc::TimeSpan h = 4.000009;
	BOOST_CHECK_EQUAL(h >= sc::TimeSpan(2), true);

	sc::TimeSpan ts = 0.000004;
	BOOST_CHECK_EQUAL(ts >= sc::TimeSpan(0.000001), true);

	sc::TimeSpan k = 2.000002;
	BOOST_CHECK_EQUAL(k >= sc::TimeSpan(2.000002), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(inDouble) {
	sc::TimeSpan k = 6.000003;
	double kd = k.operator double();
	BOOST_CHECK_EQUAL(kd, 6.000003);

	sc::TimeSpan t = 0.000008;
	double td = t.operator double();
	BOOST_CHECK_EQUAL(td, 0.000008);

	sc::TimeSpan ts = 2.000004;
	double tsd = ts.operator double();
	BOOST_CHECK_EQUAL(tsd, 2.000004);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(toTimeval) {
	const timeval tv = sc::Time(6.000003);
	sc::TimeSpan k = 6.000003;
	timeval kv = k.operator const timeval &();
	BOOST_CHECK_EQUAL(kv.tv_sec, tv.tv_sec);
	BOOST_CHECK_EQUAL(kv.tv_usec, tv.tv_usec);

	const timeval ti = sc::Time(0.000008);
	sc::TimeSpan t = 0.000008;
	timeval tvi = t.operator const timeval &();
	BOOST_CHECK_EQUAL(tvi.tv_sec, ti.tv_sec);
	BOOST_CHECK_EQUAL(tvi.tv_usec, ti.tv_usec);

	const timeval tl = sc::Time(2.000004);
	sc::TimeSpan ts = 2.000004;
	timeval tsv = ts.operator const timeval &();
	BOOST_CHECK_EQUAL(tsv.tv_sec, tl.tv_sec);
	BOOST_CHECK_EQUAL(tsv.tv_usec, tl.tv_usec);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(exception) {
	sc::TimeSpan k;
	BOOST_CHECK_THROW(k = (double)-2147483649, sc::OverflowException);

	sc::TimeSpan l;
	BOOST_CHECK_THROW(l = (double)2147483648, sc::OverflowException);
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

	// Buffer overflow test
	std::string str;
	for ( int i = 0; i < 500; ++i )
		str.push_back(' ');
	str.append(".123456");
	time = sc::Time::FromString(str.c_str(), "%F %T.%f");
	BOOST_CHECK(!time.valid());
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(toString) {
	// Buffer overflow test
	//sc::Time time = sc::Time::GMT();
	//time.toString("%F %T.f %F %T %F %T %F %T %F %T.f %F %T %F %T %F %T %F %T %F.f %T.f %F.f %T.f");
}
