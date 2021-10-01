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


#include <seiscomp/gui/map/layers/gridlayer.h>

#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/map/canvas.h>
#include <seiscomp/gui/map/projection.h>
#include <seiscomp/geo/coordinate.h>


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GridLayer::GridLayer(QObject* parent) : Layer(parent) {
	setName("grid");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GridLayer::~GridLayer() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridLayer::draw(const Seiscomp::Gui::Map::Canvas *canvas,
                     QPainter &painter) {
	if ( !isVisible() ) return;
	if ( !canvas ) return;

	Seiscomp::Gui::Map::Projection *projection = canvas->projection();
	if ( !projection ) return;

	painter.save();

	painter.setRenderHint(QPainter::Antialiasing, isAntiAliasingEnabled());
	painter.setPen(SCScheme.colors.map.grid);

#if 0
	// Y gridlines
	projection->drawLatCircle(p,  0.0);
	projection->drawLatCircle(p, 90.0);
	projection->drawLatCircle(p, 180.0);
	projection->drawLatCircle(p, 270.0);

	// X gridlines
	projection->drawLonCircle(p,  0.0);
	projection->drawLonCircle(p,  66.55);
	projection->drawLonCircle(p,  23.4333);
	projection->drawLonCircle(p, -23.4333);
	projection->drawLonCircle(p, -66.55);
#else

	Geo::GeoCoordinate c(projection->visibleCenter().y(),
	                     projection->visibleCenter().x());
	c.normalize();
	qreal modX = fmod(c.lon, _gridDistance.x());
	qreal modY = fmod(c.lat, _gridDistance.y());

	Geo::GeoCoordinate start0(c.lat - modY, c.lon - modX);
	Geo::GeoCoordinate start1;

	if ( c.lon < 0 ) {
		start1.lon = start0.lon;
		start0.lon = start0.lon - _gridDistance.x();
	}
	else
		start1.lon = start0.lon + _gridDistance.x();

	if ( c.lat < 0 ) {
		start1.lat = start0.lat;
		start0.lat = start0.lat - _gridDistance.y();
	}
	else
		start1.lat = start0.lat + _gridDistance.y();

	start0.normalize();
	start1.normalize();

	qreal x = start1.lon;
	qreal toX = c.lon + 180;

	while ( x < toX && projection->drawLatCircle(painter, x) )
		x += _gridDistance.x();

	x = start0.lon;
	toX = c.lon - 180;
	while ( x > toX && projection->drawLatCircle(painter, x) )
		x -= _gridDistance.x();

	qreal y = start1.lat;
	qreal toY = 90;

	while ( y < toY && projection->drawLonCircle(painter, y) )
		y += _gridDistance.y();

	y = start0.lat;
	toY = -90;

	while ( y > toY && projection->drawLonCircle(painter, y) )
		y -= _gridDistance.y();
#endif

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GridLayer::setGridDistance(const QPointF &p) {
	_gridDistance = p;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPointF &GridLayer::gridDistance() const {
	return _gridDistance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Map
} // namespce Gui
} // namespace Seiscomp
