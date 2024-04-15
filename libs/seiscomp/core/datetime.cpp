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


#define SEISCOMP_COMPONENT Core

#include <seiscomp/logging/log.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/exceptions.h>

#include <sstream>
#include <cmath>
#include <ctype.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>


#ifdef WIN32
#include <time.h>
#endif


using namespace Seiscomp::Core;


const double TimeSpan::MinTime = -(double)0x80000000;
const double TimeSpan::MaxTime =  (double)0x7fffffff;


/* We are linking against the multithreaded versions
   of the Microsoft runtimes - this makes gmtime
   equiv to gmtime_r in that Windows gmtime is threadsafe
*/
#if defined (WIN32)
static struct tm* gmtime_r(const time_t *timep, struct tm* result)
{
        struct tm *local;

        local = gmtime(timep);
        memcpy(result,local,sizeof(struct tm));
        return result;
}
#endif


#if defined(WIN32)

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif


struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if (nullptr != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (nullptr != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }

  return 0;
}
#endif


#if defined(WIN32)
extern "C" {
#include <seiscomp/core/strptime.h>
}
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define MICROS 1000000

namespace {

#ifdef __sun__
#define NO_COMPACT_DATE
#endif


#if defined(__SUNPRO_CC) || defined(__sun__) || defined(WIN32)
time_t timegm(struct tm *t) {
	time_t tl, tb;
	struct tm tg;

	t->tm_isdst = 0;

	tl = mktime (t);
	if (tl == -1) {
		t->tm_hour--;
		tl = mktime (t);
		if (tl == -1)
			return -1; /* can't deal with output from strptime */
		tl += 3600;
	}

	gmtime_r(&tl, &tg);
	tg.tm_isdst = 0;
	tb = mktime (tg);
	if (tb == -1) {
		--tg.tm_hour;
		tb = mktime(&tg);
		if (tb == -1)
			return -1; /* can't deal with output from gmtime */
		tb += 3600;
	}

	return (tl - (tb - tl));
}
#endif


template <typename T, typename U>
inline void normalize(T &sec, U &usec) {
	if ( usec < 0 ) {
		if ( sec > 0 || usec <= -MICROS ) {
			usec += MICROS;
			sec -= 1;
		}
	}
	else if ( usec > 0 ) {
		if ( sec < 0 || usec >= MICROS ) {
			usec -= MICROS;
			sec += 1;
		}
	}
}


const char *timeFormats[] = {
	"%FT%T.%fZ",    // YYYY-MM-DDThh:mm:ss.ssssssZ
	"%FT%T.%f",     // YYYY-MM-DDThh:mm:ss.ssssss
	"%FT%TZ",       // YYYY-MM-DDThh:mm:ssZ
	"%FT%T",        // YYYY-MM-DDThh:mm:ss
	"%FT%R",        // YYYY-MM-DDThh:mm
	"%FT%H",        // YYYY-MM-DDThh
	"%Y-%jT%T.%f",  // YYYY-DDDThh:mm:ss.ssssss
	"%Y-%jT%T",     // YYYY-DDDThh:mm:ss
	"%Y-%jT%R",     // YYYY-DDDThh:mm
	"%Y-%jT%H",     // YYYY-DDDThh
	"%F %T.%f",     // YYYY-MM-DD hh:mm:ss.ssssss
	"%F %T",        // YYYY-MM-DD hh:mm:ss
	"%F %R",        // YYYY-MM-DD hh:mm
	"%F %H",        // YYYY-MM-DD hh
	"%F",           // YYYY-MM-DD
	"%Y-%j",        // YYYY-DDD
	"%Y",           // YYYY
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan() {
	_timeval.tv_sec = 0;
	_timeval.tv_usec = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(struct timeval* t) {
	if ( t != nullptr ) {
		_timeval.tv_sec = t->tv_sec;
		_timeval.tv_usec = t->tv_usec;
	}
	else {
		_timeval.tv_sec = 0;
		_timeval.tv_usec = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(const struct timeval& t) {
	_timeval.tv_sec = t.tv_sec;
	_timeval.tv_usec = t.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(double t) {
	*this = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(long secs, long usecs) {
	_timeval.tv_sec = secs + (usecs / MICROS);
	_timeval.tv_usec = usecs % MICROS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::TimeSpan(const TimeSpan& ts) {
	_timeval.tv_sec = ts._timeval.tv_sec;
	_timeval.tv_usec = ts._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator==(const TimeSpan& t) const {
	return _timeval.tv_sec == t._timeval.tv_sec &&
	       _timeval.tv_usec == t._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator!=(const TimeSpan& t) const {
	return !(*this == t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator< (const TimeSpan& t) const {
	if ( _timeval.tv_sec > t._timeval.tv_sec )
		return false;
	if ( _timeval.tv_sec < t._timeval.tv_sec )
		return true;
	return _timeval.tv_usec < t._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator<=(const TimeSpan& t) const {
	if ( _timeval.tv_sec > t._timeval.tv_sec )
		return false;
	if ( _timeval.tv_sec < t._timeval.tv_sec )
		return true;
	return _timeval.tv_usec <= t._timeval.tv_usec;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator> (const TimeSpan& t) const {
	if ( _timeval.tv_sec < t._timeval.tv_sec )
		return false;
	if ( _timeval.tv_sec > t._timeval.tv_sec )
		return true;
	return _timeval.tv_usec > t._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::operator>=(const TimeSpan& t) const {
	if ( _timeval.tv_sec < t._timeval.tv_sec )
		return false;
	if ( _timeval.tv_sec > t._timeval.tv_sec )
		return true;
	return _timeval.tv_usec >= t._timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::operator double() const {
	return (double)_timeval.tv_sec +
	       (double)_timeval.tv_usec * 0.000001;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan::operator const timeval&() const {
	return _timeval;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::operator=(long t) {
	_timeval.tv_sec = t;
	_timeval.tv_usec = 0;

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::operator=(double t) {
	if( t > MaxTime || t < MinTime )
		throw Core::OverflowException("TimeSpan::operator=(): double doesn't fit into int");
	_timeval.tv_sec = (long)t;
	_timeval.tv_usec = (long)((t-_timeval.tv_sec)*MICROS + 0.5);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::operator=(const TimeSpan& t) {
	_timeval = t._timeval;

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan TimeSpan::operator+(const TimeSpan& t) const {
	long diff_usec = _timeval.tv_usec + t._timeval.tv_usec;
	long int sec = _timeval.tv_sec + t._timeval.tv_sec;

	normalize(sec, diff_usec);

	return TimeSpan(sec, diff_usec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan TimeSpan::operator-(const TimeSpan& t) const {
	long diff_usec = _timeval.tv_usec - t._timeval.tv_usec;
	long int sec = _timeval.tv_sec - t._timeval.tv_sec;

	normalize(sec, diff_usec);

	return TimeSpan(sec, diff_usec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::operator+=(const TimeSpan& t) {
	_timeval.tv_sec += t._timeval.tv_sec;
	_timeval.tv_usec += t._timeval.tv_usec;

	normalize(_timeval.tv_sec, _timeval.tv_usec);

	return *this;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
TimeSpan& TimeSpan::operator-=(const TimeSpan& t) {
	_timeval.tv_usec -= t._timeval.tv_usec;
	_timeval.tv_sec -= t._timeval.tv_sec;

	normalize(_timeval.tv_sec, _timeval.tv_usec);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::set(long secs) {
	_timeval.tv_sec = secs;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan& TimeSpan::setUSecs(long usecs) {
	_timeval.tv_usec = usecs % MICROS;
	_timeval.tv_sec += usecs / MICROS;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<
void TimeSpan::elapsedTime(int* days, int* hours,
                           int* minutes, int* seconds) const
{
	int elapsed = TimeSpan::seconds();
	if (days)
		*days = elapsed / 86400;
	if (hours)
		*hours = (elapsed % 86400) / 3600;
	if (minutes)
		*minutes = ((elapsed % 86400) % 3600) / 60;
	if (seconds)
		*seconds = ((elapsed % 86400) % 3600) % 60;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan TimeSpan::abs() const {
	return TimeSpan(::abs(_timeval.tv_sec), ::abs(_timeval.tv_usec));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double TimeSpan::length() const {
	return double(*this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
long TimeSpan::seconds() const {
	return _timeval.tv_sec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
long TimeSpan::microseconds() const {
	return _timeval.tv_usec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Time Time::Null(0.0);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time() : TimeSpan() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(const TimeSpan& ts)
 : TimeSpan(ts) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(const struct timeval& t)
 : TimeSpan(t) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(struct timeval* t)
 : TimeSpan(t) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(double t) {
	*this = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(int year, int month, int day,
           int hour, int min, int sec,
           int usec) {
	set(year, month, day, hour, min, sec, usec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(const Time& t)
 : TimeSpan(t) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(long secs, long usecs)
 : TimeSpan(secs, usecs) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::operator bool() const {
	return valid();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::operator time_t() const {
	return (time_t)_timeval.tv_sec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator=(const struct timeval& t) {
	_timeval = t;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator=(struct timeval* t) {
	_timeval = *t;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator=(time_t t) {
	_timeval.tv_sec = (long)t;
	_timeval.tv_usec = 0;

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator=(double t) {
	if( t > MaxTime || t < MinTime )
		throw Core::OverflowException("Time::operator=(): double doesn't fit into int");
	_timeval.tv_sec = (long)t;
	_timeval.tv_usec = (long)((t-(double)_timeval.tv_sec)*MICROS + 0.5);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::operator+(const TimeSpan& t) const {
	return Time((TimeSpan&)*this + t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::operator-(const TimeSpan& ts) const {
	return Time(TimeSpan::operator- (ts));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator+=(const TimeSpan& ts) {
	TimeSpan::operator+=(ts);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::operator-=(const TimeSpan& ts) {
	TimeSpan::operator-=(ts);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan Time::operator-(const Time& ts) const {
	return TimeSpan::operator-(ts);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::set(int year, int month, int day,
                int hour, int min, int sec,
                int usec) {
	tm t;

	t.tm_year = year - 1900;
	t.tm_mon = month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = min;
	t.tm_sec = sec;
	t.tm_isdst = -1;

	_timeval.tv_sec = (long)timegm(&t);
	setUSecs(usec);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::set2(int year, int yday,
                 int hour, int min, int sec,
                 int usec) {
	tm t;

	// Like mktime, timegm will not read tm_yday nor tm_wday when contructing a
	// time_t object. However, it does normalize other tm values. As a work
	// arround we convert yday to the yday+1th January. E.g.,
	// yday 32 = January 33th = February 2nd.
	t.tm_year = year - 1900;
	t.tm_mon = 0; // January
	t.tm_mday = yday + 1;
	t.tm_hour = hour;
	t.tm_min = min;
	t.tm_sec = sec;
	t.tm_isdst = -1;

	_timeval.tv_sec = (long)timegm(&t);
	setUSecs(usec);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::get(int *year, int *month, int *day,
               int *hour, int *min, int *sec,
               int *usec) const {
	time_t time = (time_t)_timeval.tv_sec;
	struct tm t;
	gmtime_r(&time, &t);

	if ( year )  *year = t.tm_year + 1900;
	if ( month ) *month = t.tm_mon + 1;
	if ( day )   *day = t.tm_mday;

	if ( hour )  *hour = t.tm_hour;
	if ( min )   *min = t.tm_min;
	if ( sec )   *sec = t.tm_sec;

	if ( usec )  *usec = _timeval.tv_usec;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::get2(int *year, int *yday, int *hour, int *min, int *sec,
                int *usec) const {
	time_t time = (time_t)_timeval.tv_sec;
	struct tm t;
	gmtime_r(&time, &t);

	if ( year )  *year = t.tm_year + 1900;
	if ( yday )  *yday = t.tm_yday;

	if ( hour )  *hour = t.tm_hour;
	if ( min )   *min = t.tm_min;
	if ( sec )   *sec = t.tm_sec;

	if ( usec )  *usec = _timeval.tv_usec;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::LocalTime() {
	Time t;
	t.localtime();
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::LocalTimeZone() {
	time_t t;
	struct tm *tm_;
	char tz[40];
	::time(&t);
	tm_ = ::localtime(&t);
	strftime(tz, sizeof(tz)-1, "%Z", tm_);
	tz[sizeof(tz)-1] = '\0';
	return tz;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::UTC() {
	Time t;
	t.utc();
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::GMT() {
	Time t;
	t.gmt();
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromYearDay(int year, int year_day) {
	std::stringstream ss;
	ss << year << " " << year_day;
	return FromString(ss.str().c_str(), "%Y %j");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan Time::localTimeZoneOffset() const {
	return *this - toUTC();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::localtime() {
	gettimeofday(&_timeval, nullptr);
	time_t secs = (time_t)_timeval.tv_sec;
	struct tm _tm;
	_timeval.tv_sec = (long)timegm(::localtime_r(&secs, &_tm));

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::utc() {
	gettimeofday(&_timeval, nullptr);
	time_t secs = (time_t)_timeval.tv_sec;
	struct tm _tm;
	_timeval.tv_sec = (long)mktime(::localtime_r(&secs, &_tm));

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time& Time::gmt() {
	gettimeofday(&_timeval, nullptr);
	time_t secs = (time_t)_timeval.tv_sec;
	struct tm _tm;
	_timeval.tv_sec = (long)mktime(::localtime_r(&secs, &_tm));

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::toLocalTime() const {
	Time ret;
	time_t secs = _timeval.tv_sec;
	struct tm _tm;
	ret._timeval.tv_sec = (long)timegm(::localtime_r(&secs, &_tm));
	ret._timeval.tv_usec = _timeval.tv_usec;

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::toUTC() const {
	Time ret;
	time_t secs = _timeval.tv_sec;
	struct tm _tm;
	ret._timeval.tv_sec = _timeval.tv_sec - ((long)timegm(::localtime_r(&secs, &_tm)) - _timeval.tv_sec);
	ret._timeval.tv_usec = _timeval.tv_usec;

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::toGMT() const {
	Time ret;
	time_t secs = _timeval.tv_sec;
	struct tm _tm;
	ret._timeval.tv_sec = _timeval.tv_sec - ((long)timegm(::localtime_r(&secs, &_tm)) - _timeval.tv_sec);
	ret._timeval.tv_usec = _timeval.tv_usec;

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::valid() const {
	return _timeval.tv_sec != 0 || _timeval.tv_usec != 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::toString(const char* fmt) const {
#define BUFFER_SIZE 64
	char data[BUFFER_SIZE];
	char predata[BUFFER_SIZE];

	time_t secs = (time_t)_timeval.tv_sec, usecs = _timeval.tv_usec;
	while ( usecs < 0 ) {
		secs -= 1;
		usecs += MICROS;
	}

	tm t;
	gmtime_r(&secs, &t);
	const char *f = fmt, *last = fmt;
	char *tgt = predata;

	while ( (f = strchr(f, '%')) != nullptr ) {
		int specSize = 3;

		char spec = *(f+1);
		if ( spec == '\0' ) break;
		char type = *(f+2);

		if ( (spec >= 'a' && spec <= 'z') || (spec >= 'A' && spec <= 'Z') ) {
			specSize = 2;
			type = spec;
		}

		if ( type == 'f' ) {
			int width = -1;
			if ( spec >= '0' && spec <= '6' )
				width = spec - '0';

			memcpy(tgt, last, f-last);
			tgt += f-last;

			char number[32];
			size_t numberOfDigits;
			if ( usecs > 0 ) {
				numberOfDigits = sprintf(number, "%06ld", usecs);
				if ( width != -1 )
					numberOfDigits = width;
				else {
					while ( number[numberOfDigits-1] == '0' ) --numberOfDigits;
				}
			}
			else {
				if ( width == -1 )
					numberOfDigits = 4;
				else
					numberOfDigits = width;
				sprintf(number, "%0*d", (int)numberOfDigits, 0);
			}

			memcpy(tgt, number, numberOfDigits);
			tgt += numberOfDigits;

			last = f+specSize;
		}
#if defined(WIN32) || defined(NO_COMPACT_DATE)
		else if ( type == 'F' ) {
			memcpy(tgt, last, f-last);
			tgt += f-last;
			memcpy(tgt, "%Y-%m-%d", 8);
			tgt += 8;
			last = f+specSize;
		}
#endif
#if defined(WIN32)
		else if ( type == 'T' ) {
			memcpy(tgt, last, f-last);
			tgt += f-last;
			memcpy(tgt, "%H:%M:%S", 8);
			tgt += 8;
			last = f+specSize;
		}
#endif

		++f;
	}

	strcpy(tgt, last);
	strftime(data, BUFFER_SIZE-1, predata, &t);

	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::iso() const {
	return toString("%FT%T.%fZ");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::fromString(const char* str, const char* fmt) {
	struct tm t;
	char data[BUFFER_SIZE];
	char tmpFmt[BUFFER_SIZE];
	long usec = 0;

	const char* microSeconds = strstr(fmt, "%f");
	if ( microSeconds != nullptr ) {
		const char* start = str;
		if ( microSeconds != fmt ) {
			start = strrchr(str, *(microSeconds-1));
			if ( start == nullptr )
				return false;
			++start;
		}

		const char* end = start;
		while ( *end >= '0' && *end <= '9' )
			++end;

		int size = end-start;
		if ( size > 6 ) size = 6;

		int multiplier = 100000;
		char *startNumber, *endNumber;

		memcpy(data, start, size);
		data[size] = '\0';

		for ( startNumber = data; *startNumber == '0' && *startNumber != '\0'; ++startNumber )
			multiplier /= 10;

		for ( endNumber = data + size-1; *endNumber == '0' && endNumber > startNumber; --endNumber )
			*endNumber = '\0';

		while ( endNumber-- > startNumber )
			multiplier /= 10;

		usec = atoi(data) * multiplier;

		int len = start - str;
		if ( len > BUFFER_SIZE-1 ) {
			SEISCOMP_ERROR("Time::fromString: buffer size exceeded: %d > %d",
			               len, BUFFER_SIZE-1);
			return false;
		}

		memcpy(data, str, len);
		data[len] = '\0';

		strcat(data, "%");
		++len;

		if ( len + strlen(end) > BUFFER_SIZE-1 ) {
			SEISCOMP_ERROR("Time::fromString: buffer size exceeded: %d > %d",
			               int(len + strlen(end)), BUFFER_SIZE);
			return false;
		}

		strcat(data, end);
		str = data;

		tmpFmt[BUFFER_SIZE-1] = '\0';
		strncpy(tmpFmt, fmt, BUFFER_SIZE);
		if ( tmpFmt[BUFFER_SIZE-1] != '\0' ) {
			SEISCOMP_ERROR("Time::fromString: format buffer size exceeded: %d > %d",
			               int(strlen(fmt)), BUFFER_SIZE-1);
			return false;
		}

		tmpFmt[microSeconds - fmt + 1] = '%';
		fmt = tmpFmt;
	}

#ifdef NO_COMPACT_DATE
	char tmpFmtDate[BUFFER_SIZE];
	const char* compactDate = strstr(fmt, "%F");
	if ( compactDate != nullptr ) {
		char *dst = tmpFmtDate;
		while ( fmt != compactDate ) { *dst++ = *fmt++; }
		strcpy(dst, "%Y-%m-%d");
		dst += 8;
		fmt += 2;
		while ( *fmt != '\0' ) { *dst++ = *fmt++; }
		*dst = '\0';
		fmt = tmpFmtDate;
	}
#endif

	time_t tmp_t = 0;
	gmtime_r(&tmp_t, &t);
	const char *remainder = strptime(str, fmt, &t);
	if ( !remainder || *remainder ) {
		/*
		if ( remainder ) {
			std::cerr << "'" << str << "' : '" << fmt << "' -> '" << remainder << "'" << std::endl;
		}
		*/
		*this = (time_t)0;
		return false;
	}
	else {
		*this = timegm(&t);
		setUSecs(usec);
	}

	return true;
#undef BUFFER_SIZE
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::fromString(const char* str) {
	for ( size_t i = 0; i < sizeof(timeFormats) / sizeof(const char*); ++i ) {
		if ( fromString(str, timeFormats[i]) ) {
			return true;
		}
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::fromString(const std::string &str) {
	return fromString(str.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromString(const char* str, const char* fmt) {
	Time t;
	t.fromString(str, fmt);
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(Time) Time::FromString(const char* str) {
	Time tmp;
	if ( tmp.fromString(str) ) {
		return tmp;
	}
	return None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(Time) Time::FromString(const std::string &str) {
	return FromString(str.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
