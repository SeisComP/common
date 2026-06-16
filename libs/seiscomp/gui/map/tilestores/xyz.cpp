/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                            *
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


#define SEISCOMP_COMPONENT Gui::XYZTileStore

#include <seiscomp/gui/map/imagetree.h>

#include <seiscomp/client/application.h>
#include <seiscomp/logging/log.h>

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSet>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVector>

#include <algorithm>


namespace Seiscomp {
namespace Gui {
namespace Map {
namespace {


// Tile store of type "xyz": fetches map tiles from any standard XYZ /
// slippy-map server (OpenStreetMap, OpenTopoMap, ESRI, CartoDB, custom
// servers) over HTTP and caches them on disk.
//
// The implementation lives entirely in this private namespace; it is not part
// of the public API. Adding members here therefore never affects the library
// ABI. It registers itself with the TileStore factory under the name "xyz",
// selected via map.type = xyz.
//
// It is a QObject (without Q_OBJECT/moc) only so that it can own the network
// access manager and act as the connection context; replies are dispatched
// through a member function via the functor-based connect overload.
class XYZTileStore : public QObject, public TileStore {
	public:
		XYZTileStore() = default;
		~XYZTileStore() override {
			refresh();
		}

	public:
		bool open(MapsDesc &desc) override {
			QString defaultURL = desc.location.trimmed();

			auto *app = Client::Application::Instance();

			// Single-source band, also the default for sources entries that
			// omit a band.
			int defaultMin = 0;
			int defaultMax = 19;
			if ( app ) {
				try { defaultMin = app->configGetInt("map.xyz.minLevel"); } catch ( ... ) {}
				try { defaultMax = app->configGetInt("map.xyz.maxLevel"); } catch ( ... ) {}
			}

			// "map.xyz.sources": one entry per zoom band,
			// "minLevel:maxLevel:urlTemplate". Only the first two colons are
			// separators, so the "https://" in the URL stays intact.
			if ( app ) {
				try {
					for ( const auto &raw : app->configGetStrings("map.xyz.sources") ) {
						QString entry = QString::fromStdString(raw).trimmed();
						if ( entry.isEmpty() )
							continue;

						int c1 = entry.indexOf(':');
						int c2 = c1 >= 0 ? entry.indexOf(':', c1 + 1) : -1;
						if ( c1 < 0 || c2 < 0 ) {
							SEISCOMP_WARNING("xyz: ignoring malformed map.xyz.sources entry "
							                 "(want minLevel:maxLevel:url): %s",
							                 qUtf8Printable(entry));
							continue;
						}

						bool okMin = false, okMax = false;
						Source src;
						src.minLevel = entry.left(c1).trimmed().toInt(&okMin);
						src.maxLevel = entry.mid(c1 + 1, c2 - c1 - 1).trimmed().toInt(&okMax);
						src.url      = entry.mid(c2 + 1).trimmed();
						if ( !okMin || !okMax || src.url.isEmpty() || src.minLevel > src.maxLevel ) {
							SEISCOMP_WARNING("xyz: ignoring invalid map.xyz.sources entry: %s",
							                 qUtf8Printable(entry));
							continue;
						}
						_sources.push_back(src);
					}
				}
				catch ( ... ) {}
			}

			// No per-source bands configured: fall back to map.location.
			if ( _sources.isEmpty() ) {
				if ( defaultURL.isEmpty() ) {
					SEISCOMP_ERROR("xyz: no tile source - set map.location to an XYZ URL "
					               "template (e.g. https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png) "
					               "or provide map.xyz.sources entries");
					return false;
				}
				_sources.push_back(Source{defaultMin, defaultMax, defaultURL});
			}

			// Overall level bounds = union across sources.
			_minLevel = _sources.front().minLevel;
			_maxLevel = _sources.front().maxLevel;
			for ( const auto &s : _sources ) {
				_minLevel = std::min(_minLevel, s.minLevel);
				_maxLevel = std::max(_maxLevel, s.maxLevel);
			}
			if ( _maxLevel > static_cast<int>(TileIndex::MaxLevel) )
				_maxLevel = static_cast<int>(TileIndex::MaxLevel);

			if ( app ) {
				try { _cacheDuration = app->configGetInt("map.xyz.cacheDuration"); } catch ( ... ) {}
				try { _missingTTL    = app->configGetInt("map.xyz.missingTTL");    } catch ( ... ) {}
				try {
					_cacheDir = QString::fromStdString(app->configGetString("map.xyz.cacheDir"));
				}
				catch ( ... ) {}
				try {
					_userAgent = QString::fromStdString(app->configGetString("map.xyz.userAgent"));
				}
				catch ( ... ) {}
				try {
					QString s = QString::fromStdString(app->configGetString("map.xyz.subdomains"));
					for ( auto &sub : s.split(',', Qt::SkipEmptyParts) )
						_subdomains << sub.trimmed();
				}
				catch ( ... ) {}
				try {
					// Stored as a string so scconfig presents a 256/512 dropdown.
					bool ok = false;
					int sz = QString::fromStdString(app->configGetString("map.xyz.tileSize"))
					         .trimmed().toInt(&ok);
					if ( ok && sz > 0 )
						_tilesize = QSize(sz, sz);
					else
						SEISCOMP_WARNING("xyz: ignoring invalid map.xyz.tileSize");
				}
				catch ( ... ) {}
			}

			if ( _tilesize.isEmpty() )
				_tilesize = QSize(256, 256);

			bool needSubdomains = false;
			for ( const auto &s : _sources )
				needSubdomains = needSubdomains || s.url.contains("{s}");
			if ( _subdomains.isEmpty() && needSubdomains )
				_subdomains << "a" << "b" << "c";

			_projection = Mercator;

			_nam = new QNetworkAccessManager(this);
			connect(_nam, &QNetworkAccessManager::finished,
			        this, &XYZTileStore::onRequestFinished);

			if ( !_cacheDir.isEmpty() )
				QDir().mkpath(_cacheDir);

			SEISCOMP_INFO("xyz: tile store opened - %d source(s)  levels=%d..%d  cache=%s  ttl=%ds",
			              static_cast<int>(_sources.size()), _minLevel, _maxLevel,
			              _cacheDir.isEmpty() ? "disabled" : qUtf8Printable(_cacheDir),
			              _cacheDuration);
			for ( const auto &s : _sources )
				SEISCOMP_INFO("xyz:   level %2d..%-2d -> %s",
				              s.minLevel, s.maxLevel, qUtf8Printable(s.url));
			return true;
		}

		int maxLevel() const override {
			return _maxLevel;
		}

		LoadResult load(QImage &img, const TileIndex &tile) override {
			if ( _inflight.contains(tile.id) )
				return Deferred;

			// Negative cache: don't re-hammer the server for tiles it already
			// told us it doesn't have (HTTP >= 400). Respect map.xyz.missingTTL.
			auto miss = _missing.constFind(tile.id);
			if ( miss != _missing.constEnd() ) {
				if ( _missingTTL < 0 ||
				     miss.value() + _missingTTL > QDateTime::currentSecsSinceEpoch() )
					return Error;
				_missing.erase(_missing.find(tile.id));
			}

			if ( !_cacheDir.isEmpty() ) {
				QString path = cachePath(tile);
				if ( QFile::exists(path) && isCacheFresh(path) ) {
					if ( img.load(path) )
						return OK;
					QFile::remove(path);
				}
			}

			startRequest(tile);
			return Deferred;
		}

		QString getID(const TileIndex &tile) const override {
			return QString("%1/%2/%3")
			    .arg(tile.level()).arg(tile.column()).arg(tile.row());
		}

		bool validate(int level, int column, int row) const override {
			if ( level < _minLevel || level > _maxLevel )
				return false;
			const int n = 1 << level;
			return column >= 0 && column < n && row >= 0 && row < n;
		}

		bool hasPendingRequests() const override {
			return !_inflight.isEmpty();
		}

		void refresh() override {
			for ( auto *reply : _replyMap.keys() )
				reply->abort();
			_replyMap.clear();
			_inflight.clear();
		}

	private:
		using TileId = TileIndex::Storage;

		//! A tile source serving a contiguous zoom-level band. Each source
		//! carries its own maxLevel because providers differ: OSM standard
		//! caps at 19, Google ~21-22, ESRI ArcGIS up to 23. The hard ceiling
		//! is TileIndex::MaxLevel.
		struct Source {
			int     minLevel{0};
			int     maxLevel{19};
			QString url;
		};

		const Source *sourceForLevel(int level) const {
			for ( const auto &s : _sources ) {
				if ( level >= s.minLevel && level <= s.maxLevel )
					return &s;
			}
			// Gap between bands: fall back to the source with the nearest edge
			// so a tile still renders (scaled) rather than showing a hole.
			const Source *best = nullptr;
			int bestDist = 0;
			for ( const auto &s : _sources ) {
				int d = std::min(std::abs(level - s.minLevel), std::abs(level - s.maxLevel));
				if ( !best || d < bestDist ) {
					best = &s;
					bestDist = d;
				}
			}
			return best;
		}

		QString buildURL(const TileIndex &tile) {
			const Source *src = sourceForLevel(tile.level());
			if ( !src )
				return QString();

			QString url = src->url;
			url.replace("{z}", QString::number(tile.level()));
			url.replace("{x}", QString::number(tile.column()));
			url.replace("{y}", QString::number(tile.row()));

			if ( !_subdomains.isEmpty() ) {
				url.replace("{s}", _subdomains[_subdomainIndex % _subdomains.size()]);
				++_subdomainIndex;
			}

			return url;
		}

		QString cachePath(const TileIndex &tile) const {
			return QString("%1/%2/%3/%4")
			    .arg(_cacheDir)
			    .arg(tile.level())
			    .arg(tile.column())
			    .arg(tile.row());
		}

		bool isCacheFresh(const QString &path) const {
			if ( _cacheDuration < 0 ) return true;
			if ( _cacheDuration == 0 ) return false;
			return QFileInfo(path).lastModified().secsTo(QDateTime::currentDateTime())
			       < _cacheDuration;
		}

		void startRequest(const TileIndex &tile) {
			_inflight.insert(tile.id);

			QNetworkRequest req{QUrl{buildURL(tile)}};
			req.setHeader(QNetworkRequest::UserAgentHeader, _userAgent);
			req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
			req.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
			req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
			                 QNetworkRequest::NoLessSafeRedirectPolicy);

			QNetworkReply *reply = _nam->get(req);
			_replyMap.insert(reply, tile.id);
		}

		void onRequestFinished(QNetworkReply *reply) {
			reply->deleteLater();

			auto it = _replyMap.find(reply);
			if ( it == _replyMap.end() )
				return;

			TileId tileId = it.value();
			_replyMap.erase(it);
			_inflight.remove(tileId);

			TileIndex tile;
			tile.id = tileId;

			if ( reply->error() != QNetworkReply::NoError ) {
				if ( reply->error() != QNetworkReply::OperationCanceledError ) {
					SEISCOMP_WARNING("xyz: fetch error for %s: %s",
					                 qUtf8Printable(getID(tile)),
					                 qUtf8Printable(reply->errorString()));
				}
				loadingCancelled(tile);
				return;
			}

			int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
			if ( httpStatus >= 400 ) {
				SEISCOMP_WARNING("xyz: HTTP %d for tile %s",
				                 httpStatus, qUtf8Printable(getID(tile)));
				if ( _missingTTL != 0 )
					_missing.insert(tileId, QDateTime::currentSecsSinceEpoch());
				loadingCancelled(tile);
				return;
			}

			QByteArray data = reply->readAll();
			if ( data.isEmpty() ) {
				loadingCancelled(tile);
				return;
			}

			QImage img;
			if ( !img.loadFromData(data) ) {
				SEISCOMP_WARNING("xyz: image decode failed for tile %s",
				                 qUtf8Printable(getID(tile)));
				loadingCancelled(tile);
				return;
			}

			// One-time sanity check: the configured map.xyz.tileSize must match
			// the pixels the server actually serves, otherwise the projection
			// scale (which uses tileSize) picks the wrong level and the map
			// looks blurry.
			if ( !_tileSizeChecked ) {
				_tileSizeChecked = true;
				if ( img.size() != _tilesize )
					SEISCOMP_WARNING("xyz: server tile is %dx%d but map.xyz.tileSize is %dx%d - "
					                 "set map.xyz.tileSize to %d to avoid blurry rendering",
					                 img.width(), img.height(),
					                 _tilesize.width(), _tilesize.height(), img.width());
			}

			// Only persist when caching is actually enabled for reads
			// (cacheDuration == 0 disables the cache entirely).
			if ( !_cacheDir.isEmpty() && _cacheDuration != 0 ) {
				QString path = cachePath(tile);
				QDir().mkpath(QFileInfo(path).absolutePath());
				QFile f(path);
				if ( f.open(QIODevice::WriteOnly) )
					f.write(data);
			}

			loadingComplete(img, tile);
		}

	private:
		QNetworkAccessManager   *_nam{nullptr};
		QVector<Source>          _sources;
		QStringList              _subdomains;
		int                      _subdomainIndex{0};
		int                      _minLevel{0};
		int                      _maxLevel{19};
		QString                  _cacheDir;
		int                      _cacheDuration{86400};
		QString                  _userAgent{"SeisComP-xyztiles/1.0"};

		int                      _missingTTL{300};
		QHash<TileId, qint64>    _missing;

		bool                     _tileSizeChecked{false};

		QHash<QNetworkReply *, TileId> _replyMap;
		QSet<TileId>                   _inflight;
};


REGISTER_TILESTORE_INTERFACE(XYZTileStore, "xyz");


}
}
}
}
