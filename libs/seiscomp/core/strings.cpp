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
#include <sstream>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cerrno>
#include <cmath>
#include <limits>
#include <type_traits>


#define AUTO_FLOAT_PRECISION 1


namespace Seiscomp {
namespace Core {


namespace {


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
    "%Y-%j",        // YYYY-DDD
    "%F %T.%f",     // YYYY-MM-DD hh:mm:ss.ssssss
    "%F %T",        // YYYY-MM-DD hh:mm:ss
    "%F %R",        // YYYY-MM-DD hh:mm
    "%F %H",        // YYYY-MM-DD hh
    "%F",           // YYYY-MM-DD
    "%Y",           // YYYY
};


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
	return v.toString(timeFormats[0]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string toString(const Enumeration& value) {
	return value.toString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

template <typename T, int UNSIGNED>
struct Converter {};

// Signed conversion
template <typename T>
struct Converter<T, 0> {
	static inline bool convert(T &value, const std::string &str) {
		char* endptr = nullptr;
		errno = 0;
		long long retval = strtoll(str.c_str(), &endptr, 10);
		if ( errno != 0 )
			return false;

		if ( endptr && (&str[0] + str.size() != endptr) )
			return false;

		if ( retval < std::numeric_limits<T>::min()
		  || retval > std::numeric_limits<T>::max() ) {
			errno = ERANGE;
			return false;
		}

		value = static_cast<T>(retval);
		return true;
	}
};

// Unsigned conversion
template <typename T>
struct Converter<T, 1> {
	static inline bool convert(T &value, const std::string &str) {
		char* endptr = nullptr;
		errno = 0;
		long long retval = strtoll(str.c_str(), &endptr, 10);
		if ( errno != 0 )
			return false;

		if ( endptr && (&str[0] + str.size() != endptr) )
			return false;

		if ( retval < 0 )
			return false;

		if ( static_cast<unsigned long long>(retval) > std::numeric_limits<T>::max() ) {
			errno = ERANGE;
			return false;
		}

		value = static_cast<T>(retval);
		return true;
	}
};

template <typename T>
inline bool convertFromString(T &value, const std::string &str) {
	if ( str.empty() ) return false;
	return Converter<T, std::is_unsigned<T>::value>::convert(value, str);
}

}

template <>
bool fromString(char &value, const std::string &str) {
	return convertFromString(value, str);
}
template <>
bool fromString(signed char &value, const std::string &str) {
	return convertFromString(value, str);
}
template <>
bool fromString(unsigned char &value, const std::string &str) {
	return convertFromString(value, str);
}
template <>
bool fromString(short &value, const std::string &str) {
	return convertFromString(value, str);
}
template <>
bool fromString(unsigned short &value, const std::string &str) {
	return convertFromString(value, str);
}
template <>
bool fromString(int &value, const std::string& str) {
	return convertFromString(value, str);
}
template <>
bool fromString(unsigned int &value, const std::string &str) {
	return convertFromString(value, str);
}
template <>
bool fromString(long &value, const std::string& str) {
	return convertFromString(value, str);
}
template <>
bool fromString(unsigned long &value, const std::string& str) {
	return convertFromString(value, str);
}
template <>
bool fromString(long long &value, const std::string &str) {
	return convertFromString(value, str);
}
template <>
bool fromString(unsigned long long &value, const std::string& str) {
	return convertFromString(value, str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(float &value, const std::string &str) {
	char* endptr = nullptr;

	if ( str.empty() )
		return false;

	errno = 0;
	double retval = strtod(str.c_str(), &endptr);

	if ( errno != 0 )
		return false;

	if ( endptr && (&str[0] + str.size() != endptr) )
		return false;

	if ( std::isnormal(retval) ) {
		double aretval = std::fabs(retval);
		if ( aretval < static_cast<double>(std::numeric_limits<float>::min())
		  || aretval > static_cast<double>(std::numeric_limits<float>::max()) ) {
			errno = ERANGE;
			return false;
		}
	}

	value = static_cast<float>(retval);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(double &value, const std::string &str) {
	char* endptr = nullptr;

	if ( str.empty() )
		return false;

	errno = 0;
	value = strtod(str.c_str(), &endptr);

	if ( errno != 0 )
		return false;

	if ( endptr && (&str[0] + str.size() != endptr) )
		return false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
bool fromString(bool &value, const std::string &str) {
	char* endptr = nullptr;
	errno = 0;

	if ( str.empty() )
		return false;

	if ( compareNoCase(str, "true") == 0 ) {
		value = true;
		return true;
	}

	if ( compareNoCase(str, "false") == 0 ) {
		value = false;
		return true;
	}

	long int retval = strtol(str.c_str(), &endptr, 10);

	if ( errno != 0 )
		return false;

	if ( endptr && (&str[0] + str.size() != endptr) )
		return false;

	value = retval ? true : false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool fromString(Time &value, const std::string &str) {
	for ( size_t i = 0; i < sizeof(timeFormats) / sizeof(const char*); ++i ) {
		if ( value.fromString(str.c_str(), timeFormats[i]) ) {
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool fromString(Enumeration &value, const std::string &str) {
	return value.fromString(str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool fromString(std::string &value, const std::string &str) {
	value.assign(str);
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
int compareNoCase(const std::string& a, const std::string& b) {
	std::string::const_iterator it_a = a.begin(), it_b = b.begin();
	while ( it_a != a.end() && it_b != b.end() ) {
		char upper_a = static_cast<char>(toupper(*it_a));
		char upper_b = static_cast<char>(toupper(*it_b));
		if ( upper_a < upper_b )
			return -1;
		else if ( upper_a > upper_b )
			return 1;

		++it_a; ++it_b;
	}

	return it_a == a.end()?(it_b == b.end()?0:-1):(it_b == b.end()?1:0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string& trim(std::string& str) {
	/*
	const char whitespace[] = "\t\n\v\f\r ";

	std::string::size_type pos;
	pos = str.find_first_not_of(whitespace);
	if (pos != 0) str.erase(0, pos);

	pos = str.find_last_not_of(whitespace);
	if (pos != std::string::npos) str.erase(pos + 1, std::string::npos);
	*/
	boost::trim(str);
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
