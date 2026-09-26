/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#define SEISCOMP_COMPONENT MYSQL
#include "mysqldatabaseinterface.h"
#include <seiscomp/logging/log.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/system.h>
#include <string.h>
#include <utility>
#if defined(WIN32)
#include <errmsg.h>
#else
#include <mysql/errmsg.h>
#endif

#if LIBMYSQL_VERSION_ID >= 80000
typedef bool my_bool;
#endif


namespace Seiscomp {
namespace Database {


namespace {


IMPLEMENT_SC_CLASS_DERIVED(MySQLDatabase,
                           Seiscomp::IO::DatabaseInterface,
                           "mysql_database_interface");


MySQLDatabase::~MySQLDatabase() {
	MySQLDatabase::disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::handleURIParameter(const std::string &name,
                                       const std::string &value) {
	if ( !DatabaseInterface::handleURIParameter(name, value) ) {
		return false;
	}

	if ( name == "debug" ) {
		if ( value != "0" && value != "false" ) {
			_debug = true;
		}
	}
	else if ( name == "ssl_mode" ) {
		if ( !Core::compareNoCase(value, "disabled") ) {
			_sslMode = SSLMode::Disabled;
		}
		else if ( !Core::compareNoCase(value, "preferred") ) {
			_sslMode = SSLMode::Preferred;
		}
		else if ( !Core::compareNoCase(value, "required") ) {
			_sslMode = SSLMode::Required;
		}
		else if ( !Core::compareNoCase(value, "verify_ca") ) {
			_sslMode = SSLMode::VerifyCA;
		}
		else if ( !Core::compareNoCase(value, "verify_identity") ) {
			_sslMode = SSLMode::VerifyIdentity;
		}
		else {
			SEISCOMP_ERROR("Invalid ssl_mode '%s', expected one of: disabled, "
			               "preferred, required, verify_ca, verify_identity",
			               value.c_str());
			return false;
		}
	}
	else if ( name == "ssl_ca" ) {
		_sslCA = value;
	}
	else if ( name == "ssl_capath" ) {
		_sslCAPath = value;
	}
	else if ( name == "ssl_cert" ) {
		_sslCert = value;
	}
	else if ( name == "ssl_key" ) {
		_sslKey = value;
	}
	else if ( name == "ssl_cipher" ) {
		_sslCipher = value;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::applySSLOptions() {
	bool hasSSLParameters = !_sslCA.empty() || !_sslCAPath.empty() ||
	                        !_sslCert.empty() || !_sslKey.empty() ||
	                        !_sslCipher.empty();

	if ( _sslMode == SSLMode::Default ) {
		// TLS parameters without an explicit mode ask for an encrypted
		// connection. Never fall back to plain text silently. Otherwise use
		// TLS if the server supports it with all client libraries:
		// libmysqlclient does it by default, libmariadb < 3.4 does not.
		_sslMode = hasSSLParameters ? SSLMode::Required : SSLMode::Preferred;
	}

	if ( (_sslMode == SSLMode::Disabled) && hasSSLParameters ) {
		SEISCOMP_WARNING("ssl_mode=disabled: ignoring the other ssl_* parameters");
	}

	// libmysqlclient refuses to verify without a CA and libmariadb >= 3.4
	// silently skips the verification for local connections. Require it
	// always to get the same behavior with all client libraries.
	if ( ((_sslMode == SSLMode::VerifyCA) || (_sslMode == SSLMode::VerifyIdentity))
	  && _sslCA.empty() && _sslCAPath.empty() ) {
		SEISCOMP_ERROR("ssl_mode=verify_ca and verify_identity require ssl_ca "
		               "or ssl_capath");
		return false;
	}

	if ( _sslMode != SSLMode::Disabled ) {
		const std::pair<mysql_option, const std::string*> files[] = {
			{ MYSQL_OPT_SSL_CA, &_sslCA },
			{ MYSQL_OPT_SSL_CAPATH, &_sslCAPath },
			{ MYSQL_OPT_SSL_CERT, &_sslCert },
			{ MYSQL_OPT_SSL_KEY, &_sslKey },
			{ MYSQL_OPT_SSL_CIPHER, &_sslCipher }
		};

		for ( const auto &[option, value] : files ) {
			if ( !value->empty() && mysql_options(_handle, option, value->c_str()) ) {
				SEISCOMP_ERROR("Failed to set TLS option: %s", mysql_error(_handle));
				return false;
			}
		}
	}

#if defined(MARIADB_PACKAGE_VERSION_ID) || defined(MARIADB_BASE_VERSION)
	// MariaDB Connector/C has no ssl mode. TLS is switched on with
	// MYSQL_OPT_SSL_ENFORCE and certificate verification (chain and host
	// name) with MYSQL_OPT_SSL_VERIFY_SERVER_CERT. Both are set explicitly
	// to not depend on the library defaults which changed with version 3.4.
	// Without verification the library falls back to an unencrypted
	// connection if the server does not support TLS. This matches
	// "preferred", checkSSL() rejects it for "required".
	my_bool enforce = _sslMode != SSLMode::Disabled;
	my_bool verify = (_sslMode == SSLMode::VerifyCA) || (_sslMode == SSLMode::VerifyIdentity);

	if ( _sslMode == SSLMode::VerifyCA ) {
		SEISCOMP_INFO("ssl_mode=verify_ca: MariaDB client library also "
		              "verifies the server host name");
	}

	if ( mysql_options(_handle, MYSQL_OPT_SSL_ENFORCE, &enforce)
	  || mysql_options(_handle, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &verify) ) {
		SEISCOMP_ERROR("Failed to set TLS mode: %s", mysql_error(_handle));
		return false;
	}
#elif LIBMYSQL_VERSION_ID >= 50711
	unsigned int mode;
	switch ( _sslMode ) {
		case SSLMode::Disabled:
			mode = SSL_MODE_DISABLED;
			break;
		case SSLMode::Preferred:
			mode = SSL_MODE_PREFERRED;
			break;
		case SSLMode::VerifyCA:
			mode = SSL_MODE_VERIFY_CA;
			break;
		case SSLMode::VerifyIdentity:
			mode = SSL_MODE_VERIFY_IDENTITY;
			break;
		default:
			mode = SSL_MODE_REQUIRED;
			break;
	}

	if ( mysql_options(_handle, MYSQL_OPT_SSL_MODE, &mode) ) {
		SEISCOMP_ERROR("Failed to set TLS mode: %s", mysql_error(_handle));
		return false;
	}
#else
	SEISCOMP_ERROR("ssl_mode is not supported by this MySQL client library");
	return false;
#endif

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::checkSSL() const {
	// Do not rely on the client library to refuse unencrypted connections
	// if encryption was requested.
	if ( (_sslMode == SSLMode::Disabled) || (_sslMode == SSLMode::Preferred) ) {
		return true;
	}

	if ( !mysql_get_ssl_cipher(_handle) ) {
		SEISCOMP_ERROR("Connection to %s:%d is not encrypted but TLS is "
		               "required", _host.c_str(), _port);
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::open() {
	_handle = mysql_init(nullptr);
	if ( !_handle ) {
		return false;
	}

	if ( _timeout > 0 ) {
		SEISCOMP_INFO("Apply database read timeout of %d seconds", _timeout);
		mysql_options(_handle, MYSQL_OPT_READ_TIMEOUT, (const char*)&_timeout);
	}
	if ( _host == "localhost" && _port != 3306 ) {
		SEISCOMP_WARNING("You are trying to open a MySQL TCP connection on a "
		                 "non standard port using the host string 'localhost'. "
		                 "The port might be ignored in favor of a Unix socket "
		                 "or shared memory connection. Use 127.0.0.1 or a host "
		                 "name other than 'localhost' to force the creation of "
		                 "a TCP connection.");
	}

	if ( !applySSLOptions() ) {
		mysql_close(_handle);
		_handle = nullptr;
		return false;
	}

	// CLIENT_REMEMBER_OPTIONS keeps the options (e.g. TLS) for reconnects
	// in ping() if a connection attempt failed.
	if ( !mysql_real_connect(_handle, _host.c_str(), _user.c_str(), _password.c_str(),
	                         _database.c_str(), _port, nullptr,
	                         CLIENT_REMEMBER_OPTIONS) ) {
		SEISCOMP_ERROR("Connect to %s:******@%s:%d/%s failed: %s", _user.c_str(),
		               _host.c_str(), _port, _database.c_str(),
		               mysql_error(_handle));
		mysql_close(_handle);
		_handle = nullptr;
		return false;
	}

	if ( !checkSSL() ) {
		mysql_close(_handle);
		_handle = nullptr;
		return false;
	}

	const char *cipher = mysql_get_ssl_cipher(_handle);
	SEISCOMP_DEBUG("Connected to %s:******@%s:%d/%s (%s, TLS: %s)", _user.c_str(),
	               _host.c_str(), _port, _database.c_str(),
	               _handle->host_info, cipher ? cipher : "none");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MySQLDatabase::Backend MySQLDatabase::backend() const {
	return MySQL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::connect(const char *con) {
	_host = "localhost";
	_user = "sysop";
	_password = "sysop";
	_database = "seiscomp";
	_port = 3306;
	_columnPrefix = "";
	_sslMode = SSLMode::Default;
	_sslCA.clear();
	_sslCAPath.clear();
	_sslCert.clear();
	_sslKey.clear();
	_sslCipher.clear();
	return DatabaseInterface::connect(con);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::disconnect() {
	if ( _handle ) {
		SEISCOMP_INFO("Disconnecting from database");
		if ( _result ) {
			mysql_free_result(_result);
			_result = nullptr;
		}
		mysql_close(_handle);
		_handle = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::isConnected() const {
	if ( !_handle ) {
		return false;
	}

	int err = mysql_errno(_handle);
	if ( err < CR_UNKNOWN_ERROR ) {
		return true;
	}

	SEISCOMP_ERROR("connection error %d (%s) -> ping", err, mysql_error(_handle));
	return ping();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::ping() const {
	if ( !mysql_ping(_handle) ) {
		return true;
	}

	SEISCOMP_ERROR("ping() = %d (%s)", mysql_errno(_handle), mysql_error(_handle));
	// Try to reconnect
	if ( !mysql_real_connect(_handle, _host.c_str(), _user.c_str(), _password.c_str(),
	                         _database.c_str(), _port, nullptr,
	                         CLIENT_REMEMBER_OPTIONS) ) {
		SEISCOMP_ERROR("Connect to %s:******@%s:%d/%s failed: %s", _user.c_str(),
		               _host.c_str(), _port, _database.c_str(),
		               mysql_error(_handle));
		return false;
	}

	if ( !checkSSL() ) {
		// Do not keep an unencrypted connection open. It would be used by
		// the next query otherwise.
		const_cast<MySQLDatabase*>(this)->disconnect();
		return false;
	}

	return mysql_ping(_handle) == 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::start() {
	execute("start transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::commit() {
	execute("commit");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::rollback() {
	execute("rollback");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::query(const char *c, const char *comp) {
	// No connection yet established or disconnect has been called
	if ( !_handle || !c ) return false;

	unsigned int err;
	// Copy the message, ping() may close the handle which owns the buffer
	std::string err_msg;
	bool firstTry = true;

	do {
		if ( _debug )
			SEISCOMP_DEBUG("[mysql-%s] %s", comp, c);

		int result = mysql_query(_handle, c);
		if ( result ) {
			err = mysql_errno(_handle);
			err_msg = mysql_error(_handle);
			// Client connection error?
			if ( err >= CR_UNKNOWN_ERROR ) {
				if ( firstTry ) {
					firstTry = false;
					if ( !_handle || !ping() ) {
						break;
					}
				}
				else {
					break;
				}
			}
			// Break when a query based error occured
			else {
				break;
			}
		}
		else {
			err = 0;
			err_msg.clear();
			break;
		}
	}
	while ( true );

	if ( err > 0 ) {
		SEISCOMP_ERROR("%s(\"%s\") = %d (%s)", comp, c, err,
		               err_msg.empty() ? "unknown" : err_msg.c_str());
		return false;
	}
	else if ( _debug )
		SEISCOMP_DEBUG("[mysql-%s] OK", comp);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::execute(const char* command) {
	return query(command, "execute");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::beginQuery(const char* q) {
	if ( _result ) {
		SEISCOMP_ERROR("beginQuery: nested queries are not supported");
		//SEISCOMP_DEBUG("last successfull query: %s", _lastQuery.c_str());
		return false;
	}

	if ( !query(q, "query") ) {
		return false;
	}

	_result = mysql_use_result(_handle);
	//_result = mysql_store_result(_handle);

	if ( !_result ) {
		return false;
	}

	_fieldCount = (int)mysql_field_count(_handle);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::endQuery() {
	if ( _result ) {
		mysql_free_result(_result);
		_result = nullptr;
		_lengths = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IO::DatabaseInterface::OID MySQLDatabase::lastInsertId(const char*) {
	my_ulonglong id = mysql_insert_id(_handle);
	return id == 0 ? IO::DatabaseInterface::INVALID_OID : id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
uint64_t MySQLDatabase::numberOfAffectedRows() {
	my_ulonglong r = mysql_affected_rows(_handle);
	if ( r != (my_ulonglong)~0 )
		return r;

	return (uint64_t)~0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::fetchRow() {
	_row = mysql_fetch_row(_result);
	_lengths = mysql_fetch_lengths(_result);
	return _row != nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MySQLDatabase::findColumn(const char* name) {
	MYSQL_FIELD* field;
	for ( int i = 0; i < _fieldCount; ++i ) {
		field = mysql_fetch_field_direct(_result, i);
		if ( !strcmp(field->name, name) )
			return i;
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MySQLDatabase::getRowFieldCount() const {
	return _fieldCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *MySQLDatabase::getRowFieldName(int index) {
	MYSQL_FIELD* field = mysql_fetch_field_direct(_result, index);
	return field ? field->name : nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const void* MySQLDatabase::getRowField(int index) {
	return _row[index];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t MySQLDatabase::getRowFieldSize(int index) {
	return _lengths[index];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::escape(std::string &out, const std::string &in) const {
	if ( !_handle ) return false;
	out.resize(in.size()*2);
	size_t l = mysql_real_escape_string(_handle, out.data(), in.c_str(), in.size());
	out[l] = '\0';
	out.resize(l);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}


REGISTER_DB_INTERFACE(MySQLDatabase, "mysql");
ADD_SC_PLUGIN(
	"MySQL database driver",
	"GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>",
	1, 0, 0
)


}
}
