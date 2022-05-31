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


#ifndef SEISCOMP_GUI_STATIONSYMBOL_H
#define SEISCOMP_GUI_STATIONSYMBOL_H


#include <iostream>
#include <string>
#include <vector>

#include <QColor>

#include <seiscomp/datamodel/station.h>
#include <seiscomp/gui/map/mapsymbol.h>
#include <seiscomp/gui/qt.h>
#include <seiscomp/math/coord.h>


// Forward declaration
class QPainter;


namespace Seiscomp {
namespace Gui {


class SC_GUI_API StationSymbol : public Map::Symbol {
	public:
		StationSymbol(Map::Decorator *decorator = nullptr);
		StationSymbol(double latitude,
		              double longitude,
		              Map::Decorator *decorator = nullptr);

		void setColor(const QColor &color);
		const QColor &color() const;

		/**
		 * @brief Sets the color of the outline. The default is black.
		 * @since 15
		 * @param color The outline color
		 */
		void setOutlineColor(const QColor &color);

		/**
		 * @brief Returns the current outline color.
		 * @since 15
		 * @return The outline color.
		 */
		const QColor &outlineColor() const;

		void setRadius(int radius);
		int radius() const;

		void setFrameColor(const QColor &color);
		const QColor &frameColor() const;

		void setFrameSize(int);
		int frameSize() const;

		bool isInside(int x, int y) const override;

		/**
		 * @brief Draws the station symbol
		 * @param canvas The canvas is not being used and can be nullptr
		 * @param painter The painter to draw with
		 */
		void customDraw(const Map::Canvas *canvas, QPainter &painter) override;

	protected:
		QPolygon generateShape(int posX, int posY, int radius);

		const QPolygon &stationPolygon() const;


	private:
		void init();


	private:
		int       _radius;
		int       _frameSize;
		QPolygon  _stationPolygon;

		QColor    _frameColor;
		QColor    _outlineColor;
		QColor    _color;
};


} // namespace Gui
} // namespace Seiscomp


#endif
