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


#define SEISCOMP_COMPONENT strings

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <cctype>
#include <charconv>
#include <cstdint>
#include <cerrno>
#include <cmath>
#include <limits>
#include <type_traits>


#define AUTO_FLOAT_PRECISION 1


namespace Seiscomp {
namespace Core {


namespace {


template <typename T>
inline size_t getFixedDigits10(T val) {
	T e = 1;
	T p = std::log10(val < 0 ? -val : val);

	if ( p < 0 ) {
		p = std::floor(-p);
	}
	else {
		p = 0;
	}

	val *= std::pow(10, p);

	while ( p < std::numeric_limits<T>::max_digits10 &&
	        std::abs(std::round(val * e) - val * e) > 1E-6 ) {
		e *= 10;
		++p;
	}

	return p;
}


template <typename T>
inline size_t getScientificDigits10(T val) {
	T e = 1;
	T p = std::log10(val < 0 ? -val : val);
	T n = 0;

	if ( p < 0 ) {
		p = std::floor(-p);
	}
	else {
		n = std::floor(p) + 1;
		p = 0;
	}

	if ( n >= std::numeric_limits<T>::max_digits10 ) {
		return std::numeric_limits<T>::max_digits10;
	}

	val *= std::pow(10, p);

	p = n;
	while ( p < std::numeric_limits<T>::max_digits10 &&
	        std::abs(std::round(val * e) - val * e) > 1E-6 ) {
		e *= 10;
		++p;
	}

	return p;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &ostream, const Number<float> &n) {
	ostream.precision(
#if AUTO_FLOAT_PRECISION
		ostream.flags() & std::ios_base::fixed ?
		getFixedDigits10(n.ref) :
		getScientificDigits10(n.ref)
#else
		std::numeric_limits<float>::max_digits10
#endif
	);
	ostream << n.ref;
	return ostream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &ostream, const Number<double> &n) {
	ostream.precision(
#if AUTO_FLOAT_PRECISION
		ostream.flags() & std::ios_base::fixed ?
		getFixedDigits10(n.ref) :
		getScientificDigits10(n.ref)
#else
		std::numeric_limits<double>::max_digits10
#endif
	);
	ostream << n.ref;
	return ostream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string toString(const std::string& value) {
	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string toString(bool v) {
	return std::string(v?"true":"false");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string toString(const Seiscomp::Core::Time& v) {
	return v.iso();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string toString(const Enumeration& value) {
	return value.toString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

template <typename T>
inline bool convertFromString(T &value, std::string_view &sv) {
	if ( sv.empty() ) {
		return false;
	}

	auto last = sv.data() + sv.size();
	auto r = std::from_chars(sv.data(), last, value, 10);
	if ( r.ec == std::errc() ) {
		if ( r.ptr != last ) {
			// Not all characters consumed
			errno = EINVAL;
			return false;
		}

		return true;
	}
	else if ( r.ec == std::errc::result_out_of_range ) {
		errno = ERANGE;
	}
	else if ( r.ec == std::errc::invalid_argument ) {
		errno = EINVAL;
	}

	return false;
}

}

template <>
bool fromString(char &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(signed char &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(unsigned char &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(short &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(unsigned short &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(int &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(unsigned int &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(long &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(unsigned long &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(long long &value, std::string_view sv) {
	return convertFromString(value, sv);
}
template <>
bool fromString(unsigned long long &value, std::string_view sv) {
	return convertFromString(value, sv);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(double &value, std::string_view sv) {
	if ( sv.empty() ) {
		return false;
	}

	auto last = sv.data() + sv.size();
	auto r = std::from_chars(sv.data(), last, value);

	if ( r.ec == std::errc() ) {
		return r.ptr == last;
	}
	else if ( r.ec == std::errc::result_out_of_range ) {
		errno = ERANGE;
	}
	else if ( r.ec == std::errc::invalid_argument ) {
		errno = EINVAL;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(float &value, std::string_view sv) {
	double tmp;

	if ( !fromString(tmp, sv) ) {
		return false;
	}

	if ( std::isnormal(tmp) ) {
		auto atmp = std::fabs(tmp);
		if ( atmp < static_cast<double>(std::numeric_limits<float>::min())
		  || atmp > static_cast<double>(std::numeric_limits<float>::max()) ) {
			errno = ERANGE;
			return false;
		}
	}

	value = static_cast<float>(tmp);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(bool &value, std::string_view sv) {
	if ( sv.empty() )
		return false;

	if ( compareNoCase(sv, "true") == 0 ) {
		value = true;
		return true;
	}

	if ( compareNoCase(sv, "false") == 0 ) {
		value = false;
		return true;
	}

	long int retval;
	if ( !fromString(retval, sv) ) {
		return false;
	}

	value = retval ? true : false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(TimeSpan &value, std::string_view sv) {
	return value.fromString(sv);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(Time &value, std::string_view sv) {
	return value.fromString(sv);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(Enumeration &value, std::string_view sv) {
	return value.fromString(sv);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(std::string &value, std::string_view sv) {
	value.assign(sv);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int split(std::vector<std::string> &tokens, const char *source,
          const char *delimiter, bool compressOn) {
	boost::split(tokens, source, boost::is_any_of(delimiter),
	             ((compressOn) ? boost::token_compress_on : boost::token_compress_off));
	return static_cast<int>(tokens.size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int split(std::vector<std::string> &tokens, const std::string &source,
          const char *delimiter, bool compressOn) {
	return split(tokens, source.c_str(), delimiter, compressOn);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t splitExt(std::vector<std::string> &tokens, const char *source,
                const char *delimiter, bool compressOn, bool unescape,
                bool trim, const char *whitespaces, const char *quotes) {
	tokens.clear();
	size_t lenTok;
	size_t lenSource = strlen(source);
	char delimFound = 0;
	const char *tok = nullptr;

	if ( unescape ) {
		std::string tmp(source, lenSource);
		char *sourceCopy = const_cast<char *>(tmp.c_str());
		while ( lenSource > 0 ) {
			tok = tokenizeUnescape(lenTok, lenSource, sourceCopy, delimFound,
			                       delimiter, trim, whitespaces, quotes);
			if ( tok != nullptr ) {
				tokens.push_back(std::string(tok, lenTok));
			}
			else if ( tokens.empty() || !compressOn ) {
				tokens.push_back("");
			}
		}
	}
	else {
		while ( lenSource > 0 ) {
			tok = tokenizeExt(lenTok, lenSource, source, delimFound, delimiter,
			                  trim, whitespaces, quotes);
			if ( tok != nullptr ) {
				tokens.push_back(std::string(tok, lenTok));
			}
			else if ( tokens.empty() || !compressOn ) {
				tokens.push_back("");
			}
		}
	}

	if ( delimFound )
		tokens.push_back("");

	return tokens.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *tokenizeExt(size_t &lenTok, size_t &lenSource, const char *&source,
                        char &delimFound, const char *delimiter, bool trim,
                        const char *whitespaces, const char *quotes) {
	lenTok = 0;
	delimFound = 0;

	const char *tok = nullptr;
	size_t trailing_spaces = 0;
	char quote = 0;

	for ( ; lenSource && *source != 0; --lenSource, ++source ) {
		// check for protected character
		if ( *source == '\\' ) {
			if ( lenTok == 0 ) {
				tok = source;
			}
			else {
				++lenTok;
				trailing_spaces = 0;
			}

			// skip following character
			--lenSource; ++source;
			if ( lenSource && *source != 0 ) {
				++lenTok;
			}

			continue;
		}

		// check for unprotected delimiter outside of quotes
		if ( quote == 0 and strchr(delimiter, *source) != nullptr ) {
			delimFound = *source;
			--lenSource; ++source;
			lenTok -= trailing_spaces;
			return tok;
		}

		// check for terminating quote character
		if ( *source == quote ) {
			quote = 0;
		}
		// check for beginning quote
		else if ( quote == 0 and strchr(quotes, *source) != nullptr ) {
			quote = *source;
			trailing_spaces = 0;
		}
		// trimming outside unprotected characters outside of quotes
		else if ( trim ) {
			if ( !quote and strchr(whitespaces, *source) != nullptr ) {
				// trim leading whitespaces
				if ( lenTok == 0 )
					continue;
				// count trailing spaces
				else
					++trailing_spaces;
			}
			else {
				trailing_spaces = 0;
			}
		}

		// mark beginning of string
		if ( lenTok == 0 )
			tok = source;

		++lenTok;
	}

	lenTok -= trailing_spaces;
	return tok;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *tokenizeUnescape(size_t &lenTok, size_t &lenSource, char *&source,
                             char &delimFound, const char *delimiter,
                             bool trim, const char *whitespaces,
                             const char *quotes) {
	lenTok = 0;
	delimFound = 0;

	const char *tok = nullptr;
	char *tokEnd = nullptr;
	size_t trailing_spaces = 0;
	char quote = 0;

	for ( ; lenSource && *source != 0; --lenSource, ++source ) {
		// check for backslash character
		if ( *source == '\\' ) {
			// initialize token if not done so far
			if ( lenTok == 0 )
				tok = tokEnd = source;
			else
				*tokEnd = *source;
			++lenTok;

			// read next char
			--lenSource; ++source;

			// backslash was last char: do not unescape
			if ( !lenSource || *source == 0 ) {
				return tok;
			}

			// backslash outside quotes: unescape backslash, quotes,
			// delimiter and whitespaces
			if ( quote == 0 ) {
				if ( *source != '\\' &&
				     strchr(quotes, *source) == nullptr &&
				     strchr(delimiter, *source) == nullptr &&
				     strchr(whitespaces, *source) == nullptr ) {
					++tokEnd; ++lenTok;
				}
			}
			// backslash inside quotes: unescape backslash and start quote
			// character only
			else {
				if ( *source != '\\' && *source != quote ) {
					++tokEnd; ++lenTok;
				}
			}

			*tokEnd = *source;
			++tokEnd;
			trailing_spaces = 0;
			continue;
		}

		// check for unprotected delimiter outside of quotes
		if ( quote == 0 and strchr(delimiter, *source) != nullptr ) {
			delimFound = *source;
			--lenSource; ++source;
			lenTok -= trailing_spaces;
			return tok;
		}
		// check for terminating quote character
		else if ( *source == quote ) {
			quote = 0;
			trailing_spaces = 0;
			continue;
		}
		// check for beginning quote
		else if ( quote == 0 and strchr(quotes, *source) != nullptr ) {
			quote = *source;
			trailing_spaces = 0;
			continue;
		}
		// trim outside unprotected characters outside of quotes
		else if ( trim ) {
			if ( !quote and strchr(whitespaces, *source) != nullptr ) {
				// trim leading whitespaces
				if ( lenTok == 0 )
					continue;
				// count trailing spaces
				else
					++trailing_spaces;
			}
			else {
				trailing_spaces = 0;
			}
		}

		// mark beginning of string
		if ( lenTok == 0 ) {
			tok = source;
			tokEnd = source + 1;
		}
		// copy source char and advance end pointer of result string
		else {
			*tokEnd = *source;
			++tokEnd;
		}

		++lenTok;
	}

	lenTok -= trailing_spaces;
	return tok;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool isEmpty(const char* str) {
	return str == nullptr || *str == '\0';
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int compareNoCase(std::string_view a, std::string_view b) {
	auto it_a = a.begin(), it_b = b.begin();
	while ( it_a != a.end() && it_b != b.end() ) {
		char upper_a = static_cast<char>(toupper(*it_a));
		char upper_b = static_cast<char>(toupper(*it_b));
		if ( upper_a < upper_b ) {
			return -1;
		}
		else if ( upper_a > upper_b ) {
			return 1;
		}

		++it_a; ++it_b;
	}

	return it_a == a.end()?(it_b == b.end()?0:-1):(it_b == b.end()?1:0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string_view trim(std::string_view sv) {
	auto pos = sv.find_first_not_of(WHITESPACE);
	if ( pos == std::string::npos ) {
		// All whitespace characters
		return { sv.data(), 0 };
	}

	// Remove leading whitespaces
	sv = sv.substr(pos);

	pos = sv.find_last_not_of(WHITESPACE);
	if ( pos != std::string::npos ) {
		sv = sv.substr(0, pos + 1);
	}

	return sv;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string &trim(std::string &str) {
	auto pos = str.find_first_not_of(WHITESPACE);
	if ( pos == std::string::npos ) {
		// All whitespace characters
		str = std::string();
		return str;
	}

	str.erase(str.begin(), str.begin() + pos);
	pos = str.find_last_not_of(WHITESPACE);
	if ( pos != std::string::npos ) {
		str.erase(str.begin() + pos + 1, str.end());
	}

	return str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool isWhitespace(const char c) {
	if ( WHITESPACE.find_first_of(c) == std::string::npos )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool isWhitespace(const std::string& str) {
	std::string::const_iterator cIt = str.begin();
	for ( ; cIt != str.end(); ++cIt ) {
		if ( !isWhitespace(*cIt) )
			return false;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool wildcmp(const char *pat, const char *str) {
	const char *s, *p;
	bool star = false;

loopStart:
	for ( s = str, p = pat; *s; ++s, ++p ) {
		switch ( *p ) {
			case '?':
				break;
			case '*':
				star = true;
				str = s; pat = p;
				do { ++pat; } while (*pat == '*');
				if ( !*pat ) return true;
				goto loopStart;
			default:
				if ( *s != *p )
					goto starCheck;
				break;
		} /* endswitch */
	} /* endfor */

	while (*p == '*') ++p;

	return (!*p);

starCheck:
	if ( !star ) return false;
	++str;
	goto loopStart;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool wildcmp(const std::string &wild, const std::string &str) {
	return wildcmp(wild.c_str(), str.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool wildicmp(const char *pat, const char *str) {
	const char *s, *p;
	bool star = false;

loopStart:
	for ( s = str, p = pat; *s; ++s, ++p ) {
		switch ( *p ) {
			case '?':
				break;
			case '*':
				star = true;
				str = s; pat = p;
				do { ++pat; } while (*pat == '*');
				if ( !*pat ) return true;
				goto loopStart;
			default:
				if ( toupper(*s) != toupper(*p) )
					goto starCheck;
				break;
		} /* endswitch */
	} /* endfor */

	while (*p == '*') ++p;

	return (!*p);

starCheck:
	if ( !star ) return false;
	++str;
	goto loopStart;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool wildicmp(const std::string &wild, const std::string &str) {
	return wildicmp(wild.c_str(), str.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char *strnchr(char *p, size_t n, char c) {
	while ( n-- > 0 && *p != '\0' ) {
		if ( *p == c )
			return p;
		++p;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *strnchr(const char *p, size_t n, char c) {
	while ( n-- > 0 && *p != '\0' ) {
		if ( *p == c )
			return p;
		++p;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t digits10(float value, bool fractionOnly) {
	return fractionOnly ? getFixedDigits10(value) : getScientificDigits10(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t digits10(double value, bool fractionOnly) {
	return fractionOnly ? getFixedDigits10(value) : getScientificDigits10(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Core
} // namespace Seiscomp
