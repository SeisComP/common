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


#include <seiscomp/core/enumeration.h>

#include <cctype>
#include <sstream>
#include <complex>


namespace Seiscomp {
namespace Core {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline std::string toString(const T &v) {
	std::ostringstream os;
	os << Number<T>(v);
	return os.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
std::ostream &operator<<(std::ostream &ostream, Number<T> n) {
	ostream.precision(10);
	ostream << n.ref;
	return ostream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline std::string toString(const std::complex<T>& v) {
	std::ostringstream os;
	os << "(" << toString(v.real()) << "," << toString(v.imag()) << ")";
	return os.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
std::string toString(const Enum<ENUMTYPE, END, NAMES>& value) {
	return value.toString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline std::string toString(const std::vector<T>& v) {
	typename std::vector<T>::const_iterator it = v.begin();
	std::string str;
	if ( it != v.end() )
		str += toString(*it);
	else
		return "";

	++it;
	
	while ( it != v.end() ) {
		str += " ";
		str += toString(*it);
		++it;
	}

	return str;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline std::string toString(const ::boost::optional<T>& v) {
	if ( !v )
		return "None";

	return toString(*v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
bool fromString(Enum<ENUMTYPE, END, NAMES>& value, const std::string& str) {
	return value.fromString(str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline bool fromString(std::complex<T>& value, const std::string& str) {
	size_t s = str.find_first_not_of(' ');
	size_t e = str.find_last_not_of(' ');
	if ( s == std::string::npos || e == std::string::npos )
		return false;

	if ( str[s] != '(' || str[e] != ')' )
		return false;

	size_t delimPos = str.find(',', s+1);
	if ( delimPos == std::string::npos )
		return false;

	T realPart, imgPart;
	
	if ( !fromString(realPart, str.substr(s+1, delimPos-s-1)) ) return false;
	if ( !fromString(imgPart, str.substr(delimPos+1, e-delimPos-1)) ) return false;

	value = std::complex<T>(realPart, imgPart);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline bool fromString(std::vector<T>& vec, const std::string& str) {
	std::vector<std::string> tokens;
	split(tokens, str.c_str(), " ");
	for ( size_t i = 0; i < tokens.size(); ++i ) {
		T v;
		if ( !fromString(v, tokens[i]) )
			return false;
		vec.push_back(v);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline bool fromString(std::vector<std::complex<T> >& vec, const std::string& str) {
	std::vector<std::string> tokens;
	split(tokens, str.c_str(), " ");
	for ( size_t i = 0; i < tokens.size(); ++i ) {
		std::complex<T> v;
		int count = 1;

		size_t countPos = tokens[i].find_first_not_of(' ');
		if ( countPos != std::string::npos ) {
			if ( tokens[i][countPos] != '(' ) {
				size_t bracketPos = tokens[i].find('(', countPos);
				// Invalid complex string
				if ( bracketPos == std::string::npos ) continue;
				if ( !fromString(count, tokens[i].substr(countPos, bracketPos-countPos)) )
					return false;
				tokens[i] = tokens[i].substr(bracketPos);
			}
		}
		
		if ( !fromString(v, tokens[i]) ) return false;
		for ( int i = 0; i < count; ++i )
			vec.push_back(v);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline void toHex(std::string &target, T source) {
	static const char *hex = "0123456789ABCDEF";
	unsigned char *bytes = reinterpret_cast<unsigned char*>(&source);
	for ( size_t i = 0; i < sizeof(T); ++i ) {
		target += hex[bytes[i] & 0x0F];
		target += hex[(bytes[i] >> 4) & 0x0F];
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline T *tokenize(T *&str, const char *delim, size_t &len_source, size_t &len_tok) {
	len_tok = 0;
	for ( ; len_source; --len_source, ++str ) {
		// Hit first non delimiter?
		if ( strchr(delim, *str) == nullptr ) {
			T *tok = str;

			++str; --len_source;
			len_tok = 1;

			// Hit first delimiter?
			for ( ; len_source; --len_source, ++str, ++len_tok ) {
				if ( strchr(delim, *str) != nullptr )
					break;
			}
			return tok;
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline T *tokenize2(T *&str, const char *delim, size_t &len_source, size_t &len_tok) {
	len_tok = 0;

	if ( !len_source ) return nullptr;

	T *tok = str;
	for ( ; len_source; --len_source, ++str, ++len_tok ) {
		// Hit delimiter?
		if ( strchr(delim, *str) != nullptr ) {
			// Move over delim
			++str;
			--len_source;
			return tok;
		}
	}

	return tok;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline char *trimFront(char *&data, size_t &len) {
	while ( len ) {
		if ( !isspace(*data) ) {
			break;
		}
		else {
			++data;
			--len;
		}
	}
	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const char *trimFront(const char *&data, size_t &len) {
	while ( len ) {
		if ( !isspace(*data) ) {
			break;
		}
		else {
			++data;
			--len;
		}
	}
	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline char *trimBack(char *data, size_t &len) {
	while ( len ) {
		if ( !isspace(data[len-1]) ) {
			break;
		}
		else {
			--len;
		}
	}
	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const char *trimBack(const char *data, size_t &len) {
	while ( len ) {
		if ( !isspace(data[len-1]) ) {
			break;
		}
		else {
			--len;
		}
	}
	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline char *trim(char *&data, size_t &len) {
	trimFront(data, len);
	trimBack(data, len);
	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const char *trim(const char *&data, size_t &len) {
	trimFront(data, len);
	trimBack(data, len);
	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const char *advance(const char *&data, size_t &len, size_t offset) {
	data += offset;
	len -= offset;
	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <class CONT>
inline std::string join(const CONT &tokens, const char *separator) {
	static std::string defaultSeparator = ",";

	if ( !separator ) separator = defaultSeparator.c_str();

	std::string s;
	bool first = true;
	for ( auto &&item : tokens ) {
		if ( !first )
			s += separator;
		else
			first = false;
		s += item;
	}
	return s;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <class CONT>
std::string join(const CONT &tokens, const std::string &separator) {
	return join(tokens, separator.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
