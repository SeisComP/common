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


#ifndef SEISCOMP_GUI_ORIGIN_H
#define SEISCOMP_GUI_ORIGIN_H


#include <vector>
#include <string>

#include <QPolygon>
#include <QColor>
#include <QPoint>
#include <QPainter>

#include <seiscomp/gui/qt.h>
#include <seiscomp/gui/map/mapsymbol.h>
#ifndef Q_MOC_RUN
#include <seiscomp/datamodel/origin.h>
#endif


namespace Seiscomp {
namespace Gui {


class Canvas;


class SC_GUI_API OriginSymbol : public Map::Symbol {
	public:
		OriginSymbol(Map::Decorator* decorator = nullptr);
		OriginSymbol(double latitude,
		             double longitude,
		             double depth = 0,
		             Map::Decorator* decorator = nullptr);

	public:
		void setPreferredMagnitudeValue(double magnitudeValue);
		double preferredMagnitudeValue() const;

		/**
		 * @brief Sets the symbol depth and updates the color according
		 *        to scheme.colors.originSymbol.depth.gradient.
		 * @param depth The depth in km
		 */
		void setDepth(double depth);
		double depth() const;

		/**
		 * @brief Sets the symbol color and therefore overrides what
		 *        a prior call to setDepth has been set. This method was
		 *        added with API version 11.
		 * @param c The desired color
		 */
		void setColor(const QColor &c);

		/**
		 * @brief Returns the current color. This method was added with API
		 *        version 11.
		 * @return The current symbol color.
		 */
		const QColor &color() const;

		/**
		 * @brief Sets the symbols fill color if filling is enabled. This
		 *        overrides the default fill color set with setDepth.
		 *        This method was added with API version 11.
		 * @param c The fill color
		 */
		void setFillColor(const QColor &c);

		/**
		 * @brief Returns the current fill color.
		 *        This method was added with API version 11.
		 * @return The fill color
		 */
		const QColor &fillColor() const;

		void setFilled(bool val);
		bool isFilled() const;

		bool isInside(int x, int y) const override;

		/**
		 * @brief Returns the size of an origin symbol in pixel depending on
		 *        the magnitude.
		 * @param mag The input magnitude
		 * @return The size in pixels
		 */
		static int getSize(double mag);


	protected:
		void customDraw(const Map::Canvas *canvas, QPainter &painter) override;

		void init();
		void updateSize();
		void depthColorCoding();


	protected:
		DataModel::Origin *_origin;
		QColor             _color;
		QColor             _fillColor;
		bool               _filled;
		double             _magnitude;
		double             _depth;
};


inline const QColor &OriginSymbol::color() const {
	return _color;
}

inline const QColor &OriginSymbol::fillColor() const {
	return _fillColor;
}


} // namespace Gui
} // namespace Seiscomp


#endif
