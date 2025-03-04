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


#define SEISCOMP_COMPONENT DatabaseArchive
#include <seiscomp/core/exceptions.h>
#include <seiscomp/datamodel/databasearchive.h>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/datamodel/stream.h>
#include <seiscomp/logging/log.h>

#define __STDC_FORMAT_MACROS
#include <iostream>
#include <stdlib.h>
#include <inttypes.h>
#include <strings.h>


using namespace Seiscomp::Core;


#define ATTRIBUTE_SEPERATOR    "_"
#define MICROSECONDS_POSTFIX   ATTRIBUTE_SEPERATOR"ms"
#define OBJECT_USED_POSTFIX    "used"
#define CHILD_ID_POSTFIX       "oid"


namespace Seiscomp {
namespace DataModel {


class AttributeMapper {
	public:
		AttributeMapper(const DatabaseArchive::AttributeMap &map)
		  : _it(map.begin()), _end(map.end()) {}

		inline bool next() const {
			return _it != _end;
		}

		inline const std::string &attrib() const {
			return _it++->first;
		}

	private:
		mutable DatabaseArchive::AttributeMap::const_iterator _it;
		DatabaseArchive::AttributeMap::const_iterator _end;
};


std::ostream &operator<<(std::ostream &os, const AttributeMapper &m) {
	bool first = true;
	while ( m.next() ) {
		if ( !first ) os << ",";
		os << m.attrib();
		first = false;
	}
	return os;
}


class ValueMapper {
	public:
		ValueMapper(const DatabaseArchive::AttributeMap &map)
		  : _it(map.begin()), _end(map.end()) {}

		inline bool next() const {
			return _it != _end;
		}

		inline std::string value() const {
			static std::string null = "NULL";
			OPT_CR(std::string) v = _it++->second;
			//return v?'\'' + *v + '\'':null;
			return v?*v:null;
		}

	private:
		mutable DatabaseArchive::AttributeMap::const_iterator _it;
		DatabaseArchive::AttributeMap::const_iterator _end;
};


namespace {


std::ostream &operator<<(std::ostream &os, const ValueMapper &m) {
	bool first = true;
	while ( m.next() ) {
		if ( !first ) os << ",";
		os << m.value();
		first = false;
	}
	return os;
}


std::string toSQL(IO::DatabaseInterface *db, const std::string &str) {
	std::string converted;

	if ( !db->escape(converted, str) ) {
		converted = "";
		SEISCOMP_WARNING("db string conversion from failed: %s", str.c_str());
	}

	return converted;
}


bool strtobool(bool &val, const char *str) {
	int v;
	if ( fromString(v, str) ) {
		val = v != 0;
		return true;
	}

	// Check for all 'true' options
	if ( strcasecmp(str, "t") == 0 || strcasecmp(str, "true") == 0 ||
	     strcasecmp(str, "y") == 0 || strcasecmp(str, "yes") == 0 ) {
		val = true;
		return true;
	}

	// Check for all 'false' options
	if ( strcasecmp(str, "f") == 0 || strcasecmp(str, "false") == 0 ||
	     strcasecmp(str, "n") == 0 || strcasecmp(str, "no") == 0 ) {
		val = false;
		return true;
	}

	return false;
}


}


DatabaseIterator::DatabaseIterator(DatabaseArchive *database, const RTTI *rtti)
: _rtti(rtti)
, _reader(database)
, _count(0)
, _oid(IO::DatabaseInterface::INVALID_OID)
, _parent_oid(IO::DatabaseInterface::INVALID_OID)
, _cached(false)
{
	_object = fetch();
	if ( !_object && _reader ) operator++();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator::DatabaseIterator()
: _rtti(nullptr)
, _reader(nullptr)
, _count(0)
, _object(nullptr)
, _oid(IO::DatabaseInterface::INVALID_OID)
, _parent_oid(IO::DatabaseInterface::INVALID_OID)
, _cached(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator::DatabaseIterator(const DatabaseIterator &iter)
: Seiscomp::Core::BaseObject()
, _rtti(iter._rtti)
, _reader(iter._reader)
, _count(iter._count)
, _oid(iter._oid)
, _parent_oid(iter._parent_oid)
, _cached(iter._cached)
, _lastModified(iter._lastModified)
{
	_object = iter._object;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator::~DatabaseIterator() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object *DatabaseIterator::fetch() const {
	if ( !_rtti || !_reader ) return nullptr;

	_cached = false;

	int col;
	_parent_oid = _oid = -1;
	if ( (col = _reader->_db->findColumn("_oid")) != -1) {
		fromString(_oid, std::string(static_cast<const char*>(_reader->_db->getRowField(col))));
	}

	if ( (col = _reader->_db->findColumn("_parent_oid")) != -1) {
		fromString(_parent_oid, std::string(static_cast<const char*>(_reader->_db->getRowField(col))));
	}

	if ( (col = _reader->_db->findColumn("_last_modified")) != -1) {
		_lastModified = _reader->_db->stringToTime(static_cast<const char*>(_reader->_db->getRowField(col)));
	}
	else {
		_lastModified = Core::None;
	}

	BaseObject *bobj = ClassFactory::Create(_rtti->className());
	if ( bobj == nullptr ) {
		SEISCOMP_ERROR("DatabaseIterator: object of type '%s' could not be created",
		               _rtti->className());
		_reader->_db->endQuery();
		return nullptr;
	}

	Object* obj = Object::Cast(bobj);
	if ( obj == nullptr ) {
		delete bobj;
		SEISCOMP_ERROR("DatabaseIterator: object of type '%s' could not be created",
		               _rtti->className());
		_reader->_db->endQuery();
		return nullptr;
	}

	if ( _lastModified ) {
		obj->setLastModifiedInArchive(*_lastModified);
	}

	_reader->serializeObject(obj);

	if ( !_reader->success() ) {
		SEISCOMP_WARNING("DatabaseIterator: error while reading object of type '%s': ignoring it",
		                 _rtti->className());
		delete obj;
		obj = nullptr;
	}

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator& DatabaseIterator::operator=(const DatabaseIterator &it) {
	_rtti = it._rtti;
	_reader = it._reader;
	_count = it._count;
	_object = it._object;
	_oid = it._oid;
	_parent_oid = it._parent_oid;
	_lastModified = it._lastModified;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseIterator::valid() const {
	return _reader != nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseIterator::next() {
	++(*this);
	return valid();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* DatabaseIterator::get() const {
	return _object.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DatabaseIterator::fieldCount() const {
	return _reader?_reader->_db->getRowFieldCount():0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *DatabaseIterator::field(size_t index) const {
	return _reader?(const char*)_reader->_db->getRowField(index):nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* DatabaseIterator::operator*() const {
	return _object.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator& DatabaseIterator::operator++() {
	while ( _reader->_db->fetchRow() ) {
		_object = fetch();
		if ( !_object ) continue;
		++_count;
		return *this;
	}

	close();
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator& DatabaseIterator::operator++(int) {
	return operator++();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseIterator::close() {
	if ( _reader ) {
		_reader->_db->endQuery();
		_reader = nullptr;
		_rtti = nullptr;
	}

	_object = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DatabaseIterator::count() const {
	return _count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseObjectWriter::DatabaseObjectWriter(DatabaseArchive &archive,
                                           bool addToDatabase)
: Visitor(addToDatabase?TM_TOPDOWN:TM_BOTTOMUP)
, _archive(archive)
, _addObjects(addToDatabase)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseObjectWriter::operator()(Object *object) {
	return (*this)(object, "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseObjectWriter::operator()(Object *object, const std::string &parentID) {
	if ( !_archive.driver() ) {
		return false;
	}

	_parentID = parentID;

	_errors = 0;
	_count = 0;

	object->accept(this);

	return _errors == 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseObjectWriter::visit(PublicObject *publicObject) {
	return write(publicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseObjectWriter::visit(Object *object) {
	write(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseObjectWriter::write(Object *object) {
	++_count;

	bool result = _addObjects ?
		_archive.insert(object, _parentID)
		:
		_archive.remove(object, _parentID)
	;

	if ( !result ) {
		++_errors;
		return false;
	}

	_parentID = "";

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::IO::DatabaseInterface *DatabaseArchive::driver() const {
	return _db.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::setDriver(Seiscomp::IO::DatabaseInterface *db) {
	_objectIdMutex.lock();
	_objectIdCache.clear();
	_objectIdMutex.unlock();

	_db = db;
	_errorMsg = "";

	if ( !fetchVersion() ) close();

	if ( _db ) {
		_publicIDColumn = _db->convertColumnName("publicID");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseArchive::DatabaseArchive(Seiscomp::IO::DatabaseInterface *i)
: _db(i)
, _objectAttributes(nullptr) {
	setHint(IGNORE_CHILDS);
	Object::RegisterObserver(this);
	_allowDbClose = false;

	if ( !fetchVersion() ) {
		close();
	}

	if ( _db ) {
		_publicIDColumn = _db->convertColumnName("publicID");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseArchive::~DatabaseArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::open(const char *dataSource) {
	_errorMsg = "";

	if ( !_db ) {
		return false;
	}

	if ( _db->isConnected() ) {
		return false;
	}

	if ( _db->connect(dataSource) ) {
		if ( !fetchVersion() ) {
			close();
			return false;
		}

		SEISCOMP_INFO("Connect to %s succeeded",dataSource);
		_allowDbClose = true;
		return true;
	}

	fetchVersion();

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::fetchVersion() {
	setVersion(Core::Version(0,0));

	if ( !_db ) {
		return false;
	}

	if ( !_db->beginQuery("select value from Meta where name='Schema-Version'") ) {
		SEISCOMP_WARNING("Unable to read schema version from database, "
		                 "assuming v0.0");
		_db->endQuery();
		return true;
	}

	if ( !_db->fetchRow() ) {
		_errorMsg = "Unable to read schema version from database (empty result set)";
		SEISCOMP_ERROR("%s", _errorMsg.c_str());
		_db->endQuery();
		return false;
	}

	Core::Version v;
	if ( !v.fromString((const char*)_db->getRowField(0)) ) {
		_errorMsg = "Invalid schema version in database: ";
		_errorMsg += (const char*)_db->getRowField(0);

		SEISCOMP_ERROR("%s", _errorMsg.c_str());
		_db->endQuery();
		return false;
	}

	_db->endQuery();

	setVersion(Core::Version(v.majorTag(), v.minorTag()));

	if ( version() > Core::Version(Version::Major, Version::Minor) ) {
		_errorMsg = "Database version v";
		_errorMsg += toString(static_cast<int>(version().majorTag())) + "." +
		             toString(static_cast<int>(version().minorTag()));
		_errorMsg += " not supported by client";
		SEISCOMP_ERROR("%s", _errorMsg.c_str());
		_db->endQuery();
		return false;
	}

	SEISCOMP_DEBUG("Found database version v%d.%d.%d",
	               v.majorTag(), v.minorTag(), v.patchTag());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::hasError() const {
	return !_errorMsg.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string DatabaseArchive::errorMsg() const {
	return _errorMsg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::create(const char* /*dataSource*/) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::close() {
	if ( _db && _allowDbClose ) {
		_db->disconnect();
	}

	_db = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::benchmarkQueries(int count) {
	for ( int i = 0; i < count; ++i ) {
		_db->beginQuery("select * from station where _network_oid=1");
		while ( _db->fetchRow() );
		_db->endQuery();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object *DatabaseArchive::queryObject(const Seiscomp::Core::RTTI &classType,
                                     const std::string &query) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return nullptr;
	}

	if ( !_db->beginQuery(query.c_str()) ) {
		SEISCOMP_ERROR("query [%s] failed", query.c_str());
		return nullptr;
	}

	if ( !_db->fetchRow() ) {
		_db->endQuery();
		return nullptr;
	}

	BaseObject *bobj = ClassFactory::Create(classType.className());
	if ( bobj == nullptr ) {
		SEISCOMP_ERROR("unable to create class of type '%s'", classType.className());
		_db->endQuery();
		return nullptr;
	}

	Object *obj = Object::Cast(bobj);
	if ( !obj ) {
		delete bobj;
		SEISCOMP_ERROR("unable to create class of type '%s'", classType.className());
		_db->endQuery();
		return nullptr;
	}

	serializeObject(obj);

	_db->endQuery();

	if ( !success() ) {
		delete obj;
		obj = nullptr;
	}

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject *DatabaseArchive::getObject(const RTTI &classType,
                                         const std::string &publicID) {
	if ( !classType.isTypeOf(PublicObject::TypeInfo()) ) {
		return nullptr;
	}

	std::stringstream ss;
	ss << "select " << PublicObject::ClassName() << "." << _publicIDColumn << "," <<
	        classType.className() << ".*"
	   << " from " << PublicObject::ClassName() << "," << classType.className()
	   << " where " << PublicObject::ClassName() << "._oid="
	                << classType.className() << "._oid"
	   << " and " << PublicObject::ClassName() << "." << _publicIDColumn << "='" << publicID << "'";

	Object *obj = queryObject(classType, ss.str());
	PublicObject *po = PublicObject::Cast(obj);
	if ( !po ) {
		delete obj;
		return nullptr;
	}

	return po;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjects(const std::string &parentID,
                                             const RTTI &classType,
                                             bool ignorePublicObject) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return DatabaseIterator();
	}

	if ( !parentID.empty() ) {
		OID parentID_ = publicObjectId(parentID);
		if ( !parentID_ ) {
			SEISCOMP_INFO("parent object with id '%s' not found in database", parentID.c_str());
			return DatabaseIterator();
		}

		return getObjectIterator(parentID_, classType, ignorePublicObject);
	}

	return getObjectIterator(0, classType, ignorePublicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjects(const PublicObject *parent,
                                             const Seiscomp::Core::RTTI &classType,
                                             bool ignorePublicObject) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return DatabaseIterator();
	}

	OID parentID = getCachedId(parent);
	if ( !parentID && parent ) {
		parentID = publicObjectId(parent->publicID());
		if ( !parentID ) {
			SEISCOMP_INFO("parent object with id '%s' not found in database", parent->publicID().c_str());
			return DatabaseIterator();
		}
		registerId(parent, parentID);
	}

	return getObjectIterator(parentID, classType, ignorePublicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DatabaseArchive::getObjectCount(const std::string &parentID,
                                       const Seiscomp::Core::RTTI &classType) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return 0;
	}

	std::stringstream ss;
	ss << "select count(*) from " << classType.className();
	if ( !parentID.empty() ) {
		ss << ",PublicObject where PublicObject._oid="
		   << classType.className() << "._parent_oid "
		      "and PublicObject." << _publicIDColumn << "='" << parentID << "'";
	}

	if ( !_db->beginQuery(ss.str().c_str()) ) {
		SEISCOMP_ERROR("starting query '%s' failed", ss.str().c_str());
		return 0;
	}

	size_t ret = 0;

	if ( _db->fetchRow() ) {
		ret = atoi((const char*)_db->getRowField(0));
	}

	_db->endQuery();

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::parentPublicID(const PublicObject *object) {
	std::string query;
	query = "select Parent." + _publicIDColumn +
	        " from PublicObject as Parent, PublicObject as Child, " +
	        object->className() +
	        " where Child._oid=" +
	        object->className() + "._oid and Parent._oid=" +
	        object->className() + "._parent_oid and Child." + _publicIDColumn + "='" +
	        toSQL(_db.get(), object->publicID()) + "'";

	if ( !_db->beginQuery(query.c_str()) ) {
		SEISCOMP_ERROR("starting query '%s' failed", query.c_str());
		return "";
	}

	if ( _db->fetchRow() )
		query = (const char*)_db->getRowField(0);
	else
		query = std::string();

	_db->endQuery();

	return query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DatabaseArchive::getObjectCount(const PublicObject *parent,
                                       const Seiscomp::Core::RTTI &classType) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return 0;
	}

	std::stringstream ss;
	ss << "select count(*) from " << classType.className();
	if ( parent ) {
		ss << ",PublicObject where PublicObject._oid="
		   << classType.className() << "._parent_oid "
		      "and PublicObject." << _publicIDColumn<< "='" << parent->publicID() << "'";
	}

	if ( !_db->beginQuery(ss.str().c_str()) ) {
		SEISCOMP_ERROR("starting query '%s' failed", ss.str().c_str());
		return 0;
	}

	size_t ret = 0;

	if ( _db->fetchRow() ) {
		ret = atoi((const char*)_db->getRowField(0));
	}

	_db->endQuery();

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjectIterator(OID parentID,
                                                    const RTTI &classType,
                                                    bool ignorePublicObject) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return DatabaseIterator();
	}

	std::string query;

	if ( ignorePublicObject || !classType.isTypeOf(PublicObject::TypeInfo()) )
		query = std::string("select * from ") + classType.className();
	else {
		std::stringstream ss;
		ss << "select " << PublicObject::ClassName() << "." << _publicIDColumn << ","
		   << classType.className() << ".* from "
		   << PublicObject::ClassName() << "," << classType.className()
		   << " where " << PublicObject::ClassName() << "._oid="
		   << classType.className() << "._oid";

		query = ss.str();
	}

	if ( parentID > 0 ) {
		if ( classType.isTypeOf(PublicObject::TypeInfo()) && !ignorePublicObject )
			query += " and ";
		else
			query += " where ";

		query += classType.className();
		query += "._parent_oid='" + toString(parentID) + "'";
	}

	return getObjectIterator(query, classType);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjectIterator(const std::string &query,
                                                    const Seiscomp::Core::RTTI &classType) {
	return getObjectIterator(query, &classType);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjectIterator(const std::string &query,
                                                    const Seiscomp::Core::RTTI *classType) {
	if ( !_db->beginQuery(query.c_str()) ) {
		SEISCOMP_ERROR("starting query '%s' failed", query.c_str());
		return DatabaseIterator();
	}

	if ( !_db->fetchRow() ) {
		_db->endQuery();
		return DatabaseIterator();
	}

	return DatabaseIterator(this, classType);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::toString(const Core::Time &value) const {
	return _db->timeToString(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::toString(const std::string &value) const {
	return toSQL(_db.get(), value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::toString(const char *value) const {
	return toSQL(_db.get(), std::string(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(int8_t &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(int16_t &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(int32_t &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(int64_t &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(float &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(double &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::complex<float> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::complex<double> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(bool &value) {
	if ( !strtobool(value, cfield()) ) {
		SEISCOMP_ERROR("DB: error in result field %d: could not cast '%s' to bool",
		               _fieldIndex, cfield());
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<char> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<int8_t> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<int16_t> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<int32_t> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<int64_t> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<float> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<double> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<std::string> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<Core::Time> &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<std::complex<double> > &value) {
	setValidity(fromString(value, sfield()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::string &value) {
	value.assign(cfield(), fieldSize());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(Time &value) {
	value = _db->stringToTime(cfield());
	if ( hint() & SPLIT_TIME ) {
		int microSeconds;
		_currentAttributeName += MICROSECONDS_POSTFIX;
		readAttrib();
		if ( cfield() != nullptr ) {
			if ( fromString(microSeconds, cfield()) )
				value.setUSecs(microSeconds);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(int8_t value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(int16_t value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(int32_t value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(int64_t value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(float value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(double value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::complex<float> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::complex<double> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(bool value) {
	writeAttrib(std::string(value?"'1'":"'0'"));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<char> &value) {
	writeAttrib("'" + toSQL(_db.get(), toString(value)) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<int8_t> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<int16_t> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<int32_t> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<int64_t> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<float> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<double> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<std::string> &value) {
	writeAttrib("'" + toSQL(_db.get(), toString(value)) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<Core::Time> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<std::complex<double> > &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::string &value) {
	writeAttrib("'" + toSQL(_db.get(), value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(Time &value) {
	writeAttrib("'" + toString(value) + "'");
	if ( hint() & SPLIT_TIME ) {
		std::string backupName = _currentAttributeName;
		_currentAttributeName += MICROSECONDS_POSTFIX;
		write((int)value.microseconds());
		_currentAttributeName = backupName;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::writeAttrib(OPT_CR(std::string) value) const {
	std::string indexName;
	std::string& index = indexName;

	if ( _currentAttributePrefix.empty() ) {
		if ( _currentAttributeName == "publicID" )
			return;
		index = _currentAttributeName;
	}
	else if ( _currentAttributeName.empty() )
		index = _currentAttributePrefix;
	else
		indexName = _currentAttributePrefix + ATTRIBUTE_SEPERATOR + _currentAttributeName;

	AttributeMap* map = _objectAttributes;
	if ( (hint() & INDEX_ATTRIBUTE) && _ignoreIndexAttributes )
		map = &_indexAttributes;

	if ( value )
		(*map)[_db->convertColumnName(index)] = *value;
	else
		(*map)[_db->convertColumnName(index)] = None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::renderAttributes(const AttributeMap &attributes) {
	SEISCOMP_DEBUG("collected attributes -- list:");
	std::cout << AttributeMapper(attributes) << std::endl;
	SEISCOMP_DEBUG("collected attributes -- end list");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::renderValues(const AttributeMap &attributes) {
	SEISCOMP_DEBUG("collected values -- list:");
	std::cout << ValueMapper(attributes) << std::endl;
	SEISCOMP_DEBUG("collected values -- end list");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseArchive::OID DatabaseArchive::publicObjectId(const std::string &publicId) {
	OID id = IO::DatabaseInterface::INVALID_OID;
	std::stringstream ss;
	ss << "select _oid from " << PublicObject::ClassName()
	   << " where " << _publicIDColumn << "='" << toSQL(_db.get(), publicId) << "'";
	if ( !_db->beginQuery(ss.str().c_str()) ) {
		return id;
	}

	if ( _db->fetchRow() ) {
		fromString(id, (const char*)_db->getRowField(0));
	}

	_db->endQuery();

	return id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseArchive::OID DatabaseArchive::objectId(Object *object, const std::string &parentID) {
	PublicObject *publicObject = PublicObject::Cast(object);
	if ( publicObject ) {
		return publicObjectId(publicObject->publicID());
	}

	_objectAttributes = &_rootAttributes;
	_objectAttributes->clear();
	_indexAttributes.clear();
	_childTables.clear();
	_childDepth = 0;

	_ignoreIndexAttributes = true;

	resetAttributePrefix();

	OID iParentID = 0;

	PublicObject* parentObject = object->parent();
	if ( parentObject != nullptr ) {
		iParentID = getCachedId(parentObject);
		if ( iParentID == 0 ) {
			iParentID = publicObjectId(parentObject->publicID());
			if ( iParentID ) {
				registerId(parentObject, iParentID);
			}
			else {
				SEISCOMP_ERROR("objectID: parent object with publicID '%s' has not been "
				               "found in the database", parentObject->publicID().c_str());
				return IO::DatabaseInterface::INVALID_OID;
			}
		}
	}
	else if ( !parentID.empty() ) {
		iParentID = publicObjectId(parentID);
		if ( !iParentID ) {
			SEISCOMP_ERROR("objectID: parent object with publicID '%s' has not been "
			               "found in the database", parentID.c_str());
			return IO::DatabaseInterface::INVALID_OID;
		}
	}
	else {
		SEISCOMP_ERROR("objectID: no parent object given");
		return IO::DatabaseInterface::INVALID_OID;
	}

	_isReading = false;

	_validObject = true;
	object->serialize(*this);
	if ( !_validObject ) {
		SEISCOMP_ERROR("failed to query for object");
		return IO::DatabaseInterface::INVALID_OID;
	}

	if ( _indexAttributes.empty() ) {
		SEISCOMP_WARNING("objectID: index is empty");
		_indexAttributes = *_objectAttributes;
		//_isReading = true;
		//return (long unsigned int)-1;
	}

	_indexAttributes["_parent_oid"] = toString(iParentID);

	std::stringstream ss;
	ss << "select _oid from " << object->className() << " where ";

	bool first = true;
	for ( AttributeMap::iterator it = _indexAttributes.begin();
	      it != _indexAttributes.end(); ++it ) {
		if ( !first )
			ss << " and ";
		ss << it->first;
		if ( it->second )
			ss << "=" << *it->second;
		else
			ss << " is null";
		first = false;
	}

	_isReading = true;

	if ( !_db->beginQuery(ss.str().c_str()) ) {
		return IO::DatabaseInterface::INVALID_OID;
	}

	OID id = IO::DatabaseInterface::INVALID_OID;

	if ( _db->fetchRow() ) {
		fromString(id, (const char*)_db->getRowField(0));
	}

	_db->endQuery();

	return id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseArchive::OID DatabaseArchive::insertObject() {
	std::stringstream ss;
	ss << "insert into " << Object::ClassName() << "(_oid) values("
	   << _db->defaultValue() << ")";

	if ( !_db->execute(ss.str().c_str()) ) {
		return IO::DatabaseInterface::INVALID_OID;
	}

	return _db->lastInsertId(Object::ClassName());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::insertRow(const std::string &table,
                                const AttributeMap &attribs,
                                const std::string &parentId) {
	std::stringstream ss;
	ss.precision(12);
	ss << "insert into " << table << "(";

	ss << AttributeMapper(attribs);

	ss << ") ";

	if ( parentId.empty() ) {
		ss << "values (";
	}
	else {
		ss << "select ";
	}

	ss << ValueMapper(attribs);

	if ( parentId.empty() ) {
		ss << ")";
	}
	else {
		ss << " from " << PublicObject::ClassName()
		   << " where " << PublicObject::ClassName()
		   << "." << _publicIDColumn << "='" << toSQL(_db.get(), parentId) << "'";
	}

	//SEISCOMP_DEBUG(ss.str().c_str());
	return _db->execute(ss.str().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::deleteObject(OID id) {
	std::stringstream ss;
	ss << "delete from " << Object::ClassName()
	   << " where _oid=" << id;
	SC_FMT_DEBUG("deleting object with id {}", id);
	return _db->execute(ss.str().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::insert(Object *obj, const std::string &parentId) {
	if ( !obj ) {
		return false;
	}

	if ( !validInterface() ) {
		setValidity(false);
		return false;
	}

	setValidity(false);

	_objectAttributes = &_rootAttributes;
	_objectAttributes->clear();
	_childTables.clear();
	_childDepth = 0;
	_ignoreIndexAttributes = false;

	resetAttributePrefix();

	PublicObject *po = PublicObject::Cast(obj);

	if ( po ) {
		if ( po->publicID().empty() ) {
			SEISCOMP_ERROR("%s::publicID is empty", obj->className());
			return false;
		}

		if ( publicObjectId(po->publicID()) > 0 ) {
			SEISCOMP_ERROR("%s '%s' already stored in database",
			               obj->className(), po->publicID().c_str());
			return false;
		}
	}

	_db->start();

	OID oid = insertObject();
	if ( oid == IO::DatabaseInterface::INVALID_OID ) {
		_db->rollback();
		return false;
	}

	if ( po ) {
		std::stringstream ss;
		ss << "insert into " << PublicObject::ClassName()
		   << "(_oid," << _publicIDColumn << ") values("
		   << oid << ",'" << toSQL(_db.get(), po->publicID()) << "')";

		if ( !_db->execute(ss.str().c_str()) ) {
			SEISCOMP_ERROR("writing %s '%s' failed",
			               obj->className(), po->publicID().c_str());
			_db->rollback();
			return false;
		}
	}

	_validObject = true;

	_isReading = false;
	obj->serialize(*this);
	_isReading = true;

	if ( !Core::Archive::success() ) {
		SEISCOMP_ERROR("serializing object with type '%s' failed", obj->className());
		_db->rollback();
		return false;
	}

	_rootAttributes["_oid"] = toString(oid);
	bool success = false;

	PublicObject *parentObject = obj->parent();
	if ( parentObject ) {
		OID iParentId = getCachedId(parentObject);
		if ( iParentId == 0 ) {
			iParentId = publicObjectId(parentObject->publicID());
			if ( iParentId ) {
				registerId(parentObject, iParentId);
			}
		}

		if ( iParentId ) {
			_rootAttributes["_parent_oid"] = toString(iParentId);
			success = insertRow(obj->className(), *_objectAttributes);
		}
	}
	else if ( !parentId.empty() ) {
		//_rootAttributes["_parent_oid"] = "(select _oid from " + std::string(PublicObject::ClassName()) + " where publicID='" + parentId + "')";
		OID iParentId = publicObjectId(parentId);
		if ( iParentId ) {
			_rootAttributes["_parent_oid"] = toString(iParentId);
			success = insertRow(obj->className(), *_objectAttributes);
		}
		else {
			SEISCOMP_ERROR("failed to get oid for object '%s'", parentId.c_str());
		}
	}
	else {
		success = insertRow(obj->className(), *_objectAttributes);
	}

	if ( success ) {
		_db->commit();
		registerId(obj, oid);
	}
	else {
		SEISCOMP_ERROR("writing object with type '%s' failed",
		                obj->className());
		_db->rollback();
	}

	_validObject = success;
	return success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::update(Object *object, const std::string &parentID) {
	setValidity(false);

	if ( !object ) {
		return false;
	}

	if ( !validInterface() ) {
		return false;
	}


	_objectAttributes = &_rootAttributes;
	_objectAttributes->clear();
	_indexAttributes.clear();
	_childTables.clear();
	_childDepth = 0;

	PublicObject *po = PublicObject::Cast(object);

	_ignoreIndexAttributes = (po == nullptr);

	resetAttributePrefix();

	OID iParentID = 0;
	OID iPublicID = 0;

	PublicObject *parentObject = object->parent();
	if ( parentObject ) {
		iParentID = getCachedId(parentObject);
		if ( iParentID == 0 ) {
			iParentID = publicObjectId(parentObject->publicID());
			if ( iParentID ) {
				registerId(parentObject, iParentID);
			}
			else {
				SEISCOMP_ERROR("update: parent object with publicID '%s' has not been "
				               "found in the database", parentObject->publicID().c_str());
				return false;
			}
		}
	}
	else if ( !parentID.empty() ) {
		iParentID = publicObjectId(parentID);
		if ( !iParentID ) {
			SEISCOMP_ERROR("update: parent object with publicID '%s' has not been "
			               "found in the database", parentID.c_str());
			return false;
		}
	}
	else {
		SEISCOMP_ERROR("update: no parent object given, aborting update");
		return false;
	}

	if ( po ) {
		iPublicID = getCachedId(po);
		if ( iPublicID == 0 ) {
			iPublicID = publicObjectId(po->publicID());
			if ( iPublicID )
				registerId(po, iPublicID);
		}

		if ( iPublicID == 0 ) {
			SEISCOMP_ERROR("update: object with publicID '%s' has not been found in the database",
			               po->publicID().c_str());
			return false;
		}
	}

	_validObject = true;

	_isReading = false;
	object->serialize(*this);
	_isReading = true;

	if ( !success() ) {
		SEISCOMP_ERROR("serializing updated object with type '%s' failed", object->className());
		return false;
	}

	if ( _objectAttributes->empty() ) {
		SEISCOMP_DEBUG("no update for object type '%s' possible, empty list of non-index attributes", object->className());
		return true;
	}

	if ( iPublicID ) {
		_indexAttributes["_oid"] = toString(iPublicID);
	}

	if ( _indexAttributes.empty() ) {
		SEISCOMP_ERROR("update: index is empty, no update possible");
		return false;
	}

	_indexAttributes["_parent_oid"] = toString(iParentID);

	std::stringstream ss;
	ss << "update " << object->className() << " set ";

	bool first = true;
	for ( AttributeMap::iterator it = _objectAttributes->begin();
	      it != _objectAttributes->end(); ++it ) {
		if ( !first ) ss << ",";
		ss << it->first << "=" << (it->second?*it->second:"NULL");
		first = false;
	}

	ss << " where ";

	first = true;
	for ( AttributeMap::iterator it = _indexAttributes.begin();
	      it != _indexAttributes.end(); ++it ) {
		if ( !first ) ss << " and ";
		ss << it->first;
		if ( it->second )
			ss << "=" << *it->second;
		else
			ss << " is null";
		first = false;
	}

	_validObject = _db->execute(ss.str().c_str());

	return success();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::remove(Object *object, const std::string &parentID) {
	setValidity(false);

	if ( !object ) {
		return false;
	}

	if ( !validInterface() ) {
		return false;
	}

	OID oid = getCachedId(object);
	if ( oid == IO::DatabaseInterface::INVALID_OID ) {
		oid = objectId(object, parentID);
	}

	if ( oid == IO::DatabaseInterface::INVALID_OID ) {
		SEISCOMP_WARNING("remove: object '%s' has not been found in database",
		                 object->className());
		return false;
	}

	_validObject = true;

	if ( !_db->execute((std::string("delete from ") + object->className() +
	                    " where _oid=" + toString(oid)).c_str()) ) {
		setValidity(false);
	}

	if ( PublicObject::Cast(object) ) {
		if ( !_db->execute((std::string("delete from ") + PublicObject::ClassName() +
		                    " where _oid=" + toString(oid)).c_str()) ) {
			setValidity(false);
		}
	}

	if ( !deleteObject(oid) ) {
		setValidity(false);
	}

	removeId(object);

	return success();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::readAttrib() const {
	if ( _currentAttributePrefix.empty() ) {
		_fieldIndex = _db->findColumn(_db->convertColumnName(_currentAttributeName).c_str());
		//SEISCOMP_DEBUG("looking up column '%s' %s", _currentAttributeName.c_str(), _fieldIndex != -1?"succeeded":"failed");
	}
	else {
		_fieldIndex = _db->findColumn(_db->convertColumnName(_currentAttributePrefix + ATTRIBUTE_SEPERATOR + _currentAttributeName).c_str());
		//SEISCOMP_DEBUG("looking up column '%s' %s", (_currentAttributePrefix + ATTRIBUTE_SEPERATOR + _currentAttributeName).c_str(), _fieldIndex != -1?"succeeded":"failed");
	}

	if ( _fieldIndex != -1 ) {
		_field = (const char*)_db->getRowField(_fieldIndex);
		_fieldSize = _db->getRowFieldSize(_fieldIndex);
	}
	else {
		_field = nullptr;
		_fieldSize = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::locateObjectByName(const char *name, const char *targetClass, bool nullable) {
	if ( !isReading() ) {
		if ( !isEmpty(targetClass) ) {
			if ( !isEmpty(name) )
				pushAttributePrefix(name);

			if ( hint() & DB_TABLE ) {
				_currentAttributeName = "";

				_currentChildTable = _childTables.insert(_childTables.end(), ChildTable());
				_currentChildTable->first = targetClass;
				_objectAttributes = &_currentChildTable->second;

				++_childDepth;
			}
			else if ( nullable ) {
				_currentAttributeName = OBJECT_USED_POSTFIX;
				write((bool)true);
			}
		}
		else {
			if ( !isEmpty(name) )
				_currentAttributeName = name;
			else
				_currentAttributeName.clear();
		}
		return true;
	}

	if ( !isEmpty(targetClass) && (hint() & STATIC_TYPE) ) {
		if ( !isEmpty(name) ) {
			pushAttributePrefix(name);
			_currentAttributeName = name;
		}

		if ( !(hint() & DB_TABLE) ) {
			// When the object is nullable a special column has been added
			// to signal whether the complete type is set or not.
			// This column is named '[attributeName]_used' and contains
			// either 0 or 1.
			if ( nullable ) {
				bool used = false;
				_currentAttributeName = OBJECT_USED_POSTFIX;
				readAttrib();

				if ( _field )
					strtobool(used, _field);

				if ( !used ) {
					popAttribPrefix();
					return false;
				}
			}
			return true;
		}

		_currentAttributeName = CHILD_ID_POSTFIX;
	}
	else
		_currentAttributeName = name;

	readAttrib();

	if ( hint() & DB_TABLE ) {
		if ( _field == nullptr ) {
			popAttribPrefix();
			return false;
		}

		OID childId;
		fromString(childId, _field);
		SEISCOMP_DEBUG("should read child table '%s' with _oid=%" PRIu64, targetClass, childId);
	}

	return _field != nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::locateNextObjectByName(const char *name, const char *targetClass) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::locateNullObjectByName(const char *name, const char *targetClass, bool /*first*/) {
	if ( !isEmpty(name) ) {
		if ( !isEmpty(targetClass) ) {
			if ( !(hint() & DB_TABLE) ) {
				_currentAttributeName = std::string(name) + ATTRIBUTE_SEPERATOR "" OBJECT_USED_POSTFIX;
				write((bool)false);
				return;
			}
			else
				return;
		}
		_currentAttributeName = name;
	}
	else {
		if ( isEmpty(targetClass) )
			_currentAttributeName.clear();
		else
			_currentAttributeName = targetClass;
	}

	writeAttrib(None);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void DatabaseArchive::resetAttributePrefix() const {
	_prefixPos = 0;
	_currentAttributePrefix = "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void DatabaseArchive::pushAttributePrefix(const char *name) const {
	if ( _prefixPos >= 64 ) {
		throw OverflowException("The attribute prefix cannot be pushed, stack overflow (more than 64 elements)");
		return;
	}

	_prefixOffset[_prefixPos++] = _currentAttributePrefix.size();
	if ( !name ) return;

	if ( _currentAttributePrefix.empty() )
		_currentAttributePrefix = name;
	else {
		_currentAttributePrefix += ATTRIBUTE_SEPERATOR;
		_currentAttributePrefix += name;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void DatabaseArchive::popAttribPrefix() const {
	--_prefixPos;
	if ( _prefixPos < 0 ) {
		_prefixPos = 0;
		return;
	}

	_currentAttributePrefix.erase(_prefixOffset[_prefixPos]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::serialize(RootType *object) {
	BaseObject::Archive::serialize(object);
	popAttribPrefix();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::serialize(SerializeDispatcher &disp) {
	if ( hint() & DB_TABLE ) {
		if ( !isReading() ) {
			std::string backupPrefix(_currentAttributePrefix);
			_currentAttributePrefix = "";
			BaseObject::Archive::serialize(disp);
			_currentAttributePrefix = backupPrefix;
			_currentAttributeName = CHILD_ID_POSTFIX;
			--_childDepth;

			if ( !_childDepth )
				_objectAttributes = &_rootAttributes;
			else {
				--_currentChildTable;
				_objectAttributes = &_currentChildTable->second;
			}

			if ( !insertRow(_currentChildTable->first,
			                _currentChildTable->second) )
				return;

			writeAttrib(toString(_db->lastInsertId(Object::ClassName())));
		}
		else {
			std::string backupPrefix(_currentAttributePrefix);
			_currentAttributePrefix = "";
			_currentAttributePrefix = backupPrefix;
		}
	}
	else
		BaseObject::Archive::serialize(disp);

	if ( hint() & STATIC_TYPE )
		popAttribPrefix();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::determineClassName() {
	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::setClassName(const char *) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::buildQuery(const std::string &table,
                                        const std::string &filter) {
	if ( filter.empty() )
		return std::string("select * from ") + table;
	else
		return std::string("select * from ") + table + " where " + filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::buildExtendedQuery(const std::string &what,
                                                   const std::string &tables,
                                                   const std::string &filter)
{
	if ( filter.empty() )
		return std::string("select ") + what + " from " + tables;
	else
		return std::string("select ") + what + " from " + tables +
		       " where " + filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::validInterface() const {
	return _db != nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::onObjectDestroyed(Object *object) {
	removeId(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseArchive::OID DatabaseArchive::getCachedId(const Object *o) const {
	_objectIdMutex.lock();

	ObjectIdMap::const_iterator it = _objectIdCache.find(o);
	if ( it == _objectIdCache.end() ) {
		_objectIdMutex.unlock();
		return IO::DatabaseInterface::INVALID_OID;
	}

	OID oid = it->second;
	_objectIdMutex.unlock();
	return oid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::registerId(const Object *o, OID id) {
	_objectIdMutex.lock();
	_objectIdCache[o] = id;
	_objectIdMutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseArchive::getCacheSize() const {
	_objectIdMutex.lock();
	size_t n = _objectIdCache.size();
	_objectIdMutex.unlock();
	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::serializeObject(Object *obj) {
	if ( obj == nullptr ) return;

	resetAttributePrefix();

	_validObject = true;
	obj->serialize(*this);

	if ( _db != nullptr && isReading() ) {
		int idId = _db->findColumn("_oid");
		if ( idId != -1 ) {
			OID oid;
			fromString(oid, (const char*)_db->getRowField(idId));
			registerId(obj, oid);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::removeId(Object *o) {
	_objectIdMutex.lock();

	ObjectIdMap::iterator it = _objectIdCache.find(o);
	if ( it != _objectIdCache.end() ) {
		_objectIdCache.erase(it);
	}

	_objectIdMutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
