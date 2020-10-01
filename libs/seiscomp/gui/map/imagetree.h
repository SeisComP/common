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
#endif
#include <seiscomp/gui/core/maps.h>
#include <seiscomp/gui/map/maptree.h>
#include <seiscomp/gui/map/texturecache.h>


namespace Seiscomp {
namespace Gui {
namespace Map {


class ImageTree;


DEFINE_SMARTPOINTER(TileStore);
class SC_GUI_API TileStore : public Alg::MapTree {
	public:
		TileStore();

		//! Sets the parent image tree that gets notificiations about
		//! state changed, e.g. finishedLoading.
		void setImageTree(ImageTree *tree);

		//! Opens a tile repository and sets the desc flags accordingly
		//! if necessary.
		virtual bool open(MapsDesc &desc) = 0;

		//! Load a tile for the given node
		virtual bool load(QImage &img, Alg::MapTreeNode *node) = 0;

		//! Return a unique ID for a node
		virtual QString getID(const MapTreeNode *node) const = 0;

		//! Validate the existance of a tile
		virtual bool validate(int level, int column, int row) const = 0;

		virtual bool hasPendingRequests() const = 0;

		//! Refresh the image store, e.g. invalidate its cache
		virtual void refresh() = 0;


	protected:
		//! Async notification that a tile has been loaded.
		void finishedLoading(QImage &img, Alg::MapTreeNode *node);

		//! Invalidates the tile of a particular node
		void invalidate(Alg::MapTreeNode *node);


	protected:
		ImageTree *_tree;
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
		void finishedLoading(QImage &img, Alg::MapTreeNode *node);
		void invalidate(Alg::MapTreeNode *node);


	signals:
		void tilesUpdated();

		//! Emitted when all tiles are loaded and no asynchronous requests
		//! are still pending.
		//! This signal was introduced in API 1.1.
		void tilesComplete();


	protected:
		TextureCachePtr  _cache;
		TileStorePtr     _store;
		bool             _isMercatorProjected;
		size_t           _cacheSize;


	friend class TileStore;
};


}
}
}


#endif
