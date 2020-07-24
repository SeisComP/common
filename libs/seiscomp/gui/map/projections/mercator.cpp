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


#include <seiscomp/client/application.h>
#include <seiscomp/gui/map/projections/mercator.h>
#include <seiscomp/gui/map/texturecache.ipp>

#include <math.h>
#include <iostream>

#define deg2rad(d) (M_PI*(d)/180.0)
#define rad2deg(d) (180.0*(d)/M_PI)
#define HALF_PI (M_PI/2)


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_PROJECTION_INTERFACE(MercatorProjection, "Mercator");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


const qreal ooLat = 1.0 / 90.0;
const qreal ooLon = 1.0 / 180.0;
const qreal ooPi = 1.0 / M_PI;
const qreal oo2Pi = 1.0 / HALF_PI;


class MercatorProjectionProxy : public MercatorProjection {
	template <typename COMPOSITOR>
	friend void drawLowQualityImage(MercatorProjectionProxy *proj, QImage &buffer, const QRectF &geoReference, const QImage &image);

	template <typename COMPOSITOR>
	friend void drawHighQualityImage(MercatorProjectionProxy *proj, QImage &buffer, const QRectF &geoReference, const QImage &image);
};


// Low quality
template <typename COMPOSITOR>
void drawLowQualityImage(MercatorProjectionProxy *proj, QImage &buffer, const QRectF &geoReference, const QImage &image) {
	QPoint p00, p11;

	qreal minLat, maxLat;
	qreal minLon, maxLon;
	qreal rangeLat;

	minLat = geoReference.top();
	maxLat = geoReference.bottom();

	rangeLat = maxLat - minLat;

	minLon = geoReference.left();
	maxLon = geoReference.right();

	if ( minLat > maxLat ) std::swap(minLat, maxLat);

	proj->project(p00, QPointF(minLon, minLat));
	proj->project(p11, QPointF(maxLon, maxLat));

	bool wrap = fabs(maxLon - minLon) >= 360.;

	int x0 = p00.x();
	int x1 = p11.x();

	int y0 = p00.y();
	int y1 = p11.y();

	// X can be wrapped, so we have to check more cases
	if ( geoReference.width() < 180.0f ) {
		if ( x0 >= proj->_width ) {
			if ( x1 < 0 || x1 >= proj->_width ) return;
		}

		if ( x1 < 0 ) {
			if ( x0 < 0 || x0 >= proj->_width ) return;
		}
	}

	if ( y0 > y1 ) std::swap(y0,y1);

	// Y has no wrapping, so the checks are more simply
	if ( y0 >= proj->_height ) return;
	if ( y1 < 0 ) return;

	bool drawTwoParts = false;

	// Quick hack just for testing
	// TODO: This special case has to be handled.
	if ( x0 >= x1 || wrap ) {
		drawTwoParts = true;
		if ( x0 >= x1 ) x0 -= proj->_mapWidth/2;
		else if ( wrap ) x0 = x1 - proj->_mapWidth/2;
	}

	int scaledWidth = x1-x0+1;

	Coord ratioX;

	ratioX.parts.hi = image.width();
	ratioX.parts.lo = 0;
	ratioX.value /= scaledWidth;

	while ( true ) {
		int width = image.width();
		int height = image.height();

		Coord xofs;

		int x0c = x0,
		    y0c = y0,
		    x1c = x1;

		QRgb *targetData = (QRgb*)buffer.bits();
		int targetWidth = buffer.width();

		if ( x0c < 0 ) {
			xofs.value = ratioX.value * -x0c;
			x0c = 0;
		}
		else
			xofs.value = 0;

		if ( y0c < 0 )
			y0c = 0;

		if ( x1c >= proj->_width )
			x1c = proj->_width-1;

		if ( y1 >= proj->_height )
			y1 = proj->_height-1;

		targetData += targetWidth * y0c + x0c;

		const QRgb *image_data = (const QRgb*)image.bits();
		const QRgb *data;

		for ( int i = y0c; i <= y1; ++i ) {
			QRgb *targetPixel = targetData;

			Coord x;
			x.value = xofs.value;

			qreal lat0 = proj->_halfHeight - i;
			lat0 = lat0 / (qreal)proj->_scale + proj->_centerY;
			lat0 = atan(sinh(lat0*M_PI))*oo2Pi*90.0;

			qreal y_ = 1.0 - (lat0 - minLat) / rangeLat;
			if ( y_ < 0 ) y_ = 0;
			else if ( y_ > 1 ) y_ = 1;

			int y_ofs = (int)(y_*height);
			if ( y_ofs >= height ) y_ofs = height-1;
			data = image_data + width*y_ofs;

			for ( int j = x0c; j <= x1c; ++j ) {
				QRgb c = data[x.parts.hi];
				COMPOSITOR::combine(*targetPixel, c);
				++targetPixel;

				x.value += ratioX.value;
			}

			targetData += targetWidth;
		}

		if ( drawTwoParts ) {
			x0 += proj->_mapWidth/2;
			x1 += proj->_mapWidth/2;
			drawTwoParts = false;
		}
		else
			break;
	}
}


template <typename COMPOSITOR>
void drawHighQualityImage(MercatorProjectionProxy *proj, QImage &buffer, const QRectF &geoReference, const QImage &image) {
	QPoint p00, p11;

	qreal minLat, maxLat;
	qreal minLon, maxLon;
	qreal rangeLat;

	minLat = geoReference.top();
	maxLat = geoReference.bottom();

	rangeLat = maxLat - minLat;

	minLon = geoReference.left();
	maxLon = geoReference.right();

	if ( minLat > maxLat ) std::swap(minLat, maxLat);

	proj->project(p00, QPointF(minLon, minLat));
	proj->project(p11, QPointF(maxLon, maxLat));

	bool wrap = fabs(maxLon - minLon) >= 360.;

	int x0 = p00.x();
	int x1 = p11.x();

	int y0 = p00.y();
	int y1 = p11.y();

	// X can be wrapped, so we have to check more cases
	if ( geoReference.width() < 180.0f ) {
		if ( x0 >= proj->_width ) {
			if ( x1 < 0 || x1 >= proj->_width ) return;
		}

		if ( x1 < 0 ) {
			if ( x0 < 0 || x0 >= proj->_width ) return;
		}
	}

	if ( y0 > y1 ) std::swap(y0,y1);

	// Y has no wrapping, so the checks are more simply
	if ( y0 >= proj->_height ) return;
	if ( y1 < 0 ) return;

	bool drawTwoParts = false;

	// Quick hack just for testing
	// TODO: This special case has to be handled.
	if ( x0 >= x1 || wrap ) {
		drawTwoParts = true;
		if ( x0 >= x1 ) x0 -= proj->_mapWidth/2;
		else if ( wrap ) x0 = x1 - proj->_mapWidth/2;
	}

	int scaledWidth = x1-x0+1;

	Coord ratioX;

	ratioX.parts.hi = image.width();
	ratioX.parts.lo = 0;
	ratioX.value /= scaledWidth;

	int width = image.width();
	int height = image.height();

	Coord maxYOffs;
	maxYOffs.parts.lo = 0;
	maxYOffs.parts.hi = height-1;

	while ( true ) {
		Coord xofs;

		int x0c = x0,
		    y0c = y0,
		    x1c = x1;

		QRgb *targetData = (QRgb*)buffer.bits();
		int targetWidth = buffer.width();

		if ( x0c < 0 ) {
			xofs.value = ratioX.value * -x0c;
			x0c = 0;
		}
		else
			xofs.value = 0;

		if ( y0c < 0 )
			y0c = 0;

		if ( x1c >= proj->_width )
			x1c = proj->_width-1;

		if ( y1 >= proj->_height )
			y1 = proj->_height-1;

		targetData += targetWidth * y0c + x0c;

		const QRgb *image_data = (const QRgb*)image.bits();

		for ( int i = y0c; i <= y1; ++i ) {
			QRgb *targetPixel = targetData;

			Coord x, y;
			x.value = xofs.value;

			qreal lat0 = proj->_halfHeight - i;
			lat0 = lat0 / (qreal)proj->_scale + proj->_centerY;
			lat0 = atan(sinh(lat0*M_PI))*oo2Pi*90.0;
			qreal y_ = 1.0 - ((lat0 - minLat) / rangeLat);
			if ( y_ < 0 ) y_ = 0;
			else if ( y_ > 1 ) y_ = 1;

			y.parts.hi = height;
			y.parts.lo = 0;
			y.value *= y_;
			if ( y.value > Coord::fraction_half_max )
				y.value -= Coord::fraction_half_max;
			else
				y.value = 0;

			if ( y.value >= maxYOffs.value ) y = maxYOffs;

			for ( int j = x0c; j <= x1c; ++j ) {
				QRgb c;
				if ( x.value > Coord::fraction_half_max )
					getTexelBilinear(c, image_data, width, height, x.value - Coord::fraction_half_max, y);
				else
					getTexelBilinear(c, image_data, width, height, 0, y);
				COMPOSITOR::combine(*targetPixel, c);
				++targetPixel;

				x.value += ratioX.value;
			}

			targetData += targetWidth;
		}

		if ( drawTwoParts ) {
			x0 += proj->_mapWidth/2;
			x1 += proj->_mapWidth/2;
			drawTwoParts = false;
		}
		else
			break;
	}
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MercatorProjection::MercatorProjection()
: RectangularProjection() {
	_pixelPerDegreeFact = 180.0f;
	if ( SCCoreApp ) {
		try { _discreteSteps = SCCoreApp->configGetBool("map.mercator.discrete"); }
		catch ( ... ) { _discreteSteps = false; }
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MercatorProjection::isRectangular() const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename PROC>
void MercatorProjection::render(QImage &img, TextureCache *cache) {
	//_screenRadius = std::min(_halfWidth/2, _halfHeight);
	//_screenRadius = cache != NULL?cache->tileWidth()*0.5:256;
	_screenRadius = std::max(_halfWidth, _halfHeight);
	_windowRect = QRect(0,0,img.width(),img.height());
	_clipRect = _windowRect.adjusted(-10, -10, 10, 10);

	QSize size(img.size());

	int radius = _screenRadius * _radius;
	double dt;
	qreal visibleRadius;

	if ( !_enableLowZoom ) {
		if ( radius < _halfWidth )
			radius = _halfWidth;

		if ( radius < _halfHeight )
			radius = _halfHeight;

		visibleRadius = (qreal)radius / _screenRadius;
	}
	else
		visibleRadius = _radius;

	// Adjust visible radius to fixed zoom levels
	if ( _discreteSteps ) {
		qreal scale = _screenRadius * visibleRadius;
		qreal pixelRatio = scale / (cache ? cache->tileHeight() : 256);
		if ( !cache || cache->isMercatorProjected() )
			pixelRatio *= 2;
		if ( pixelRatio < 1 ) pixelRatio = 1;
		int level = (int)(log(pixelRatio) / log(2.0) + 0.7);
		if ( cache ) {
			if ( level > cache->maxLevel() )
				level = cache->maxLevel();
		}
		else {
			if ( level > 18 ) level = 18;
		}

		scale = (1 << int(level-0.7)) * (cache != NULL?cache->tileHeight():256);
		if ( scale < 1 ) scale = 1;
		while ( scale < _halfWidth ) scale *= 2;
		while ( scale < _halfHeight ) scale *= 2;
		visibleRadius = scale / _screenRadius;
		radius = _screenRadius * visibleRadius;
	}

	dt = 1.0f / qreal(radius-1);
	setVisibleRadius(visibleRadius);

	QPoint center = QPoint(_halfWidth, _halfHeight);

	int fromY, toY;
	fromY = 0;
	toY = size.height();

	qreal iyf = center.y() * dt;
	if ( iyf > 1.0f ) {
		if ( _enableLowZoom ) {
			fromY = (iyf - 1.0f) * radius;
			toY = img.height() - fromY;
		}
		iyf = 1.0f;
	}

	QRgb *data = (QRgb *)img.bits();

	int centerX = center.x();

	// Map geographic coordinate to screen coordinate
	_centerY = asinh(tan(_center.y()*HALF_PI))*ooPi;

	qreal upY = _centerY + iyf;
	qreal downY = upY - (toY - fromY) * dt;

	if ( downY < -1.0 ) {
		downY = -1.0;
		upY = downY + (toY - fromY) * dt;
		_centerY = (upY + downY) * 0.5;
		// Map screen coordinate to geographic coordinate
		_visibleCenter.setY(atan(sinh(_centerY*M_PI))*oo2Pi);
	}

	if ( upY > 1.0 ) {
		upY = 1.0;
		downY = upY - (toY - fromY) * dt;
		_centerY = (upY + downY) * 0.5;
		// Map screen coordinate to geographic coordinate
		_visibleCenter.setY(atan(sinh(_centerY*M_PI))*oo2Pi);
	}

	data += fromY * img.width();

	qreal y = upY;

	//qreal ixf = (qreal)centerX / radius;
	qreal ixf = 2.0f;
	qint64 pxf = qint64(ixf*radius);
	qint64 fx, tx;

	fx = centerX - pxf;
	tx = centerX + pxf;

	if ( fx < 0 ) {
		ixf += fx * dt;
		fx = 0;
	}

	// Clip to left border
	if ( fx < 2 )
		fx = 0;

	// Clip to right border
	if ( tx >= size.width()-2 )
		tx = size.width()-1;

	int fromX = (int)fx;
	int toX = (int)tx;

	if ( cache == NULL ) return;

	qreal pixelRatio = _scale / cache->tileHeight();
	if ( cache->isMercatorProjected() )
		pixelRatio *= 2;
	if ( pixelRatio < 1 ) pixelRatio = 1;
	int level = (int)(log(pixelRatio) / log(2.0) + 0.7);
	if ( level > cache->maxLevel() )
		level = cache->maxLevel();

	qreal leftX = _center.x() - ixf;
	qreal rightX = _center.x() + ixf;

	int pixels = toX - fromX + 1;

	Coord leftTu;
	Coord rightTu;

	leftTu.value = (leftX+1.0f) * Coord::value_type(Coord::fraction_half_max);
	rightTu.value = (rightX+1.0f) * Coord::value_type(Coord::fraction_half_max);

	if ( cache->isMercatorProjected() ) {
		for ( int i = fromY; i < toY; ++i, y -= dt ) {
			if ( y <= -1.0 ) y = -1.0 + dt;

			Coord tv;
			tv.value = (1.0f-y) * Coord::value_type(Coord::fraction_half_max);

			PROC::fetch(cache, data[fromX], leftTu, tv, level);
			PROC::fetch(cache, data[toX], rightTu, tv, level);

			// Shift only by 30 bits to keep the sign bit in the lower 32 bit
			Coord::value_type xDelta = rightTu.value - leftTu.value;
			qint64 stepU = (qint64(xDelta) << 30) / pixels;
			qint64 stepper;
			Coord lon;
			stepper = qint64(leftTu.value) << 30;
			stepper += stepU;

			for ( int k = 1; k < pixels; ++k ) {
				lon.value = stepper >> 30;
				PROC::fetch(cache, data[fromX + k], lon, tv, level);
				stepper += stepU;
			}

			data += size.width();
		}
	}
	else {
		for ( int i = fromY; i < toY; ++i, y -= dt ) {
			if ( y <= -1.0 ) y = -1.0 + dt;

			Coord tv;
			qreal lat = atan(sinh(y*M_PI))*oo2Pi;
			tv.value = (1.0f-lat) * Coord::value_type(Coord::fraction_half_max);

			PROC::fetch(cache, data[fromX], leftTu, tv, level);
			PROC::fetch(cache, data[toX], rightTu, tv, level);

			// Shift only by 30 bits to keep the sign bit in the lower 32 bit
			Coord::value_type xDelta = rightTu.value - leftTu.value;
			qint64 stepU = (qint64(xDelta) << 30) / pixels;
			qint64 stepper;
			Coord lon;
			stepper = qint64(leftTu.value) << 30;
			stepper += stepU;

			for ( int k = 1; k < pixels; ++k ) {
				lon.value = stepper >> 30;
				PROC::fetch(cache, data[fromX + k], lon, tv, level);
				stepper += stepU;
			}

			data += size.width();
		}
	}

/* Lighting test

	qreal stepX;
	if ( pixels > 0 )
		stepX = (rightX - leftX) / pixels;
	else
		stepX = 0;

	{
		float lat0 = HALF_PI*y;
		float lon0 = HALF_PI*xl;

		float v[3] = { -cos(lat0)*cos(lon0),
						sin(lat0),
						cos(lat0)*sin(lon0)
					 };

		float light = (v[1] * 0.5 + v[2] * 0.866025403784)*0.5;
		if ( light < 0 ) light = 0;
		light += 0.5;

		QRgb c;
		PROC::fetch(cache, c, lon, tv, level);

		data[fromX + k] = qRgb(qRed(c)*light, qGreen(c)*light, qBlue(c)*light);
	}

*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MercatorProjection::render(QImage& img, bool highQuality, TextureCache *cache) {
	if ( highQuality )
		render<BilinearFilter>(img, cache);
	else
		render<NearestFilter>(img, cache);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void MercatorProjection::projectUnwrapped(QPoint &screenCoords, const QPointF &geoCoords) const {
	qreal x = geoCoords.x() * ooLon;

	qreal lat = geoCoords.y();
	if ( lat > 90.0 ) {
		lat = 180.0 - lat;
		x += 1.0;
		if ( x > 1.0 ) x -= 2.0;
	}
	else if ( lat < -90.0 ) {
		lat = -180.0 - lat;
		x += 1.0;
		if ( x > 1.0 ) x -= 2.0;
	}

	qreal sy = sin(deg2rad(lat));
	if ( sy > 1.0 ) sy = 1.0;
	else if ( sy < -1.0 ) sy = -1.0;

	qreal y = fabs(sy) < 1.0?atanh(sy) * ooPi:sy;

	x = (x - _visibleCenter.x()) * _scale;
	y = (y - _centerY) * _scale;

	screenCoords.setX(_halfWidth + x);
	screenCoords.setY(_halfHeight - y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MercatorProjection::project(QPoint &screenCoords, const QPointF &geoCoords) const {
	qreal x = geoCoords.x() * ooLon;

	qreal lat = geoCoords.y();
	if ( lat > 90.0 ) {
		lat = 180.0 - lat;
		x += 1.0;
		if ( x > 1.0 ) x -= 2.0;
	}
	else if ( lat < -90.0 ) {
		lat = -180.0 - lat;
		x += 1.0;
		if ( x > 1.0 ) x -= 2.0;
	}

	qreal sy = sin(deg2rad(lat));
	if ( sy > 1.0 ) sy = 1.0;
	else if ( sy < -1.0 ) sy = -1.0;

	qreal y = fabs(sy) < 1.0?atanh(sy) * ooPi:sy;

	x = (x - _visibleCenter.x()) * _scale;
	y = (y - _centerY) * _scale;

	if ( x > _scale )
		x -= _halfMapWidth;

	if ( x < -_scale )
		x += _halfMapWidth;

	screenCoords.setX(_halfWidth + x);
	screenCoords.setY(_halfHeight - y);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MercatorProjection::unproject(QPointF &geoCoords, const QPoint &screenCoords) const {
	qreal x = screenCoords.x() - _halfWidth;
	qreal y = _halfHeight - screenCoords.y();

	if ( x < -_scale || x > _scale ) return false;
	if ( y < -_scale || y > _scale ) return false;

	x = x / (qreal)_scale + _visibleCenter.x();
	y = y / (qreal)_scale + _centerY;

	y = atan(sinh(y*M_PI))*oo2Pi;

	x *= 180.0f;
	y *= 90.0f;

	if ( x < -180.0f ) x += 360.0f;
	if ( x >  180.0f ) x -= 360.0f;

	geoCoords.setX(x);
	geoCoords.setY(y);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MercatorProjection::centerOn(const QPointF &geoCoords) {
	qreal x = geoCoords.x() * ooLon;
	qreal y = geoCoords.y() * ooLat;

	if ( x < -1.0f ) x += 2.0f;
	if ( x >  1.0f ) x -= 2.0f;

	if ( y < -1.0f ) y = -1.0f;
	if ( y >  1.0f ) y =  1.0f;

	_center = QPointF(x,y);
	_visibleCenter = _center;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF MercatorProjection::gridDistance() const {
	qreal dist = std::min(double((_width / 4) / pixelPerDegree()), 180.0);
	if ( dist < 0.01 )
		dist = std::max(int((dist*1000 + 0.6f)), 1)*0.001;
	else if ( dist < 0.1 )
		dist = std::max(int((dist*100 + 0.6f)), 1)*0.01;
	else if ( dist < 1 )
		dist = std::max(int((dist*10 + 0.6f)), 1)*0.1;
	else if ( dist < 5 )
		dist = std::max(int((dist + 0.6f)), 1);
	else
		dist = std::max(int((dist + 0.6f*5) / 5) * 5, 5);

	return QPointF(dist, dist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MercatorProjection::drawImage(QImage &buffer, const QRectF &geoReference,
                                   const QImage &image, bool highQuality,
                                   CompositionMode cm) {
	if ( image.format() != QImage::Format_RGB32 &&
	     image.format() != QImage::Format_ARGB32 )
		return;

	bool useAlpha = image.format() == QImage::Format_ARGB32;

	// This step is required to give draw*QualityImage access to protected methods.
	// Declaring friend in the header would expose the functions as public symbols
	// which is not desired.
	MercatorProjectionProxy *self = (MercatorProjectionProxy*)this;

	if ( cm == CompositionMode_Default )
		cm = useAlpha ? CompositionMode_SourceOver : CompositionMode_Source;

	switch ( cm ) {
		case CompositionMode_Source:
			if ( highQuality )
				drawHighQualityImage<CompositionSource>(self, buffer, geoReference, image);
			else
				drawLowQualityImage<CompositionSource>(self, buffer, geoReference, image);
			break;
		case CompositionMode_SourceOver:
			if ( highQuality )
				drawHighQualityImage<CompositionSourceOver>(self, buffer, geoReference, image);
			else
				drawLowQualityImage<CompositionSourceOver>(self, buffer, geoReference, image);
			break;
		case CompositionMode_Multiply:
			if ( highQuality )
				drawHighQualityImage<CompositionMultiply>(self, buffer, geoReference, image);
			else
				drawLowQualityImage<CompositionMultiply>(self, buffer, geoReference, image);
			break;
		case CompositionMode_Xor:
			if ( highQuality )
				drawHighQualityImage<CompositionXor>(self, buffer, geoReference, image);
			else
				drawLowQualityImage<CompositionXor>(self, buffer, geoReference, image);
			break;
		case CompositionMode_Plus:
			if ( highQuality )
				drawHighQualityImage<CompositionPlus>(self, buffer, geoReference, image);
			else
				drawLowQualityImage<CompositionPlus>(self, buffer, geoReference, image);
			break;
		default:
			std::cerr << "ERROR: Invalid composition mode: " << cm << std::endl;
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


struct AbstractBorderClipper {
	AbstractBorderClipper() : initialized(false) {}
	bool   initialized;
	QPoint firstP;
	bool   firstClipped;
	bool   clipped;
	QPoint lastP;
};


template <class Impl, class NextStage>
struct BorderClipper : AbstractBorderClipper {
	BorderClipper(NextStage &ns) : nextStage(ns) {}

	inline void feed(const QPoint &p, const QRect &clipRect) {
		QPoint rp;

		//std::cerr << this << "  + " << p.x() << " / " << p.y() << std::endl;

		if ( impl.clipped(p, clipRect) ) {
			if ( !initialized ) {
				initialized = true;
				firstClipped = true;
				firstP = p;
				lastP = p;
				clipped = true;
				return;
			}
			else {
				// Both clipped, ignore them
				if ( clipped ) {
					lastP = p;
					return;
				}

				impl.interpolate(rp, lastP, p, clipRect);

				clipped = true;
				lastP = p;

				nextStage.feed(rp, clipRect);
				return;
			}
		}
		else {
			if ( !initialized ) {
				initialized = true;
				firstClipped = false;
				firstP = p;
				lastP = p;
				clipped = false;
			}
			else {
				if ( clipped ) {
					impl.interpolate(rp, lastP, p, clipRect);
					nextStage.feed(rp, clipRect);
				}

				clipped = false;
				lastP = p;
			}
		}

		nextStage.feed(p, clipRect);
	}

	inline void finalize(const QRect &clipRect) {
		if ( !initialized ) return;
		if ( clipped != firstClipped ) {
			QPoint rp;
			impl.interpolate(rp, lastP, firstP, clipRect);
			nextStage.feed(rp, clipRect);
		}
		nextStage.finalize(clipRect);
	}

	Impl       impl;
	NextStage &nextStage;
};


struct North {
	inline bool clipped(const QPoint &p, const QRect &clipRect) {
		return p.y() < clipRect.top();
	}

	inline void interpolate(QPoint &p, const QPoint &p0,
	                        const QPoint &p1, const QRect &clipRect) {
		p.setX(p0.x()+(qint64(p0.x()-p1.x())*(clipRect.top()-p0.y()))/(p0.y()-p1.y()));
		p.setY(clipRect.top());
	}
};

struct East {
	East() : maxValue(0) {}

	inline bool clipped(const QPoint &p, const QRect &clipRect) {
		if ( p.x() > clipRect.right() ) {
			if ( p.x() > maxValue ) maxValue = p.x();
			return true;
		}
		else
			return false;
	}

	inline void interpolate(QPoint &p, const QPoint &p0,
	                        const QPoint &p1, const QRect &clipRect) {
		p.setY(p0.y()+(qint64(p0.y()-p1.y())*(clipRect.right()-p0.x()))/(p0.x()-p1.x()));
		p.setX(clipRect.right());
	}

	int maxValue;
};

struct South {
	inline bool clipped(const QPoint &p, const QRect &clipRect) {
		return p.y() > clipRect.bottom();
	}

	inline void interpolate(QPoint &p, const QPoint &p0,
	                        const QPoint &p1, const QRect &clipRect) {
		p.setX(p0.x()+(qint64(p0.x()-p1.x())*(clipRect.bottom()-p0.y()))/(p0.y()-p1.y()));
		p.setY(clipRect.bottom());
	}
};

struct West {
	West() : minValue(0) {}

	inline bool clipped(const QPoint &p, const QRect &clipRect) {
		if ( p.x() < clipRect.left() ) {
			if ( p.x() < minValue ) minValue = p.x();
			return true;
		}
		else
			return false;
	}

	inline void interpolate(QPoint &p, const QPoint &p0,
	                        const QPoint &p1, const QRect &clipRect) {
		p.setY(p0.y()+(qint64(p0.y()-p1.y())*(clipRect.left()-p0.x()))/(p0.x()-p1.x()));
		p.setX(clipRect.left());
	}

	int minValue;
};


struct Renderer {
	Renderer(QPainterPath &pp_) : pp(pp_), firstPoint(true) {}

	inline void feed(const QPoint &p, const QRect &) {
		//std::cerr << p.x() << "  " << p.y() << std::endl;
		if ( firstPoint ) {
			pp.moveTo(p);
			firstP = p;
			firstPoint = false;
		}
		else {
			lastP = p;
			pp.lineTo(p);
		}
	}

	inline void finalize(const QRect &) {}

	QPoint firstP;
	QPoint lastP;
	QPainterPath &pp;
	bool firstPoint;
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MercatorProjection::project(QPainterPath &screenPath, size_t n,
                                 const Geo::GeoCoordinate *poly, bool closed,
                                 uint minPixelDist, ClipHint clipHint) const {
	if ( n == 0 || !poly ) return false;

	float minDist = ((float)minPixelDist)/pixelPerDegree();
	int duplicationDirection = 0;

	QPointF v(poly[0].lon, poly[0].lat);
	QPoint p;

	bool spansNorthPole = false;
	bool spansSouthPole = false;

	if ( closed ) {
		spansNorthPole = Geo::contains(Geo::GeoCoordinate(+90, 0), poly, n);
		spansSouthPole = Geo::contains(Geo::GeoCoordinate(-90, 0), poly, n);
	}

	bool spansAnyPole = spansNorthPole || spansSouthPole;

	if ( clipHint == DoClip ) {
		bool reuseScreenPath = false;
		project(p, v);

		if ( spansAnyPole && closed ) {
			// Spanning a pole does not need special attention to duplication.
			/* Compose clippers */
			Renderer renderer(screenPath);

			typedef BorderClipper<West, Renderer> WestClipper;
			WestClipper clipWest(renderer);

			typedef BorderClipper<South, WestClipper> SouthClipper;
			SouthClipper clipSouth(clipWest);

			typedef BorderClipper<East, SouthClipper> EastClipper;
			EastClipper clipEast(clipSouth);

			BorderClipper<North, EastClipper> clipNorth(clipEast);

			clipNorth.feed(p, _clipRect);
			int px = p.x(), py = p.y();
			int spanY = spansNorthPole ? _clipRect.top() : _clipRect.bottom();

			std::cerr << "DRAW" << std::endl;
			for ( size_t i = 1; i < n; ++i ) {
				Math::Geo::CoordF::ValueType lonDiff = poly[i].lon - v.x();
				if ( lonDiff > 180 ) lonDiff -= 360;
				else if ( lonDiff < -180 ) lonDiff += 360;

				if ( std::abs(lonDiff) <= minDist
				  && std::abs(poly[i].lat - v.y()) <= minDist )
					continue;

				v.setX(poly[i].lon); v.setY(poly[i].lat);
				project(p, v);

				if ( (((p.x() - px) < 0) != (lonDiff < 0)) && (p.x() != px) ) {
					int wrappedX, unwrappedX;
					if ( p.x() < px ) {
						wrappedX = p.x() + _halfMapWidth;
						unwrappedX = px - _halfMapWidth;
					}
					else {
						wrappedX = p.x() - _halfMapWidth;
						unwrappedX = px + _halfMapWidth;
					}
					clipNorth.feed(QPoint(wrappedX, p.y()), _clipRect);
					clipNorth.feed(QPoint(wrappedX, spanY), _clipRect);
					clipNorth.feed(QPoint(unwrappedX, spanY), _clipRect);
					clipNorth.feed(QPoint(unwrappedX, py), _clipRect);
				}
				clipNorth.feed(p, _clipRect);
				px = p.x(); py = p.y();
			}

			clipNorth.finalize(_clipRect);

			if ( screenPath.isEmpty() )
				return false;

			screenPath.closeSubpath();
		}
		else {
			{
				/* Compose clippers */
				Renderer renderer(screenPath);

				typedef BorderClipper<West, Renderer> WestClipper;
				WestClipper clipWest(renderer);

				typedef BorderClipper<South, WestClipper> SouthClipper;
				SouthClipper clipSouth(clipWest);

				typedef BorderClipper<East, SouthClipper> EastClipper;
				EastClipper clipEast(clipSouth);

				BorderClipper<North, EastClipper> clipNorth(clipEast);

				clipNorth.feed(p, _clipRect);
				qreal xOfs = 0;
				int px = p.x();

				for ( size_t i = 1; i < n; ++i ) {
					Math::Geo::CoordF::ValueType lonDiff = poly[i].lon - v.x();
					if ( lonDiff > 180 ) lonDiff -= 360;
					else if ( lonDiff < -180 ) lonDiff += 360;

					if ( std::abs(lonDiff) <= minDist
					  && std::abs(poly[i].lat - v.y()) <= minDist )
						continue;

					v.setX(v.x()+lonDiff); v.setY(poly[i].lat);

					qreal x = lonDiff * ooLon;

					qreal lat = poly[i].lat;
					if ( lat > 90.0 ) {
						lat = 180.0 - lat;
						x += 1.0;
						if ( x > 1.0 ) x -= 2.0;
					}
					else if ( lat < -90.0 ) {
						lat = -180.0 - lat;
						x += 1.0;
						if ( x > 1.0 ) x -= 2.0;
					}

					qreal sy = sin(deg2rad(lat));
					if ( sy > 1.0 ) sy = 1.0;
					else if ( sy < -1.0 ) sy = -1.0;

					qreal y = fabs(sy) < 1.0?atanh(sy) * ooPi:sy;
					xOfs += x * _scale;

					p.setX(px + xOfs);
					p.setY(_halfHeight - (y - _centerY) * _scale);

					clipNorth.feed(p, _clipRect);
				}

				if ( closed ) {
					clipNorth.finalize(_clipRect);
				}

				if ( clipEast.impl.maxValue > 0 && clipEast.impl.maxValue >= _halfMapWidth )
					duplicationDirection = 1;
				else if ( clipWest.impl.minValue < 0 && clipWest.impl.minValue <= (_width-_halfMapWidth) )
					duplicationDirection = -1;

				if ( !screenPath.isEmpty() ) {
					if ( closed ) screenPath.closeSubpath();
				}
				else if ( duplicationDirection )
					reuseScreenPath = true;
				else
					return false;
			}

			if ( duplicationDirection ) {
				QPainterPath wrappedPath;
				Renderer renderer(reuseScreenPath ? screenPath : wrappedPath);

				int offsetX = -duplicationDirection*_halfMapWidth;

				typedef BorderClipper<West, Renderer> WestClipper;
				WestClipper clipWest(renderer);

				typedef BorderClipper<South, WestClipper> SouthClipper;
				SouthClipper clipSouth(clipWest);

				typedef BorderClipper<East, SouthClipper> EastClipper;
				EastClipper clipEast(clipSouth);

				BorderClipper<North, EastClipper> clipNorth(clipEast);

				v = QPointF(poly[0].lon, poly[0].lat);

				project(p, v);
				p.rx() += offsetX;
				clipNorth.feed(p, _clipRect);

				qreal xOfs = 0;
				int px = p.x();

				for ( size_t i = 1; i < n; ++i ) {
					Math::Geo::CoordF::ValueType lonDiff = poly[i].lon - v.x();
					if ( lonDiff > 180 ) lonDiff -= 360;
					else if ( lonDiff < -180 ) lonDiff += 360;

					if ( std::abs(lonDiff) <= minDist
					  && std::abs(poly[i].lat - v.y()) <= minDist )
						continue;

					v.setX(v.x()+lonDiff); v.setY(poly[i].lat);

					qreal x = lonDiff * ooLon;

					qreal lat = poly[i].lat;
					if ( lat > 90.0 ) {
						lat = 180.0 - lat;
						x += 1.0;
						if ( x > 1.0 ) x -= 2.0;
					}
					else if ( lat < -90.0 ) {
						lat = -180.0 - lat;
						x += 1.0;
						if ( x > 1.0 ) x -= 2.0;
					}

					qreal sy = sin(deg2rad(lat));
					if ( sy > 1.0 ) sy = 1.0;
					else if ( sy < -1.0 ) sy = -1.0;

					qreal y = fabs(sy) < 1.0?atanh(sy) * ooPi:sy;
					xOfs += x * _scale;

					p.setX(px + xOfs);
					p.setY(_halfHeight - (y - _centerY) * _scale);

					clipNorth.feed(p, _clipRect);
				}

				if ( closed ) {
					clipNorth.finalize(_clipRect);
				}

				if ( !renderer.pp.isEmpty() ) {
					if ( closed ) renderer.pp.closeSubpath();
					if ( !reuseScreenPath )
						screenPath.addPath(renderer.pp);
				}
			}
		}

		return true;
	}
	else {
		projectUnwrapped(p, v);

		if ( p.x() >= _width )
			duplicationDirection = 1;
		else if ( p.x() < 0 )
			duplicationDirection = -1;

		screenPath.moveTo(p);

		if ( minDist == 0 ) {
			for ( size_t i = 1; i < n; ++i ) {
				Math::Geo::CoordF::ValueType lonDiff = poly[i].lon - v.x();
				if ( lonDiff > 180 ) lonDiff -= 360;
				else if ( lonDiff < -180 ) lonDiff += 360;

				v.setX(v.x()+lonDiff); v.setY(poly[i].lat);
				projectUnwrapped(p, v);
				if ( p.x() >= _width )
					duplicationDirection = 1;
				else if ( p.x() < 0 )
					duplicationDirection = -1;

				screenPath.lineTo(p);
			}
		}
		else {
			for ( size_t i = 1; i < n; ++i ) {
				Math::Geo::CoordF::ValueType lonDiff = poly[i].lon - v.x();
				if ( lonDiff > 180 ) lonDiff -= 360;
				else if ( lonDiff < -180 ) lonDiff += 360;

				if ( std::abs(lonDiff) > minDist ||
				     std::abs(poly[i].lat - v.y()) > minDist ) {
					v.setX(v.x()+lonDiff); v.setY(poly[i].lat);
					projectUnwrapped(p, v);
					if ( p.x() >= _width )
						duplicationDirection = 1;
					else if ( p.x() < 0 )
						duplicationDirection = -1;

					screenPath.lineTo(p);
				}
			}
		}

		if ( screenPath.isEmpty() )
			return false;

		if ( duplicationDirection )
			screenPath.addPath(screenPath.translated(-_halfMapWidth*duplicationDirection, 0));

		if ( closed ) screenPath.closeSubpath();
		return true;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
