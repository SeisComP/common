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


#ifndef SEISCOMP_GUI_MAP_IMAGETREE_H
#define SEISCOMP_GUI_MAP_IMAGETREE_H


#ifndef Q_MOC_RUN
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/core/baseobject.h>
#endif
#include <seiscomp/gui/core/maps.h>

#include <QImage>


namespace Seiscomp {
namespace Gui {
namespace Map {


DEFINE_SMARTPOINTER(TextureCache);
class ImageTree;


struct SC_GUI_API TileIndex {
	typedef uint64_t Storage;
	enum Traits {
		Invalid = uint64_t(-1),
		MaxLevel = 29,
		LevelBits = 5,
		RowBits = MaxLevel,
		ColumnBits = MaxLevel
	};

	Storage id;

	TileIndex();
	TileIndex(uint8_t level, uint32_t row, uint32_t column);

	uint8_t level() const;
	uint32_t row() const;
	uint32_t column() const;

	TileIndex parent() const;

	bool operator==(const TileIndex &other) const;
	bool operator!=(const TileIndex &other) const;
	bool operator<(const TileIndex &other) const;

	operator bool() const;
};


std::ostream &operator<<(std::ostream &os, const TileIndex &index);


DEFINE_SMARTPOINTER(TileStore);
class SC_GUI_API TileStore : public Core::BaseObject {
	public:
		enum LoadResult {
			OK,
			Deferred,
			Error
		};

		TileStore();

	public:
		const QSize &tileSize() const { return _tilesize; }

		//! Sets the parent image tree that gets notificiations about
		//! state changed, e.g. finishedLoading.
		void setImageTree(ImageTree *tree);

	public:
		virtual int maxLevel() const = 0;

		//! Opens a tile repository and sets the desc flags accordingly
		//! if necessary.
		virtual bool open(MapsDesc &desc) = 0;

		/**
		 * @brief Load a tile for a given index.
		 * Loading can happen synchronously or asynchronously. The later case
		 * should return the status LoadResult::Deferred and an invalid
		 * image. If a deferred image could not be loaded by the store it
		 * should then call invalidate with the failed index.
		 * @param img The image to be populated.
		 * @param tile The tile index
		 * @return Success flag.
		 */
		virtual LoadResult load(QImage &img, const TileIndex &tile) = 0;

		//! Return a unique ID for a node
		virtual QString getID(const TileIndex &tile) const = 0;

		//! Validate the existance of a tile
		virtual bool validate(int level, int column, int row) const = 0;

		virtual bool hasPendingRequests() const = 0;

		//! Refresh the image store, e.g. invalidate its cache
		virtual void refresh() = 0;


	protected:
		//! Async notification that a tile has been loaded.
		void finishedLoading(QImage &img, const TileIndex &tile);

		//! Async notification that a tile has been loaded.
		void loadingComplete(QImage &img, TileIndex tile);
		//! Async notification that loading a tile has been aborted
		void loadingCancelled(TileIndex tile);

		//! Invalidates the tile of a particular node
		void invalidate(const TileIndex &tile);


	protected:
		ImageTree *_tree;
		QSize      _tilesize;

};


DEFINE_INTERFACE_FACTORY(TileStore);

#define REGISTER_TILESTORE_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Gui::Map::TileStore, Class> __##Class##InterfaceFactory__(Service)


DEFINE_SMARTPOINTER(ImageTree);
class SC_GUI_API ImageTree : public QObject, public Core::BaseObject {
	Q_OBJECT

	public:
		ImageTree(const MapsDesc &desc);
		~ImageTree();


	public:
		bool valid() const { return _store.get(); }

		//! This function was introduced in API 1.1
		bool hasPendingRequests() const { return _store && _store->hasPendingRequests(); }

		//! Returns the currently attached cache instance.
		//! If no cache is yet attached a new cache is
		//! created and stored in the object.
		TextureCache *getCache();

		//! Empties the texture cache and tells the store to do a refresh
		//! as well.
		//! This function was introduced in API 1.1.
		void refresh();


	public:
		void finishedLoading(QImage &img, const TileIndex &tile);
		void loadingComplete(QImage &img, TileIndex tile);
		void loadingCancelled(TileIndex tile);
		void invalidate(const TileIndex &tile);


	signals:
		void tilesUpdated();

		//! Emitted when all tiles are loaded and no asynchronous requests
		//! are still pending.
		//! This signal was introduced in API 1.1.
		void tilesComplete();


	protected:
		TextureCachePtr _cache;
		TileStorePtr    _store;
		bool            _isMercatorProjected;
		size_t          _cacheSize;


	friend class TileStore;
};


inline TileIndex::TileIndex() : id(Invalid) {}
inline TileIndex::TileIndex(uint8_t level, uint32_t row, uint32_t column) {
	id = ((Storage(level) & ((1 << LevelBits) - 1)) << (RowBits + ColumnBits)) |
	     ((Storage(row) & ((1 << RowBits) - 1)) << ColumnBits) |
	     (Storage(column) & ((1 << ColumnBits) - 1));
}

inline uint8_t TileIndex::level() const {
	return (id >> (RowBits + ColumnBits)) & ((1 << LevelBits) - 1);
}

inline uint32_t TileIndex::row() const {
	return (id >> ColumnBits) & ((1 << RowBits) - 1);
}

inline uint32_t TileIndex::column() const {
	return id & ((1 << ColumnBits) - 1);
}

inline TileIndex TileIndex::parent() const {
	auto l = level();
	return l ? TileIndex(l-1, row() >> 1, column() >> 1) : TileIndex();
}

inline bool TileIndex::operator==(const TileIndex &other) const {
	return id == other.id;
}

inline bool TileIndex::operator!=(const TileIndex &other) const {
	return id != other.id;
}

inline bool TileIndex::operator<(const TileIndex &other) const {
	return id < other.id;
}

inline TileIndex::operator bool() const {
	return id != Invalid;
}

inline std::ostream &operator<<(std::ostream &os, const TileIndex &index) {
	os << static_cast<int>(index.level()) << "/" << index.row() << "/" << index.column();
	return os;
}


}
}
}


#endif
