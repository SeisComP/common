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


#define SEISCOMP_COMPONENT Gui::ImageTree

#include <seiscomp/gui/map/imagetree.h>
#include <seiscomp/gui/map/texturecache.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <seiscomp/logging/log.h>

#include <cstdio>
#include <clocale>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Gui::Map::TileStore, SC_GUI_API);


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString generateID(int level, int column, int row) {
	QString id = "";

	for ( int i = 0; i < level; ++i ) {
		int ofs = 1 << (level-i-1);
		int x,y,c;

		if ( column >= ofs ) {
			x = 1;
			column -= ofs;
		}
		else
			x = 0;

		if ( row >= ofs ) {
			y = 1;
			row -= ofs;
		}
		else
			y = 0;

		if ( x )
			c = y?3:0;
		else
			c = y?2:1;

		id += char('0' + c);
	}

	return id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString generatePath(int level, int col, int row,
                         const std::string &pattern) {
	QString path;

	for ( size_t i = 0; i < pattern.size(); ++i ) {
		if ( pattern[i] != '%' )
			path += pattern[i];
		else {
			++i;
			int len = 0;
			while ( i < pattern.size() ) {
				if ( pattern[i] >= '0' && pattern[i] <= '9' ) {
					len *= 10;
					len += int(pattern[i] - '0');
					++i;
					continue;
				}
				else if ( pattern[i] == '%' )
					path += pattern[i];
				else if ( pattern[i] == 's' )
					path += generateID(level, col, row);
				else if ( pattern[i] == 'l' )
					path += QString::number(level);
				else if ( pattern[i] == 'c' )
					path += QString::number(col);
				else if ( pattern[i] == 'r' )
					path += QString::number(row);
				break;
			}
		}
	}

	return path;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class TileDirectory : public TileStore {
	public:
		TileDirectory() {}

		bool open(MapsDesc &desc) override {
			_filePattern = desc.location.toStdString();

			if ( validate(0, 0, 0) ) {
				QString id = generatePath(0, 0, 0, _filePattern);
				QImage img(id);
				_tilesize = img.size();

				return true;
			}

			return false;
		}

		int maxLevel() const override {
			return TileIndex::MaxLevel;
		}

		LoadResult load(QImage &img, const TileIndex &tile) override {
			return img.load(getID(tile)) ? OK : Error;
		}

		QString getID(const TileIndex &tile) const override {
			return generatePath(tile.level(), tile.column(), tile.row(), _filePattern);
		}

		// Tiles are never loaded asynchronously
		bool hasPendingRequests() const override {
			return false;
		}

		void refresh() override {}


	protected:
		bool validate(int level, int column, int row) const {
			QString id = generatePath(level, column, row, _filePattern);

			FILE* fp = fopen(id.toLatin1(), "rb");
			if ( !fp ) return false;
			fclose(fp);

			return true;
		}

	private:
		std::string _filePattern;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TileStore::TileStore()
: _tree(nullptr) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TileStore::setImageTree(ImageTree *tree) {
	_tree = tree;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TileStore::finishedLoading(QImage &img, const TileIndex &tile) {
	if ( _tree )
		_tree->finishedLoading(img, tile);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TileStore::invalidate(const TileIndex &tile) {
	if ( _tree )
		_tree->invalidate(tile);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImageTree::ImageTree(const MapsDesc &meta) {
	if ( meta.type.isEmpty() )
		_store = new TileDirectory;
	else
		_store = TileStoreFactory::Create(meta.type.toLatin1());

	if ( _store ) {
		_store->setImageTree(this);

		MapsDesc desc(meta);

		const char *oldLocale = setlocale(LC_ALL, nullptr);

		if ( !_store->open(desc) ) {
			SEISCOMP_ERROR("Failed to open tile store at %s",
			               (const char*)desc.location.toLatin1());
			_store = nullptr;
		}
		else {
			_isMercatorProjected = desc.isMercatorProjected;
			_cacheSize = desc.cacheSize;
		}

		setlocale(LC_ALL, oldLocale);
	}
	else {
		SEISCOMP_ERROR("Could not create tile store: %s",
		               (const char*)meta.type.toLatin1());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImageTree::~ImageTree() {
	_cache = nullptr;
	_store = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TextureCache *ImageTree::getCache() {
	if ( !_cache && _store ) {
		_cache = new TextureCache(_store.get(), _isMercatorProjected);
		if ( _cacheSize > 0 )
			_cache->setCacheLimit(_cacheSize);
	}

	return _cache.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImageTree::refresh() {
	if ( _store ) _store->refresh();
	if ( _cache ) _cache->clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImageTree::finishedLoading(QImage &img, const TileIndex &tile) {
	if ( !_cache ) return;
	if ( _cache->setTexture(img, tile) )
		tilesUpdated();

	if ( !hasPendingRequests() )
		tilesComplete();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ImageTree::invalidate(const TileIndex &tile) {
	if ( _cache ) _cache->invalidateTexture(tile);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
