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


#ifndef SEISCOMP_GUI_MAP_PROJECTIONS_MERCATOR_H
#define SEISCOMP_GUI_MAP_PROJECTIONS_MERCATOR_H


#include <QImage>
#include <seiscomp/gui/map/mapwidget.h>
#include <seiscomp/gui/map/projections/rectangular.h>


namespace Seiscomp {
namespace Gui {
namespace Map {


class SC_GUI_API MercatorProjection : public RectangularProjection {
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		MercatorProjection();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		virtual bool isRectangular() const;

		virtual bool project(QPoint &screenCoords, const QPointF &geoCoords) const;
		virtual bool unproject(QPointF &geoCoords, const QPoint &screenCoords) const;

		virtual void centerOn(const QPointF &geoCoords);

		virtual QPointF gridDistance() const;

		virtual void drawImage(QImage &buffer, const QRectF &geoReference,
		                       const QImage &image,
		                       bool highQuality, CompositionMode compositionMode);

		virtual bool project(QPainterPath &screenPath, size_t n,
		                     const Geo::GeoCoordinate *poly, bool closed,
		                     uint minPixelDist, ClipHint hint = NoClip) const;

	protected:
		template <typename PROC>
		void render(QImage &img, TextureCache *cache);

		void render(QImage &img, bool highQuality, TextureCache *cache);

		void projectUnwrapped(QPoint &screenCoords, const QPointF &geoCoords) const;


	protected:
		QRect _windowRect;
		QRect _clipRect;
		qreal _centerY;
		bool  _discreteSteps;
};


}
}
}


#endif
