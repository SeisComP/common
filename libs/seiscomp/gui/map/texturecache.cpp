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



#include <QHash>
#include <QMutex>

#include <iostream>

#include <seiscomp/gui/map/texturecache.h>
#include <seiscomp/gui/map/imagetree.h>



namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QMap<QString, TextureCache::CacheEntry> TextureCache::_images;
QMutex imageCacheMutex(QMutex::Recursive);
Texture dummyTexture(true);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TextureCache *getTexelCache = nullptr;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Texture::Texture(bool isDummyTexture) {
	isDummy = isDummyTexture;

	if ( !isDummy ) {
		data = nullptr;
		w = 0;
		h = 0;
	}
	else {
		image = QImage(1,1, QImage::Format_RGB32);
		QRgb *bits = reinterpret_cast<QRgb*>(image.bits());
		*bits = qRgb(224,224,224);
		w = image.width();
		h = image.height();
		data = bits;
		id = TileIndex(0,0,0);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Texture::numBytes() const {
#if QT_VERSION >= 0x050a00
	return image.sizeInBytes();
#else
	return image.byteCount();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Texture::setImage(QImage &img) {
	image = img;
	w = image.width();
	h = image.height();
	data = (const QRgb*)image.bits();
	isDummy = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TextureCache::TextureCache(TileStore *store, bool mercatorProjected) {
	_tileStore = store;
	_isMercatorProjected = mercatorProjected;
	_storedBytes = 0;
	_textureCacheLimit = 128*1024*1024; // 128mb cache limit
	_lastTile[0] = _lastTile[1] = nullptr;
	_currentIndex = 0;
	_currentTick = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TextureCache::~TextureCache() {
	// remove storage textures
	for ( auto it = _storage.begin(); it != _storage.end(); ++it ) {
		remove(_tileStore->getID(it->first));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TextureCache::beginPaint() {
	/*
	if ( _lastTile[0] ) {
		std::cerr << " Cache[0]: " << _lastId[0] << "; " << _lastTile[0]->id << " with dims "
		          << _lastTile[0]->image.width() << "x"
		          << _lastTile[0]->image.height() << std::endl;
	}

	if ( _lastTile[1] ) {
		std::cerr << " Cache[1]: " << _lastId[1] << "; " << _lastTile[1]->id << " with dims "
		          << _lastTile[1]->image.width() << "x"
		          << _lastTile[1]->image.height() << std::endl;
	}
	*/

	_invalidMapping.clear();

	quint64 oldTick = _currentTick;
	++_currentTick;
	// Wrap. Reset lastUsed of all other tiles
	if ( _currentTick < oldTick ) {
		for ( auto it = _storage.begin(); it != _storage.end(); ++it )
			it->second->lastUsed = _currentTick;
		++_currentTick;
	}

	// std::cerr << _storage.size() << ": " << _storedBytes << " @ " << _currentTick << std::endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TextureCache::setCacheLimit(int limit) {
	_textureCacheLimit = limit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int TextureCache::maxLevel() const {
	if ( _tileStore )
		return std::min(_tileStore->maxLevel(), static_cast<int>(TileIndex::MaxLevel));
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int TextureCache::tileWidth() const {
	if ( _tileStore ) return _tileStore->tileSize().width();
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int TextureCache::tileHeight() const {
	if ( _tileStore ) return _tileStore->tileSize().height();
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TileStore::LoadResult TextureCache::load2(QImage &img, const TileIndex &tile) {
	QMutexLocker lock(&imageCacheMutex);

	ImageCache::iterator it;
	QString id = _tileStore->getID(tile);
	it = _images.find(id);
	if ( it == _images.end() ) {
		//std::cerr << "L " << tile << std::endl;
		auto r = _tileStore->load(img, tile);
		if ( r != TileStore::OK ) {
			return r;
		}

		if ( img.format() != QImage::Format_RGB32 &&
		     img.format() != QImage::Format_ARGB32 ) {
			img = img.convertToFormat(QImage::Format_ARGB32);
		}

		if ( !img.isNull() ) {
			// Add to global image cache
			_images[id] = CacheEntry(img, 1);
		}
		else {
			return TileStore::Error;
		}
	}
	else {
		img = it.value().first;
		++it.value().second;
		//std::cerr << "IC " << tile << ": " << id.toStdString() << std::endl;
	}

	/*
	for ( int i = 0; i < 5; ++i ) {
		for ( int j = 0; j < w; ++j ) {
			data[i*w + j] = qRgb(255,0,0);
			data[(h-1-i)*w + j] = qRgb(255,0,0);
		}
		for ( int j = 0; j < h; ++j ) {
			data[j*w + i] = qRgb(255,0,0);
			data[j*w + w-1-i] = qRgb(255,0,0);
		}
	}
	*/

	return TileStore::OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TextureCache::load(QImage &img, const TileIndex &tile) {
	return load2(img, tile) == TileStore::OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Texture *TextureCache::fetch(const TileIndex &tile, bool &deferred) {
	deferred = false;

	QImage image;
	if ( !tile ) {
		return nullptr;
	}

	auto r = load2(image, tile);
	if ( r != TileStore::OK ) {
		deferred = r == TileStore::Deferred;
		return nullptr;
	}

	if ( image.isNull() ) {
		return nullptr;
	}

	Texture *tex = new Texture;
	tex->id = tile;
	tex->setImage(image);

	return tex;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TextureCache::checkResources(Texture *tex) {
	while ( _storedBytes > _textureCacheLimit ) {
		quint64 min = _currentTick;
		Storage::iterator it, min_it = _storage.end();
		for ( it = _storage.begin(); it != _storage.end(); ++it ) {
			if ( (it->second->lastUsed < min || min_it == _storage.end()) && it->second != tex ) {
				min = it->second->lastUsed;
				min_it = it;
			}
		}

		// Remove texture completely
		if ( min_it != _storage.end() ) {
			TexturePtr min_tex = min_it->second;
			remove(_tileStore->getID(min_it->first));
			// std::cerr << "Uncaching texture " << min_it->second->id << std::endl;
			_storage.erase(min_it);
			_storedBytes -= min_tex->numBytes();

			for ( auto iit = _invalidMapping.begin(); iit != _invalidMapping.end(); ) {
				if ( iit->second == min_tex.get() ) {
					iit = _invalidMapping.erase(iit);
				}
				else {
					++iit;
				}
			}

			if ( _lastTile[0] == min_tex.get() )
				_lastTile[0] = nullptr;

			if ( _lastTile[1] == min_tex.get() )
				_lastTile[1] = nullptr;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TextureCache::setTexture(QImage &img, const TileIndex &tile) {
	if ( img.isNull() ) {
		// Special case when a texture could not be loaded from the source
		// e.g. asynchonously. No update requested.
		return false;
	}

	//std::cerr << "U " << tile << std::endl;
	// Erase level1 cache if tile is part of it
	if ( _lastId[0] == tile ) {
		_lastTile[0] = nullptr;
	}

	if ( _lastId[1] == tile ) {
		_lastTile[1] = nullptr;
	}

	if ( !img.isNull() &&
	     img.format() != QImage::Format_RGB32 &&
	     img.format() != QImage::Format_ARGB32 ) {
		img = img.convertToFormat(QImage::Format_ARGB32);
	}

	auto iit = _invalidMapping.find(tile);
	if ( iit != _invalidMapping.end() ) {
		_invalidMapping.erase(iit);
	}

	// Update texture cache
	auto it = _storage.find(tile);
	if ( it != _storage.end() ) {
		Texture *tex = it->second.get();

		// Update storage size
		_storedBytes -= tex->numBytes();
		tex->setImage(img);
		_storedBytes += tex->numBytes();

		// Update image cache
		{
			QMutexLocker lock(&imageCacheMutex);

			ImageCache::iterator it;
			QString id = _tileStore->getID(tile);
			it = _images.find(id);
			if ( it != _images.end() ) {
				it->first = img;
			}
		}

		// Check level1 cache for updates
		if ( _lastId[0] == tile ) {
			_lastTile[0] = tex;
		}

		if ( _lastId[1] == tile ) {
			_lastTile[1] = tex;
		}

		checkResources(tex);
	}
	else {
		Texture *tex = new Texture;
		tex->lastUsed = _currentTick;
		tex->id = tile;
		tex->setImage(img);
		cache(tex);
	}

	// Request update
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TextureCache::setTextureOrLoadParent(QImage &img, const TileIndex &tile) {
	if ( setTexture(img, tile) ) {
		return true;
	}

	auto parent = tile.parent();
	if ( parent ) {
		if ( _storage.find(parent) != _storage.end() ) {
			return false;
		}

		auto tex = get(parent);
		if ( tex ) {
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TextureCache::cache(Texture *tex) {
	{
		QMutexLocker lock(&imageCacheMutex);

		//std::cerr << "C " << tex->id << std::endl;
		_storage[tex->id] = tex;
		_storedBytes += tex->numBytes();
		// Add image to global cache
		_images[_tileStore->getID(tex->id)] = CacheEntry(tex->image, 1);
	}

	checkResources(tex);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TextureCache::invalidateTexture(const TileIndex &tile) {
	remove(_tileStore->getID(tile));

	{
		// Remove node from texture cache
		Storage::iterator it = _storage.find(tile);
		if ( it != _storage.end() ) {
			Texture *tex = it->second.get();

			if ( _lastTile[0] == tex )
				_lastTile[0] = nullptr;

			if ( _lastTile[1] == tex )
				_lastTile[1] = nullptr;

			// Update storage size
			_storedBytes -= tex->numBytes();

			//std::cerr << "I " << tex->id << std::endl;
			_storage.erase(it);

			for ( auto iit = _invalidMapping.begin(); iit != _invalidMapping.end(); ) {
				if ( iit->second == tex ) {
					iit = _invalidMapping.erase(iit);
				}
				else {
					++iit;
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TextureCache::clear() {
	QMutexLocker lock(&imageCacheMutex);

	_storage.clear();
	_images.clear();
	_invalidMapping.clear();
	_storedBytes = 0;
	_lastTile[0] = _lastTile[1] = nullptr;
	_currentIndex = 0;
	_currentTick = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TextureCache::remove(const QString &name) {
	QMutexLocker lock(&imageCacheMutex);
	ImageCache::iterator it;
	it = _images.find(name);
	if ( it != _images.end() ) {
		--it.value().second;
		if ( it.value().second == 0 )
			_images.erase(it);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Texture *TextureCache::get(const TileIndex &requestTile) {
	Texture *tex;

	TileIndex tile = requestTile;
	int maxTileLevel = std::max(maxLevel(), 0);

	while ( tile.level() > maxTileLevel )
		tile = tile.parent();

	/*
	while ( tile.level > 0
	     && !_tileStore->validate(tile.level, tile.column, tile.row) )
		tile = tile.parent();
	*/

	bool invalid = false;
	bool deferred = false;

	auto it = _storage.find(tile);
	if ( it != _storage.end() ) {
		tex = it->second.get();
	}
	else {
		auto iit = _invalidMapping.find(tile);
		if ( iit != _invalidMapping.end() ) {
			tex = iit->second;
		}
		else {
			// std::cerr << "F " << tile << std::endl;
			tex = fetch(tile, deferred);
			if ( tex ) {
				cache(tex);
			}
			else {
				tex = &dummyTexture;
				invalid = true;
			}
		}
	}

	// If it's a dummy texture then travel up the parent chain to check
	// for valid textures
	if ( tex->isDummy ) {
		TileIndex ptile = tile;
		while ( (ptile = ptile.parent()) ) {
			it = _storage.find(ptile);
			if ( it == _storage.end() ) {
				// Parent not in cache
				if ( !deferred ) {
					auto iit = _invalidMapping.find(ptile);
					if ( iit == _invalidMapping.end() ) {
						// Parent not in invalid list try to fetch it
						auto tmp = fetch(ptile, deferred);
						if ( tmp ) {
							cache(tmp);
							tex = tmp;
							break;
						}
					}
				}
			}
			else {
				Texture *tmp = it->second.get();
				if ( !tmp->isDummy ) {
					tex = tmp;
					break;
				}
			}
		}

		if ( invalid ) {
			_invalidMapping[tile] = tex;
		}
	}

	// std::cerr << "> " << tex->id << " with dims " << tex->image.width() << "x" << tex->image.height() << std::endl;
	tex->lastUsed = _currentTick;
	return tex;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
