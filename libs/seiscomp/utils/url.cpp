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

#include <algorithm>
#include <iostream>
#include <vector>


using namespace std;


namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Url::Url(const string &url) {
	setUrl(url);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Url::setUrl(const string &url) {
	_url = url;
	_isValid = parse(url);
	return _isValid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Url::scheme() const {
	return _scheme;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Url::username() const {
	return _username;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Url::password() const {
	return _password;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Url::host() const {
	return _host;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Url::port() const {
	return _port;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Url::path() const {
	return _path;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Url::query() const {
	return _query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Url::fragment() const {
	return _fragment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Url::hasQuery() const {
	return !_query.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Url::hasQueryItem(const string &key) const {
	return _queryItems.find(key) != _queryItems.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Url::queryItemValue(const string &key) const{
	QueryItems::const_iterator it = _queryItems.find(key);
	if ( it != _queryItems.end() )
		return it->second;
	return "";
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
bool Url::parse(const std::string &url) {
	size_t start = 0, end = 0;

	// Set defaults
	_scheme = std::string();
	_username = std::string();
	_password = std::string();
	_host = std::string();
	_port = -1;
	_path = std::string();
	_query = std::string();
	_queryItems.clear();

	end = url.find("://", start);
	if ( end != string::npos ) {
		_scheme = url.substr(start, end - start);
		start = end+3;
		if ( start == string::npos )
			return false;
	}

	end = url.find('/', start);
	if ( end == string::npos ) {
		end = url.find('?', start);
		_host = url.substr(start, (end!=string::npos)?end - start:string::npos);
		if (_host.empty() )
			return false;

		_path= "/";
	}
	else {
		_host = url.substr(start, end - start);
		if (_host.empty() )
			return false;

		start = end;
		end = url.find('?', start);
		_path = url.substr(start, (end!=string::npos)?end - start:string::npos);
	}

	if ( end != string::npos ) {
		_query = url.substr(end+1);
		end = _query.find('#');
		if ( end != string::npos ) {
			_fragment = _query.substr(end+1);
			_query.erase(end);
		}
	}

	// Extract userinfo
	end = _host.find('@');
	if ( end != string::npos ) {
		string userinfo = _host.substr(0, end);
		_host.erase(0, end+1);

		end = userinfo.find(':');
		if ( end != string::npos ) {
			_username = userinfo.substr(0, end);
			_password = userinfo.substr(end+1);
		}
		else
			_username = userinfo;
	}

	end = _host.find(':');
	if ( end != string::npos ) {
		Core::fromString(_port, _host.substr(end + 1));
		_host.erase(end);
	}

	start = 0;
	do {
		end = _query.find('=', start);
		if ( end == string::npos ) break;

		string key = _query.substr(start, end - start);
		if ( key.empty() ) break;

		start = end + 1;
		if ( start == string::npos ) break;

		end = _query.find('&', start);
		if ( end == string::npos ) 
			_queryItems[key] = _query.substr(start);
		else {
			_queryItems[key] = _query.substr(start, end - start);
			++end;
		}

		start = end;
	}
	while ( start != string::npos );

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Url::debug() const {
	cerr << "Scheme: " << _scheme << endl
	     << "Host: " << _host << endl
	     << "Port: " << _port << endl
	     << "Path: " << _path << endl
	     << "Query: " << _query << endl
	     << "Query items: " << endl;
	for ( QueryItems::const_iterator it = _queryItems.begin(); it != _queryItems.end(); ++it ) {
		cerr << "\t" << it->first << " " << it->second << endl;
	}
	cerr << "Valid:" << (_isValid ? "true" : "false") << endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Url::withoutScheme() const {
	size_t pos = _url.find("://");
	if ( pos != string::npos ) {
		return _url.substr(pos+3);
	}

	return _url;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
