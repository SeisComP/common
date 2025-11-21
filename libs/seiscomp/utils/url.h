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


#ifndef SEISCOMP_UTILS_URL_H
#define SEISCOMP_UTILS_URL_H


#include <seiscomp/core.h>
#include <seiscomp/core/enumeration.h>

#include <map>
#include <string>
#include <string_view>


namespace Seiscomp {
namespace Util {


/**
 * @brief The Url class provides an interface to parse an URL.
 *
 * The general layout of an URL is:
 * scheme://username:password@host:port/path?query#fragment
 *
 * There is no support for the generic form:
 * scheme:<scheme-specific-part>
 *
 * The scheme must be separated from the scheme specific part
 * with "://".
 */
class SC_SYSTEM_CORE_API Url {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		using QueryItems = std::map<std::string, std::string>;

		MAKEENUM(
			Status,
			EVALUES(
				STATUS_OK,
				STATUS_ERROR,
				STATUS_EMPTY,
				STATUS_SCHEME_ERROR,
				STATUS_EMPTY_USER_INFO,
				STATUS_EMPTY_USERNAME,
				STATUS_INVALID_HOST,
				STATUS_PORT_IS_NO_NUMBER,
				STATUS_PORT_OUT_OF_RANGE,
				STATUS_INVALID_QUERY
			),
			ENAMES(
				"OK",
				"Error",
				"URL is empty",
				"Scheme is required",
				"Empty user info not allowed",
				"Empty username not allowed",
				"Invalid host",
				"Expected port as number",
				"Port out of range",
				"Invalid query"
			)
		);

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Url(std::string_view url = {});
		Url(std::string_view scheme,
		    std::string_view username, std::string_view password,
		    std::string_view host, OPT(uint16_t) port,
		    std::string_view path, std::string_view fragment,
		    QueryItems queryItems);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:

		/**
		 * @brief Sets a URL from string and decomposes it into its parts.
		 * @param url The URL string
		 * @return true if the URL could be parsed, false in case of any
		 *         invalid URL.
		 */
		bool setUrl(std::string_view url);

		/**
		 * @brief Checks if the URL is valid
		 * @return True is the URL is not empty and valid.
		 */
		bool isValid() const;

		/**
		 * @brief Returns the string representation of the URL.
		 * @note After setting individual member, encode() must be called to update
		 *       the internal string!
		 * @return The URL as string
		 */
		const std::string &toString() const;

		/**
		 * @brief Sets the URL scheme.
		 * @param The URL scheme
		 */
		Url &setScheme(std::string_view scheme);

		/**
		 * @brief Returns the URL scheme
		 * @return The URL scheme
		 */
		const std::string &scheme() const;

		/**
		 * @brief Returns the authority part of the URL
		 * @return The authority
		 */
		const std::string &authority() const;

		/**
		 * @brief Sets the username
		 * @param username The username
		 */
		Url &setUsername(std::string_view username);

		/**
		 * @brief Returns the username as extracted from the user info
		 * @return The username
		 */
		const std::string &username() const;

		/**
		 * @brief Sets the password
		 * @param password The password
		 */
		Url &setPassword(std::string_view password);

		/**
		 * @brief Returns the password as extracted from the user info
		 * @return  The password
		 */
		const std::string &password() const;

		/**
		 * @brief Sets the host
		 * @param host The host
		 */
		Url &setHost(std::string_view host);

		/**
		 * @brief Returns the host from the authority part
		 * @return The host, IP or host name
		 */
		const std::string &host() const;

		/**
		 * @brief Sets the port.
		 * @param port The optional port
		 */
		Url &setPort(OPT(uint16_t) port);

		/**
		 * @brief Returns the optional port from authority part.
		 * @return Returns the port
		 */
		OPT(uint16_t) port() const;

		/**
		 * @brief Set the path.
		 * @param path The path
		 */
		Url &setPath(std::string_view path);

		/**
		 * @brief Returns the path of the URL. If the scheme and authority are
		 * not set the path can be interpreted as file location
		 * @return The path
		 */
		const std::string &path() const;

		/**
		 * @brief Returns the query string
		 * @return The query string
		 */
		const std::string &query() const;

		/**
		 * @brief Sets the query items.
		 * @param items The query items
		 */
		Url &setQueryItems(QueryItems items);

		/**
		 * @brief Checks if a specific query parameter is set
		 * @param The query item key
		 * @return True if the parameter is set
		 */
		bool hasQueryItem (const std::string &key) const;

		/**
		 * @brief Returns the value of a specific query parameter
		 * @param The query item key
		 * @return The value
		 */
		std::string queryItemValue(const std::string &key) const;

		/**
		 * @brief Returns a reference to a query item map
		 * @return Reference to query item map
		 */
		const QueryItems &queryItems() const;

		/**
		 * @brief Sets the fragment.
		 * @param fragment The fragment
		 */
		void setFragment(std::string_view fragment);

		/**
		 * @brief Returns the fragement part of the URL
		 * @return The fragment
		 */
		const std::string &fragment() const;

		/**
		 * @brief Returns the status of the URL
		 * @return The status
		 */
		Status status() const;

		/**
		 * @brief Print parsed URL components
		 */
		void debug() const;

		/**
		 * @brief Returns the current URL without scheme, e.g. turning
		 *        http://localhost into localhost.
		 * @return The URL without scheme.
		 */
		std::string withoutScheme() const;

		/**
		 * @brief Encodes the URL from its attributes.
		 * This method combines the following attributes:
		 * - scheme
		 * - username
		 * - password
		 * - host
		 * - port
		 * - path
		 * - fragment
		 * - query items
		 * Each of the values is properly encoded.
		 * Note that query items are sorted by key when they are encoded in
		 * the final URL.
		 * After this method was called, the
		 */
		Url &encode();

		/**
		 * @brief Encodes C string based as defined in RFC 3986:
		 * RFC 3986 section 2.2 Reserved Characters (January 2005) and
		 * RFC 3986 section 2.3 Unreserved Characters (January 2005)
		 *
		 * @note This method does not take & (ampersand) into account.
		 * @param s C string
		 * @return Encoded STL string
		 */
		static std::string Encode(std::string_view sv);

		/**
		 * @brief Encodes C string based as defined in RFC 3986:
		 * RFC 3986 section 2.2 Reserved Characters (January 2005) and
		 * RFC 3986 section 2.3 Unreserved Characters (January 2005)
		 *
		 * @note This method takes & (ampersand) into account.
		 * @param s C string
		 * @return Encoded STL string
		 */
		static std::string EncodeComponent(std::string_view sv);

		/**
		 * @brief Decodes C string based as defined in RFC 3986 defined:
		 * RFC 3986 section 2.2 Reserved Characters (January 2005) and
		 * RFC 3986 section 2.3 Unreserved Characters (January 2005)
		 * @param s C string
		 * @return Encoded STL string
		 */
		static std::string Decode(std::string_view sv);


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		operator bool() const;

		bool operator==(std::string_view sv) const;
		bool operator!=(std::string_view sv) const;


	// ----------------------------------------------------------------------
	//  Protected methods
	// ----------------------------------------------------------------------
	protected:
		/**
		 * @brief Resets all URL components
		 */
		void reset();

		/**
		 * @brief Parses URL from string. Components and sub components are
		 * parsed and separated before those are decoded.
		 * @param url The URL
		 * @param implyAuthority Whether to imply an authority if no scheme is
		 *                       given.
		 * @return Status code
		 */
		Status parse(std::string_view url, bool implyAuthority);

		/**
		 * @brief Parses scheme from URL
		 * @param url The URL
		 * @return Status code
		 */
		Status parseScheme(std::string_view url);

		/**
		 * @brief Parses authority from URL
		 * @param The URL
		 * @return Status code
		 */
		Status parseAuthority(std::string_view url);

		/**
		 * @brief Parses path from URL
		 * @param The URL
		 * @return Status code
		 */
		Status parsePath(std::string_view url);

		/**
		 * @brief Parses query from the URL and builds key/value param lookup
		 * @return Status code
		 */
		Status parseQuery();

		/**
		 * @brief Loads scheme specific default values, e.g., for HTTP port 80
		 */
		void setSchemeDefaults();


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	protected:
		std::string     _url;
		std::string     _scheme;
		std::string     _authority;
		std::string     _user;
		std::string     _password;
		std::string     _host;
		OPT(uint16_t)   _port;
		std::string     _path;
		std::string     _query;
		std::string     _fragment;
		QueryItems      _queryItems;
		bool            _isValid{false};
		std::string     _errorString;
		size_t          _currentPos{0};
		Status          _status{STATUS_EMPTY};
};



inline bool Url::isValid() const {
	return _isValid;
}

inline Url::operator bool() const {
	return isValid();
}

inline const std::string &Url::toString() const {
	return _url;
}

inline bool Url::operator==(std::string_view sv) const {
	return _url == sv;
}

inline bool Url::operator!=(std::string_view sv) const {
	return _url != sv;
}


}
}


#endif
