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


#ifndef __SEISOMP_SERVICES_DATABASE_SQLITE_INTERFACE_H__
#define __SEISOMP_SERVICES_DATABASE_SQLITE_INTERFACE_H__

#include <seiscomp/io/database.h>
#include <sqlite3.h>


namespace Seiscomp {
namespace Database {


class SQLiteDatabase : public Seiscomp::IO::DatabaseInterface {
	DECLARE_SC_CLASS(SQLiteDatabase);

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		SQLiteDatabase() = default;
		~SQLiteDatabase() override;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		bool connect(const char *con) override;
		void disconnect() override;

		bool isConnected() const override;

		void start() override;
		void commit() override;
		void rollback() override;

		bool execute(const char* command) override;
		bool beginQuery(const char* query) override;
		void endQuery() override;

		const char *defaultValue() const override;
		OID lastInsertId(const char*) override;
		uint64_t numberOfAffectedRows() override;

		bool fetchRow() override;
		int findColumn(const char* name) override;
		int getRowFieldCount() const override;
		const char *getRowFieldName(int index) override;
		const void *getRowField(int index) override;
		size_t getRowFieldSize(int index) override;
		bool escape(std::string &out, const std::string &in) override;


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		bool handleURIParameter(const std::string &name,
		                        const std::string &value) override;
		bool open() override;

		uint16_t _debugUMask{0};


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		sqlite3      *_handle{nullptr};
		sqlite3_stmt *_stmt{nullptr};
		int           _columnCount{0};
		int           _sync{1};
};


}
}


#endif
