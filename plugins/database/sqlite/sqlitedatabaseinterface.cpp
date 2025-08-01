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


#define SEISCOMP_COMPONENT SQLITE3
#include "sqlitedatabaseinterface.h"
#include <seiscomp/logging/log.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/system/environment.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace std;

namespace Seiscomp {
namespace Database {

namespace {

#if SQLITE_VERSION_NUMBER < 3014000
#define SQLITE_TRACE_STMT    0x01
#define SQLITE_TRACE_PROFILE 0x02
// https://www.sqlite.org/c3ref/profile.html
void sqliteTraceFunc(void */*unused*/, const char *sql) {
	if ( strncmp(sql, "--", 2) == 0 ) {
		SEISCOMP_DEBUG("[stmt] Execute trigger with comment: %s", sql);
	}
	else {
		SEISCOMP_DEBUG("[stmt] %s", sql);
	}
}

void sqliteProfileFunc(void */*unused*/, const char *sql, sqlite3_uint64 ns) {
	SEISCOMP_DEBUG("[profile] %.6gs to execute: %s",
	               static_cast<float>(ns * 1E-9), sql);
}
#else
// https://www.sqlite.org/c3ref/trace_v2.html
int sqliteCallbackFunc(unsigned T, void *C, void *P, void *X) {
	switch (T) {
		case SQLITE_TRACE_STMT: {
			auto *sql = static_cast<char *>(X);
			if ( strncmp(sql, "--", 2) == 0 ) {
				SEISCOMP_DEBUG("[stmt] Execute trigger with comment: %s", sql);
			}
			else {
				sqlite3_stmt *pStmt = static_cast<sqlite3_stmt*>(P);
				SEISCOMP_DEBUG("[stmt] %s", sqlite3_expanded_sql(pStmt));
			}
			break;
		}
		case SQLITE_TRACE_PROFILE: {
			auto *ns = static_cast<uint64_t *>(X);
			sqlite3_stmt *pStmt = static_cast<sqlite3_stmt*>(P);
			SEISCOMP_DEBUG("[profile] %.6gs to execute: %s",
			               static_cast<float>(*ns * 1E-9),
			               sqlite3_expanded_sql(pStmt));
			break;
		}
		case SQLITE_TRACE_ROW: {
			sqlite3_stmt *pStmt = static_cast<sqlite3_stmt*>(P);
			auto cols = sqlite3_data_count(pStmt);
			if ( cols) {
				stringstream ss;
				for ( int i = 0; i < cols; ++i ) {
					ss << sqlite3_column_text(pStmt, i);
					if ( i ) {
						ss << "\t";
					}
				}
				SEISCOMP_DEBUG("[row] %s", ss.str().c_str());
			}
			else {
				SEISCOMP_DEBUG("[row] <empty>");
			}
			break;
		}
		case SQLITE_TRACE_CLOSE: {
			SEISCOMP_DEBUG("[closed]");
			break;
		}
		default:
			SEISCOMP_WARNING("[sqlite] Unsupported trace callback type: %u", T);
			break;
	}

	return 0;
}
#endif


IMPLEMENT_SC_CLASS_DERIVED(SQLiteDatabase,
                           Seiscomp::IO::DatabaseInterface,
                           "sqlite3_database_interface");


SQLiteDatabase::~SQLiteDatabase() {
	SQLiteDatabase::disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::handleURIParameter(const std::string &name,
                                        const std::string &value) {
	if ( !DatabaseInterface::handleURIParameter(name, value) ) {
		return false;
	}

	if ( (name == "debug") && (value != "0") && (value != "false") ) {
		if ( value.empty() || (value == "true") ) {
			_debugUMask = SQLITE_TRACE_STMT;
		}
		else if ( !Core::fromString(_debugUMask, value) ) {
			SEISCOMP_ERROR("Invalid debug value: %s", value.c_str());
			return false;
		}
	}
	else if ( name == "sync" ) {
		if ( value == "false" || value == "off" || value == "0" ) {
			_sync = 0;
		}
		else if ( value == "normal" || value == "on" || value == "1" ) {
			_sync = 1;
		}
		else if ( value == "full" || value == "2" ) {
			_sync = 2;
		}
		else if ( value == "extra" || value == "3" ) {
			_sync = 3;
		}
		else {
			SEISCOMP_ERROR("Invalid sync value: %s", value.c_str());
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::open() {
	string uri(_host);
	if ( uri != ":memory:" ) {
		uri = Environment::Instance()->absolutePath(_host);

		auto *fp = fopen(uri.c_str(), "rb");
		if ( !fp ) {
			SEISCOMP_ERROR("databasefile '%s' not found", uri.c_str());
			return false;
		}
		fclose(fp);
	}

	int res = sqlite3_open(uri.c_str(), &_handle);
	if ( res != SQLITE_OK ) {
		SEISCOMP_ERROR("sqlite3 open error: %d", res);
		sqlite3_close(_handle);
		return false;
	}

	if ( _debugUMask ) {
#if SQLITE_VERSION_NUMBER < 3014000
		if ( _debugUMask & SQLITE_TRACE_STMT ) {
			sqlite3_trace(_handle, &sqliteTraceFunc, nullptr);
		}
		else if ( _debugUMask & SQLITE_TRACE_PROFILE ) {
			sqlite3_profile(_handle, &sqliteProfileFunc, nullptr);
		}
#else
		sqlite3_trace_v2(_handle, _debugUMask, &sqliteCallbackFunc, nullptr);
#endif
	}

	if ( _sync != 1 ) {
		switch ( _sync ) {
			case 0:
				SEISCOMP_DEBUG("Disable disc synchronization");
				execute("PRAGMA synchronous = OFF");
				break;
			case 2:
				SEISCOMP_DEBUG("Set disc synchronization to 'full'");
				execute("PRAGMA synchronous = FULL");
				break;
			case 3:
				SEISCOMP_DEBUG("Set disc synchronization to 'extra'");
				execute("PRAGMA synchronous = EXTRA");
				break;
			default:
				SEISCOMP_WARNING("Unknown sync mode: %d", _sync);
				break;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SQLiteDatabase::Backend SQLiteDatabase::backend() const {
	return SQLite3;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::connect(const char *con) {
	_host = con;
	_columnPrefix = "";
	_sync = 1;

	string params;
	size_t pos = _host.find('?');
	if ( pos != string::npos ) {
		params = _host.substr(pos+1);
		_host.erase(_host.begin() + pos, _host.end());

		vector<string> tokens;
		Core::split(tokens, params.c_str(), "&");
		for ( auto &token : tokens) {
			vector<string> param;
			Core::split(param, token.c_str(), "=");
			if ( ( param.size() == 1 && !handleURIParameter(param[0], "") ) ||
			     ( param.size() == 2 && !handleURIParameter(param[0], param[1]) ) ) {
				return false;
			}
		}
	}

	return open();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::disconnect() {
	if ( _handle ) {
		sqlite3_close(_handle);
		_handle = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::isConnected() const {
	return _handle;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::start() {
	execute("begin transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::commit() {
	execute("commit transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::rollback() {
	execute("rollback transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::execute(const char* command) {
	if ( !isConnected() || !command ) {
		return false;
	}

	char* errmsg = nullptr;
	int result = sqlite3_exec(_handle, command, nullptr, nullptr, &errmsg);
	if ( errmsg ) {
		SEISCOMP_ERROR("sqlite3 execute: %s", errmsg);
		sqlite3_free(errmsg);
	}

	if ( _debugUMask ) {
		SEISCOMP_DEBUG("%s", command);
	}

	return result == SQLITE_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::beginQuery(const char* query) {
	if ( !isConnected() || !query ) {
		return false;
	}

	if ( _stmt ) {
		SEISCOMP_ERROR("beginQuery: nested queries are not supported");
		return false;
	}

	const char* tail = nullptr;
	int res = sqlite3_prepare(_handle, query, -1, &_stmt, &tail);
	if ( res != SQLITE_OK || !_stmt ) {
		return false;
	}

	if ( _debugUMask ) {
		SEISCOMP_DEBUG("%s", query);
	}

	_columnCount = sqlite3_column_count(_stmt);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::endQuery() {
	if ( _stmt ) {
		sqlite3_finalize(_stmt);
		_stmt = nullptr;
		_columnCount = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* SQLiteDatabase::defaultValue() const {
	return "null";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IO::DatabaseInterface::OID SQLiteDatabase::lastInsertId(const char* /*table*/) {
	sqlite3_int64 id = sqlite3_last_insert_rowid(_handle);
	return id <= 0 ? IO::DatabaseInterface::INVALID_OID : id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
uint64_t SQLiteDatabase::numberOfAffectedRows() {
	return max(sqlite3_changes(_handle), 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::fetchRow() {
	return sqlite3_step(_stmt) == SQLITE_ROW;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SQLiteDatabase::findColumn(const char* name) {
	for ( int i = 0; i < _columnCount; ++i ) {
		if ( !strcmp(sqlite3_column_name(_stmt, i), name) ) {
			return i;
		}
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SQLiteDatabase::getRowFieldCount() const {
	return _columnCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *SQLiteDatabase::getRowFieldName(int index) {
	return sqlite3_column_name(_stmt, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const void* SQLiteDatabase::getRowField(int index) {
	return sqlite3_column_blob(_stmt, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t SQLiteDatabase::getRowFieldSize(int index) {
	return sqlite3_column_bytes(_stmt, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::escape(string &out, const string &in) const {
	out.resize(in.size()*2+1);
	size_t length = in.length();
	const char *in_buf = in.c_str();
	char *out_buf = out.data();
	size_t j = 0;

	for ( size_t i = 0; i < length && *in_buf; ++length, ++in_buf ) {
		switch ( *in_buf ) {
			case '\'':
				out_buf[j++] = '\'';
				out_buf[j++] = '\'';
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
} // ns anonymous


REGISTER_DB_INTERFACE(SQLiteDatabase, "sqlite3");
ADD_SC_PLUGIN("SQLite3 database driver", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 1, 0, 0)


}
}
