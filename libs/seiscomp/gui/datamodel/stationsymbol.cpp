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


#define SEISCOMP_COMPONENT "StationSymbol"
#include <seiscomp/logging/log.h>

#include "stationsymbol.h"

#include <QPoint>
#include <QPolygon>
#include <QPainter>

#include <seiscomp/gui/map/canvas.h>
#include <seiscomp/gui/map/projection.h>
#include <seiscomp/gui/core/application.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationSymbol::StationSymbol(Map::Decorator* decorator)
: Symbol(decorator) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationSymbol::StationSymbol(double latitude,
                             double longitude,
                             Map::Decorator* decorator)
: Symbol(latitude, longitude, decorator) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationSymbol::isInside(int x, int y) const {
	return _stationPolygon.boundingRect().contains(x, y);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationSymbol::setColor(const QColor& color) {
	_color = color;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor& StationSymbol::color() const {
	return _color;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationSymbol::setOutlineColor(const QColor &color) {
	_outlineColor = color;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor &StationSymbol::outlineColor() const {
	return _outlineColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationSymbol::setFrameColor(const QColor& color) {
	_frameColor = color;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QColor& StationSymbol::frameColor() const {
	return _frameColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationSymbol::setFrameSize(int frameSize) {
	_frameSize = frameSize;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationSymbol::frameSize() const {
	return _frameSize;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationSymbol::setRadius(int radius) {
	_radius = radius;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationSymbol::radius() const {
	return _radius;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationSymbol::customDraw(const Map::Canvas *, QPainter& painter) {
	painter.save();

	QPen pen(Qt::MiterJoin);
	QBrush brush(Qt::SolidPattern);

	if ( _frameSize > 0 ) {
		pen.setColor(_frameColor);
		painter.setPen(pen);

		brush.setColor(_frameColor);
		painter.setBrush(brush);

		_stationPolygon = generateShape(_position.x(), _position.y(), _radius + _frameSize);
		painter.drawPolygon(_stationPolygon);
	}

	pen.setColor(_outlineColor);
	painter.setPen(pen);

	brush.setColor(_color);
	painter.setBrush(brush);

	_stationPolygon = generateShape(_position.x(), _position.y(), _radius);

	painter.drawPolygon(_stationPolygon);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationSymbol::init() {
	_frameColor   = Qt::black;
	_outlineColor = Qt::black;
	_color        = Qt::black;
	_frameSize    = 1;
	setRadius(SCScheme.map.stationSize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPolygon StationSymbol::generateShape(int posX, int posY, int radius) {
	QPolygon polygon;

	switch ( SCScheme.map.stationSymbol ) {
		default:
		case Gui::Scheme::Map::StationSymbol::Triangle:
			polygon
				<< QPoint(posX, posY - radius * 3 / 2)
				<< QPoint(posX + int(0.867 * radius), posY)
				<< QPoint(posX - int(0.867 * radius), posY);
			break;
		case Gui::Scheme::Map::StationSymbol::Diamond:
		{
			int radius1 = radius / 2;
			int radius2 = radius / 4;
			polygon
				<< QPoint(posX, posY - radius - radius1 - radius2)
				<< QPoint(posX + int(0.867 * radius), posY - radius - radius1)
				<< QPoint(posX, posY)
				<< QPoint(posX - int(0.867 * radius), posY - radius - radius1);
			break;
		}
		case Gui::Scheme::Map::StationSymbol::Box:
		{
			int radius1 = radius;
			int radius2 = radius / 3;
			int halfWidth = radius1;
			int height = radius * 2 * 3 / 4;
			polygon
				<< QPoint(posX, posY)
				<< QPoint(posX - radius2, posY - radius2)
				<< QPoint(posX - halfWidth, posY - radius2)
				<< QPoint(posX - halfWidth, posY - radius2 - height)
				<< QPoint(posX + halfWidth, posY - radius2 - height)
				<< QPoint(posX + halfWidth, posY - radius2)
				<< QPoint(posX + radius2, posY - radius2);
			break;
		}
	}

	return polygon;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QPolygon &StationSymbol::stationPolygon() const {
	return _stationPolygon;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Gui
} // namespace Seiscomp
