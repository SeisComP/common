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


#include <seiscomp/gui/map/layers/citieslayer.h>

#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/map/canvas.h>
#include <seiscomp/gui/map/projection.h>
#include <seiscomp/gui/map/standardlegend.h>


namespace Seiscomp {
namespace Gui {
namespace Map {

#define CITY_NORMAL_SYMBOL_SIZE 4
#define CITY_BIG_SYMBOL_SIZE    6
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CitiesLayer::CitiesLayer(QObject* parent) : Layer(parent), _selectedCity(nullptr) {
	setName("cities");
	_topPopulatedPlaces = -1;

	StandardLegend *legend = new StandardLegend(this);
	legend->setTitle(tr("Cities"));
	legend->setArea(Qt::Alignment(Qt::AlignTop | Qt::AlignRight));
	legend->addItem(new StandardLegendItem(SCScheme.colors.map.cityOutlines,
	                                       SCScheme.colors.map.cityNormal, tr("1Mio- inhabitants"),
	                                       CITY_NORMAL_SYMBOL_SIZE));
	legend->addItem(new StandardLegendItem(SCScheme.colors.map.cityOutlines,
	                                       SCScheme.colors.map.cityNormal, tr("1Mio+ inhabitants"),
	                                       CITY_BIG_SYMBOL_SIZE));
	legend->addItem(new StandardLegendItem(SCScheme.colors.map.cityOutlines,
	                                       SCScheme.colors.map.cityCapital, tr("Capital"),
	                                       CITY_BIG_SYMBOL_SIZE));

	_penHalo = QPen(SCScheme.colors.map.cityHalo, 1 + 2 * SCScheme.map.cityHaloWidth);

	addLegend(legend);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CitiesLayer::~CitiesLayer() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CitiesLayer::init(const Config::Config &cfg) {
	try {
		_topPopulatedPlaces = cfg.getInt("map.layers." + name().toStdString() + ".topPopulatedPlaces");
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CitiesLayer::draw(const Seiscomp::Gui::Map::Canvas* canvas,
                       QPainter& painter) {
	if ( !isVisible() ) return;
	if ( canvas == nullptr ) return;

	Seiscomp::Gui::Map::Projection* projection = canvas->projection();
	if ( projection == nullptr ) return;

	painter.save();

	painter.setRenderHint(QPainter::Antialiasing, isAntiAliasingEnabled());

	QFont font(SCScheme.fonts.cityLabels);
	font.setBold(true);
	painter.setFont(font);

	QFontMetrics fontMetrics = painter.fontMetrics();

	int height = canvas->height(),
	    width = canvas->width(),
	    fontHeight = fontMetrics.height() + SCScheme.map.cityHaloWidth * 2,
	    gridHeight = height / fontHeight;

	Grid grid(gridHeight);

	double radius = -1;

	if ( SCScheme.map.cityPopulationWeight > 0 )
		radius = Math::Geo::deg2km(width / projection->pixelPerDegree()) *
		                           SCScheme.map.cityPopulationWeight;

	size_t maxRenderedCitites = SCCoreApp->cities().size();
	if ( _topPopulatedPlaces > 0 )
		maxRenderedCitites = _topPopulatedPlaces;

	bool lastUnderline = false;
	bool lastBold = true;

	if ( _selectedCity )
		drawCity(painter, grid, font, lastUnderline, lastBold, projection,
		         *_selectedCity, fontMetrics, width, fontHeight);

	size_t citiesRendered = 0;

	foreach ( const Math::Geo::CityD& city, SCCoreApp->cities() ) {
		if ( citiesRendered >= maxRenderedCitites ) break;
		if ( city.population() < radius ) break;
		if ( &city == _selectedCity ) continue;

		if ( drawCity(painter, grid, font, lastUnderline, lastBold, projection,
		         city, fontMetrics, width, fontHeight) )
			++citiesRendered;
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CitiesLayer::drawCity(QPainter& painter, Grid &grid, QFont &font,
                           bool &lastUnderline, bool &lastBold,
                           const Projection* projection,
                           const Math::Geo::CityD& city,
                           const QFontMetrics& fontMetrics,
                           int width, int rowHeight) {
	QPoint p;
	if ( !projection->project(p, QPointF(city.lon, city.lat)) ) {
		return false;
	}

	int gridY, gridPrevY, gridNextY;

	gridY = p.y() / rowHeight;

	if ( gridY < 0 || gridY >= grid.count() ) {
		return false;
	}

	if ( p.x() < 0 || p.x() >= width ) {
		return false;
	}

	bool capital = (city.category() == "B" || city.category() == "C");
	bool bold = city.population() >= 1000000;

	int symbolSize = 4;
	if ( bold )
		symbolSize = 6;

	QRect labelRect(fontMetrics.boundingRect(city.name().c_str()));
	labelRect.adjust(-SCScheme.map.cityHaloWidth, -SCScheme.map.cityHaloWidth,
	                  SCScheme.map.cityHaloWidth,  SCScheme.map.cityHaloWidth);
	labelRect.moveTo(QPoint(p.x() + symbolSize / 2, p.y()));

	QList<QRect> &gridRow = grid[gridY];

	bool foundPlace = true;
	for ( auto &item : gridRow ) {
		if ( item.intersects(labelRect) ) {
			foundPlace = false;
			break;
		}
	}

	if ( !foundPlace ) {
		labelRect.moveTo(labelRect.left() - labelRect.width() - symbolSize,
		                 labelRect.top());
		foundPlace = true;
		for ( auto &item : gridRow ) {
			if ( item.intersects(labelRect) ) {
				foundPlace = false;
				break;
			}
		}
	}

	if ( !foundPlace ) {
		return false;
	}

	gridPrevY = gridY - 1;
	gridNextY = gridY + 1;

	gridRow = grid[gridY];
	gridRow.append(labelRect);
	if ( gridPrevY >= 0 ) grid[gridPrevY].append(labelRect);
	if ( gridNextY < grid.count() ) grid[gridNextY].append(labelRect);

	if ( capital ) {
		painter.setPen(SCScheme.colors.map.cityOutlines);
		painter.setBrush(SCScheme.colors.map.cityCapital);
	}
	else {
		painter.setPen(SCScheme.colors.map.cityOutlines);
		painter.setBrush(SCScheme.colors.map.cityNormal);
	}
	painter.drawRect(p.x() - symbolSize / 2, p.y() - symbolSize / 2,
	                 symbolSize, symbolSize);

	if ( capital != lastUnderline ) {
		lastUnderline = capital;
		font.setUnderline(capital);
		painter.setFont(font);
	}

	if ( bold != lastBold ) {
		lastBold = bold;
		font.setBold(bold);
		painter.setFont(font);
	}

	painter.setBrush(Qt::NoBrush);

	QPoint pos = labelRect.bottomLeft() + QPoint(SCScheme.map.cityHaloWidth, -SCScheme.map.cityHaloWidth - painter.fontMetrics().descent());
	QString text = city.name().c_str();
	if ( SCScheme.map.cityHaloWidth > 0 ) {
		QPainterPath path;
		path.addText(pos, font, text);
		painter.setPen(_penHalo);
		painter.drawPath(path);
	}

	painter.setPen(SCScheme.colors.map.cityLabels);
	painter.drawText(pos, text);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CitiesLayer::setSelectedCity(const Math::Geo::CityD* c) {
	_selectedCity = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Math::Geo::CityD* CitiesLayer::selectedCity() const {
	return _selectedCity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Map
} // namespce Gui
} // namespace Seiscomp
