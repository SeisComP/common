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
		SQLiteDatabase();
		~SQLiteDatabase();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		virtual bool connect(const char *con) override;
		virtual void disconnect() override;

		virtual bool isConnected() const override;

		virtual void start() override;
		virtual void commit() override;
		virtual void rollback() override;

		virtual bool execute(const char* command) override;
		virtual bool beginQuery(const char* query) override;
		virtual void endQuery() override;

		virtual const char *defaultValue() const override;
		virtual OID lastInsertId(const char*) override;
		virtual uint64_t numberOfAffectedRows();

		virtual bool fetchRow() override;
		virtual int findColumn(const char* name) override;
		virtual int getRowFieldCount() const override;
		virtual const char *getRowFieldName(int index) override;
		virtual const void *getRowField(int index) override;
		virtual size_t getRowFieldSize(int index) override;
		virtual bool escape(std::string &out, const std::string &in) override;


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		bool open();


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		sqlite3      *_handle;
		sqlite3_stmt *_stmt;
		int           _columnCount;
};


}
}


#endif
