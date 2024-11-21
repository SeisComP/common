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


#ifndef SEISCOMP_CORE_DATETIME_H
#define SEISCOMP_CORE_DATETIME_H


#include <cstdint>
#include <chrono>
#include <limits>
#include <iostream>
#include <cmath>


namespace Seiscomp {
namespace Core {


/**
 * @brief The TimeSpan class holds the amount of microseconds passed between
 *        two times.
 *
 * Internally it is represented with a std::chrono::duration with microsecond
 * precision. It can be constructed from a std::chrono::duration object
 * Its binary represention is 64bit signed integer regardless of the
 * underlying architecture.
 *
 * The limit of a representable time span is +/-32768 years.
 */
class TimeSpan {
	public:
		using Storage = int64_t;
		using Weeks = std::chrono::duration<Storage, std::ratio<7*86400> >;
		using Days = std::chrono::duration<Storage, std::ratio<86400> >;
		using Hours = std::chrono::duration<Storage, std::ratio<3600> >;
		using Minutes = std::chrono::duration<Storage, std::ratio<60> >;
		using Seconds = std::chrono::duration<Storage, std::ratio<1> >;
		using MilliSeconds = std::chrono::duration<Storage, std::milli>;
		using MicroSeconds = std::chrono::duration<Storage, std::micro>;
		using F1 = std::chrono::duration<Storage, std::ratio<1, 10> >;
		using F2 = std::chrono::duration<Storage, std::ratio<1, 100> >;
		using F3 = std::chrono::duration<Storage, std::ratio<1, 1000> >;
		using F4 = std::chrono::duration<Storage, std::ratio<1, 10000> >;
		using F5 = std::chrono::duration<Storage, std::ratio<1, 100000> >;
		using F6 = std::chrono::duration<Storage, std::ratio<1, 1000000> >;
		using Duration = MicroSeconds;

		static const double  MinSpan;
		static const double  MaxSpan;
		static const Storage MinSeconds;
		static const Storage MaxSeconds;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		TimeSpan() = default;
		TimeSpan(const TimeSpan &other) = default;
		constexpr TimeSpan(Storage secs, Storage usecs = 0);
		TimeSpan(double ts);

		template<typename Rep, typename Period>
		constexpr TimeSpan(const std::chrono::duration<Rep, Period> &duration);


	// ----------------------------------------------------------------------
	//  Assignment operators
	// ----------------------------------------------------------------------
	public:
		//! Assigns integer epoch seconds
		TimeSpan &operator=(int t);
		//! Assigns integer epoch seconds
		TimeSpan &operator=(Storage t);
		//! Assigns floating point epoch seconds
		TimeSpan &operator=(double t);


	// ----------------------------------------------------------------------
	//  Comparison operators
	// ----------------------------------------------------------------------
	public:
		constexpr bool operator==(const TimeSpan &other) const noexcept;
		constexpr bool operator!=(const TimeSpan &other) const noexcept;
		constexpr bool operator<=(const TimeSpan &other) const noexcept;
		constexpr bool operator<(const TimeSpan &other) const noexcept;
		constexpr bool operator>=(const TimeSpan &other) const noexcept;
		constexpr bool operator>(const TimeSpan &other) const noexcept;


	// ----------------------------------------------------------------------
	//  Arithmetic operators
	// ----------------------------------------------------------------------
	public:
		constexpr TimeSpan operator+(const TimeSpan &other) const;
		constexpr TimeSpan operator-(const TimeSpan &other) const;
		constexpr TimeSpan operator*(int op) const;
		constexpr TimeSpan operator*(size_t op) const;
		constexpr TimeSpan operator*(double op) const;
		constexpr TimeSpan operator/(int op) const;
		constexpr TimeSpan operator/(size_t op) const;
		constexpr TimeSpan operator/(double op) const;
		constexpr TimeSpan operator-() const;
		TimeSpan &operator+=(const TimeSpan &other);
		TimeSpan &operator-=(const TimeSpan &other);
		TimeSpan &operator*=(int op);
		TimeSpan &operator*=(size_t op);
		TimeSpan &operator*=(double op);
		TimeSpan &operator/=(int op);
		TimeSpan &operator/=(size_t op);
		TimeSpan &operator/=(double op);


	// ----------------------------------------------------------------------
	//  Conversion operators
	// ----------------------------------------------------------------------
	public:
		constexpr explicit operator Duration() const noexcept;
		constexpr explicit operator double() const noexcept;
		//! Returns whether the timespan is empty or not.
		constexpr explicit operator bool() const noexcept;

		//! Returns an std::chrono::time_point
		constexpr const Duration &repr() const noexcept;


	// ----------------------------------------------------------------------
	//  Setter and getter
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns the maximum number of full seconds of this
		 *        time span.
		 * @return Number of full seconds.
		 */
		constexpr Storage seconds() const;

		/**
		 * @brief Returns the fractional part of the time span in
		 *        microseconds.
		 * The number of microseconds returns is guaranteed to be less than
		 * one million (1000000).
		 * @return Fraction in microseconds.
		 */
		constexpr Storage microseconds() const noexcept;

		/**
		 * @brief Returns this time span expressed in microseconds.
		 * @return Number of ticks in microseconds.
		 */
		constexpr Storage count() const noexcept;

		//! Returns the absolute value of time
		TimeSpan abs() const;

		TimeSpan &set(Storage seconds, Storage usecs = 0);

		void get(int *days, int *hours = nullptr,
		         int *minutes = nullptr, int *seconds = nullptr) const;

		[[deprecated("Use get() instead")]]
		void elapsedTime(int *days, int *hours = nullptr,
		                 int *minutes = nullptr, int *seconds = nullptr) const;

		//! Sets the microseconds
		TimeSpan &setUSecs(Storage);

		/**
		 * @brief Returns the time span in frational seconds.
		 * @return Fractional seconds as double.
		 */
		constexpr double length() const;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @return A string representing the current time span.
		 */
		std::string toString() const;

		/**
		 * @brief Converts a string to a time span representation.
		 * @param str The input string
		 * @return The conversion result
		 */
		bool fromString(const std::string &str);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		Duration _repr{0};


	friend class Time;
};


namespace Literals {


TimeSpan operator "" _weeks(long double weeks);
TimeSpan operator "" _weeks(unsigned long long int weeks);
TimeSpan operator "" _days(long double days);
TimeSpan operator "" _days(unsigned long long int days);
TimeSpan operator "" _hours(long double hours);
TimeSpan operator "" _hours(unsigned long long int hours);
TimeSpan operator "" _minutes(long double minutes);
TimeSpan operator "" _minutes(unsigned long long int minutes);
TimeSpan operator "" _seconds(long double seconds);
TimeSpan operator "" _seconds(unsigned long long int seconds);
TimeSpan operator "" _milliseconds(long double milliseconds);
TimeSpan operator "" _milliseconds(unsigned long long int milliseconds);


}

std::ostream &operator<<(std::ostream &os, const TimeSpan &ts);


/**
 * @brief The Time class implements the representation of a point in time
 *        with microsecond precision.
 *
 * Internally it is represented with a date::time_point instance from the
 * C++ date library of Howard Hinnant [1].
 *
 * Due to microseconds precision the valid date range is defined from
 * -32768-01-01 to 32767-12-31.
 *
 * [1] https://github.com/HowardHinnant/date
 */
class Time {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		using Storage = TimeSpan::Storage;
		using Duration = TimeSpan::Duration;
		using TimePoint = std::chrono::time_point<std::chrono::system_clock, Duration>;

		static Time Null;

		static constexpr const double MinTime = std::numeric_limits<Storage>::min() * 1E-6;
		static constexpr const double MaxTime = std::numeric_limits<Storage>::max() * 1E-6;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Time() = default;
		Time(const Time &other) = default;
		explicit constexpr Time(const TimePoint &tp);
		explicit constexpr Time(Storage epochSeconds, Storage microSeconds = 0);
		explicit Time(double);
		Time(int year, int month, int day,
		     int hour = 0, int min = 0, int sec = 0,
		     int usec = 0);


	// ----------------------------------------------------------------------
	//  Assignment operators
	// ----------------------------------------------------------------------
	public:
		Time &operator=(double epoch);


	// ----------------------------------------------------------------------
	//  Comparison operators
	// ----------------------------------------------------------------------
	public:
		constexpr bool operator==(const Time &other) const noexcept;
		constexpr bool operator!=(const Time &other) const noexcept;
		constexpr bool operator<(const Time &other) const noexcept;
		constexpr bool operator<=(const Time &other) const noexcept;
		constexpr bool operator>(const Time &other) const noexcept;
		constexpr bool operator>=(const Time &other) const noexcept;


	// ----------------------------------------------------------------------
	//  Arithmetic operators
	// ----------------------------------------------------------------------
	public:
		Time &operator+=(const TimeSpan &ts);
		Time &operator-=(const TimeSpan &ts);

		constexpr Time operator+(const TimeSpan &ts) const;
		constexpr Time operator-(const TimeSpan &ts) const;
		constexpr TimeSpan operator-(const Time &tp) const;


	// ----------------------------------------------------------------------
	//  Conversion operators
	// ----------------------------------------------------------------------
	public:
		constexpr explicit operator TimePoint() const noexcept;
		constexpr explicit operator double() const noexcept;
		[[deprecated("Use OPT(Time) instead")]]
		constexpr explicit operator bool() const noexcept;
		constexpr const TimePoint &repr() const noexcept;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns whether the time object is a valid time or not.
		 * Internally the minimum possible value is used to declare an invalid
		 * time which is for backwards compatibility 1970-01-01 00:00:00.
		 * @return True if valid, false otherwise.
		 */
		[[deprecated("Use OPT(Time) instead")]]
		constexpr bool valid() const;

		/**
		 * @brief Sets the time object with y/m/d and h/m/s/u individually.
		 * The resulting time will be defined in seconds from Jan. 1, 1970 UTC.
		 * If local time is desired then call toLocalTime() afterwards.
		 * \code
		 * Time t;
		 * t.set(2020,1,1,12,00,00).toLocalTime()
		 * \endcode
		 * @param year The year between -32767 and 32768
		 * @param month The month between 1 and 12
		 * @param day The day between 1 and 31
		 * @param hour The hour of the day between 0 and 23
		 * @param min The minute of the hour between 0 and 59
		 * @param sec The second of the minute between 0 and 59
		 * @param usec The microseconds of the second between 0 and 999999
		 * @return this instance
		 */
		Time &set(int year, int month, int day,
		          int hour, int min, int sec,
		          int usec);

		/**
		 * @brief Retrieves individual date and time components.
		 * @param year The year between -32767 and 32768
		 * @param month The month of the year between 1 and 12
		 * @param day The day of the month between 1 and 31
		 * @param hour The hour of the day between 0 and 23
		 * @param min The minute of the hour between 0 and 59
		 * @param sec The second of the minute between 0 and 59
		 * @param usec The microseconds of the second between 0 and 999999
		 * @return Success flag
		 */
		bool get(int *year, int *month = nullptr, int *day = nullptr,
		         int *hour = nullptr, int *min = nullptr, int *sec = nullptr,
		         int *usec = nullptr) const;

		/**
		 * @brief Sets the time object with y/d and h/m/s/u individually.
		 * The resulting time will be defined in seconds from Jan. 1, 1970 UTC.
		 * If local time is desired then call toLocalTime() afterwards.
		 * \code
		 * Time t;
		 * t.set(2020,1,1,12,00,00).toLocalTime()
		 * \endcode
		 * @param year The year between -32767 and 32768
		 * @param yday The day of the year between 0 and 365
		 * @param hour The hour of the day between 0 and 23
		 * @param min The minute of the hour between 0 and 59
		 * @param sec The second of the minute between 0 and 59
		 * @param usec The microseconds of the second between 0 and 999999
		 * @return this instance
		 */
		Time &set2(int year, int yday,
		           int hour, int min, int sec,
		           int usec);

		/**
		 * @brief Retrieves individual date and time components.
		 * @param year The year between -32767 and 32768
		 * @param yday The day of the year between 0 and 365
		 * @param hour The hour of the day between 0 and 23
		 * @param min The minute of the hour between 0 and 59
		 * @param sec The second of the minute between 0 and 59
		 * @param usec The microseconds of the second between 0 and 999999
		 * @return Success flag
		 */
		bool get2(int *year, int *yday = nullptr,
		          int *hour = nullptr, int *min = nullptr, int *sec = nullptr,
		          int *usec = nullptr) const;

		/**
		 * @brief Alias for utc()
		 * @return The current time in UTC
		 */
		Time &now();

		/**
		 * This is an alias for utc().
		 * @return The current time in UTC
		 */
		Time &gmt();

		/**
		 * @return The current time in UTC
		 */
		Time &utc();

		/**
		 * @return The ISO string of the time in UTC
		 */
		std::string iso() const;

		/**
		 * @return A string representing the current time point without
		 *         time zone information, UTC time.
		 */
		std::string toString(const char *format) const;

		/**
		 * @brief Converts the time point to a string in the current
		 *        system's time zone.
		 * @param format The string format.
		 * @return The formatted string
		 */
		std::string toLocalString(const char *format) const;

		/**
		 * @brief Converts the time point to a string in a given
		 *        time zone.
		 * This method may throw an std::runtime_error if the provided time
		 * zone does not exist.
		 * @param format The string format.
		 * @return The formatted string
		 */
		std::string toZonedString(const char *format, const std::string &tz) const;

		/**
		 * Converts a string into a time representation.
		 * @param str The string representation of the time
		 * @return The conversion result
		 */
		bool fromString(const std::string &str);

		/**
		 * Converts a string into a time representation.
		 * @param str The string representation of the time
		 * @param fmt The format string containing the conversion
		 *            specification (-> toString)
		 * @return The conversion result
		 */
		bool fromString(const std::string &str, const char *format);

		/**
		 * @brief Converts a string to a Time object.
		 * This method throws a runtime_error if the input string cannot
		 * be parsed into a valid Time.
		 * @param str The string representation of the time
		 * @return A Time object
		 */
		static Time FromString(const std::string &str);

		/**
		 * @brief Converts a string to a Time object.
		 * This method throws a runtime_error if the input string cannot
		 * be parsed into a valid Time given the format.
		 * @param str The string representation of the time
		 * @param fmt The format string containing the conversion
		 *            specification (-> toString)
		 * @return A Time object
		 */
		static Time FromString(const std::string &str, const char *format);

		/**
		 * @brief Constructs a time object from a year and the day of the year.
		 * @param year The year between -32767 and 32768
		 * @param doy The day of the year between 1 and 366. Note that in
		 *            contrast to set2 and get2 the doy is not starting with 0
		 *            at January 1st but with 1.
		 * @return
		 */
		static Time FromYearDay(int year, int doy);

		/**
		 * @return The number of seconds since Jan. 1, 1970.
		 *
		 * Backwards compatibility alias for epochSeconds().
		 */
		[[deprecated("Use epochSeconds() instead")]]
		constexpr Storage seconds() const;

		/**
		 * @return The number of seconds since Jan. 1, 1970.
		 */
		constexpr Storage epochSeconds() const;

		/**
		 * @return The number of seconds with fractional seconds since
		 *         Jan. 1, 1970.
		 */
		constexpr double epoch() const;

		/**
		 * @return Seconds fraction.
		 */
		constexpr int microseconds() const;

		Time &setUSecs(Storage ms);

		static Time FromEpoch(Storage secs, Storage usecs);
		static Time FromEpoch(double secs);

		//! Returns the current time in UTC. This is an alias for UTC().
		static Time Now();

		//! Returns the current time in GMT. This is an alias for UTC().
		[[deprecated("Use UTC() instead")]] static Time GMT();

		//! Returns the current time in UTC
		static Time UTC();

		//! Returns the current time as UTC plus the time zone offset.
		static Time LocalTime();

		static std::string LocalTimeZone();

		//! Converts the time of the UTC representation to local time.
		//! This effectively adds the local timezone offset.
		Time toLocalTime() const;

		//! Converts the time of the local time representation to UTC. This
		//! effectively removes the local timezone offset.
		Time toUTC() const;

		//! Converts the time of the local time representation to UTC. This
		//! effectively removes the local timezone offset.
		[[deprecated("Use toUTC() instead")]] Time toGMT() const;

		/**
		 * @brief Returns the local timezone offset with respect to the
		 *        time currently stored.
		 * @return The local timezone offset as @TimeSpan
		 */
		TimeSpan localTimeZoneOffset() const;

		/**
		 * @brief Returns the timezone offset for a given timezone.
		 *
		 * If the timezone is unknown then a runtime_error will be thrown.
		 * @return The timezone offset as @TimeSpan
		 */
		TimeSpan timeZoneOffset(const std::string &tzName) const;


	private:
		TimePoint _repr{Duration{0}};
};


std::ostream &operator<<(std::ostream &os, const Time &time);


#include "datetime.ipp"


}
}


#endif
