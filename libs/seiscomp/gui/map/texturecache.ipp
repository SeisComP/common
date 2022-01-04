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

namespace Seiscomp {
namespace Gui {
namespace Map {


inline void TextureCache::getTexel(QRgb &c, Coord u, Coord v, int level) {
	u.parts.hi = 0;
	v.parts.hi = 0;

	TileIndex id(level, (v.value << level) >> Coord::fraction_shift, (u.value << level) >> Coord::fraction_shift);
	Texture *tex;

	if ( !_lastTile[0] || _lastId[0] != id ) {
		if ( !_lastTile[1] || _lastId[1] != id ) {
			tex = get(id);
			_lastTile[_currentIndex] = tex;
			_lastId[_currentIndex] = id;
			_currentIndex = 1-_currentIndex;
		}
		else {
			tex = _lastTile[1];
		}
	}
	else
		tex = _lastTile[0];

	u.value <<= tex->id.level();
	v.value <<= tex->id.level();

	u.value = Coord::value_type(u.parts.lo) * tex->w;
	v.value = Coord::value_type(v.parts.lo) * tex->h;

	c = tex->data[v.parts.hi*tex->w +u.parts.hi];
}


inline void TextureCache::getTexelBilinear(QRgb &c, Coord u, Coord v, int level) {
	u.parts.hi = 0;
	v.parts.hi = 0;

	TileIndex id(level, (v.value << level) >> Coord::fraction_shift, (u.value << level) >> Coord::fraction_shift);
	Texture *tex;

	if ( !_lastTile[0] || _lastId[0] != id ) {
		if ( !_lastTile[1] || _lastId[1] != id ) {
			tex = get(id);
			_lastTile[_currentIndex] = tex;
			_lastId[_currentIndex] = id;
			_currentIndex = 1-_currentIndex;
		}
		else {
			tex = _lastTile[1];
		}
	}
	else
		tex = _lastTile[0];

	u.value <<= tex->id.level();
	v.value <<= tex->id.level();

	u.value = Coord::value_type(u.parts.lo) * tex->w;
	v.value = Coord::value_type(v.parts.lo) * tex->h;

	Coord::part_type itnx = u.parts.hi + 1;
	Coord::part_type itny = v.parts.hi + 1;

	if ( itnx < tex->w ) {
		if ( itny < tex->h ) {
			int tidx = v.parts.hi*tex->w + u.parts.hi;
			QRgb t = tex->data[tidx];
			QRgb tnx = tex->data[tidx + 1];
			QRgb tny = tex->data[tidx + tex->w];
			QRgb tnxy = tex->data[tidx + tex->w + 1];

			Coord::value_type ftx = u.parts.lo;
			Coord::value_type fty = v.parts.lo;
			Coord::value_type iftx = Coord::fraction_max - ftx;
			Coord::value_type ifty = Coord::fraction_max - fty;

			QRgb y0 = qRgb((qRed(t) * iftx + qRed(tnx) * ftx) >> Coord::fraction_shift,
			               (qGreen(t) * iftx + qGreen(tnx) * ftx) >> Coord::fraction_shift,
			               (qBlue(t) * iftx + qBlue(tnx) * ftx) >> Coord::fraction_shift);

			QRgb y1 = qRgb((qRed(tny) * iftx + qRed(tnxy) * ftx) >> Coord::fraction_shift,
			               (qGreen(tny) * iftx + qGreen(tnxy) * ftx) >> Coord::fraction_shift,
			               (qBlue(tny) * iftx + qBlue(tnxy) * ftx) >> Coord::fraction_shift);

			c = qRgb((qRed(y0) * ifty + qRed(y1) * fty) >> Coord::fraction_shift,
			         (qGreen(y0) * ifty + qGreen(y1) * fty) >> Coord::fraction_shift,
			         (qBlue(y0) * ifty + qBlue(y1) * fty) >> Coord::fraction_shift);
		}
		// Interpolate only x
		else {
			int tidx = v.parts.hi*tex->w + u.parts.hi;
			QRgb t = tex->data[tidx];
			QRgb tnx = tex->data[tidx + 1];

			Coord::value_type ftx = u.parts.lo;
			Coord::value_type iftx = Coord::fraction_max - ftx;

			c = qRgb((qRed(t) * iftx + qRed(tnx) * ftx) >> Coord::fraction_shift,
			         (qGreen(t) * iftx + qGreen(tnx) * ftx) >> Coord::fraction_shift,
			         (qBlue(t) * iftx + qBlue(tnx) * ftx) >> Coord::fraction_shift);
		}
	}
	else {
		// Interpolate only y
		if ( itny < tex->h ) {
			int tidx = v.parts.hi*tex->w + u.parts.hi;
			QRgb t = tex->data[tidx];
			QRgb tny = tex->data[tidx + tex->w];

			Coord::value_type fty = v.parts.lo;
			Coord::value_type ifty = Coord::fraction_max - fty;

			c = qRgb((qRed(t) * ifty + qRed(tny) * fty) >> Coord::fraction_shift,
			         (qGreen(t) * ifty + qGreen(tny) * fty) >> Coord::fraction_shift,
			         (qBlue(t) * ifty + qBlue(tny) * fty) >> Coord::fraction_shift);
		}
		else
			// Do not interpolate at all
			c = tex->data[v.parts.hi*tex->w +u.parts.hi];
	}
}



inline void getTexel(QRgb &c, const QRgb *data, int w, int, Coord u, Coord v) {
	c = data[v.parts.hi*w + u.parts.hi];
}


inline void getTexelBilinear(QRgb &c, const QRgb *data, int w, int h, Coord u, Coord v) {
	Coord::part_type itnx = u.parts.hi + 1;
	Coord::part_type itny = v.parts.hi + 1;

	if ( (int)itnx < w ) {
		if ( (int)itny < h ) {
			int tidx = v.parts.hi*w + u.parts.hi;
			QRgb t = data[tidx];
			QRgb tnx = data[tidx + 1];
			QRgb tny = data[tidx + w];
			QRgb tnxy = data[tidx + w + 1];

			Coord::value_type ftx = u.parts.lo;
			Coord::value_type fty = v.parts.lo;
			Coord::value_type iftx = Coord::fraction_max - ftx;
			Coord::value_type ifty = Coord::fraction_max - fty;

			QRgb y0 = qRgba((qRed(t) * iftx + qRed(tnx) * ftx) >> Coord::fraction_shift,
			                (qGreen(t) * iftx + qGreen(tnx) * ftx) >> Coord::fraction_shift,
			                (qBlue(t) * iftx + qBlue(tnx) * ftx) >> Coord::fraction_shift,
			                (qAlpha(t) * iftx + qAlpha(tnx) * ftx) >> Coord::fraction_shift);

			QRgb y1 = qRgba((qRed(tny) * iftx + qRed(tnxy) * ftx) >> Coord::fraction_shift,
			                (qGreen(tny) * iftx + qGreen(tnxy) * ftx) >> Coord::fraction_shift,
			                (qBlue(tny) * iftx + qBlue(tnxy) * ftx) >> Coord::fraction_shift,
			                (qAlpha(tny) * iftx + qAlpha(tnxy) * ftx) >> Coord::fraction_shift);

			c = qRgba((qRed(y0) * ifty + qRed(y1) * fty) >> Coord::fraction_shift,
			          (qGreen(y0) * ifty + qGreen(y1) * fty) >> Coord::fraction_shift,
			          (qBlue(y0) * ifty + qBlue(y1) * fty) >> Coord::fraction_shift,
			          (qAlpha(y0) * ifty + qAlpha(y1) * fty) >> Coord::fraction_shift);
		}
		// Interpolate only x
		else {
			int tidx = v.parts.hi*w + u.parts.hi;
			QRgb t = data[tidx];
			QRgb tnx = data[tidx + 1];

			Coord::value_type ftx = u.parts.lo;
			Coord::value_type iftx = Coord::fraction_max - ftx;

			c = qRgba((qRed(t) * iftx + qRed(tnx) * ftx) >> Coord::fraction_shift,
			          (qGreen(t) * iftx + qGreen(tnx) * ftx) >> Coord::fraction_shift,
			          (qBlue(t) * iftx + qBlue(tnx) * ftx) >> Coord::fraction_shift,
			          (qAlpha(t) * iftx + qAlpha(tnx) * ftx) >> Coord::fraction_shift);
		}
	}
	else {
		// Interpolate only y
		if ( (int)itny < h ) {
			int tidx = v.parts.hi*w + u.parts.hi;
			QRgb t = data[tidx];
			QRgb tny = data[tidx + w];

			Coord::value_type fty = v.parts.lo;
			Coord::value_type ifty = Coord::fraction_max - fty;

			c = qRgba((qRed(t) * ifty + qRed(tny) * fty) >> Coord::fraction_shift,
			          (qGreen(t) * ifty + qGreen(tny) * fty) >> Coord::fraction_shift,
			          (qBlue(t) * ifty + qBlue(tny) * fty) >> Coord::fraction_shift,
			          (qAlpha(t) * ifty + qAlpha(tny) * fty) >> Coord::fraction_shift);
		}
		else
			// Do not interpolate at all
			c = data[v.parts.hi*w + u.parts.hi];
	}
}


}
}
}
