/***************************************************************************
 * Copyright (C) 2026 by Mustafa Comoglu                                    *
 *                                                                         *
 * GNU Affero General Public License Usage                                  *
 * This file may be used under the terms of the GNU Affero                  *
 * Public License version 3.0 as published by the Free Software Foundation  *
 * and appearing in the file LICENSE included in the packaging of this      *
 * file. Please review the following information to ensure the GNU Affero   *
 * Public License version 3.0 requirements will be met:                     *
 * https://www.gnu.org/licenses/agpl-3.0.html.                              *
 *                                                                          *
 * Equal Earth projection after Savric, Patterson & Jenny (2019), "The      *
 * Equal Earth map projection", International Journal of Geographical        *
 * Information Science 33(3):454-465. doi:10.1080/13658816.2018.1504949     *
 ***************************************************************************/


#ifndef SEISCOMP_GUI_MAP_PROJECTIONS_EQUALEARTH_H
#define SEISCOMP_GUI_MAP_PROJECTIONS_EQUALEARTH_H


#include <QImage>
#include <seiscomp/gui/map/projection.h>


namespace Seiscomp {
namespace Gui {
namespace Map {


/**
 * @brief Equal Earth pseudocylindrical, equal-area map projection.
 *
 * Equal Earth (Savric, Patterson & Jenny, 2018) is an equal-area
 * pseudocylindrical projection with an overall shape similar to the
 * Robinson projection. Unlike RectangularProjection it is not rectangular:
 * parallels are straight horizontal lines but meridians are curved and the
 * projected domain is a rounded shape. Screen pixels outside that shape are
 * rendered fully transparent (the canvas allocates an ARGB32 buffer for
 * non-rectangular projections).
 *
 * All geographic coordinates use the SeisComP convention:
 * QPointF(x = longitude [deg], y = latitude [deg]).
 */
class SC_GUI_API EqualEarthProjection : public Projection {
	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
	public:
		EqualEarthProjection();


	// ----------------------------------------------------------------------
	// Public projection interface
	// ----------------------------------------------------------------------
	public:
		virtual bool isRectangular() const;
		virtual bool wantsGridAntialiasing() const;

		virtual bool project(QPoint &screenCoords, const QPointF &geoCoords) const;
		virtual bool unproject(QPointF &geoCoords, const QPoint &screenCoords) const;

		virtual void centerOn(const QPointF &geoCoords);

		virtual int lineSteps(const QPointF &p0, const QPointF &p1);

		virtual bool project(QPainterPath &screenPath, size_t n,
		                     const Geo::GeoCoordinate *poly, bool closed,
		                     uint minPixelDist, ClipHint hint = NoClip) const;

		//! Parallels are straight horizontal segments here, so draw one span
		//! between the two rims instead of the base class longitude sweep
		//! (which wraps across the map at the antimeridian).
		virtual bool drawLonCircle(QPainter &p, qreal lat);

		virtual void updateBoundingBox();


	// ----------------------------------------------------------------------
	// Protected interface
	// ----------------------------------------------------------------------
	protected:
		void render(QImage &img, bool highQuality, TextureCache *cache);


	// ----------------------------------------------------------------------
	// Private methods
	// ----------------------------------------------------------------------
	private:
		//! Recomputes the cached quantities that depend on the current
		//! projection centre (central meridian / parallel).
		void updateCenter();

		//! Clamps the centre latitude (both _center and _visibleCenter) so
		//! neither map border can be scrolled into the viewport. Applied in
		//! centerOn() as well as render() so Projection::center() - which
		//! the canvas reads back while dragging - stays consistent with
		//! what is displayed (no panning dead zone).
		void clampVerticalCenter();

		//! Forward projection without longitude wrapping. lonDeg is a running
		//! longitude that may exceed +/-180 deg; the caller keeps the vertex
		//! longitudes continuous so an edge never jumps 360 deg.
		void projectContinuous(QPointF &screen, double lonDeg, double latDeg) const;


	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
	private:
		double _lam0;    //!< central meridian [rad]
		double _phi0;    //!< central parallel [rad]
		double _y0Norm;  //!< normalized projected Y of the projection centre
};


}
}
}


#endif
