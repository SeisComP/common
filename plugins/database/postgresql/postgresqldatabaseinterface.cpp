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


#define SEISCOMP_COMPONENT POSTGRESQL
#include <seiscomp/logging/log.h>
#include <seiscomp/core/plugin.h>
#include "postgresqldatabaseinterface.h"

#include <cstdlib>
#include <iostream>


namespace Seiscomp::Database {


IMPLEMENT_SC_CLASS_DERIVED(PostgreSQLDatabase,
                           Seiscomp::IO::DatabaseInterface,
                           "postgresql_database_interface");

REGISTER_DB_INTERFACE(PostgreSQLDatabase, "postgresql");
ADD_SC_PLUGIN("PostgreSQL database driver", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 12, 0)


#define XFREE(ptr) \
	do {\
		if ( ptr ) {\
			PQfreemem(ptr);\
			ptr = nullptr;\
			ptr##Size = 0;\
		}\
	} while (0)


#define PG_TYPE_BYTEA 17
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PostgreSQLDatabase::~PostgreSQLDatabase() {
	disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::handleURIParameter(const std::string &name,
                                            const std::string &value) {
	if ( !DatabaseInterface::handleURIParameter(name, value) ) {
		return false;
	}

	if ( name == "debug" && value != "0" && value != "false" ) {
		_debug = true;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::open() {
	std::stringstream ss;
	if ( _port ) {
		ss << _port;
	}

	_handle = PQsetdbLogin(_host.c_str(), ss.str().c_str(),
	                       nullptr,
	                       nullptr,
	                       _database.c_str(),
	                       _user.c_str(),
	                       _password.c_str());

	/* Check to see that the backend connection was successfully made */
	if ( PQstatus(_handle) != CONNECTION_OK ) {
		SEISCOMP_ERROR("Connect to %s:******@%s:%d/%s failed: %s", _user.c_str(),
		               _host.c_str(), _port, _database.c_str(),
		               PQerrorMessage(_handle));

		disconnect();
		return false;
	}

	SEISCOMP_DEBUG("Connected to %s:******@%s:%d/%s", _user.c_str(),
	               _host.c_str(), _port, _database.c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::connect(const char *con) {
	_host = "localhost";
	_user = "sysop";
	_password = "sysop";
	_database = "seiscomp";
	_port = 0;
	_columnPrefix = "m_";
	return DatabaseInterface::connect(con);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::disconnect() {
	if ( _result ) {
		PQclear(_result);
		_result = nullptr;
	}

	PQfinish(_handle);
	_handle = nullptr;

	XFREE(_unescapeBuffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::isConnected() const {
	if ( !_handle ) {
		return false;
	}

	auto stat = PQstatus(_handle);
	if ( stat == CONNECTION_OK ) {
		return true;
	}

	return reconnect(stat);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::start() {
	execute("start transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::commit() {
	execute("commit");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::rollback() {
	execute("rollback");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::execute(const char *command) {
	if ( !isConnected() || !command ) {
		return false;
	}

	if ( _debug ) {
		SEISCOMP_DEBUG("[postgresql-execute] %s", command);
	}

	auto *result = PQexec(_handle, command);
	if ( !result ) {
		SEISCOMP_ERROR("execute(\"%s\"): %s", command, PQerrorMessage(_handle));
		return false;
	}

	// A connection problem is not detected by PQstatus used in isConnected()
	// until PQexec is called. The following is testing the connection state
	// again in case of a fatal result status. If a connection failure is
	// detected a reconnect attempt is made and the command is executed a second
	// time.
	auto resultStatus = PQresultStatus(result);
	if ( resultStatus == PGRES_FATAL_ERROR ) {
		auto handleStatus = PQstatus(_handle);
		if ( handleStatus != CONNECTION_OK ) {
			PQclear(result);

			if ( !reconnect(handleStatus) ) {
				return false;
			}

			result = PQexec(_handle, command);
			if ( !result ) {
				SEISCOMP_ERROR("execute(\"%s\"): %s",
				               command, PQerrorMessage(_handle));
				return false;
			}

			resultStatus = PQresultStatus(result);
		}
	}

	if ( resultStatus != PGRES_TUPLES_OK && resultStatus != PGRES_COMMAND_OK ) {
		SEISCOMP_ERROR("Command failed\n"
		               "  command  : %s\n"
		               "  err msg: %s", command, PQerrorMessage(_handle));
		PQclear(result);
		return false;
	}

	PQclear(result);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::beginQuery(const char *query) {
	if ( !isConnected() || !query ) {
		return false;
	}

	if ( _result ) {
		SEISCOMP_ERROR("beginQuery(\"%s\"): nested queries are not supported",
		               query);
		//SEISCOMP_DEBUG("last successfull query: %s", _lastQuery.c_str());
		return false;
	}

	endQuery();

	if ( _debug ) {
		SEISCOMP_DEBUG("[postgresql-query] %s", query);
	}

	_result = PQexec(_handle, query);
	if ( !_result ) {
		SEISCOMP_ERROR("beginQuery(\"%s\"): %s", query, PQerrorMessage(_handle));
		return false;
	}

	// A connection problem is not detected by PQstatus used in isConnected()
	// until PQexec is called. The following is testing the connection state
	// again in case of a fatal result status. If a connection failure is
	// detected a reconnect attempt is made and the query is executed a second
	// time.
	auto resultStatus = PQresultStatus(_result);
	if ( resultStatus == PGRES_FATAL_ERROR ) {
		auto handleStatus = PQstatus(_handle);
		if ( handleStatus != CONNECTION_OK ) {
			endQuery();

			if ( !reconnect(handleStatus) ) {
				return false;
			}

			_result = PQexec(_handle, query);
			if ( !_result ) {
				SEISCOMP_ERROR("beginQuery(\"%s\"): %s",
				               query, PQerrorMessage(_handle));
				return false;
			}

			resultStatus = PQresultStatus(_result);
		}
	}

	if ( resultStatus != PGRES_TUPLES_OK && resultStatus != PGRES_COMMAND_OK ) {
		SEISCOMP_ERROR("Query failed\n"
		               "  query  : %s\n"
		               "  err msg: %s", query, PQerrorMessage(_handle));
		endQuery();
		return false;
	}

	_nRows = PQntuples(_result);
	_fieldCount = PQnfields(_result);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::endQuery() {
	_row = -1;
	_nRows = -1;
	if ( _result ) {
		PQclear(_result);
		_result = nullptr;
		XFREE(_unescapeBuffer);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IO::DatabaseInterface::OID PostgreSQLDatabase::lastInsertId(const char* table) {
	if ( !beginQuery((std::string("select currval('") + table + "_seq')").c_str()) ) {
		return 0;
	}

	auto *value = PQgetvalue(_result, 0, 0);

	endQuery();

	return OID(value?atoll(value):IO::DatabaseInterface::INVALID_OID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
uint64_t PostgreSQLDatabase::numberOfAffectedRows() {
	auto *number = PQcmdTuples(_result);
	if ( !number || *number == '\0' ) {
		return static_cast<uint64_t>(~0);
	}

	uint64_t count;
	if ( sscanf(number, "%lud", &count) == 1 ) {
		return count;
	}

	return static_cast<uint64_t>(~0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::fetchRow() {
	XFREE(_unescapeBuffer);

	++_row;

	if ( _row < _nRows ) {
		return true;
	}

	_row = _nRows;
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int PostgreSQLDatabase::findColumn(const char* name) {
	return PQfnumber(_result, name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int PostgreSQLDatabase::getRowFieldCount() const {
	return PQnfields(_result);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *PostgreSQLDatabase::getRowFieldName(int index) {
	return PQfname(_result, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const void* PostgreSQLDatabase::getRowField(int index) {
	const void *value;

	if ( PQgetisnull(_result, _row, index) ) {
		return nullptr;
	}

	value = PQgetvalue(_result, _row, index);

	if ( PQftype(_result, index) == PG_TYPE_BYTEA ) {
		// bytea
		XFREE(_unescapeBuffer);

		_unescapeBuffer = PQunescapeBytea(reinterpret_cast<const unsigned char *>(value), &_unescapeBufferSize);
		static_cast<char*>(_unescapeBuffer)[_unescapeBufferSize] = '\0';
		value = _unescapeBuffer;
	}

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t PostgreSQLDatabase::getRowFieldSize(int index) {
	if ( PQftype(_result, index) == PG_TYPE_BYTEA ) {
		// bytea
		return _unescapeBufferSize;
	}

	return PQgetlength(_result, _row, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::escape(std::string &out, const std::string &in) {
	if ( !_handle ) {
		return false;
	}

	int error;
	out.resize(in.size()*2);
	auto l = PQescapeStringConn(_handle, &out[0], in.c_str(), in.size(), &error);
	out[l] = '\0';
	out.resize(l);
	return !error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::reconnect(ConnStatusType stat) const {
	SEISCOMP_WARNING("Connection bad (%d) -> reconnect",
	                 static_cast<int>(stat));
	PQreset(_handle);

	stat = PQstatus(_handle);
	if ( stat != CONNECTION_OK ) {
		SEISCOMP_ERROR("Connection bad (%d), reconnect attempt failed",
		               static_cast<int>(stat));
		return false;
	}

	SEISCOMP_DEBUG("Reconnect attempt successful");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
