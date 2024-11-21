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


#ifndef __SEISOMP_SERVICES_DATABASE_MYSQL_INTERFACE_H__
#define __SEISOMP_SERVICES_DATABASE_MYSQL_INTERFACE_H__

#include <seiscomp/core/platform/platform.h>
#include <seiscomp/io/database.h>
#if defined(WIN32)
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif


namespace Seiscomp {
namespace Database {


namespace {


class MySQLDatabase : public Seiscomp::IO::DatabaseInterface {
	DECLARE_SC_CLASS(MySQLDatabase);

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		MySQLDatabase() = default;
		~MySQLDatabase();


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

		OID lastInsertId(const char*) override;
		uint64_t numberOfAffectedRows() override;

		bool fetchRow() override;
		int findColumn(const char* name) override;
		int getRowFieldCount() const override;
		const char *getRowFieldName(int index) override;
		const void* getRowField(int index) override;
		size_t getRowFieldSize(int index) override;
		bool escape(std::string &out, const std::string &in) const override;


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		bool handleURIParameter(const std::string &name,
		                        const std::string &value) override;

		bool open() override;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		bool ping() const;
		bool query(const char *c, const char *comp);


	private:
		MYSQL                 *_handle{nullptr};
		MYSQL_RES*             _result{nullptr};
		MYSQL_ROW              _row{nullptr};
		bool                   _debug{false};
		//std::string _lastQuery;
		mutable int            _fieldCount{0};
		mutable unsigned long *_lengths{nullptr};
};


}


}
}


#endif
