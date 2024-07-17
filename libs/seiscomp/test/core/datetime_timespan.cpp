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

	if ( secDiff > 0 )
		microSeconds += secDiff * 1000000;
	else if ( secDiff < 0 )
		micro += abs(secDiff) * 1000000;

	if ( abs(microSeconds - micro) <= offset )
		return true;

	return false;
}


BOOST_AUTO_TEST_SUITE(seiscomp_core_timespan)


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

BOOST_AUTO_TEST_CASE(construction) {
	bu::unit_test_log.set_threshold_level(bu::log_warnings);
	bu::unit_test_log.set_threshold_level(bu::log_messages);

	sc::Time time;
	BOOST_CHECK(time == sc::Time(0.0));

	// long
	sc::Time tPositive(200, 600);
	BOOST_CHECK(tPositive == sc::Time(200.000600));

	sc::Time tNegativeUsec(3000, -789);
	BOOST_WARN_EQUAL(tNegativeUsec.seconds(), 3000);
	BOOST_WARN_EQUAL(tNegativeUsec.microseconds(), -789);

	sc::Time tNegativeSec(-12, 12345);
	BOOST_WARN_EQUAL(tNegativeSec.seconds(), -12);
	BOOST_WARN_EQUAL(tNegativeSec.microseconds(), 12345);

	sc::Time tNegative(-15,-9876);
	BOOST_WARN_EQUAL(tNegative.seconds(), -15);
	BOOST_WARN_EQUAL(tNegative.microseconds(), -9876);

	// TimeSpan
	sc::Time tsPositive(sc::TimeSpan(5.345));
	BOOST_WARN_EQUAL(tsPositive.seconds(), 5);
	BOOST_WARN_EQUAL(tsPositive.microseconds(), 345000);

	// timeval
	timeval number;
	number.tv_sec = 150;
	number.tv_usec = 6000;
	sc::Time tvPositive(number);
	BOOST_WARN_EQUAL(tvPositive.seconds(), 150);
	BOOST_WARN_EQUAL(tvPositive.microseconds(), 6000);

	number.tv_sec = -150;
	number.tv_usec = 9000;
	sc::Time tvNegativeSec(number);
	BOOST_WARN_EQUAL(tvNegativeSec.seconds(), -150);
	BOOST_WARN_EQUAL(tvNegativeSec.microseconds(),9000);

	number.tv_sec = 4000;
	number.tv_usec = -98876;
	sc::Time tvNegativeUsec(number);
	BOOST_WARN_EQUAL(tvNegativeUsec.seconds(), 4000);
	BOOST_WARN_EQUAL(tvNegativeUsec.microseconds(), -98876);

	number.tv_sec = -9877;
	number.tv_usec = -874547;
	sc::Time tvNegative(number);
	BOOST_WARN_EQUAL(tvNegative.seconds(), -9877);
	BOOST_WARN_EQUAL(tvNegative.microseconds(), -874547);

	// double
	double val = 5678.9864;
	sc::Time tdPositive(val);
	BOOST_WARN_EQUAL(tdPositive.seconds(), 5678);
	BOOST_CHECK_EQUAL(tdPositive.microseconds(), 986400);

	val = -89765.745377;
	sc::Time tdNegative(val);
	BOOST_WARN_EQUAL(isClose(tdNegative, -89765, -745377), true);

	// pointer
	timeval pointer;
	pointer.tv_sec = 76656;
	pointer.tv_usec = 8900;

	sc::Time tpPositive(&pointer);
	BOOST_WARN_EQUAL(tpPositive.seconds(), 76656);
	BOOST_WARN_EQUAL(tpPositive.microseconds(), 8900);

	pointer.tv_sec = -76656;
	pointer.tv_usec = 8900;
	sc::Time tpNegativeSec(&pointer);
	BOOST_WARN_EQUAL(tpNegativeSec.seconds(), -76656);
	BOOST_WARN_EQUAL(tpNegativeSec.microseconds(), 8900);

	pointer.tv_sec = 98744;
	pointer.tv_usec = -8965;
	sc::Time tpNegativeUsec(&pointer);
	BOOST_WARN_EQUAL(tpNegativeUsec.seconds(), 98744);
	BOOST_WARN_EQUAL(tpNegativeUsec.microseconds(), -8965);

	pointer.tv_sec = -44;
	pointer.tv_usec = -895;
	sc::Time tpNegative(&pointer);
	BOOST_WARN_EQUAL(tpNegative.seconds(), -44);
	BOOST_WARN_EQUAL(tpNegative.microseconds(), -895);

	// copy
	sc::Time copyPositive(sc::Time(758.9975));
	BOOST_CHECK_EQUAL(copyPositive.seconds(), 758);
	BOOST_CHECK_EQUAL(copyPositive.microseconds(), 997500);

	sc::Time copyNegative(sc::Time(-877.963));
	BOOST_WARN_EQUAL(isClose(copyNegative, -877, -963000), true);

	// date
	sc::Time date(1971,1,3,1,1,4,6544);
	double dayInSeconds = 86400;
	double yearInSeconds = 31536000;
	BOOST_WARN_CLOSE(double(date), dayInSeconds*2 + yearInSeconds,0.3);
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
BOOST_AUTO_TEST_CASE(addition) {
	sc::Time a(7,5);
	sc::TimeSpan b = 9.000004;
	sc::TimeSpan result = a + b;
	BOOST_CHECK_EQUAL(result.microseconds(), 9);
	BOOST_CHECK_EQUAL(result.seconds(), 16);

	sc::Time c(7,5);
	sc::TimeSpan d = -3.000004;
	result = c + d;
	BOOST_CHECK_EQUAL(result.microseconds(), 1);
	BOOST_CHECK_EQUAL(result.seconds(), 4);

	sc::Time e(-7,5);
	sc::TimeSpan f = 9.000004;
	result = e + f;
	BOOST_CHECK_EQUAL(result.microseconds(),9);
	BOOST_CHECK_EQUAL(result.seconds(), 2);

	sc::Time g(900,789);
	sc::TimeSpan h;
	result = h += g;
	BOOST_CHECK_EQUAL(result.microseconds(),789);
	BOOST_CHECK_EQUAL(result.seconds(), 900);

	sc::Time i(455, -355);
	sc::TimeSpan j = 80.000444;
	i += j;
	BOOST_CHECK_EQUAL(i.microseconds(),89);
	BOOST_CHECK_EQUAL(i.seconds(), 535);

	sc::Time k(-899, 22255);
	sc::TimeSpan l = 773.992;
	l += k;
	BOOST_WARN_EQUAL(l.seconds(), -125);
	BOOST_WARN_EQUAL(l.microseconds(), 14255);

	sc::Time m(500, 987);
	sc::TimeSpan n(-30, 876);
	int result2 = m.microseconds() + n.microseconds();
	int result3 = m.seconds() + n.seconds();
	m += n;
	BOOST_WARN_EQUAL(m.microseconds(),result2);
	BOOST_WARN_EQUAL(m.seconds(),result3);

	sc::Time o(-60, 47);
	sc::TimeSpan p(-44,5);
	long sec = o.seconds() + p.seconds();
	long micro = o.microseconds() + p.microseconds();
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
	sc::Time a(7,5);
	sc::TimeSpan b(9,000004);
	sc::TimeSpan result = a - b;
	long sec = a.seconds() - b.seconds();
	long micro = a.microseconds() - b.microseconds();
	BOOST_CHECK_EQUAL(isClose(result, sec, micro), true);

	sc::Time c(7,5);
	sc::TimeSpan d = -3.000004;
	result = c - d;
	BOOST_CHECK_EQUAL(result.microseconds(), 9);
	BOOST_CHECK_EQUAL(result.seconds(), 10);

	sc::Time e(-7,5);
	sc::TimeSpan f(9,000004);
	result = e - f;
	sec = e.seconds() - f.seconds();
	micro = e.microseconds() -f.microseconds();
	BOOST_WARN_EQUAL(isClose(result, sec, micro),true);

	sc::Time g(900,789);
	sc::TimeSpan h;
	sec = h.seconds() - g.seconds();
	micro = h.microseconds() - g.microseconds();
	h -= g;
	BOOST_CHECK_EQUAL(isClose(h, sec, micro), true);

	sc::Time i(455, -355);
	sc::TimeSpan j(80, 444);
	sec = i.seconds() - j.seconds();
	micro = i.microseconds() - j.microseconds();
	i -= j;
	BOOST_CHECK_EQUAL(isClose(i, sec, micro), true);

	sc::Time k(-899, 22255);
	sc::TimeSpan l(773, 992);
	sec = l.seconds() - k.seconds();
	micro = l.microseconds() - k.microseconds();
	l -= k;
	BOOST_CHECK_EQUAL(isClose(l, sec, micro), true);

	sc::Time m(500,987);
	sc::TimeSpan n = -30.876;
	m -= n;
	BOOST_CHECK_EQUAL(m.microseconds(), 876987);
	BOOST_CHECK_EQUAL(m.seconds(), 530);

	sc::Time o(-60, 47);
	sc::TimeSpan p = -44.05;
	sec = o.seconds() - p.seconds();
	micro = o.microseconds() - p.microseconds();
	o -= p;
	BOOST_CHECK_EQUAL(isClose(o, sec, micro), true);

	sc::Time q(9876, -6748);
	sc::TimeSpan r = -876.987;
	sec = q.seconds() -r.seconds();
	micro = q.microseconds() - r.microseconds();
	q -= r;
	BOOST_CHECK_EQUAL(isClose(q, sec, micro), true);

	sc::Time s(50, 778), t(4, 221);
	result = s - t;
	BOOST_CHECK_EQUAL(result.microseconds(), 557);
	BOOST_CHECK_EQUAL(result.seconds(), 46);

	sc::Time u(-30,0),v(60,66);
	result = u - v;
	sec = u.seconds() -v.seconds();
	micro = u.microseconds() - v.microseconds();
	BOOST_CHECK_EQUAL(isClose(result, sec, micro), true);

	sc::Time w(798, -444),x(6, 0321);
	sec = w.seconds() - x.seconds();
	micro = w.microseconds() - x.microseconds();
	result = w - x;
	BOOST_CHECK_EQUAL(isClose(result, sec, micro), true);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(setAndGet) {
	sc::Time date;
	sc::TimeSpan oneDay (86400); // one day in seconds
	sc::TimeSpan oneYear (31536000); // one year in seconds
	sc::TimeSpan toNextYear (26524800); // seconds to the next year
	int year = 1970, month = 8, day = 5,h = 7,min = 50,sec = 33,uSec= 80;
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

	sc::Time pure;
	pure.get(&year,&month,&day,&h,&min,&sec,&uSec);
	BOOST_CHECK_EQUAL(year, 1970);

	pure -= oneYear;
	pure.get(&year,&month,&day,&h,&min,&sec,&uSec);
	BOOST_CHECK_EQUAL(year, 1969);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(localTime) {
	sc::Time local;
	local.set(1970,3,14,5,30,3,39);
	sc::Time time(local);
	BOOST_CHECK_EQUAL(double(local), double(time));
	std::string check1 = local.toString("%FT%T.%fZ");
	std::string check2 = "1970-03-14T05:30:03.000039Z";
	bool equal = boost::iequals(check1,check2);
	BOOST_CHECK_EQUAL(equal, true);
	sc::Time localtest = local.LocalTime();
	local = local.LocalTime();
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
	sc::Time time1 = local.LocalTime();
	time1.setUSecs(0);
	sc::Time time2 = before1970.LocalTime();
	time2.setUSecs(0);
	check1 = time1.toString("%FT%T.%fZ");
	check2 = time2.toString("%FT%T.%fZ");
	BOOST_CHECK_EQUAL(check1, check2);

	before1970.set(1914,9,4,7,8,66,11);
	check1 = "1914-09-04T07:09:06.000011Z";
	check2 = before1970.toString("%FT%T.%fZ");
	BOOST_CHECK_EQUAL(check1, check2);

	sc::Time lastYear(2016,8,26,15,44,9,644);
	t = lastYear.toGMT();
	check1 = lastYear.toString("%FT%T.%fZ");
	check2 = t.toString("%FT%T.%fZ");
	equal = boost::iequals(check1,check2);
	if ( equal == false ) {
		t += t.localTimeZoneOffset();
		check2 = t.toString("%FT%T.%fZ");
		equal = boost::iequals(check1,check2);
		BOOST_CHECK_EQUAL(equal,true);
	}
	else
		BOOST_CHECK_EQUAL(equal, true);

	sc::Time yearDay = yearDay.FromYearDay(1971, 3);
	double dayInSeconds = 86400;
	double yearInSeconds = 31536000;
	BOOST_CHECK_EQUAL(double(yearDay),dayInSeconds*2 + yearInSeconds);
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
BOOST_AUTO_TEST_CASE(validStrings) {
	sc::Time date(2016,8,26,15,44,9,644);
	std::string test = date.toString("%FT%T.%fZ");
	std::string check = "2016-08-26T15:44:09.000644Z";
	bool equal = boost::iequals(test,check);
	BOOST_CHECK_EQUAL(equal, true);
	BOOST_CHECK(date.FromString(test.c_str(),"%FT%T.%fZ") == date);

	BOOST_CHECK(test == date.iso());

	BOOST_CHECK(date.fromString(test.c_str(),"%FT%T.%fZ") == true);

	BOOST_CHECK_EQUAL(date.valid(), true);
}


BOOST_AUTO_TEST_SUITE_END()
