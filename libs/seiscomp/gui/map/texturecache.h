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


#ifndef SEISCOMP_GUI_MAP_TEXTURECACHE_H
#define SEISCOMP_GUI_MAP_TEXTURECACHE_H


#include <QHash>
#include <QMap>
#include <QImage>
#include <QPair>

#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/datetime.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {

namespace Alg {

class MapTreeNode;

}

namespace Map {


class TextureCache;
class TileStore;


union Coord {
	enum limits {
		fraction_half_max = Q_INT64_C(2147483648),
		fraction_max = Q_INT64_C(4294967296),
		fraction_shift = 32
	};

	typedef qint64 value_type;
	typedef quint32 part_type;

	Coord() {}
	Coord(value_type v) : value(v) {}

	struct {
		part_type lo;
		part_type hi;
	} parts;
	value_type value;
};


struct SC_GUI_API TextureID {
	int level;
	int row;
	int column;

	TextureID() {}
	TextureID(int l, int r, int c) : level(l), row(r), column(c) {}

	bool operator==(const TextureID &other) const {
		return level == other.level &&
		       row == other.row &&
		       column == other.column;
	}

	bool operator!=(const TextureID &other) const {
		return level != other.level ||
		       row != other.row ||
		       column != other.column;
	}
};



DEFINE_SMARTPOINTER(Texture);

struct SC_GUI_API Texture : public Core::BaseObject {
	Texture();

	int numBytes() const;
	bool load(TextureCache *cache, Alg::MapTreeNode *node);
	void setImage(QImage &img);

	QImage      image;
	const QRgb *data;
	TextureID   id;
	quint32     w;
	quint32     h;
	qint64      lastUsed;
	bool        isDummy;
};


DEFINE_SMARTPOINTER(TextureCache);

class SC_GUI_API TextureCache : public Core::BaseObject {
	public:
		TextureCache(TileStore *mapTree, bool mercatorProjected);
		~TextureCache();

		void beginPaint();

		void setCacheLimit(int limit);
		void setCurrentTime(const Core::Time &t);

		int maxLevel() const;
		int tileWidth() const;
		int tileHeight() const;

		bool isMercatorProjected() const { return _isMercatorProjected; }

		void getTexel(QRgb &c, Coord u, Coord v, int level);
		void getTexelBilinear(QRgb &c, Coord u, Coord v, int level);

		Texture *get(const TextureID &id);

		const quint64 &startTick() const { return _currentTick; }

		bool load(QImage &img, Alg::MapTreeNode *node);

		void setTexture(QImage &img, Alg::MapTreeNode *node);

		//! Invalidates a texture and causes load() to be called next time
		//! it needs to be accessed.
		//! This function was introduced in API 1.1
		void invalidateTexture(Alg::MapTreeNode *node);

		//! Clears the cache
		void clear();


	private:
		Alg::MapTreeNode *
		getNode(Alg::MapTreeNode *node, const TextureID &id) const;

		void checkResources(Texture *tex = nullptr);

		static void remove(const QString &name);


	private:
		typedef QHash<TextureID, Texture*> Lookup;
		typedef QMap<Alg::MapTreeNode*, TexturePtr> Storage;

		TileStore        *_mapTree;
		bool              _isMercatorProjected;
		Lookup            _firstLevel;
		Storage           _storage;
		int               _storedBytes;
		int               _textureCacheLimit;
		quint64           _currentTick;

		Texture          *_lastTile[2];
		TextureID         _lastId[2];
		int               _currentIndex;

		typedef QPair<QImage, int> CacheEntry;
		typedef QMap<QString, CacheEntry> ImageCache;

		static  ImageCache _images;
};


void getTexel(QRgb &c, const QRgb *data, int w, int h, Coord u, Coord v);
void getTexelBilinear(QRgb &c, const QRgb *data, int w, int h, Coord u, Coord v);


}
}
}


#endif
