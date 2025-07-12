// --------------------------------------------------------------------------
// Implementation of inline functions
// --------------------------------------------------------------------------




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr TimeSpan::TimeSpan(Storage secs, Storage usecs)
: _repr(Seconds(secs) + MicroSeconds(usecs)) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline TimeSpan::TimeSpan(double ts) {
	*this = ts;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename Rep, typename Period>
inline constexpr TimeSpan::TimeSpan(const std::chrono::duration<Rep, Period> &duration)
: _repr(std::chrono::duration_cast<Duration>(duration)) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr bool TimeSpan::operator==(const TimeSpan &other) const noexcept {
	return _repr == other._repr;
}

inline constexpr bool TimeSpan::operator!=(const TimeSpan &other) const noexcept {
	return _repr != other._repr;
}

inline constexpr bool TimeSpan::operator<=(const TimeSpan &other) const noexcept {
	return _repr <= other._repr;
}

inline constexpr bool TimeSpan::operator<(const TimeSpan &other) const noexcept {
	return _repr < other._repr;
}

inline constexpr bool TimeSpan::operator>=(const TimeSpan &other) const noexcept {
	return _repr >= other._repr;
}

inline constexpr bool TimeSpan::operator>(const TimeSpan &other) const noexcept {
	return _repr > other._repr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr TimeSpan TimeSpan::operator+(const TimeSpan &other) const {
	return TimeSpan(_repr + other._repr);
}

inline constexpr TimeSpan TimeSpan::operator-(const TimeSpan &other) const {
	return TimeSpan(_repr - other._repr);
}

inline constexpr TimeSpan TimeSpan::operator*(int op) const {
	return TimeSpan(Duration(Storage(_repr.count() * op)));
}

inline constexpr TimeSpan TimeSpan::operator*(size_t op) const {
	return TimeSpan(Duration(Storage(_repr.count() * op)));
}

inline constexpr TimeSpan TimeSpan::operator*(double op) const {
	return TimeSpan(Duration(Storage(_repr.count() * op)));
}

inline constexpr TimeSpan TimeSpan::operator/(int op) const {
	return TimeSpan(Duration(Storage(_repr.count() / op)));
}

inline constexpr TimeSpan TimeSpan::operator/(size_t op) const {
	return TimeSpan(Duration(Storage(_repr.count() / op)));
}

inline constexpr TimeSpan TimeSpan::operator/(double op) const {
	return TimeSpan(Duration(Storage(_repr.count() / op)));
}

inline constexpr TimeSpan TimeSpan::operator-() const {
	return TimeSpan(Duration(Storage(-_repr.count())));
}

inline TimeSpan &TimeSpan::operator+=(const TimeSpan &other) {
	_repr += other._repr;
	return *this;
}

inline TimeSpan &TimeSpan::operator-=(const TimeSpan &other) {
	_repr -= other._repr;
	return *this;
}

inline TimeSpan &TimeSpan::operator*=(int op) {
	_repr = Duration(Storage(_repr.count() * op));
	return *this;
}

inline TimeSpan &TimeSpan::operator*=(size_t op) {
	_repr = Duration(Storage(_repr.count() * op));
	return *this;
}

inline TimeSpan &TimeSpan::operator*=(double op) {
	_repr = Duration(Storage(_repr.count() * op));
	return *this;
}

inline TimeSpan &TimeSpan::operator/=(int op) {
	_repr = Duration(Storage(_repr.count() / op));
	return *this;
}

inline TimeSpan &TimeSpan::operator/=(size_t op) {
	_repr = Duration(Storage(_repr.count() / op));
	return *this;
}

inline TimeSpan &TimeSpan::operator/=(double op) {
	_repr = Duration(Storage(_repr.count() / op));
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr TimeSpan::operator Duration() const noexcept {
	return _repr;
}

inline constexpr TimeSpan::operator double() const noexcept {
	return length();
}

inline constexpr TimeSpan::operator bool() const noexcept {
	return count() > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr const TimeSpan::Duration &TimeSpan::repr() const noexcept {
	return _repr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Literals {


inline TimeSpan operator "" _weeks(long double weeks) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<double, std::ratio<86400*7> >(weeks));
}

inline TimeSpan operator "" _weeks(unsigned long long int weeks) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<unsigned long long int, std::ratio<86400*7> >(weeks));
}

inline TimeSpan operator "" _days(long double days) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<double, std::ratio<86400> >(days));
}

inline TimeSpan operator "" _days(unsigned long long int days) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<unsigned long long int, std::ratio<86400> >(days));
}

inline TimeSpan operator "" _hours(long double hours) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<double, std::ratio<3600> >(hours));
}

inline TimeSpan operator "" _hours(unsigned long long int hours) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<unsigned long long int, std::ratio<3600> >(hours));
}

inline TimeSpan operator "" _minutes(long double minutes) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<double, std::ratio<60> >(minutes));
}

inline TimeSpan operator "" _minutes(unsigned long long int minutes) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<unsigned long long int, std::ratio<60> >(minutes));
}

inline TimeSpan operator "" _seconds(long double seconds) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<double, std::ratio<1> >(seconds));
}

inline TimeSpan operator "" _seconds(unsigned long long int seconds) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<unsigned long long int, std::ratio<1> >(seconds));
}

inline TimeSpan operator "" _milliseconds(long double mseconds) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<double, std::ratio<1, 1000> >(mseconds));
}

inline TimeSpan operator "" _milliseconds(unsigned long long int mseconds) {
	using namespace std::chrono;
	return duration_cast<TimeSpan::Duration>(duration<unsigned long long int, std::ratio<1, 1000> >(mseconds));
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void TimeSpan::elapsedTime(int* days, int* hours,
                                  int* minutes, int* seconds) const {
	get(days, hours, minutes, seconds);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr TimeSpan::Storage TimeSpan::seconds() const {
	return std::chrono::duration_cast<Seconds>(_repr).count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr TimeSpan::Storage TimeSpan::microseconds() const noexcept {
	// _repr is microseconds
	return _repr.count() % 1000000;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr TimeSpan::Storage TimeSpan::count() const noexcept {
	return _repr.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline TimeSpan &TimeSpan::set(Storage secs, Storage usecs) {
	using namespace std::chrono;
	_repr = MicroSeconds(secs * 1000000 + usecs);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline TimeSpan &TimeSpan::setUSecs(Storage usecs) {
	using namespace std::chrono;
	_repr = MicroSeconds(seconds() * 1000000 + usecs);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr double TimeSpan::length() const {
	return std::chrono::duration_cast<MicroSeconds>(_repr).count() * 1E-6;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr Time::Time(Storage epochSeconds, Storage epochMicroSeconds)
: _repr(TimePoint(TimeSpan::Seconds(epochSeconds) + TimeSpan::MicroSeconds(epochMicroSeconds)))
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr Time::Time(const TimePoint &tp) : _repr(tp) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr bool Time::operator==(const Time &other) const noexcept {
	return _repr == other._repr;
}

inline constexpr bool Time::operator!=(const Time &other) const noexcept {
	return _repr != other._repr;
}

inline constexpr bool Time::operator<(const Time &other) const noexcept {
	return _repr < other._repr;
}

inline constexpr bool Time::operator<=(const Time &other) const noexcept {
	return _repr <= other._repr;
}

inline constexpr bool Time::operator>(const Time &other) const noexcept {
	return _repr > other._repr;
}

inline constexpr bool Time::operator>=(const Time &other) const noexcept {
	return _repr >= other._repr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time &Time::operator+=(const TimeSpan &ts) {
	_repr += ts._repr;
	return *this;
}

inline Time &Time::operator-=(const TimeSpan &ts) {
	_repr -= ts._repr;
	return *this;
}

inline constexpr Time Time::operator+(const TimeSpan &ts) const {
	return Time(_repr + ts._repr);
}

inline constexpr Time Time::operator-(const TimeSpan &ts) const {
	return Time(_repr - ts._repr);
}

inline constexpr TimeSpan Time::operator-(const Time &tp) const {
	return TimeSpan(_repr - tp._repr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr Time::operator TimePoint() const noexcept {
	return _repr;
}

inline constexpr Time::operator double() const noexcept {
	return epoch();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr bool Time::valid() const {
	return *this != Null;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr Time::operator bool() const noexcept {
	return *this != Null;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr const Time::TimePoint &Time::repr() const noexcept {
	return _repr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr Time::Storage Time::seconds() const {
	return epochSeconds();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr Time::Storage Time::epochSeconds() const {
	return std::chrono::duration_cast<std::chrono::seconds>(_repr.time_since_epoch()).count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr double Time::epoch() const {
	return std::chrono::duration_cast<std::chrono::microseconds>(_repr.time_since_epoch()).count() * 1E-6;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time &Time::setUSecs(Storage usecs) {
	using namespace std::chrono;
	_repr = TimePoint(
		std::chrono::duration_cast<Duration>(
			TimeSpan::MicroSeconds(
				static_cast<Storage>(
					std::chrono::duration_cast<std::chrono::seconds>(
						_repr.time_since_epoch()
					).count() * 1000000 + usecs
				)
			)
		)
	);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline constexpr int Time::microseconds() const {
	return std::chrono::duration_cast<std::chrono::microseconds>(_repr.time_since_epoch()).count() % 1000000;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time &Time::gmt() {
	return utc();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time Time::UTC() {
	return Now();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time Time::LocalTime() {
	return UTC().toLocalTime();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time Time::GMT() {
	return Now();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time Time::Now() {
	return Time().now();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time Time::toLocalTime() const {
	return *this + localTimeZoneOffset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time Time::toUTC() const {
	return *this - localTimeZoneOffset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline Time Time::toGMT() const {
	return toUTC();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
