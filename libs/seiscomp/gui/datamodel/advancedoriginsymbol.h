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


#ifndef SEISCOMP_GUI_ADVANCEDORIGINSYMBOL_H
#define SEISCOMP_GUI_ADVANCEDORIGINSYMBOL_H


#include <QVector>
#include <QPainterPath>

#include <seiscomp/gui/datamodel/originsymbol.h>

#ifndef Q_MOC_RUN
#include <seiscomp/geo/coordinate.h>
#endif

namespace Seiscomp {
namespace DataModel {
class Origin;
}

namespace Gui {


class SC_GUI_API AdvancedOriginSymbol : public OriginSymbol {
	public:
		AdvancedOriginSymbol(Map::Decorator* decorator = nullptr);
		AdvancedOriginSymbol(double latitude,
		                     double longitude,
		                     double depth = 0,
		                     Map::Decorator* decorator = nullptr);
		AdvancedOriginSymbol(DataModel::Origin *origin,
		                     Map::Decorator* decorator = nullptr);

	public:
		/**
		 * @brief setOrigin sets hypocenter, magnitude and confidence ellipse
		 * based on supplied
		 * @param origin
		 */
		void setOrigin(DataModel::Origin *origin);

		/**
		 * @brief setConfidenceEllipse sets confidence ellipse
		 * @param major major semi axis in km
		 * @param minor minor semi axis in km
		 * @param azimuth angle in degree of the major axis relative to the#
		 * prime meridian
		 * @param points number of geo coordinates used to draw the ellipse
		 */
		void setConfidenceEllipse(double major, double minor,
		                          double azimuth = 0, int points = 36);

		void setConfidenceEllipsePen(const QPen &pen);
		const QPen& confidenceEllipsePen() const;

		void setConfidenceEllipseBrush(const QBrush &brush);
		const QBrush& confidenceEllipseBrush() const;

		void setConfidenceEllipseEnabled(bool enabled);
		bool confidenceEllipseEnabled() const;

	protected:
		virtual void calculateMapPosition(const Map::Canvas *canvas);
		virtual void customDraw(const Map::Canvas *canvas, QPainter &painter);

		void init();

	protected:
		QVector<Geo::GeoCoordinate> _confidenceEllipse;
		QPainterPath                _renderPath;
		QPen                        _confidenceEllipsePen;
		QBrush                      _confidenceEllipseBrush;
		bool                        _confidenceEllipseEnabled;
};


} // namespace Gui
} // namespace Seiscomp


#endif
