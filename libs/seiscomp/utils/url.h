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


#include <map>
#include <string>


namespace Seiscomp {
namespace Util {


/**
 * @brief The Url class provides an interface to parse an URL.
 *
 * The general layout of an URL is:
 * scheme://username:password@host:port/path?query#fragment
 */
class SC_SYSTEM_CORE_API Url {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef std::map<std::string, std::string> QueryItems;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Url(const std::string &url = std::string());


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:

		/**
		 * @brief Sets a URL from string and decomposes it into its parts.
		 * @param url The URL string
		 * @return true if the URL could be parsed, false in case of an
		 *         invalid URL.
		 */
		bool setUrl(const std::string &url);

		const std::string &scheme() const;
		const std::string &username() const;
		const std::string &password() const;
		const std::string &host() const;
		size_t port() const;
		const std::string &path() const;
		const std::string &query() const;

		bool hasQuery() const;
		bool hasQueryItem (const std::string&) const;
		std::string queryItemValue(const std::string&) const;
		const QueryItems &queryItems() const;

		const std::string &fragment() const;

		bool isValid() const;
		void debug() const;

		/**
		 * @brief Returns the current URL without scheme, e.g. turning
		 *        http://localhost into localhost.
		 * @return The URL without scheme.
		 */
		std::string withoutScheme() const;


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		operator bool() const;
		operator const char*() const;
		operator const std::string &() const;


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		bool parse(const std::string&);
		std::string subString(const std::string&, size_t&, const std::string& = "", 
		                      bool required = false);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		std::string     _url;
		std::string     _scheme;
		std::string     _username;
		std::string     _password;
		std::string     _host;
		size_t          _port;
		std::string     _path;
		std::string     _query;
		std::string     _fragment;
		QueryItems      _queryItems;
		bool            _isValid;
};


inline Url::operator bool() const {
	return _isValid;
}

inline Url::operator const char*() const {
	return _url.c_str();
}

inline Url::operator const std::string &() const {
	return _url;
}


}
}


#endif
