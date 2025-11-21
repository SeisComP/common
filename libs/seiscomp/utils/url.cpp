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
Url::Url(std::string_view url) {
	setUrl(url);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Url(std::string_view scheme,
         std::string_view username, std::string_view password,
         std::string_view host, OPT(uint16_t) port,
         std::string_view path, std::string_view fragment,
         QueryItems queryItems) {
	setScheme(scheme);
	setUsername(username);
	setPassword(username);
	setHost(host);
	setPort(port);
	setPath(path);
	setFragment(fragment);
	setQueryItems(queryItems);
	encode();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Url::setUrl(std::string_view url) {
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
Util::Url &Url::setScheme(std::string_view scheme) {
	_scheme = scheme;
	return *this;
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
Util::Url &Url::setUsername(std::string_view username) {
	_user = username;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::username() const {
	return _user;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Util::Url &Url::setPassword(std::string_view password) {
	_password = password;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::password() const {
	return _password;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Util::Url &Url::setHost(std::string_view host) {
	_host = host;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::host() const {
	return _host;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url &Url::setPort(OPT(uint16_t) port) {
	_port = port;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(uint16_t) Url::port() const {
	return _port;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url &Url::setPath(std::string_view path) {
	_path = path;
	return *this;
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
void Url::setFragment(std::string_view fragment) {
	_fragment = fragment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::fragment() const {
	return _fragment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url &Url::setQueryItems(QueryItems items) {
	_queryItems = items;
	return *this;
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
Url::Status Url::parseScheme(std::string_view url) {
	auto end = url.find(':');
	if ( end == std::string::npos ) {
		return STATUS_SCHEME_ERROR;
	}

	_scheme = url.substr(0, end);
	if ( _scheme.empty() ) {
		return STATUS_SCHEME_ERROR;
	}

	_scheme = Decode(_scheme);

	_currentPos = end + 1;
	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parseAuthority(std::string_view url) {
	auto slashPos = url.find('/', _currentPos);
	auto markPos = url.find('?', _currentPos);
	auto hashPos = url.find('#', _currentPos);

	auto end = std::min(slashPos, std::min(markPos, hashPos));
	if ( end == std::string::npos ) {
		end = url.size();
	}

	_authority = url.substr(_currentPos, end - _currentPos);
	if ( _authority.empty() ) {
		return STATUS_OK;
	}

	_currentPos = end;

	auto userInfoEnd = _authority.find('@');
	if ( userInfoEnd != std::string::npos ) {
		std::string_view sv(_authority);
		auto userInfo = sv.substr(0, userInfoEnd);
		if ( userInfo.empty() ) {
			return STATUS_EMPTY_USER_INFO;
		}

		end = userInfo.find(':');
		if ( end != std::string::npos ) {
			_user = Decode(userInfo.substr(0, end));
			if ( _user.empty() ) {
				return STATUS_EMPTY_USERNAME;
			}

			if ( userInfo.size() > end ) {
				_password = Decode(userInfo.substr(end + 1, userInfo.size() - end));
			}
		}
		else {
			_user = Decode(userInfo);
		}

		++userInfoEnd;
	}
	else {
		userInfoEnd = 0;
	}

	if ( userInfoEnd < _authority.size() && _authority[userInfoEnd] == '[' ) {
		// IPv6 specification
		end = _authority.find(']', userInfoEnd + 1);
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
		end = _authority.find(':', userInfoEnd);
	}

	if ( end != std::string::npos ) {
		_host = Decode(_authority.substr(userInfoEnd, end - userInfoEnd));
		if ( _host.empty() ) {
			return STATUS_INVALID_HOST;
		}

		++end;
		auto portStr = _authority.substr(end, _authority.size() - end);
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
		_host = Decode(_authority.substr(userInfoEnd, _authority.size() - userInfoEnd));
	}

	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parsePath(std::string_view url) {
	auto markPos = url.find('?', _currentPos);
	auto hashPos = url.find('#', _currentPos);
	auto end = std::min(markPos, hashPos);
	if ( end != std::string::npos ) {
		_path = url.substr(_currentPos, end - _currentPos);
		_currentPos = end + 1;
	}
	else {
		_path = url.substr(_currentPos, url.size() - _currentPos);
		_currentPos = std::string::npos;
	}

	_path = Decode(_path);

	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parseQuery() {
	size_t start = 0;

	std::string_view sv(_query);

	do {
		std::string_view param;
		auto end = sv.find('&', start);
		if ( end == std::string::npos ) {
			param = sv.substr(start);
		}
		else {
			param = sv.substr(start, end - start);
		}

		if ( !param.empty() ) {
			auto pend = param.find('=');
			if ( pend == std::string::npos ) {
				auto key = Decode(param);
				if ( key.empty() ) {
					return STATUS_INVALID_QUERY;
				}

				_queryItems[key] = "";
			}
			else {
				auto key = Decode(param.substr(0, pend));
				if ( key.empty() ) {
					return STATUS_INVALID_QUERY;
				}

				_queryItems[key] = Decode(param.substr(pend + 1));
			}
		}

		start = end;
		if ( start != std::string::npos ) {
			++start;
		}
	}
	while ( start != std::string::npos );

	return STATUS_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Status Url::parse(std::string_view url, bool implyAuthority) {
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
	else if ( url.find(':') != std::string::npos ) {
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
	auto fragmentPos = url.find('#', _currentPos);
	if ( fragmentPos == std::string::npos ) {
		_query = url.substr(_currentPos, url.size() - _currentPos);
	}
	else {
		_query = url.substr(_currentPos, fragmentPos - _currentPos);
		_fragment = url.substr(fragmentPos + 1, url.size() - fragmentPos);
	}

	_fragment = Decode(_fragment);

	auto ret = parseQuery();
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
Url &Url::encode() {
	_url = EncodeComponent(_scheme);

	_url += "://";

	if ( !_user.empty() ) {
		_url += EncodeComponent(_user);
	}

	if ( !_password.empty() ) {
		_url += ':';
		_url += EncodeComponent(_password);
	}

	if ( !_user.empty() || !_password.empty() ) {
		_url += '@';
	}

	_url += EncodeComponent(_host);
	if ( _port ) {
		_url += ':';
		_url += Core::toString(*_port);
	}

	if ( !_path.empty() ) {
		if ( _path[0] != '/' ) {
			_url += '/';
		}
		_url += EncodeComponent(_path);
	}

	if ( !_fragment.empty() ) {
		_url += '#';
		_url += EncodeComponent(_fragment);
	}

	if ( !_queryItems.empty() ) {
		_url += '?';
		bool first = true;
		for ( const auto &[key, value] : _queryItems ) {
			if ( key.empty() ) {
				continue;
			}

			if ( !first ) {
				_url += '&';
			}

			_url += EncodeComponent(key);
			if ( !value.empty() ) {
				_url += '=';
				_url += EncodeComponent(value);
			}
			first = false;
		}
	}

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::Encode(std::string_view sv) {
	// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURI#description
	// RFC 3986 section 2.2 Reserved Characters (January 2005)
	// RFC 3986 section 2.3 Unreserved Characters (January 2005)

	static const std::string dontEscape =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
		"-_.!~*'()"
		";/?:@&=+$,#";

	std::string escaped = {};
	for ( decltype(sv.length()) i = 0; i < sv.length(); ++i ) {
		if ( dontEscape.find_first_of(sv[i]) != std::string::npos ) {
			escaped += sv[i];
		}
		else {
			escaped += '%';
			char buf[3];
			sprintf(buf, "%.2X", sv[i]);
			escaped += buf;
		}
	}

	return escaped;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::EncodeComponent(std::string_view sv) {
	// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURIComponent#description

	static const std::string dontEscape =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
		"-_.!~*'()";

	std::string escaped = {};
	for ( decltype(sv.length()) i = 0; i < sv.length(); ++i ) {
		if ( dontEscape.find_first_of(sv[i]) != std::string::npos ) {
			escaped += sv[i];
		}
		else {
			escaped += '%';
			char buf[3];
			sprintf(buf, "%.2X", sv[i]);
			escaped += buf;
		}
	}

	return escaped;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::Decode(std::string_view sv) {
	std::string decoded = {};
	for ( decltype(sv.length()) i = 0; i < sv.length(); ++i ) {
		if ( sv[i] == '+' ) {
			decoded += ' ';
		}
		else if ( sv[i] == '%' ) {
			++i;
			char hi;
			char lo;
			if ( i < sv.length() ) {
				hi = sv[i];
			}
			else {
				break;
			}
			++i;
			if ( i < sv.length() ) {
				lo = sv[i];
			}
			else {
				break;
			}

			hi = (hi >= 'A'?((hi & 0xDF) - 'A') + 10 : (hi - '0'));
			lo = (lo >= 'A'?((lo & 0xDF) - 'A') + 10 : (lo - '0'));

			decoded += ((hi << 4) + lo);
		}
		else {
			decoded += sv[i];
		}
	}

	return decoded;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
