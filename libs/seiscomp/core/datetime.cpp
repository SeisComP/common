#include "datetime.h"
#include "exceptions.h"
#include "strings.h"

// Disable remote time zone database API
#define HAS_REMOTE_API 0
// Use the OS provided time zone database
#define USE_OS_TZDB 1
// Only use C locales
#define ONLY_C_LOCALE 1

#include "date/date.h"
#include "date/tz.h"

#include <iostream>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Core {

namespace {


const char *timeFormats[] = {
	"%FT%T.%fZ",    // YYYY-MM-DDThh:mm:ss.ssssssZ
	"%FT%T.%f",     // YYYY-MM-DDThh:mm:ss.ssssss
	"%FT%TZ",       // YYYY-MM-DDThh:mm:ssZ
	"%FT%T.%f%z",   // YYYY-MM-DDThh:mm:ss.ssssss+hh:mm
	"%FT%T%z",      // YYYY-MM-DDThh:mm:ss+hh:mm
	"%FT%T",        // YYYY-MM-DDThh:mm:ss
	"%FT%R",        // YYYY-MM-DDThh:mm
	"%FT%H",        // YYYY-MM-DDThh
	"%6Y-%jT%T.%f", // YYYY-DDDThh:mm:ss.ssssss
	"%6Y-%jT%T",    // YYYY-DDDThh:mm:ss
	"%6Y-%jT%R",    // YYYY-DDDThh:mm
	"%6Y-%jT%H",    // YYYY-DDDThh
	"%F %T.%f",     // YYYY-MM-DD hh:mm:ss.ssssss
	"%F %T",        // YYYY-MM-DD hh:mm:ss
	"%F %R",        // YYYY-MM-DD hh:mm
	"%F %H",        // YYYY-MM-DD hh
	"%F",           // YYYY-MM-DD
	"%6Y-%j",       // YYYY-DDD
	"%6Y",          // YYYY
};


OPT(const date::time_zone*) current_zone = None;

void fetch_current_zone() {
	if ( !current_zone ) {
		try {
			current_zone = date::current_zone();
		}
		catch ( std::exception &e ) {
			std::cerr << "[datetime] warning: " << e.what() << ", setting timezone to UTC" << std::endl;
			current_zone = nullptr;
		}
	}
}


}


const double TimeSpan::MinSpan = static_cast<double>(std::numeric_limits<TimeSpan::Storage>::min()) * 1E-6;
const double TimeSpan::MaxSpan = static_cast<double>(std::numeric_limits<TimeSpan::Storage>::max()) * 1E-6;
const TimeSpan::Storage TimeSpan::MinSeconds = std::chrono::duration_cast<TimeSpan::Seconds>(TimeSpan::MicroSeconds(std::numeric_limits<TimeSpan::Storage>::min())).count();
const TimeSpan::Storage TimeSpan::MaxSeconds = std::chrono::duration_cast<TimeSpan::Seconds>(TimeSpan::MicroSeconds(std::numeric_limits<TimeSpan::Storage>::max())).count();
const double Time::MinTime = static_cast<double>(std::numeric_limits<Time::Storage>::min()) * 1E-6;
const double Time::MaxTime = static_cast<double>(std::numeric_limits<Time::Storage>::max()) * 1E-6;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const TimeSpan &ts) {
	TimeSpan::Duration d = ts.repr();
	if ( !d.count() ) {
		return os << "0";
	}

	int64_t days = std::chrono::duration_cast<TimeSpan::Days>(d).count();
	if ( days ) {
		os << days << "d";
	}
	d -= TimeSpan::Days(days);

	if ( d.count() ) {
		int64_t hours = std::chrono::duration_cast<TimeSpan::Hours>(d).count();
		if ( hours ) {
			os << hours << "h";
		}
		d -= TimeSpan::Hours(hours);

		if ( d.count() ) {
			int64_t minutes = std::chrono::duration_cast<TimeSpan::Minutes>(d).count();
			if ( minutes ) {
				os << minutes << "m";
			}
			d -= TimeSpan::Minutes(minutes);

			if ( d.count() ) {
				int64_t seconds = std::chrono::duration_cast<TimeSpan::Seconds>(d).count();
				if ( seconds ) {
					os << seconds << "s";
				}
				d -= TimeSpan::Seconds(seconds);

				if ( d.count() ) {
					os << d.count() << "us";
				}
			}
		}
	}

	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan &TimeSpan::operator=(int ts) {
	// Integer values cannot overflow as storage is signed 64bit.
	_repr = std::chrono::duration_cast<Duration>(Seconds(ts));
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan &TimeSpan::operator=(Storage ts) {
	if ( ts < MinSeconds || ts > MaxSeconds ) {
		throw OverflowException("Integer span not fit into TimeSpan storage");
	}

	_repr = std::chrono::duration_cast<Duration>(Seconds(ts));
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan &TimeSpan::operator=(double ts) {
	if ( ts < MinSpan || ts > MaxSpan ) {
		throw OverflowException("Double span does not fit into TimeSpan storage");
	}

	_repr = std::chrono::duration_cast<Duration>(MicroSeconds(static_cast<Storage>(std::round(ts * 1E6))));
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan TimeSpan::abs() const {
	return Duration(std::abs(_repr.count()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TimeSpan::get(int* days, int* hours,
                   int* minutes, int* seconds) const {
	Storage elapsed = this->seconds();
	if ( days ) {
		*days = elapsed / 86400;
	}
	if ( hours ) {
		*hours = (elapsed % 86400) / 3600;
	}
	if ( minutes ) {
		*minutes = ((elapsed % 86400) % 3600) / 60;
	}
	if ( seconds ) {
		*seconds = ((elapsed % 86400) % 3600) % 60;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string TimeSpan::toString() const {
	std::ostringstream oss;
	oss << *this;
	return oss.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TimeSpan::fromString(std::string_view sv) {
	double secs;
	if ( !Core::fromString(secs, sv) ) {
		return false;
	}
	try {
		*this = secs;
		return true;
	}
	catch ( OverflowException & ) {
		return false;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::Null(Time::TimePoint{Time::Duration{0}});
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time::Time(double epoch) {
	*this = epoch;
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
Time &Time::operator=(double epoch) {
	if ( epoch < MinTime || epoch > MaxTime ) {
		throw OverflowException("Double epoch does not fit into Time storage");
	}

	_repr = TimePoint(
		std::chrono::duration_cast<Duration>(
			TimeSpan::MicroSeconds(static_cast<Storage>(std::round(epoch * 1E6)))
		)
	);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time &Time::set(int year, int month, int day,
                int hour, int min, int sec,
                int usec) {
	_repr = date::sys_days{date::year{year}/month/day} +
	        std::chrono::hours{hour} +
	        std::chrono::minutes{min} +
	        std::chrono::seconds{sec} +
	        std::chrono::microseconds(usec);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time &Time::set2(int year, int yday,
                 int hour, int min, int sec,
                 int usec) {
	_repr = static_cast<date::sys_days>(date::year_month_day{date::local_days(date::year{year}/1/1) +
	        date::days{yday}}) +
	        std::chrono::hours{hour} +
	        std::chrono::minutes{min} +
	        std::chrono::seconds{sec} +
	        std::chrono::microseconds(usec);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::get(int *year, int *month, int *day,
               int *hour, int *min, int *sec,
               int *usec) const {
	auto dp = date::floor<date::days>(_repr);
	auto ymd = date::year_month_day{dp};
	*year = static_cast<int>(ymd.year());
	if ( month ) {
		*month = static_cast<unsigned>(ymd.month());
	}
	if ( day ) {
		*day = static_cast<unsigned>(ymd.day());
	}
	auto time = date::make_time(_repr-dp);
	if ( hour ) {
		*hour = time.hours().count();
	}
	if ( min ) {
		*min = time.minutes().count();
	}
	if ( sec ) {
		*sec = time.seconds().count();
	}
	if ( usec ) {
		*usec = time.subseconds().count();
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::get2(int *year, int *yday,
                int *hour , int *min, int *sec,
                int *usec) const {
	auto dp = date::floor<date::days>(_repr);
	auto ymd = date::year_month_day{dp};
	*year = (int)ymd.year();
	if ( yday ) {
		*yday = (dp - date::sys_days{ymd.year()/date::January/1}).count();
	}
	auto time = date::make_time(_repr-dp);
	if ( hour ) {
		*hour = time.hours().count();
	}
	if ( min ) {
		*min = time.minutes().count();
	}
	if ( sec ) {
		*sec = time.seconds().count();
	}
	if ( usec ) {
		*usec = time.subseconds().count();
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time &Time::now() {
	return utc();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time &Time::utc() {
	_repr = std::chrono::time_point_cast<Duration>(std::chrono::system_clock::now());
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromEpoch(Storage seconds, Storage microseconds) {
	return Time(seconds, microseconds);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromEpoch(double seconds) {
	return Time(
		TimePoint(
			std::chrono::duration_cast<Duration>(
				std::chrono::duration<double, std::ratio<1>>(seconds)
			)
		)
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::iso() const {
	return toString("%FT%T.%fZ");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::toString(const char *format) const {
	std::ostringstream oss;
	oss << date::format(format, _repr);
	return oss.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::toLocalString(const char *format) const {
	std::ostringstream oss;
	fetch_current_zone();
	if ( *current_zone ) {
		oss << date::format(format, date::make_zoned(*current_zone, _repr));
	}
	else {
		oss << date::format(format, _repr);
	}
	return oss.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::toZonedString(const char *format, const std::string &tz) const {
	if ( tz.empty() ) {
		return toString(format);
	}

	const auto *zone = date::locate_zone(tz);
	if ( !zone ) {
		throw std::runtime_error(tz + " not found in time zone database");
	}

	std::ostringstream oss;
	oss << date::format(format, date::make_zoned(zone, _repr));
	return oss.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::fromString(std::string_view sv, const char *format) {
	std::chrono::time_point<date::local_t, Duration> lt;
	std::string tz_name;
	std::chrono::minutes offset(0);
	InputStringViewStream in{sv};

	in >> date::parse(format, lt, tz_name, offset);
	if ( in.fail() ) {
		// Invalid parser state
		return false;
	}

	if ( (in.tellg() < static_cast<std::streamoff>(sv.size())) && !in.eof() ) {
		// Not all characters read from string
		return false;
	}

	if ( !tz_name.empty() ) {
		try {
			_repr = make_zoned(tz_name, lt).get_sys_time();
		}
		catch ( ... ) {
			return false;
		}
	}
	else {
		static const date::time_zone *zone_utc = date::locate_zone("UTC");
		_repr = make_zoned(zone_utc, lt - offset).get_sys_time();
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Time::fromString(std::string_view sv) {
	for ( size_t i = 0; i < sizeof(timeFormats) / sizeof(const char*); ++i ) {
		if ( fromString(sv, timeFormats[i]) ) {
			return true;
		}
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromString(const std::string &str) {
	Time t;
	if ( t.fromString(str) ) {
		return t;
	}
	throw std::runtime_error("Invalid datetime string '" + str + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromString(const std::string &str, const char *format) {
	Time t;
	if ( t.fromString(str, format) ) {
		return t;
	}
	throw std::runtime_error(std::string("Datetime string '") + str + "' does not match format '" + format + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Time Time::FromYearDay(int year, int yday) {
	return Time(
		TimePoint(
			static_cast<date::sys_days>(
				date::year_month_day{date::local_days(date::year{year}/1/1) + date::days{yday - 1}}
			)
		)
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Time::LocalTimeZone() {
	fetch_current_zone();
	const auto *zone = *current_zone;
	return zone ? zone->name() : std::string("UTC");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan Time::localTimeZoneOffset() const {
	fetch_current_zone();
	if ( *current_zone ) {
		auto i = (*current_zone)->get_info(_repr);
		return std::chrono::duration_cast<TimeSpan::Duration>(i.offset);
	}
	else {
		return TimeSpan(0, 0);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeSpan Time::timeZoneOffset(const std::string &tzName) const {
	const date::time_zone *zone = date::locate_zone(tzName);
	if ( !zone ) {
		if ( tzName == "UTC" ) {
			return TimeSpan(0, 0);
		}
		throw std::runtime_error(tzName + " not found in time zone database");
	}

	auto i = zone->get_info(_repr);
	return std::chrono::duration_cast<Duration>(i.offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const Time &time) {
	os << date::format("%F %T.%f", time.repr());
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



#include "date/tz.cpp"
