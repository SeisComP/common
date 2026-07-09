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


#define SEISCOMP_TEST_MODULE SeisComP


#include <seiscomp/unittest/unittests.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/io/database.h>

#include <iostream>


using namespace std;
using namespace Seiscomp;


class DummyInterface : public IO::DatabaseInterface {
	public:
		DummyInterface() {
			_columnPrefix = "m_";
		}

	public:
		Backend backend() const override {
			return _backend;
		}

		void disconnect() override {}
		bool isConnected() const override { return true; }
		void start() override {}
		void commit() override {}
		void rollback() override {}
		bool execute(const char* command) override { return false; }
		bool beginQuery(const char* query) override { return false; }
		void endQuery() override {}
		OID lastInsertId(const char* table) override { return INVALID_OID; }
		uint64_t numberOfAffectedRows() override { return 0; }
		bool fetchRow() override { return false; }
		int findColumn(const char* name) override { return -1; }
		int getRowFieldCount() const override { return -1; }
		const char *getRowFieldName(int index) override { return nullptr; }
		const void *getRowField(int index) override { return nullptr; }
		size_t getRowFieldSize(int index) override { return 0; }
		bool open() override { return false; }
};



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE(seiscomp_io_db_database)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_CASE(Query) {
	DummyInterface db;
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select $methodID from @Origin"), "select m_methodID from Origin");
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select $methodID from @Origin where _oid=123456"), "select m_methodID from Origin where _oid=123456");
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID='LOCSAT'"), "select * from Origin where m_methodID='LOCSAT'");
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=\"LOCSAT\""), "select * from Origin where m_methodID='LOCSAT'");
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=\"LOC'SAT\""), "select * from Origin where m_methodID='LOC''SAT'");
	BOOST_CHECK_THROW(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID='LOC"), std::runtime_error);

	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=?", "StdLoc"), "select * from Origin where m_methodID='StdLoc'");
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=?", "Std'Loc"), "select * from Origin where m_methodID='Std''Loc'");
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=? and 1=1", "Std'Loc"), "select * from Origin where m_methodID='Std''Loc' and 1=1");
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=? and $stationCount=?", "StdLoc", 123), "select * from Origin where m_methodID='StdLoc' and m_stationCount=123");

	BOOST_CHECK_THROW(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=?"), std::runtime_error);
	BOOST_CHECK_THROW(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=?", "StdLoc", 123), std::runtime_error);
	BOOST_CHECK_THROW(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=? and 1=1", "StdLoc", 123), std::runtime_error);

	// IsSQLNumber gates the unquoted path: numbers yes, char/string no.
	BOOST_CHECK(Core::TypeTraits::IsSQLNumber<int>::value);
	BOOST_CHECK(Core::TypeTraits::IsSQLNumber<double>::value);
	BOOST_CHECK(!Core::TypeTraits::IsSQLNumber<char>::value);
	BOOST_CHECK(!Core::TypeTraits::IsSQLNumber<std::string>::value);
	// A std::string parameter is escaped and quoted (previously emitted raw).
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $methodID=?", std::string("x' OR '1'='1")), "select * from Origin where m_methodID='x'' OR ''1''=''1'");
	// A numeric parameter stays unquoted.
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select * from @Origin where $stationCount=?", 42), "select * from Origin where m_stationCount=42");
	// Non-string, non-number parameters are rejected at compile time by the
	// static_assert in Query(); e.g. the following must NOT compile:
	//   IO::DatabaseInterface::Query(&db, "... $t=?", Core::Time::GMT());

	// --- Parser hardening (problem 3) ---
	// Empty identifiers are rejected instead of producing malformed SQL.
	BOOST_CHECK_THROW(IO::DatabaseInterface::Query(&db, "select * from @ where x=1"), std::runtime_error);
	BOOST_CHECK_THROW(IO::DatabaseInterface::Query(&db, "select $ from @T"), std::runtime_error);
	// Digits and underscores remain valid identifier characters.
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select $a1_b2 from @T_2"), "select m_a1_b2 from T_2");
	// A high-bit (UTF-8) byte terminates the identifier deterministically and
	// does not invoke isalnom() with a negative char (no undefined behaviour).
	BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, "select 1 from @caf\xC3\xA9"), "select 1 from caf\xC3\xA9");
	// A non-null-terminated string_view is parsed within its bounds only.
	{
		std::string full = "select $x from @T and more";
		BOOST_CHECK_EQUAL(IO::DatabaseInterface::Query(&db, std::string_view(full.data(), 17)), "select m_x from T");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BOOST_AUTO_TEST_SUITE_END()
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
