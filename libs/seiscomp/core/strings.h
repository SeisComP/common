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


#ifndef SEISCOMP_CORE_STRINGS_H
#define SEISCOMP_CORE_STRINGS_H


#include <seiscomp/core/optional.h>
#include <seiscomp/core/enumeration.h>
#include <seiscomp/core/datetime.h>

#include <boost/iostreams/stream.hpp>
#include <string>
#include <vector>
#include <complex>


namespace Seiscomp {
namespace Core {


/** Array of whitespace characters */
const std::string WHITESPACE = "\t\n\v\f\r ";


/**
 * Converts a value into a string. Conversions are supported
 * for following types:
 *   char
 *   int, long
 *   float, double
 *   Core::Time, std::complex
 *   and any other type that is supported by std::ostream
 *   std::vector and OPT() of all above types
 *
 * @param value The value
 * @return The value as string
 */
template <typename T>
std::string toString(const T &value);

template <typename T>
std::string toString(const std::complex<T> &value);

SC_SYSTEM_CORE_API std::string toString(const std::string &value);
SC_SYSTEM_CORE_API std::string toString(bool value);
SC_SYSTEM_CORE_API std::string toString(const Time &value);
SC_SYSTEM_CORE_API std::string toString(const Enumeration &value);

template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
std::string toString(const Enum<ENUMTYPE, END, NAMES> &value);

template <typename T>
std::string toString(const std::vector<T> &v);

template <typename T>
std::string toString(const ::boost::optional<T> &v);


template <typename T>
struct Number {
	Number(const T &v) : ref(v) {}
	const T &ref;
};

template <typename T>
Number<T> number(const T &v) { return Number<T>(v); }

template <typename T>
std::ostream &operator<<(std::ostream &ostream, Number<T> s);

/**
 * Converts a string into a value. Conversions are supported
 * for following types:
 *   int/uint with 8,16,32 and 64 bit
 *   float, double
 *   std::vector of all above types
 * IMPORTANT: integer types are converted in base 10!
 *
 * @param value The target value
 * @param str The source string
 */
template <typename T>
bool fromString(T &value, const std::string &str);

template <typename T>
bool fromString(std::complex<T> &value, const std::string &str);

SC_SYSTEM_CORE_API bool fromString(Time &value, const std::string &str);
SC_SYSTEM_CORE_API bool fromString(Enumeration &value, const std::string &str);
SC_SYSTEM_CORE_API bool fromString(std::string &value, const std::string &str);

template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
bool fromString(Enum<ENUMTYPE, END, NAMES> &value, const std::string &str);

template <typename T>
bool fromString(std::vector<T> &vec, const std::string &str);


/**
 * @brief Produces output according to a format as used by printf. The output
 *        is written to a string and returned.
 * @param fmt A format description as used by printf
 * @return The string containing the output
 */
SC_SYSTEM_CORE_API std::string stringify(const char *fmt, ...);

/**
 * @brief Splits a string into several tokens separated by one of the specified
 *        delimiter characters.
 * @param tokens Result vector containing the individual tokens
 * @param source The source string
 * @param delimiter Sequence of characters to spit the string at
 * @param compressOn If enabled, adjacent separators are merged together.
 *        Otherwise, every two separators delimit a token.
 * @return The number of generated tokens.
 */
SC_SYSTEM_CORE_API
int split(std::vector<std::string> &tokens, const char *source,
          const char *delimiter, bool compressOn = true);

/**
 * @brief Convenience function which takes an std::string as source parameter
 *        rather than a const char pointer.
 *        See @ref split(std::vector<std::string> &, const char *, const char *, bool).
 */
SC_SYSTEM_CORE_API
int split(std::vector<std::string> &tokens, const std::string &source,
          const char *delimiter, bool compressOn = true);

/**
 * @brief Splits a string into several tokens separated by one of the specified
 *        delimiter characters. A delimiter character is ignored if it occurs in
 *        a quoted string or if it is protected by a backslash. Likewise quotes
 *        may be protected by a backslash. By default, leading and trailing
 *        whitespaces will be trimmed if they occur outside of a quoted string
 *        and if they are not protected by a backslash.
 * @param tokens Result vector containing the individual tokens
 * @param source The source string
 * @param delimiter Sequence of characters to spit the string at
 * @param compressOn If enabled, adjacent separators are merged together.
 *        Otherwise, every two separators delimit a token.
 * @param unescape Request removal of surrounding quotes and unescape of certain
 *        characters.
 *        In particular a backslash outside quotes is removed if it protects a
 *        backslash, a whitespace, a quote or a delimiter character. While a
 *        backslash inside quotes only is removed if it protects a backslash or
 *        the beginning quote character.
 * @param trim Request trimming of whitespaces
 * @param whitespace Sequence of characters to interpret as a whitespace
 * @param quotes Sequence of characters to interpret as a quote
 * @return Number of tokens found
 */
SC_SYSTEM_CORE_API
size_t splitExt(std::vector<std::string> &tokens, const char *source,
                const char *delimiter = ",", bool compressOn = true,
                bool unescape = false, bool trim = true,
                const char *whitespaces = " \t\n\v\f\r",
                const char *quotes = "\"'");

/**
 * @brief Creates and returns a new string by concatenating all of the elements
 *        in an array, separated by commas or a specified separator string.
 *
 * If the array has only one item, then that item will be returned without
 * using the separator.
 *
 * @param tokens[in] The array to be joined
 * @param separator[in] The optional separator string
 * @return The resulting string
 */
SC_SYSTEM_CORE_API
template <class CONT>
std::string join(const CONT &tokens, const char *separator = nullptr);

SC_SYSTEM_CORE_API
template <class CONT>
std::string join(const CONT &tokens, const std::string &separator);

SC_SYSTEM_CORE_API bool isEmpty(const char*);

/**
 * A case-insensitive comparison.
 * @return Result as defined by strcmp
 */
SC_SYSTEM_CORE_API int compareNoCase(const std::string &a, const std::string &b);

/** Removes whitespace at the beginning and end of the string.
 * @param string to be trimmed (in/out parameter)
 * @return returns the trimmed string
 */
SC_SYSTEM_CORE_API std::string &trim(std::string &str);

template <typename T>
void toHex(std::string &target, T source);

/** Checks if given character is whitespace */
bool isWhitespace(const char c);

/** Checks if the given string solely contanins whitespace */
bool isWhitespace(const std::string &str);

/** wildcmp() compares a string containing wildcards with another
 * string where '?' represents a single character and '*'
 * represents zero to unlimited characters.
 * wildicmp() performs the same operation, but is case insensitive
 * This code has been written by Alessandro Cantatore
 * http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html
 * @param wild The string containing the wildcards
 * @param str The string checked against the wildcard string
 * @return The match result
 */
SC_SYSTEM_CORE_API bool wildcmp(const char *wild, const char *str);
SC_SYSTEM_CORE_API bool wildcmp(const std::string &wild, const std::string &str);
SC_SYSTEM_CORE_API bool wildicmp(const char *wild, const char *str);
SC_SYSTEM_CORE_API bool wildicmp(const std::string &wild, const std::string &str);


// -------------------------------------------------------------------------
//  Plain C string functions which do not modify the input string and work
//  mostly with length rather than an terminating null byte.
// -------------------------------------------------------------------------

/**
 * @brief Tokenizes an input string. Empty tokens will be skipped and not
 *        returned (also referred to as compression).
 * @param str The input string. The address is modified that it will point to
 *            the next token.
 * @param delim A string of characters of allowed delimiters
 * @param len_source The source length. This parameter will be modified
 *                   to match the remaining length of the string.
 * @param len_tok The length of the returned token.
 * @return The address to the token found or nullptr.
 */
template <typename T>
T *tokenize(T *&str, const char *delim, size_t &len_source, size_t &len_tok);

/**
 * @brief Works like tokenize but does not compress empty tokens.
 * @param str The input string. The address is modified that it will point to
 *            the next token.
 * @param delim A string of characters of allowed delimiters
 * @param len_source The source length. This parameter will be modified
 *                   to match the remaining length of the string.
 * @param len_tok The length of the returned token.
 * @return The address to the token found or nullptr.
 */
template <typename T>
T *tokenize2(T *&str, const char *delim, size_t &len_source, size_t &len_tok);

/**
 * @brief Splits a string into several tokens separated by one of the specified
 *        delimiter characters. A delimiter character is ignored if it occurs in
 *        a quoted string or if it is protected by a backslash. Likewise quotes
 *        may be protected by a backslash. By default, leading and trailing
 *        whitespaces will be trimmed if they occur outside of a quoted string
 *        and if they are not protected by a backslash.
 *        In contrast to the splitExt method this function operates entirely on
 *        the source string without modifying it or creating any copies.
 * @param lenSource Returns remaining length of source string
 * @param lenTok Returns length of current token
 * @param source The source string
 * @param delimFound Returns the delimiter character found or 0 if the end of
 *        the source string was reached
 * @param delimiter Sequence of characters to spit the string at
 * @param trim Request trimming of whitespaces
 * @param whitespace Sequence of characters to interpret as a whitespace
 * @param quotes Sequence of characters to interpret as a quote
 * @return Pointer to the next token within the source string, length of the
 *         token and number of remaining characters in the source string.
 */
SC_SYSTEM_CORE_API
const char *tokenizeExt(size_t &lenTok, size_t &lenSource, const char *&source,
                        char &delimFound, const char *delimiter = ",",
                        bool trim = true,
                        const char *whitespaces = " \t\n\v\f\r",
                        const char *quotes = "\"'");

/**
 * @brief Splits a string into several tokens separated by one of the specified
 *        delimiter characters. A delimiter character is ignored if it occurs in
 *        a quoted string or if it is protected by a backslash. Likewise quotes
 *        may be protected by a backslash. By default, leading and trailing
 *        whitespaces will be trimmed if they occur outside of a quoted string
 *        and if they are not protected by a backslash.
 *        Unlike the tokenizeExt variant this function will modify the source
 *        string and remove surrounding quotes and will unescape certain
 *        characters.
 *        In particular a backslash outside quotes is removed if it protects a
 *        backslash, a whitespace, a quote or a delimiter character. While a
 *        backslash inside quotes only is removed if it protects a backslash or
 *        the beginning quote character.
 * @param lenSource Returns remaining length of source string
 * @param lenTok Returns length of current token
 * @param source The source string
 * @param delimFound Returns the delimiter character found or 0 if the end of
 *        the source string was reached
 * @param delimiter Sequence of characters to spit the string at
 * @param trim Request trimming of whitespaces
 * @param whitespace Sequence of characters to interpret as a whitespace
 * @param quotes Sequence of characters to interpret as a quote
 * @return Pointer to the next token within the source string, length of the
 *         token and number of remaining characters in the source string.
 */
SC_SYSTEM_CORE_API
const char *tokenizeUnescape(size_t &lenTok, size_t &lenSource, char *&source,
                             char &delimFound, const char *delimiter = ",",
                             bool trim = true,
                             const char *whitespaces = " \t\n\v\f\r",
                             const char *quotes = "\"'");

SC_SYSTEM_CORE_API bool isEmpty(const char*);

/**
 * @brief Removes whitespaces from the front of a string.
 * @param data The data pointer which will be advanced until the first
 *             non-whitespace was found.
 * @param len The length of the input string which will hold the effective length
 *            if the trimmed string.
 * @return The pointer to the first non-whitespace character.
 */
char *trimFront(char *&data, size_t &len);
const char *trimFront(const char *&data, size_t &len);

/**
 * @brief Removes whitespaces from the back of a string.
 * @param data The input data pointer.
 * @param len The length of the input string which will hold the effective length
 *            if the trimmed string.
 * @return The input pointer.
 */
char *trimBack(char *data, size_t &len);
const char *trimBack(const char *data, size_t &len);

/**
 * @brief Strips whitespaces from the front and the back of the string.
 * @param data The data pointer which will be advanced until the first
 *             non-whitespace was found.
 * @param len The length of the input string which will hold the effective length
 *            if the trimmed string.
 * @return The pointer to the first non-whitespace character.
 */
char *trim(char *&data, size_t &len);
const char *trim(const char *&data, size_t &len);

char *strnchr(char *p, size_t n, char c);
const char *strnchr(const char *p, size_t n, char c);

/**
 * @brief Advances a data pointer
 * @param data Pointer to beginning of the string
 * @param len Length of input and output string.
 * @param offset The number of characters to advance the data pointer.
 * @return The pointer to the advanced string.
 */
const char *advance(const char *&data, size_t &len, size_t offset);


/**
 * @brief A helper class that allows to write to STL containers in
 *        combination with boost iostreams.
 */
template<typename Container>
class ContainerSink {
	public:
		typedef typename Container::value_type  char_type;
		typedef boost::iostreams::sink_tag      category;
		ContainerSink(Container &container)
		: _container(container) { }

		std::streamsize write(const char_type *s, std::streamsize n) {
			_container.insert(_container.end(), s, s + n);
			return n;
		}

		Container &container() { return _container; }

	private:
		ContainerSink operator=(const ContainerSink&);
		Container &_container;
};


/**
 * @brief A helper class that allows to read from STL containers in
 *        combination with boost iostreams.
 */
template<typename Container>
class ContainerSource {
	public:
		typedef typename Container::value_type  char_type;
		typedef boost::iostreams::source_tag    category;
		ContainerSource(const Container &container)
		: _container(container), _pos(0) {}

		std::streamsize read(char_type* s, std::streamsize n) {
			std::streamsize amt = static_cast<std::streamsize>(_container.size() - _pos);
			std::streamsize result = (std::min)(n, amt);
			if ( result != 0 ) {
					copy(_container.begin() + _pos, _container.begin() + _pos + result, s);
					_pos += result;
					return result;
			}
			return -1; // EOF
		}

		Container &container() { return _container; }

	private:
		typedef typename Container::size_type   size_type;
		const Container &_container;
		size_type        _pos;
};


}
}


#include <seiscomp/core/strings.ipp>

#endif
