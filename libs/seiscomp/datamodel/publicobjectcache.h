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


#ifndef SEISCOMP_DATAMODEL_PUBLICOBJECT_CACHE_H__
#define SEISCOMP_DATAMODEL_PUBLICOBJECT_CACHE_H__


#include <seiscomp/core/timewindow.h>
#include <seiscomp/datamodel/publicobject.h>
#include <queue>
#include <functional>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DatabaseArchive);

struct SC_SYSTEM_CORE_API CachePopCallback {
	virtual void handle(PublicObject *obj) {}
	virtual ~CachePopCallback() {}
};


/**
 * @brief The PublicObjectCache class implements an PublicObject store to
 * prevent PublicObjects from de-registration when they go out of scope.
 *
 * The global registraty of PublicObject and the database access are common
 * patterns in user code. To simplify the following code sequence:
 *
 * @code
 * auto po = PublicObject::Find(publicID);
 * if ( !po ) {
 *     po = _archive->getObject(Pick::TypeInfo(), publicID)
 * }
 * auto pick = Pick::Cast(po);
 * if ( pick ) {
 *     // Do something
 * }
 * @endcode
 *
 * The cache encapsulates this code in a single function and can implement
 * several strategies for storing objects in memory. Currently a cache size
 * based strategy and a time span based strategy are implemented.
 *
 * The cache receives an optional pointer to the database archive to load
 * objects from database if required. That makes the object lookup a breeze:
 *
 * @code
 * auto pick = _cache.get<Pick>(publicID);
 * if ( pick ) {
 *     // Do something
 * }
 * @endcode
 *
 * The following two code examples illustrate the mode of operation of the
 * cache:
 *
 * @code
 * // A pick is being created and registered in the global object pool.
 * PickPtr pick = Pick::Create();
 * string publicID = pick->publicID();
 *
 * // Although the pick has never been added to the cache it will be found
 * // through the global object registry. And it will be added to the cache
 * // automatically for later retrieval.
 * auto cachedPick = _cache.get<Pick>(publicID);
 * cachedPick = nullptr; // Just drop the reference
 *
 * // Even if the pick pointer is set to nullptr which should cause the
 * // pick to be disposed (-> SmartPointer) it is not the case. The cache still
 * // manages a smart pointer to the pick and prevents it from being destroyed
 * // until it will be removed from the cache.
 * pick = nullptr;
 *
 * // The following lookup will succeed because the cache is still holding the
 * // pick.
 * cachedPick = _cache.get<Pick>(publicID);
 * @endcode
 *
 * Continuing from the above code snippet, a totally different part of the code
 * might want to lookup the object through the global registry. Due to the
 * cache storage, it will success:
 *
 * @code
 * // The pick is still registered globally because the cache is still holding
 * // it.
 * Pick *pick = PublicObject::Find(publicID);
 * @endcode
 */
class SC_SYSTEM_CORE_API PublicObjectCache : public Core::BaseObject {
	private:
		struct CacheItem;

		typedef std::map<std::string, CacheItem*> CacheLookup;

		// Simple double linked list
		struct CacheItem {
			PublicObjectPtr       object;
			time_t                timestamp;
			CacheItem            *prev;
			CacheItem            *next;
			CacheLookup::iterator lookup;
		};


	public:
		typedef std::function<void (PublicObject*)> PopCallback;
		typedef std::function<void (PublicObject*)> PushCallback;


	public:
		class const_iterator {
			// ------------------------------------------------------------------
			//  Xstruction
			// ------------------------------------------------------------------
			public:
				//! C'tor
				const_iterator();
				//! Copy c'tor
				const_iterator(const const_iterator &it);


			// ------------------------------------------------------------------
			//  Operators
			// ------------------------------------------------------------------
			public:
				PublicObject* operator*();

				const_iterator &operator=(const const_iterator &it);
				const_iterator &operator++();
				const_iterator  operator++(int);

				bool operator==(const const_iterator &it);
				bool operator!=(const const_iterator &it);


			// ------------------------------------------------------------------
			//  Interface
			// ------------------------------------------------------------------
			public:
				time_t timeStamp() const;


			// ------------------------------------------------------------------
			//  Implementation
			// ------------------------------------------------------------------
			private:
				const_iterator(CacheItem *);

				CacheItem *_item;

			friend class PublicObjectCache;
		};


	public:
		PublicObjectCache();
		PublicObjectCache(DatabaseArchive *ar);
		~PublicObjectCache() override;

	public:
		void setDatabaseArchive(DatabaseArchive *);

		void setPopCallback(const PopCallback &);
		void setPopCallback(CachePopCallback *);

		void removePopCallback();

		/**
		 * Insert a new object into the cache.
		 * @param po The PublicObject pointer to insert
		 * @return True or False
		 */
		virtual bool feed(PublicObject *po) = 0;

		/**
		 * Removes an object from the cache. This function
		 * can be quite time consuming since its a linear search.
		 * @param po The PublicObject pointer to be removed
		 * @return True or False
		 */
		bool remove(PublicObject *po);

		//! Clears the cache.
		void clear();

		/**
		 * @brief Retrieves the type information of a public object which
		 *        is either globally registered or stored in the cache.
		 *
		 * A database lookup will not be performed!
		 *
		 * @param publicID The publicID of the PublicObject
		 * @return The RTTI pointer if found, nullptr otherwise.
		 */
		const Core::RTTI *typeInfo(const std::string &publicID) const;

		/**
		 * @brief Retrieves an object by publicID.
		 *
		 * If the object is not in the cache and not globally registered it
		 * will be fetched from the database and inserted into the cache.
		 * Please note that any object retrieved will be fed again to the
		 * cache to raise its priority.
		 *
		 * @param classType The required class type of the searched objects.
		 *                  If the object is not the same type or derived from
		 *                  it then nullptr will be returned.
		 * @param publicID The publicID of the PublicObject to be
		 *                 retrieved
		 * @return The PublicObject pointer or nullptr
		 */
		PublicObject *find(const Core::RTTI &classType,
		                   const std::string &publicID);

		/**
		 * Returns the cached state the the last object returned by find.
		 * This function was introduced in API 1.1.
		 * @return Whether an already cached object has been returned or not
		 */
		bool cached() const { return _cached; }

		//! Time window currently in buffer
		Core::TimeWindow timeWindow() const;

		template <typename T>
		typename Core::SmartPointer<T>::Impl
		get(const std::string &publicID) {
			return static_cast<T*>(find(T::TypeInfo(), publicID));
		}

		//! Returns the time of the oldest entry
		Core::Time oldest() const;

		//! Returns whether the cache is empty or not
		bool empty() const { return _front == nullptr; }

		//! Returns the number of cached elements
		size_t size() const { return _size; }

		const_iterator begin() const;
		const_iterator end() const;

		/**
		 * @brief Checks if the passed object pointer is part of the cache.
		 * @param object The object to be checked
		 * @return true if part of this cache, false otherwise
		 */
		bool contains(PublicObject *object) const;

		/**
		 * @brief Checks if an instance with the passed publicID is part of
		 *        the cache.
		 * @param publicID The publicID of the object that is part of the cache.
		 * @return true if part of this cache, false otherwise
		 */
		bool contains(const std::string &publicID) const;


	protected:
		void pop();
		void push(PublicObject *obj);

		void setCached(bool cached) { _cached = cached; }
		DatabaseArchive *databaseArchive() { return _archive.get(); }


	private:
		DatabaseArchivePtr _archive;
		size_t             _size;
		CacheItem         *_front;
		CacheItem         *_back;
		CacheLookup        _lookup;
		bool               _cached;

		PushCallback       _pushCallback;
		PopCallback        _popCallback;
};


class SC_SYSTEM_CORE_API PublicObjectRingBuffer : public PublicObjectCache {
	public:
		PublicObjectRingBuffer();
		PublicObjectRingBuffer(DatabaseArchive *ar,
		                       size_t bufferSize);

	public:
		bool setBufferSize(size_t bufferSize);

		bool feed(PublicObject *po) override;

	private:
		size_t _bufferSize;
};


class SC_SYSTEM_CORE_API PublicObjectTimeSpanBuffer : public PublicObjectCache {
	public:
		PublicObjectTimeSpanBuffer();
		PublicObjectTimeSpanBuffer(DatabaseArchive *ar,
		                           const Core::TimeSpan &length);

	public:
		bool setTimeSpan(const Core::TimeSpan &);

		bool feed(PublicObject *po) override;

	private:
		Core::TimeSpan _timeSpan;
};


}
}


#endif
