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


#include <seiscomp/utils/url.h>
#include <seiscomp/core/strings.h>

#include <iostream>


namespace sc = Seiscomp::Core;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Url(const std::string &url) {
	setUrl(url);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Url::setUrl(const std::string &url) {
	reset();

	_url = url;
	if ( _url.empty() ) {
		_status = STATUS_EMPTY;
		return false;
	}

	auto ret = parse(_url, true);
	_isValid = ret == STATUS_OK;

	return _isValid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::scheme() const {
	return _scheme;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::authority() const {
	return _authority;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::username() const {
	return _user;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::password() const {
	return _password;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::host() const {
	return _host;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(uint16_t) Url::port() const {
	return _port;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::path() const {
	return _path;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::query() const {
	return _query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::fragment() const {
	return _fragment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Url::hasQueryItem(const std::string &key) const {
	return _queryItems.find(key) != _queryItems.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::queryItemValue(const std::string &key) const{
	const auto it = _queryItems.find(key);
	if ( it != _queryItems.end() ) {
		return it->second;
	}

	return {};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Url::QueryItems &Url::queryItems() const {
	return _queryItems;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Url::isValid() const {
	return _isValid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::status() const {
	return _status;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Url::reset() {
	_scheme = {};
	_authority = {};
	_user = {};
	_password = {};
	_host = {};
	_port = Core::None;
	_path = {};
	_query = {};
	_queryItems.clear();
	_fragment = {};
	_isValid = false;
	_currentPos = 0;
	_status = STATUS_EMPTY;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parseScheme(const std::string &url) {
	auto end = url.find(":");
	if ( end == std::string::npos ) {
		return STATUS_SCHEME_ERROR;
	}

	_scheme = url.substr(0, end);
	if ( _scheme.empty() ) {
		return STATUS_SCHEME_ERROR;
	}

	_scheme = Decoded(_scheme);

	_currentPos = end + 1;
	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parseAuthority(const std::string &url) {
	auto slashPos = url.find("/", _currentPos);
	auto markPos = url.find("?", _currentPos);
	auto hashPos = url.find("#", _currentPos);

	auto end = std::min(slashPos, std::min(markPos, hashPos));
	if ( end == std::string::npos ) {
		end = url.size();
	}

	_authority = url.substr(_currentPos, end - _currentPos);
	if ( _authority.empty() ) {
		return STATUS_OK;
	}

	_currentPos = end;

	auto userInfoEnd = _authority.find("@");
	if ( userInfoEnd != std::string::npos ) {
		std::string userInfo = _authority.substr(0, userInfoEnd);
		if ( userInfo.empty() ) {
			return STATUS_EMPTY_USER_INFO;
		}

		end = userInfo.find(":");
		if ( end != std::string::npos ) {
			_user = Decoded(userInfo.substr(0, end));
			if ( _user.empty() ) {
				return STATUS_EMPTY_USERNAME;
			}

			if ( userInfo.size() > end ) {
				_password = Decoded(userInfo.substr(end + 1, userInfo.size() - end));
			}
		}
		else {
			_user = Decoded(userInfo);
		}

		++userInfoEnd;
	}
	else {
		userInfoEnd = 0;
	}

	if ( userInfoEnd < _authority.size() && _authority[userInfoEnd] == '[' ) {
		// IPv6 specification
		end = _authority.find("]", userInfoEnd + 1);
		if ( end == std::string::npos ) {
			return STATUS_INVALID_HOST;
		}

		_authority.erase(_authority.begin() + userInfoEnd);
		--end;
		_authority.erase(_authority.begin() + end);

		if ( end < _authority.size() ) {
			if ( _authority[end] != ':' ) {
				return STATUS_INVALID_HOST;
			}
		}
		else {
			end = std::string::npos;
		}
	}
	else {
		end = _authority.find(":", userInfoEnd);
	}

	if ( end != std::string::npos ) {
		_host = Decoded(_authority.substr(userInfoEnd, end));
		if ( _host.empty() ) {
			return STATUS_INVALID_HOST;
		}

		++end;
		std::string portStr = _authority.substr(end, _authority.size() - end);
		if ( !portStr.empty() ) {
			int port = -1;
			if ( !sc::fromString(port, portStr) ) {
				return STATUS_PORT_IS_NO_NUMBER;
			}

			if ( (port < 0) || (port > 65535) ) {
				return STATUS_PORT_OUT_OF_RANGE;
			}

			_port = port;
		}
	}
	else {
		_host = Decoded(_authority.substr(userInfoEnd, _authority.size() - userInfoEnd));
	}

	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parsePath(const std::string &url) {
	auto markPos = url.find("?", _currentPos);
	auto hashPos = url.find("#", _currentPos);
	auto end = std::min(markPos, hashPos);
	if ( end != std::string::npos ) {
		_path = url.substr(_currentPos, end - _currentPos);
		_currentPos = end + 1;
	}
	else {
		_path = url.substr(_currentPos, url.size() - _currentPos);
		_currentPos = std::string::npos;
	}

	_path = Decoded(_path);

	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parseQuery(const std::string &url) {
	size_t start = 0;
	do {
		auto end = _query.find('=', start);
		if ( end == std::string::npos ) {
			break;
		}

		std::string key = Decoded(_query.substr(start, end - start));
		if ( key.empty() ) {
			break;
		}

		start = end + 1;
		if ( start == std::string::npos ) {
			break;
		}

		end = _query.find('&', start);
		if ( end == std::string::npos )
			_queryItems[key] = Decoded(_query.substr(start));
		else {
			_queryItems[key] = Decoded(_query.substr(start, end - start));
			++end;
		}

		start = end;
	}
	while ( start != std::string::npos );

	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parse(const std::string &url, bool implyAuthority) {
	auto pAuthority = url.find("://");
	bool hasAuthority = pAuthority != std::string::npos;

	if ( hasAuthority ) {
		auto ret = parseScheme(url);
		if ( ret != STATUS_OK ) {
			return ret;
		}

		setSchemeDefaults();

		_currentPos += 2;
		ret = parseAuthority(url);
		if ( ret != STATUS_OK ) {
			return ret;
		}
	}
	else if ( implyAuthority ) {
		auto ret = parseAuthority(url);
		if ( ret != STATUS_OK ) {
			return ret;
		}
	}
	else if ( url.find(":") != std::string::npos ) {
		auto ret = parseScheme(url);
		if ( ret != STATUS_OK ) {
			return ret;
		}

		setSchemeDefaults();
	}

	parsePath(url);
	if ( _currentPos == std::string::npos ) {
		return STATUS_OK;
	}

	// Fragment check
	auto fragmentPos = url.find("#", _currentPos);
	if ( fragmentPos == std::string::npos ) {
		_query = url.substr(_currentPos, url.size() - _currentPos);
	}
	else {
		_query = url.substr(_currentPos, fragmentPos - _currentPos);
		_fragment = url.substr(fragmentPos + 1, url.size() - fragmentPos);
	}

	_fragment = Decoded(_fragment);

	auto ret = parseQuery(url);
	if ( ret != STATUS_OK ) {
		return ret;
	}

	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Url::debug() const {
	std::cerr << "Scheme: " << _scheme << std::endl
	          << "Host:" << _host << std::endl
	          << "Port:" << (_port ? *_port : -1) << std::endl
	          << "User:" << _user << std::endl
	          << "Password:" << _password << std::endl;

	std::cerr << "Path: " << _path << std::endl
	          << "Query: " << _query << std::endl
	          << "Query items: " << std::endl;
	for ( const auto &[key, value] : _queryItems ) {
		std::cerr << "  " << key << " " << value << std::endl;
	}

	std::cerr << "Fragment: " << _fragment << std::endl
	          << "Valid:" << (_isValid ? "true" : "false") << std::endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Url::setSchemeDefaults() {
	if ( sc::compareNoCase(_scheme, "http") == 0) {
		_port = 80;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::withoutScheme() const {
	size_t pos = _url.find("://");
	if ( pos == std::string::npos ) {
		return _url;
	}

	if ( true ) {
		if ( _url.size() < pos + 3 ) {
			return _url;
		}

		if ( _url[pos + 1] != '/' || _url[pos + 2] != '/' ) {
			return _url;
		}

		pos += 2;
	}

	return _url.substr(pos + 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::Encoded(const std::string &s) {
	//RFC 3986 section 2.2 Reserved Characters (January 2005)
	//RFC 3986 section 2.3 Unreserved Characters (January 2005)

	static const std::string dontEscape =
		":/?#[]@!$&'()*+,;="
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

	std::string escaped = {};
	for ( size_t i = 0; i < s.length(); ++i ) {
		if ( dontEscape.find_first_of(s[i]) != std::string::npos ) {
			escaped += s[i];
		} else {
			escaped += '%';
			char buf[3];
			sprintf(buf, "%.2X", s[i]);
			escaped += buf;
		}
	}

	return escaped;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::Encoded(const char *s, int len) {
	//RFC 3986 section 2.2 Reserved Characters (January 2005)
	//RFC 3986 section 2.3 Unreserved Characters (January 2005)

	static const std::string dontEscape =
		":/?#[]@!$&'()*+,;="
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

	std::string escaped = {};
	for ( int i = 0; i < len; ++i ) {
		if ( dontEscape.find_first_of(s[i]) != std::string::npos )
			escaped += s[i];
		else {
			escaped += '%';
			char buf[3];
			sprintf(buf, "%.2X", s[i]);
			escaped += buf;
		}
	}

	return escaped;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::Decoded(const std::string &s) {
	std::string decoded = {};
	for( size_t i = 0; i < s.length(); ++i ) {
		if ( s[i] == '+' )
			decoded += ' ';
		else if ( s[i] == '%' ) {
			++i;
			char hi, lo;
			if ( i < s.length() ) hi = s[i];
			else break;
			++i;
			if ( i < s.length() ) lo = s[i];
			else break;

			hi = (hi >= 'A' ? ((hi & 0xDF) - 'A') + 10 : (hi - '0'));
			lo = (lo >= 'A' ? ((lo & 0xDF) - 'A') + 10 : (lo - '0'));

			decoded += ((hi << 4) + lo);
		}
		else
			decoded += s[i];
	}

	return decoded;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::Decoded(const char *s, int len) {
	std::string decoded = {};
	for ( int i = 0; i < len; ++i ) {
		if ( s[i] == '+' )
			decoded += ' ';
		else if ( s[i] == '%' ) {
			++i;
			char hi, lo;
			if ( i < len ) hi = s[i];
			else break;
			++i;
			if ( i < len ) lo = s[i];
			else break;

			hi = (hi >= 'A'?((hi & 0xDF) - 'A') + 10 : (hi - '0'));
			lo = (lo >= 'A'?((lo & 0xDF) - 'A') + 10 : (lo - '0'));

			decoded += ((hi << 4) + lo);
		}
		else
			decoded += s[i];
	}

	return decoded;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
