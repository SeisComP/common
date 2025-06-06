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

#ifndef SEISCOMP_DATAMODEL_DATABASEARCHIVE_H__
#define SEISCOMP_DATAMODEL_DATABASEARCHIVE_H__

#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/io.h>
#include <seiscomp/io/database.h>
#include <seiscomp/datamodel/publicobject.h>

#include <list>
#include <mutex>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DatabaseArchive);


/**
 * \brief An iterator class to iterate over a sequence of database objects.
 * The iteration is done on the database side. Only the current object
 * is hold in memory unless a reference of previous objects has been
 * stored somewhere else.
 * The iterator does not destroy or end a started query in its
 * destructor. The query will be finished after iterating over all
 * objects. To stop an iteration you have to call close() explicitly
 * unless you receive a nullptr object at the end of iteration.
 */
class SC_SYSTEM_CORE_API DatabaseIterator : public Seiscomp::Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef IO::DatabaseInterface::OID OID;


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	protected:
		//! Protected c'tor used by DatabaseArchive
		DatabaseIterator(DatabaseArchive *database,
		                 const Seiscomp::Core::RTTI *rtti);

	public:
		//! C'tor
		DatabaseIterator();
		//! Copy c'tor
		DatabaseIterator(const DatabaseIterator &iter);

		//! D'tor
		~DatabaseIterator();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		Object *get() const;

		//! Returns the current result column count
		size_t fieldCount() const;

		//! Returns the current result field
		const char *field(size_t index) const;

		DatabaseIterator &operator=(const DatabaseIterator& it);
		Object* operator*() const;

		DatabaseIterator &operator++();
		DatabaseIterator &operator++(int);

		//! Returns if the current objectiterator is valid
		//! and has a valid result set
		bool valid() const;

		//! Steps to the next result set and returns valid()
		bool next();

		/**
		 * Closes the iterator and ends the started query.
		 * This method is useful if you want to break an
		 * iteration and to start a new one.
		 */
		void close();

		//! Returns the number of elements read
		size_t count() const;

		Core::Time lastModified() const {
			if ( _lastModified ) return *_lastModified;
			throw Core::ValueException("DatabaseIterator.lastModified is not set");
		}

		OID oid() const { return _oid; }
		OID parentOid() const { return _parent_oid; }

		//! Returns whether the object has been read from the database
		//! directly (false) or fetched from the global object pool (true)
		bool cached() const { return _cached; }


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		Object *fetch() const;


	private:
		const Seiscomp::Core::RTTI *_rtti;
		DatabaseArchive *_reader;
		mutable size_t _count;
		ObjectPtr _object;

		mutable OID _oid;
		mutable OID _parent_oid;
		mutable bool _cached;
		mutable OPT(Core::Time) _lastModified;

	//! Make DatabaseArchive a friend class
	friend class DatabaseArchive;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class SC_SYSTEM_CORE_API DatabaseObjectWriter : protected Visitor {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		DatabaseObjectWriter(DatabaseArchive &archive,
		                     bool addToDatabase = true);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Does the actual writing
		bool operator()(Object *object);
		bool operator()(Object *object, const std::string &parentID);

		//! Returns the number of handled objects
		int count() const { return _count; }
		//! Returns the number of errors while writing
		int errors() const { return _errors; }


	// ----------------------------------------------------------------------
	//  Visitor interface
	// ----------------------------------------------------------------------
	protected:
		bool visit(PublicObject *publicObject) override;
		void visit(Object *object) override;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		bool write(Object *object);

	private:
		DatabaseArchive &_archive;
		std::string      _parentID;
		bool             _addObjects{true};
		int              _errors{0};
		int              _count{0};
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * \brief A class containing basic functionality to read and write
 * \brief schema objects from and to a database.
 */
class SC_SYSTEM_CORE_API DatabaseArchive : protected Core::Archive,
                                           public Observer {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		using OID = IO::DatabaseInterface::OID;


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		//! @param dbDriver The database driver used to access a
		//!                 database. If the database has not been
		//!                 opened by the DatabaseReader it will not
		//!                 be closed by it. Neither by destruction nor
		//!                 by calling DatabaseReader::close().
		DatabaseArchive(Seiscomp::IO::DatabaseInterface* dbDriver);

		//! Destructor
		virtual ~DatabaseArchive();

	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		using Seiscomp::Core::Archive::version;
		using Seiscomp::Core::Archive::versionMajor;
		using Seiscomp::Core::Archive::versionMinor;
		using Seiscomp::Core::Archive::isVersion;
		using Seiscomp::Core::Archive::isLowerVersion;
		using Seiscomp::Core::Archive::isHigherVersion;
		using Seiscomp::Core::Archive::supportsVersion;


		//! Implements derived  method
		//! @param dataSource user:password@host:port/database
		bool open(const char* dataSource) override;

		//! Implements derived  method
		void close() override;

		//! Returns the used database driver
		Seiscomp::IO::DatabaseInterface* driver() const;

		//! Sets the database driver to use
		void setDriver(Seiscomp::IO::DatabaseInterface*);

		//! Returns if the archive is in an erroneous state eg after setting
		//! a database interface.
		bool hasError() const;

		//! Returns the error message if hasError() return true
		const std::string errorMsg() const;

		void benchmarkQueries(int count);

		/**
		 * @brief Returns the query to fetch objects of a particular type.
		 * @param parentID The publicID of the parent object. When empty then
		 *                 it won't be included in the query.
		 * @param classType The type of the object to be read.
		 * @param ignorePublicObject If true then the PublicObject table will
		 *                           not be joined. That might be important if
		 *                           during a schema evolution an objectsturned
		 *                           into a PublicObject but an old version
		 *                           should be read.
		 * @return The query and a flag if a where clause is present.
		 */
		std::pair<std::string,bool> getObjectsQuery(const std::string &parentID,
		                                            const Seiscomp::Core::RTTI &classType,
		                                            bool ignorePublicObject = false);

		/**
		 * Reads a public object from the database.
		 * @param classType The type of the object to be read. The type has
		 *                  to be derived from PublicObject
		 * @param publicId The publicId of the public object
		 * @return An unmanaged object pointer. The ownership goes over
		 *         to the caller.
		 */
		PublicObject* getObject(const Seiscomp::Core::RTTI &classType,
		                        const std::string &publicID);

		/**
		 * Returns an iterator over all objects of a given type.
		 * @param parentID The publicID of the parent object. When empty,
		 *                 an iterator for all objects with type 'classType'
		 *                 is returned.
		 * @param classType The type of the objects to iterate over.
		 * @param ignorePublicObject If true then the PublicObject table will
		 *                           not be joined. That might be important if
		 *                           during a schema evolution an objects turned
		 *                           into a PublicObject but an old version
		 *                           should be read.
		 * @return The database iterator
		 */
		DatabaseIterator getObjects(const std::string &parentID,
		                            const Seiscomp::Core::RTTI &classType,
		                            bool ignorePublicObject = false);

		/**
		 * Returns an iterator over all objects of a given type for a parent
		 * object.
		 * @param parent The parent object. When nullptr, an iterator for all
		 *               objects with type 'classType' is returned.
		 * @param classType The type of the objects to iterate over.
		 * @param ignorePublicObject If true then the PublicObject table will
		 *                           not be joined. That might be important if
		 *                           during a schema evolution an objects turned
		 *                           into a PublicObject but an old version
		 *                           should be read.
		 * @return The database iterator
		 */
		DatabaseIterator getObjects(const PublicObject *parent,
		                            const Seiscomp::Core::RTTI &classType,
		                            bool ignorePublicObject = false);

		/**
		 * Returns the number of objects of a given type.
		 * @param parentID The publicID of the parent object. When empty,
		 *                 an iterator for all objects with type 'classType'
		 *                 is returned.
		 * @param classType The type of the objects to iterate over.
		 * @return The object count
		 */
		size_t getObjectCount(const std::string &parentID,
		                      const Seiscomp::Core::RTTI &classType);

		/**
		 * Returns the number of objects of a given type for a parent
		 * object.
		 * @param parent The parent object. When nullptr, an iterator for all
		 *               objects with type 'classType' is returned.
		 * @param classType The type of the objects to iterate over.
		 * @return The object count
		 */
		size_t getObjectCount(const PublicObject *parent,
		                      const Seiscomp::Core::RTTI &classType);


		//! Returns the database id for an object
		//! @return The id or 0 of no id was cached for this object
		OID getCachedId(const Object*) const;

		/**
		 * Returns the publicID of the parent object if any.
		 * @param object The PublicObject whose parent is queried.
		 * @return The publicID of the parent or an empty string.
		 */
		std::string parentPublicID(const PublicObject *object);

		/**
		 * Inserts an object into the database.
		 * @param object The object to be written
		 * @param parentId The publicID of the parent object used
		 *                 when no parent pointer is set.
		 */
		bool insert(Object* object, const std::string &parentID = "");

		/**
		 * Updates an object in the database. While serializing the
		 * index attributes (Archive hint: INDEX_ATTRIBUTE) will not be
		 * recognized when object is not a PublicObject. Then the index
		 * attributes will be used to lookup the corresponding row in the
		 * object table. The publicID is readonly in any case.
		 * @param object The object to be updated
		 * @param parentId The publicID of the parent object used
		 *                 when no parent pointer is set.
		 */
		bool update(Object *object, const std::string &parentID = "");

		/**
		 * Removes an object in the database. If the object is not a
		 * PublicObject the index attributes (serialized with Archive hint
		 * set to INDEX_ATTRIBUTE) will be used instead.
		 * @param object The object to be removed
		 * @param parentID The parentID of the parent objects used
		 *                 when no parent pointer is set.
		 */
		bool remove(Object *object, const std::string &parentID = "");

		//! Returns an iterator for objects of a given type.
		DatabaseIterator getObjectIterator(const std::string &query,
		                                   const Seiscomp::Core::RTTI &classType);

		DatabaseIterator getObjectIterator(const std::string &query,
		                                   const Seiscomp::Core::RTTI *classType);

		template <typename T>
		std::string toString(const T& value) const { return Core::toString(value); }

		std::string toString(const std::string &value) const;
		std::string toString(const char *value) const;
		std::string toString(const Core::Time& value) const;


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		//! Implements derived  method
		bool create(const char* dataSource) override;


	// ----------------------------------------------------------------------
	//  Protected Archive Interface
	// ----------------------------------------------------------------------
	protected:
		//! Reads an integer
		virtual void read(std::int8_t& value) override;
		virtual void read(std::int16_t& value) override;
		virtual void read(std::int32_t& value) override;
		virtual void read(std::int64_t& value) override;
		//! Reads a float
		virtual void read(float& value) override;
		//! Reads a double
		virtual void read(double& value) override;
		//! Reads a float complex
		virtual void read(std::complex<float>& value) override;
		//! Reads a double complex
		virtual void read(std::complex<double>& value) override;
		//! Reads a boolean
		virtual void read(bool& value) override;

		//! Reads a vector of native types
		virtual void read(std::vector<char>& value) override;
		virtual void read(std::vector<int8_t>& value) override;
		virtual void read(std::vector<int16_t>& value) override;
		virtual void read(std::vector<int32_t>& value) override;
		virtual void read(std::vector<int64_t>& value) override;
		virtual void read(std::vector<float>& value) override;
		virtual void read(std::vector<double>& value) override;
		virtual void read(std::vector<std::string>& value) override;
		virtual void read(std::vector<Core::Time>& value) override;

		//! Reads a vector of complex doubles
		virtual void read(std::vector<std::complex<double> >& value) override;

		//! Reads a string
		virtual void read(std::string& value) override;

		//! Reads a time
		virtual void read(Seiscomp::Core::Time& value) override;


	// ------------------------------------------------------------------
	//  Write methods
	// ------------------------------------------------------------------
	protected:
		//! Writes an integer
		virtual void write(std::int8_t value) override;
		virtual void write(std::int16_t value) override;
		virtual void write(std::int32_t value) override;
		virtual void write(std::int64_t value) override;
		//! Writes a float
		virtual void write(float value) override;
		//! Writes a double
		virtual void write(double value) override;
		//! Writes a float complex
		virtual void write(std::complex<float>& value) override;
		//! Writes a double complex
		virtual void write(std::complex<double>& value) override;
		//! Writes a boolean
		virtual void write(bool value) override;

		//! Writes a vector of native types
		virtual void write(std::vector<char>& value) override;
		virtual void write(std::vector<int8_t>& value) override;
		virtual void write(std::vector<int16_t>& value) override;
		virtual void write(std::vector<int32_t>& value) override;
		virtual void write(std::vector<int64_t>& value) override;
		virtual void write(std::vector<float>& value) override;
		virtual void write(std::vector<double>& value) override;
		virtual void write(std::vector<std::string>& value) override;
		virtual void write(std::vector<Core::Time>& value) override;

		//! Writes a vector of complex doubles
		virtual void write(std::vector<std::complex<double> >& value) override;

		//! Writes a string
		virtual void write(std::string& value) override;

		//! Writes a time
		virtual void write(Seiscomp::Core::Time& value) override;


	// ------------------------------------------------------------------
	//  Protected observer interface
	// ------------------------------------------------------------------
	protected:
		virtual void onObjectDestroyed(Object* object) override;


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		//! Implements derived  method
		virtual bool locateObjectByName(const char* name, const char* targetClass, bool nullable) override;
		//! Implements derived  method
		virtual bool locateNextObjectByName(const char* name, const char* targetClass) override;
		//! Implements derived  method
		virtual void locateNullObjectByName(const char* name, const char* targetClass, bool first) override;

		//! Implements derived  method
		virtual std::string determineClassName() override;

		//! Implements derived  method
		virtual void setClassName(const char*) override;

		//! Implements derived  method
		void serialize(RootType* object) override;

		//! Implements derived  method
		void serialize(SerializeDispatcher&) override;

		std::string buildQuery(const std::string& table,
		                       const std::string& filter = "");
		std::string buildExtendedQuery(const std::string& what,
		                               const std::string& tables,
		                               const std::string& filter = "");

		bool validInterface() const;

		//! Associates an objects with an id and caches
		//! this information
		void registerId(const Object*, OID id);

		//! Returns the number of cached object
		int getCacheSize() const;

		//! Serializes an objects and registeres its id in the cache
		void serializeObject(Object*);

		Object* queryObject(const Seiscomp::Core::RTTI& classType,
		                    const std::string& query);


	// ----------------------------------------------------------------------
	//  Privates types
	// ----------------------------------------------------------------------
	private:
		typedef std::map<const Object*, OID> ObjectIdMap;
		typedef std::map<std::string, OPT(std::string)> AttributeMap;

		typedef std::pair<std::string, AttributeMap> ChildTable;
		typedef std::list<ChildTable> ChildTables;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		bool fetchVersion();

		//! Removes an objects from the id cache
		void removeId(Object*);

		//! Returns the current field content
		const char* cfield() const { return _field; }

		std::string sfield() const { return std::string(_field, _fieldSize); }

		//! Returns the current field size
		size_t fieldSize() const { return _fieldSize; }

		//! Writes an attribute into the attribute map
		void writeAttrib(OPT_CR(std::string) value) const;

		//! Reads an attribute from the query result
		void readAttrib() const;

		//! Resets the attribute prefix
		void resetAttributePrefix() const;

		//! Appends a name to the current attribute prefix
		void pushAttributePrefix(const char* name) const;

		//! Removes the last attribute prefix when using complex types
		//! in one table.
		void popAttribPrefix() const;

		void renderAttributes(const AttributeMap&);
		void renderValues(const AttributeMap&);

		//! Returns an iterator for objects of a given type.
		DatabaseIterator getObjectIterator(OID parentID,
		                                   const Seiscomp::Core::RTTI& classType,
		                                   bool ignorePublicObject = false);

		//! Queries for the database id of a PublicObject for
		//! a given publicID
		OID publicObjectId(const std::string& publicId);

		//! Queries for the database id of an Object
		OID objectId(Object*, const std::string& parentID);

		//! Insert a base object column and return its database id
		OID insertObject();

		//! Insert a row into a table
		bool insertRow(const std::string& table,
		               const AttributeMap& attributes,
		               const std::string& parentId = "");

		//! Delete an object with a given database id
		bool deleteObject(OID id);

	protected:
		Seiscomp::IO::DatabaseInterfacePtr _db;

	private:
		std::string _errorMsg;
		std::string _publicIDColumn;

		mutable std::mutex _objectIdMutex;
		mutable ObjectIdMap _objectIdCache;
		mutable int _fieldIndex;
		mutable const char* _field;
		mutable size_t _fieldSize;

		mutable AttributeMap _rootAttributes;
		mutable AttributeMap _indexAttributes;
		mutable AttributeMap* _objectAttributes;
		mutable ChildTables _childTables;
		mutable ChildTables::iterator _currentChildTable;
		mutable int _childDepth;

		mutable std::string _currentAttributePrefix;
		mutable std::string _currentAttributeName;
		mutable bool _ignoreIndexAttributes;

		mutable int _prefixPos;
		mutable std::string::size_type _prefixOffset[64];

		bool _allowDbClose;

	friend class DatabaseIterator;
	friend class AttributeMapper;
	friend class ValueMapper;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}


#endif
