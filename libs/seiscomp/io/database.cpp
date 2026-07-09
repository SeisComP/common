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


#define SEISCOMP_COMPONENT Database
#include <seiscomp/io/database.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <seiscomp/logging/log.h>
#include <seiscomp/utils/url.h>

#include <cstring>
#include <exception>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::IO::DatabaseInterface, SC_SYSTEM_CORE_API);


#define TABLE_PREFIX     '@' // E.g. @Origin
#define ATTRIBUTE_PREFIX '$' // E.g. @Origin.$time_value
#define STRING_PREFIX1   '\''
#define STRING_PREFIX2   '"'
#define PARAM_TOKEN      '?'


using namespace std;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


inline bool validIdentifierChar(char ch) {
	// isalnum() has undefined behavior for arguments that are not representable
	// as unsigned char (e.g. bytes >= 0x80 from UTF-8 input), so cast first.
	return isalnum(static_cast<unsigned char>(ch)) || (ch == '_');
}

string fillIdentifier(size_t n, const char *in, size_t &i) {
	string id;

	// Identifiers are strictly [A-Za-z0-9_]. The first non-identifier character
	// terminates the identifier; it is then processed by the main loop (so an
	// escaped special character still works, but can no longer be smuggled into
	// an unescaped table/column name).
	++i;
	while ( i < n && validIdentifierChar(in[i]) ) {
		id += in[i];
		++i;
	}

	return id;
}

string fillString(size_t n, const char *in, size_t &i, char quote) {
	string s;

	while ( i < n ) {
		if ( in[i] == '\\' ) {
			++i;
			if ( i >= n ) {
				break;
			}
		}
		else if ( in[i] == quote ) {
			++i;
			return s;
		}

		s += in[i];
		++i;
	}

	throw runtime_error(string("expected closing (") + quote + ")");
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace IO {


IMPLEMENT_SC_ABSTRACT_CLASS(DatabaseInterface, "DatabaseInterface");
const DatabaseInterface::OID DatabaseInterface::INVALID_OID = 0;


DatabaseInterface::DatabaseInterface() : _timeout(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseInterface::~DatabaseInterface() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseInterface* DatabaseInterface::Create(const char* service) {
	if ( !service ) {
		return nullptr;
	}

	return DatabaseInterfaceFactory::Create(service);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseInterface* DatabaseInterface::Open(const char* uri) {
	const char* tmp;
	std::string service;
	std::string source;

	tmp = strstr(uri, "://");
	if ( tmp ) {
		std::copy(uri, tmp, std::back_inserter(service));
		uri = tmp + 3;
	}
	else {
		service = "mysql";
	}

	source = uri;

	DatabaseInterface* db = Create(service.c_str());
	if ( !db ) {
		SEISCOMP_ERROR("Database driver '%s' is not supported", service.c_str());
		return nullptr;
	}

	if ( !db->connect(source.c_str()) ) {
		if ( db->_user.empty() ) {
			SEISCOMP_ERROR("Connection failed to %s://%s/%s",
			               service, db->_host, db->_database);
		}
		else {
			SEISCOMP_ERROR("Connection failed to %s://%s:******@%s/%s",
			               service, db->_user, db->_host, db->_database);
		}
		delete db;
		return nullptr;
	}

	return db;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string DatabaseInterface::Query(
	const DatabaseInterface *db, string_view s
) {
	size_t tail = 0;
	string sql;
	while ( tail < s.length() ) {
		sql += Parse(db, s.substr(tail), &tail);
		if ( tail != string::npos ) {
			throw runtime_error("parameter underflow");
		}
	}
	return sql;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseInterface::Parse(
	const DatabaseInterface *db, std::string_view s, size_t *tail
) {
	size_t lastOut = 0;

	string sql;
	auto n = s.length();
	*tail = string::npos;

	for ( size_t i = 0; (s[i] != '\0') && (i < n); ) {
		switch ( s[i] ) {
			case '\\':
				++i;
				sql += s.substr(lastOut, i - lastOut - 1);
				if ( (s[i] != '\0') && (i < n) ) {
					sql += s[i];
					lastOut = i + 1;
				}
				break;
			case STRING_PREFIX1:
			case STRING_PREFIX2:
			{
				auto quote = s[i];
				sql += s.substr(lastOut, i - lastOut);
				string in = fillString(n, s.data(), ++i, quote), out;

				if ( !db->escape(out, in) ) {
					throw runtime_error("escape error: " + in);
				}

				sql += STRING_PREFIX1;
				sql += out;
				sql += STRING_PREFIX1;
				lastOut = i;
				continue;
			}
			case PARAM_TOKEN:
				sql += s.substr(lastOut, i - lastOut);
				*tail = i + 1;
				return sql;
			default:
				switch ( s[i] ) {
					case TABLE_PREFIX:
					{
						// Table replace
						sql += s.substr(lastOut, i - lastOut);
						auto id = fillIdentifier(n, s.data(), i);
						if ( id.empty() ) {
							throw runtime_error("empty table name after '@'");
						}
						sql += id;
						lastOut = i;
						continue;
					}
					case ATTRIBUTE_PREFIX:
						if ( !i || !validIdentifierChar(s[i - 1]) ) {
							// Attribute replace
							sql += s.substr(lastOut, i - lastOut);
							auto id = fillIdentifier(n, s.data(), i);
							if ( id.empty() ) {
								throw runtime_error("empty column name after '$'");
							}
							sql += db->convertColumnName(id);
							lastOut = i;
							continue;
						}
						break;
					default:
						break;
				}
				break;
		}

		++i;
	}

	size_t i = lastOut;
	while ( (i < n) && (s[i] != '\0') ) {
		++i;
	}

	sql += s.substr(lastOut, i - lastOut);

	return sql;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseInterface::connect(const char* con) {
	if ( isConnected() ) {
		return false;
	}

	_timeout = 0;

	Util::Url url(con);
	auto path = url.path();
	if ( !path.empty() && (path != "/") ) {
		if ( path[0] == '/' ) {
			path = path.substr(1);
		}
		_database = path;
	}
	if ( !url.username().empty() ) {
		_user = url.username();
	}
	if ( !url.password().empty() ) {
		_password = url.password();
	}
	if ( !url.host().empty() ) {
		_host = url.host();
	}
	if ( url.port().has_value() ) {
		_port = *url.port();
	}

	if ( (url.status() != Util::Url::STATUS_EMPTY) && !url.isValid() ) {
		// Only empty URLs are accepted if invalid as they are populated with
		// default values.
		SEISCOMP_ERROR("Invalid database URL: %s", url.errorMessage());
		return false;
	}

	for ( auto [key, value] : url.queryItems() ) {
		if ( !handleURIParameter(key, value) ) {
			return false;
		}
	}

	return open();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseInterface::handleURIParameter(const std::string &name,
                                           const std::string &value) {
	if ( name == "column_prefix" ) {
		if ( !value.empty() )
			_columnPrefix = value;
	}
	else if ( name == "timeout" ) {
		if ( !value.empty() ) {
			if ( !Core::fromString(_timeout, value) ) {
				SEISCOMP_ERROR("Invalid timeout parameter '%s' for database connection",
				               value.c_str());
				return false;
			}
			else
				SEISCOMP_DEBUG("Request database read timeout of %d seconds", _timeout);
		}
		else {
			SEISCOMP_ERROR("Database timeout parameter expects a value");
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* DatabaseInterface::defaultValue() const {
	return "default";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseInterface::timeToString(const Seiscomp::Core::Time& t) {
	return t.toString("%Y-%m-%d %H:%M:%S");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time DatabaseInterface::stringToTime(const char* str) {
	Seiscomp::Core::Time t;
	if ( str == nullptr ) return t;
	t.fromString(str, "%F %T");
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& DatabaseInterface::columnPrefix() const {
	return _columnPrefix;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseInterface::convertColumnName(const std::string& name) const {
	return _columnPrefix + name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseInterface::getRowFieldString(int index) {
	const void *data = getRowField(index);
	if ( !data ) return "";
	return (const char*)data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseInterface::escape(std::string &out, const std::string &in) const {
	out.resize(in.size()*2+1);
	size_t length = in.length();
	const char *in_buf = in.c_str();
	char *out_buf = out.data();
	size_t j = 0;

	for ( size_t i = 0; i < length && *in_buf; ++length, ++in_buf ) {
		switch ( *in_buf ) {
			case '\'':
				out_buf[j++] = '\'';
				out_buf[j++]   = '\'';
				break;
			case '\\':
				out_buf[j++] = '\\';
				out_buf[j++]   = '\\';
				break;
			case '\n':
				out_buf[j++] = '\\';
				out_buf[j++]   = 'n';
				break;
			case '\t':
				out_buf[j++] = '\\';
				out_buf[j++]   = 't';
				break;
			case '\r':
				// Filter out
				break;
			case '\a':
				out_buf[j++] = '\\';
				out_buf[j++]   = 'a';
				break;
			case '\b':
				out_buf[j++] = '\\';
				out_buf[j++]   = 'b';
				break;
			case '\f':
				out_buf[j++] = '\\';
				out_buf[j++]   = 'f';
				break;
			case '\v':
				out_buf[j++] = '\\';
				out_buf[j++]   = 'v';
				break;
			default:
				out_buf[j++] = *in_buf;
				break;
		}
	}

	out_buf[j] = '\0';
	out.resize(j);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
