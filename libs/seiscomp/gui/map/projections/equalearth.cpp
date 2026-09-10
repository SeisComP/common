/***************************************************************************
 * Copyright (C) 2026 by Mustafa Comoglu                                    *
 *                                                                          *
 * GNU Affero General Public License Usage                                  *
 * This file may be used under the terms of the GNU Affero                  *
 * Public License version 3.0 as published by the Free Software Foundation  *
 * and appearing in the file LICENSE included in the packaging of this      *
 * file. Please review the following information to ensure the GNU Affero   *
 * Public License version 3.0 requirements will be met:                     *
 * https://www.gnu.org/licenses/agpl-3.0.html.                              *
 *                                                                          *
 * Forward and inverse transforms implement equation (1) of Savric,         *
 * Patterson & Jenny, "The Equal Earth map projection", International       *
 * Journal of Geographical Information Science 33(3):454-465, 2019          *
 * (online 2018). doi:10.1080/13658816.2018.1504949                        *
 ***************************************************************************/


#include <seiscomp/gui/map/projections/equalearth.h>
#include <seiscomp/gui/map/texturecache.ipp>
#include <seiscomp/geo/coordinate.h>

#include <QPainter>

#include <algorithm>
#include <cmath>
#include <math.h>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


namespace Seiscomp {
namespace Gui {
namespace Map {


REGISTER_PROJECTION_INTERFACE(EqualEarthProjection, "EqualEarth");


namespace {


// Equal Earth polynomial coefficients (Savric et al., 2018, eq. 1).
const double A1 =  1.340264;
const double A2 = -0.081106;
const double A3 =  0.000893;
const double A4 =  0.003796;

// sqrt(3), sqrt(3)/2 and its partner 2/sqrt(3).
const double SQRT3  = 1.7320508075688772;
const double M_COEF = 0.8660254037844386;  // sqrt(3) / 2
const double INV_M  = 1.1547005383792515;  // 2 / sqrt(3)

// Parametric latitude at the geographic pole:
//   theta = asin( (sqrt(3)/2) * sin(90 deg) ) = asin(sqrt(3)/2) = pi/3
const double THETA_MAX = M_PI / 3.0;

// Degree <-> radian helpers (seiscomp/math/math.h #defines deg2rad/rad2deg).
inline double eeD2R(double d) { return d * (M_PI / 180.0); }
inline double eeR2D(double r) { return r * (180.0 / M_PI); }


// y(theta) = theta * (A1 + A2*theta^2 + A3*theta^6 + A4*theta^8)
// The northing part of the forward transform (Savric et al., 2018, eq. 1);
// depends on the parametric latitude only.
inline double eePolyY(double theta) {
	const double t2 = theta * theta;
	const double t6 = t2 * t2 * t2;
	const double t8 = t6 * t2;
	return theta * (A1 + A2 * t2 + A3 * t6 + A4 * t8);
}


// P(theta) = A1 + 3*A2*theta^2 + theta^6*(7*A3 + 9*A4*theta^2)
//          = A1 + 3*A2*theta^2 + 7*A3*theta^6 + 9*A4*theta^8
//
// This one polynomial is both the denominator of the forward x-equation
// (Savric et al., 2018, eq. 1) and d/dtheta of eePolyY(theta), i.e. the
// derivative used by the Newton-Raphson inverse (ibid., appendix 1). It
// stays >= ~1.06 for |theta| <= pi/3; the clamp only guards against a
// pathological FPU state.
inline double eeDenomP(double t2, double t6) {
	const double p = A1 + 3.0 * A2 * t2 + t6 * (7.0 * A3 + 9.0 * A4 * t2);
	return std::fabs(p) < 1.0e-12 ? A1 : p;
}


// Half height of the projected graticule in "projection units" (northing at
// the pole). Everything below is normalized by this value so that the
// vertical extent of the map becomes [-1, +1] - the same convention
// RectangularProjection uses for latitude - which keeps the zoom behaviour
// consistent when the user switches projections.
const double Y_POLE = eePolyY(THETA_MAX);                        // ~1.3174

// Half width of the projected graticule, normalized like everything else.
//   easting at (lat = 0, lon = 180 deg): x = 2*sqrt(3)*pi / (3*A1)
const double HALF_WIDTH_NORM =
	(2.0 * SQRT3 * M_PI / (3.0 * A1)) / Y_POLE;                  // ~2.0546

// Small tolerances used for outline / domain tests.
const double EPS_LON = 1.0e-9;
const double EPS_Y   = 1.0e-9;


// Forward transform, unit sphere, central meridian already subtracted
// (Savric, Patterson & Jenny, 2018, eq. 1):
//
//   theta = asin( (sqrt(3)/2) * sin(phi) )
//   x     = 2*sqrt(3) * lambda * cos(theta) / ( 3 * P(theta) )
//   y     = theta * ( A1 + A2*theta^2 + A3*theta^6 + A4*theta^8 )
//
// The sphere is projected directly: SeisComP feeds spherical geographic
// coordinates, which matches the paper's recommendation for spherical
// input (an ellipsoid would need a conversion to authalic latitude first,
// not applicable here). Inputs / outputs are radians / projection units.
inline void eeForward(double lambda, double phi, double &x, double &y) {
	double s = M_COEF * std::sin(phi);
	// Guard the asin() domain against round-off (|sqrt(3)/2 * sin| <= 0.8661).
	if ( s >  1.0 ) s =  1.0;
	else if ( s < -1.0 ) s = -1.0;

	const double theta = std::asin(s);
	const double t2 = theta * theta;
	const double t6 = t2 * t2 * t2;

	x = (2.0 * SQRT3 * lambda * std::cos(theta)) / (3.0 * eeDenomP(t2, t6));
	y = theta * (A1 + A2 * t2 + A3 * t6 + A4 * t6 * t2);
}


// Solve  y = eePolyY(theta)  for theta using Newton-Raphson.
//
//   f (theta) = theta*(A1 + A2*th^2 + A3*th^6 + A4*th^8) - y
//   f'(theta) = eeDenomP(th^2, th^6)   (>= ~1.06 for |theta| <= pi/3)
//
// so the iteration is well conditioned and converges to double precision
// in a handful of steps.
inline double eeSolveTheta(double y) {
	if ( y >=  Y_POLE ) return  THETA_MAX;
	if ( y <= -Y_POLE ) return -THETA_MAX;

	// First guess: near the equator y ~= A1*theta.
	double theta = y / A1;

	for ( int i = 0; i < 20; ++i ) {
		const double t2 = theta * theta;
		const double t6 = t2 * t2 * t2;
		const double t8 = t6 * t2;

		const double f  = theta * (A1 + A2 * t2 + A3 * t6 + A4 * t8) - y;
		const double dt = f / eeDenomP(t2, t6);
		theta -= dt;
		if ( std::fabs(dt) < 1.0e-13 ) break;
	}

	if ( theta >  THETA_MAX ) theta =  THETA_MAX;
	else if ( theta < -THETA_MAX ) theta = -THETA_MAX;
	return theta;
}


// Inverse transform, unit sphere, central meridian NOT yet added back.
// Returns false if (x, y) is outside the projection outline, i.e. does not
// correspond to any location on the globe.
inline bool eeInverse(double x, double y, double &lambda, double &phi) {
	if ( y >  Y_POLE + EPS_Y ) return false;
	if ( y < -Y_POLE - EPS_Y ) return false;

	const double theta = eeSolveTheta(y);
	const double t2 = theta * theta;
	const double t6 = t2 * t2 * t2;

	// Recover geographic latitude:  sin(theta) = (sqrt(3)/2) * sin(phi)
	double sinPhi = std::sin(theta) * INV_M;
	if ( sinPhi >  1.0 ) sinPhi =  1.0;
	else if ( sinPhi < -1.0 ) sinPhi = -1.0;
	phi = std::asin(sinPhi);

	// Recover longitude by inverting the easting equation.
	const double cosT = std::cos(theta);
	if ( cosT < 1.0e-12 ) {
		// Geographic pole: longitude is undefined. Only x == 0 is on the map.
		lambda = 0.0;
		return std::fabs(x) < 1.0e-6;
	}

	lambda = (3.0 * eeDenomP(t2, t6) * x) / (2.0 * SQRT3 * cosT);

	// Antimeridian clipping: |lambda| > pi is beyond the left / right rim.
	if ( lambda >  M_PI + EPS_LON ) return false;
	if ( lambda < -M_PI - EPS_LON ) return false;

	return true;
}


// Wrap a longitude in degrees into (-180, 180].
inline double wrapLonDeg(double lon) {
	lon = std::fmod(lon + 180.0, 360.0);
	if ( lon < 0.0 ) lon += 360.0;
	return lon - 180.0;
}


// Latitude grid label, same formatting as RectangularProjection.
QString lat2String(qreal lat) {
	int nlat = (lat * 100000) + (lat < 0 ? -0.5 : +0.5);

	if ( nlat % 10 )
		return QString("%1%2").arg(fabs(lat), 0, 'f', 5).arg(lat < 0 ? " S" : lat > 0 ? " N" : "");
	else if ( nlat % 100 )
		return QString("%1%2").arg(fabs(lat), 0, 'f', 4).arg(lat < 0 ? " S" : lat > 0 ? " N" : "");
	else if ( nlat % 1000 )
		return QString("%1%2").arg(fabs(lat), 0, 'f', 3).arg(lat < 0 ? " S" : lat > 0 ? " N" : "");
	else if ( nlat % 10000 )
		return QString("%1%2").arg(fabs(lat), 0, 'f', 2).arg(lat < 0 ? " S" : lat > 0 ? " N" : "");
	else if ( nlat % 100000 )
		return QString("%1%2").arg(fabs(lat), 0, 'f', 1).arg(lat < 0 ? " S" : lat > 0 ? " N" : "");
	else
		return QString("%1%2").arg(abs((int)lat)).arg(lat < 0 ? " S" : lat > 0 ? " N" : "");
}


} // anonymous namespace




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EqualEarthProjection::EqualEarthProjection()
: Projection()
, _lam0(0.0)
, _phi0(0.0)
, _y0Norm(0.0) {
	updateCenter();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EqualEarthProjection::isRectangular() const {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EqualEarthProjection::wantsGridAntialiasing() const {
	// Meridians and off-centre parallels are drawn as poly-lines;
	// anti-aliasing removes the visible staircase.
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EqualEarthProjection::updateCenter() {
	// _visibleCenter is stored by the base class as (lon/180, lat/90).
	_lam0 = _visibleCenter.x() * M_PI;
	_phi0 = _visibleCenter.y() * (M_PI / 2.0);

	double x, y;
	eeForward(0.0, _phi0, x, y);
	_y0Norm = y / Y_POLE;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EqualEarthProjection::centerOn(const QPointF &geoCoords) {
	double lon = wrapLonDeg(geoCoords.x());
	double lat = geoCoords.y();

	if ( lat >  90.0 ) lat =  90.0;
	else if ( lat < -90.0 ) lat = -90.0;

	_center        = QPointF(lon / 180.0, lat / 90.0);
	_visibleCenter = _center;

	clampVerticalCenter();
	updateCenter();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Keep the centre latitude within the range that still shows the map on
// both the top and bottom viewport edges. Applied from centerOn() and
// render() so the value the canvas reads back with Projection::center()
// while dragging matches what is drawn - otherwise the over-panned excess
// has to be dragged back before the map starts moving again.
void EqualEarthProjection::clampVerticalCenter() {
	if ( _scale <= 0.0 || _halfHeight <= 0 )
		return;

	const double vHalf = _halfHeight / _scale;   // viewport half-height, normalized

	double latLim;                               // as (latitude / 90)
	if ( vHalf >= 1.0 ) {
		latLim = 0.0;
	}
	else {
		const double thetaLim = eeSolveTheta((1.0 - vHalf) * Y_POLE);
		double sinLat = std::sin(thetaLim) * INV_M;
		if ( sinLat >  1.0 ) sinLat =  1.0;
		else if ( sinLat < -1.0 ) sinLat = -1.0;
		latLim = std::asin(sinLat) / (M_PI / 2.0);
	}

	double cy = _center.y();
	if ( cy >  latLim ) cy =  latLim;
	else if ( cy < -latLim ) cy = -latLim;

	_center.setY(cy);
	_visibleCenter.setY(cy);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Every geographic coordinate maps to exactly one screen position, so the
// method always returns true (matching RectangularProjection and the
// expectations of Projection::updateBoundingBox()).
bool EqualEarthProjection::project(QPoint &screenCoords,
                                   const QPointF &geoCoords) const {
	if ( _scale <= 0.0 ) return false;

	double lat = geoCoords.y();
	if ( lat >  90.0 ) lat =  90.0;
	else if ( lat < -90.0 ) lat = -90.0;

	// Longitude relative to the central meridian, wrapped to [-pi, pi] in
	// O(1) (fmod, not a loop, so a pathological input cannot stall the GUI).
	// This is where the antimeridian seam is placed.
	double lambda = std::fmod(eeD2R(geoCoords.x()) - _lam0, 2.0 * M_PI);
	if ( lambda >  M_PI ) lambda -= 2.0 * M_PI;
	else if ( lambda < -M_PI ) lambda += 2.0 * M_PI;

	double x, y;
	eeForward(lambda, eeD2R(lat), x, y);

	// Normalize so that latitude +/-90 deg -> +/-1, then apply the same
	// pixel scale the base class uses for the vertical axis.
	const double nx = x / Y_POLE;
	const double ny = y / Y_POLE - _y0Norm;

	screenCoords.setX(int(std::lround(_halfWidth  + nx * _scale)));
	screenCoords.setY(int(std::lround(_halfHeight - ny * _scale)));
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Returns false when the pixel is outside the projection outline (the
// rounded corners of the map), so callers can reject clicks / hover
// positions that are not on the globe.
bool EqualEarthProjection::unproject(QPointF &geoCoords,
                                     const QPoint &screenCoords) const {
	if ( _scale <= 0.0 ) return false;

	const double nx = (double(screenCoords.x()) - _halfWidth) / _scale;
	const double ny = (double(_halfHeight) - screenCoords.y()) / _scale + _y0Norm;

	// Cheap rejection against the normalized bounding rectangle.
	if ( ny >  1.0 + EPS_Y || ny < -1.0 - EPS_Y ) return false;
	if ( nx >  HALF_WIDTH_NORM + EPS_Y || nx < -HALF_WIDTH_NORM - EPS_Y )
		return false;

	double lambda, phi;
	if ( !eeInverse(nx * Y_POLE, ny * Y_POLE, lambda, phi) )
		return false;

	double lon = wrapLonDeg(eeR2D(lambda + _lam0));
	double lat = eeR2D(phi);
	if ( lat >  90.0 ) lat =  90.0;
	else if ( lat < -90.0 ) lat = -90.0;

	geoCoords.setX(lon);
	geoCoords.setY(lat);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Forward projection without longitude wrapping. Used to build polygon
// outlines from a running (continuous) longitude so that an edge across the
// antimeridian stays a short segment.
void EqualEarthProjection::projectContinuous(QPointF &screen,
                                             double lonDeg, double latDeg) const {
	if ( latDeg >  90.0 ) latDeg =  90.0;
	else if ( latDeg < -90.0 ) latDeg = -90.0;

	// Clamp the longitude offset to +/- pi: a running longitude may leave
	// the [-180, 180] window of this world copy, and beyond the antimeridian
	// there is no map. Placing the vertex on the rim clips the polygon to
	// the projection outline; the part that wrapped around is drawn by the
	// neighbouring world copy at the opposite rim.
	double lambda = eeD2R(lonDeg) - _lam0;
	if ( lambda >  M_PI ) lambda =  M_PI;
	else if ( lambda < -M_PI ) lambda = -M_PI;

	double x, y;
	eeForward(lambda, eeD2R(latDeg), x, y);

	const double nx = x / Y_POLE;
	const double ny = y / Y_POLE - _y0Norm;

	screen.setX(_halfWidth  + nx * _scale);
	screen.setY(_halfHeight - ny * _scale);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Compared with the base implementation this:
//   * accumulates a continuous longitude, so an edge crossing the
//     antimeridian stays a short segment instead of a line spanning the map;
//   * subdivides edges with a latitude change (parallels are straight in
//     Equal Earth, meridians curve) so a sparse polygon does not cut across
//     the graticule;
//   * emits every world copy (longitude +/- k*360 deg) that overlaps the
//     viewport, each re-projected - a rigid screen-space translation is not
//     valid because the Equal Earth horizontal period shrinks towards the
//     poles;
//   * caps polygons that enclose a pole against the top / bottom map edge so
//     the polar area fills instead of leaving an open ribbon.
bool EqualEarthProjection::project(QPainterPath &screenPath, size_t n,
                                   const Geo::GeoCoordinate *poly, bool closed,
                                   uint minPixelDist, ClipHint) const {
	if ( n < 2 || !poly || _scale <= 0.0 )
		return false;

	const double minDeg = (minPixelDist > 0)
	                      ? double(minPixelDist) / pixelPerDegree()
	                      : 0.0;

	// 1. Vertex list with a continuous (never-jumping) longitude.
	std::vector<QPointF> gc;   // x = running longitude [deg], y = latitude [deg]
	gc.reserve(n);

	double runLon = poly[0].lon;
	double prevLon = poly[0].lon;
	double latMin = poly[0].lat;
	double latMax = poly[0].lat;
	gc.push_back(QPointF(runLon, poly[0].lat));

	for ( size_t i = 1; i < n; ++i ) {
		double d = poly[i].lon - prevLon;
		prevLon = poly[i].lon;
		if ( d >  180.0 ) d -= 360.0;
		else if ( d < -180.0 ) d += 360.0;
		runLon += d;

		if ( poly[i].lat < latMin ) latMin = poly[i].lat;
		if ( poly[i].lat > latMax ) latMax = poly[i].lat;

		const bool keepLast = !closed && (i == n - 1);
		if ( !keepLast && minDeg > 0.0
		  && std::fabs(runLon - gc.back().x()) <= minDeg
		  && std::fabs(poly[i].lat - gc.back().y()) <= minDeg )
			continue;

		gc.push_back(QPointF(runLon, poly[i].lat));
	}

	if ( gc.size() < 2 )
		return false;

	double minLon = gc.front().x();
	double maxLon = gc.front().x();
	for ( const QPointF &c : gc ) {
		if ( c.x() < minLon ) minLon = c.x();
		if ( c.x() > maxLon ) maxLon = c.x();
	}

	// 2. Pole test: a closed ring whose longitude winds a full turn encloses
	//    a pole. Total continuous longitude travelled, closing edge included:
	double lonSweep = runLon - poly[0].lon;
	double dClose = poly[0].lon - prevLon;
	if ( dClose >  180.0 ) dClose -= 360.0;
	else if ( dClose < -180.0 ) dClose += 360.0;
	lonSweep += dClose;

	bool spansNorth = false;
	bool spansSouth = false;
	if ( closed && std::fabs(lonSweep) > 270.0 ) {
		// The enclosed pole is the one the ring hugs.
		if ( (90.0 - latMax) <= (latMin + 90.0) ) spansNorth = true;
		else                                      spansSouth = true;
	}
	const bool spansPole = spansNorth || spansSouth;

	// 3. Emit every visible world copy.
	const double centerLon = _visibleCenter.x() * 180.0;

	int kMin = int(std::floor((centerLon - 180.0 - maxLon) / 360.0));
	int kMax = int(std::ceil ((centerLon + 180.0 - minLon) / 360.0));
	if ( kMax - kMin > 4 ) kMax = kMin + 4;   // guard against bad input

	bool any = false;

	for ( int k = kMin; k <= kMax; ++k ) {
		const double off = k * 360.0;

		// Draw this world copy only if the polygon's longitude span really
		// overlaps the visible [-180, 180] window. projectContinuous() then
		// clips whatever part still pokes past a rim; without the strict
		// test a copy whose polygon lies just outside the window would
		// otherwise collapse onto the rim as a spurious sliver.
		if ( maxLon + off < centerLon - 180.0 ) continue;
		if ( minLon + off > centerLon + 180.0 ) continue;

		QPointF p, first;
		projectContinuous(first, gc.front().x() + off, gc.front().y());
		screenPath.moveTo(first);

		QPointF prev = first;
		double  prevLonC = gc.front().x();
		double  prevLatC = gc.front().y();

		for ( size_t j = 1; j < gc.size(); ++j ) {
			const double curLon = gc[j].x();
			const double curLat = gc[j].y();
			projectContinuous(p, curLon + off, curLat);

			// Parallels are exact straight lines in Equal Earth, but any edge
			// with a latitude change follows a curved meridian and has to be
			// subdivided or it cuts across the graticule.
			const double dLat = std::fabs(curLat - prevLatC);
			int steps = 1;
			if ( dLat > 0.5
			  && std::hypot(p.x() - prev.x(), p.y() - prev.y()) > 6.0 ) {
				const double dLon = std::fabs(curLon - prevLonC);
				steps = 1 + int(dLat / 1.5 + dLon / 12.0);
				if ( steps > 64 ) steps = 64;
			}

			for ( int s = 1; s < steps; ++s ) {
				const double t = double(s) / steps;
				QPointF ip;
				projectContinuous(ip,
				                  prevLonC + t * (curLon - prevLonC) + off,
				                  prevLatC + t * (curLat - prevLatC));
				screenPath.lineTo(ip);
			}
			screenPath.lineTo(p);

			prev = p;
			prevLonC = curLon;
			prevLatC = curLat;
		}

		if ( closed && spansPole ) {
			// Route back along the map edge to close off the polar cap.
			const double edgeY = spansNorth ? -10.0 : double(_height) + 10.0;
			const QPointF last = screenPath.currentPosition();
			screenPath.lineTo(last.x(),  edgeY);
			screenPath.lineTo(first.x(), edgeY);
			screenPath.closeSubpath();
		}
		else if ( closed ) {
			screenPath.closeSubpath();
		}

		any = true;
	}

	return any && !screenPath.isEmpty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// A parallel is a straight horizontal segment between the two rims. Drawing
// it as one line avoids the base class' longitude sweep, which projects
// each sample independently and therefore jumps across the whole map when
// it passes the antimeridian.
bool EqualEarthProjection::drawLonCircle(QPainter &painter, qreal lat) {
	if ( _scale <= 0.0 )
		return false;

	if ( lat >  90.0 ) lat =  90.0;
	else if ( lat < -90.0 ) lat = -90.0;

	// Rim points at this latitude: lambda = +/- pi. Easting is odd in
	// lambda, so the two rims are symmetric about the centre column.
	double x, y;
	eeForward(M_PI, eeD2R(lat), x, y);
	const double dx = (x / Y_POLE) * _scale;
	const double sy = (y / Y_POLE - _y0Norm) * _scale;

	const int py = int(std::lround(_halfHeight - sy));
	if ( py < 0 || py >= _height )
		return false;                       // off screen -> stop the grid loop

	const int xl = int(std::lround(_halfWidth - dx));
	const int xr = int(std::lround(_halfWidth + dx));

	painter.drawLine(xl, py, xr, py);
	painter.drawText(
		QRect(std::max(0, xl) + painter.fontMetrics().height() / 4, py,
		      _width, _height),
		Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, lat2String(lat));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int EqualEarthProjection::lineSteps(const QPointF &p0, const QPointF &p1) {
	double dLon = std::fabs(p1.x() - p0.x());
	if ( dLon > 180.0 ) dLon = 360.0 - dLon;
	const double dLat = std::fabs(p1.y() - p0.y());

	int steps = int((dLon + dLat) / 2.0);
	if ( steps < 2 )   steps = 2;
	if ( steps > 100 ) steps = 100;
	return steps;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// For a fixed screen row the projected Y (hence the parametric latitude
// theta, the geographic latitude and the longitude scale factor) is
// constant, so those are evaluated once per row and the longitude then
// varies linearly along the row. Pixels whose back-projected longitude
// exceeds +/-180 deg fall outside the map outline and stay transparent.
void EqualEarthProjection::render(QImage &img, bool highQuality,
                                  TextureCache *cache) {
	// 1. Pixel scale: a normalized projection radius of 1.0 maps to
	//    _screenRadius pixels, chosen so the whole Equal Earth outline fits
	//    the viewport at zoom == 1.
	_screenRadius = std::min(_width  * 0.5 / HALF_WIDTH_NORM,
	                         _height * 0.5);

	if ( _screenRadius <= 0.0 ) {            // zero-sized canvas
		img.fill(qRgba(0, 0, 0, 0));
		return;
	}

	qreal radius = _screenRadius * _radius;   // _radius == zoom factor

	// Do not allow the map to shrink below the viewport size.
	const qreal minRadius = std::max(_width  * 0.5 / HALF_WIDTH_NORM,
	                                 _height * 0.5);
	if ( radius < minRadius )
		radius = minRadius;

	setVisibleRadius(radius / _screenRadius); // -> _scale == radius

	clampVerticalCenter();                    // keep both borders on the map
	updateCenter();

	const int  w = img.width();
	const int  h = img.height();
	const QRgb transparent = qRgba(0, 0, 0, 0);

	if ( cache == nullptr ) {
		img.fill(transparent);
		return;
	}

	// 2. Texture pyramid level (same heuristic as RectangularProjection).
	qreal pixelRatio = 2.0 * _scale / cache->tileHeight();
	const bool mercatorTiles = cache->isMercatorProjected();
	if ( mercatorTiles )
		pixelRatio *= 2;
	if ( pixelRatio < 1.0 ) pixelRatio = 1.0;

	int level = int(std::log(pixelRatio) / std::log(2.0) + 0.7);
	if ( level < 0 ) level = 0;
	if ( level > cache->maxLevel() ) level = cache->maxLevel();

	// Mercator tile stores only cover roughly +/-85 deg.
	const double MERC_LAT_LIMIT = eeD2R(85.05113);

	// 3. Scan-line loop.
	for ( int iy = 0; iy < h; ++iy ) {
		QRgb *scan = reinterpret_cast<QRgb*>(img.scanLine(iy));

		const double ny = (double(_halfHeight) - iy) / _scale + _y0Norm;
		if ( ny > 1.0 || ny < -1.0 ) {
			for ( int ix = 0; ix < w; ++ix ) scan[ix] = transparent;
			continue;
		}

		const double theta = eeSolveTheta(ny * Y_POLE);
		const double t2   = theta * theta;
		const double t6   = t2 * t2 * t2;
		const double cosT = std::cos(theta);

		if ( cosT < 1.0e-9 ) {          // degenerate pole row
			for ( int ix = 0; ix < w; ++ix ) scan[ix] = transparent;
			continue;
		}

		// Geographic latitude of this row.
		double sinPhi = std::sin(theta) * INV_M;
		if ( sinPhi >  1.0 ) sinPhi =  1.0;
		else if ( sinPhi < -1.0 ) sinPhi = -1.0;
		const double phi    = std::asin(sinPhi);
		const double latDeg = eeR2D(phi);

		// Constant texture V coordinate for the whole row.
		Coord v;
		if ( mercatorTiles ) {
			double p = phi;
			if ( p >  MERC_LAT_LIMIT ) p =  MERC_LAT_LIMIT;
			else if ( p < -MERC_LAT_LIMIT ) p = -MERC_LAT_LIMIT;
			const double my = std::asinh(std::tan(p)) / M_PI;   // [-1, 1]
			v.value = Coord::value_type((1.0 - my)
			          * double(Coord::fraction_half_max));
		}
		else {
			v.value = Coord::value_type((1.0 - latDeg / 90.0)
			          * double(Coord::fraction_half_max));
		}

		// Longitude is linear in the pixel column x:
		//   lonRad(x) = dLambda * (x - _halfWidth) + _lam0
		// and the texture U coordinate is linear in lonRad, so U is stepped
		// across the row with a single add per pixel - as
		// RectangularProjection does - instead of a divide plus fmod. The
		// outline |lonRad| <= pi bounds the visible columns; getTexel()'s
		// fractional masking takes care of the longitude wrap.
		const double Kx      = (3.0 * eeDenomP(t2, t6)) / (2.0 * SQRT3 * cosT);
		const double dLambda = (Kx * Y_POLE) / _scale;   // lonRad per pixel (> 0)
		const double span    = M_PI / dLambda;           // pixels: centre -> rim

		int xl = int(std::ceil (_halfWidth - span));
		int xr = int(std::floor(_halfWidth + span));
		if ( xl < 0 ) xl = 0;
		if ( xr > w - 1 ) xr = w - 1;

		int ix = 0;
		for ( ; ix < xl; ++ix ) scan[ix] = transparent;

		if ( xl <= xr ) {
			const double fh    = double(Coord::fraction_half_max);
			const double uStep = (dLambda / M_PI) * fh;
			double       uu    = ((dLambda * (xl - _halfWidth) + _lam0) / M_PI
			                      + 1.0) * fh;

			for ( ; ix <= xr; ++ix, uu += uStep ) {
				Coord u;
				u.value = Coord::value_type(uu);

				QRgb c;
				if ( highQuality )
					cache->getTexelBilinear(c, u, v, level);
				else
					cache->getTexel(c, u, v, level);

				scan[ix] = c | 0xff000000u;   // force opaque inside the map
			}
		}

		for ( ; ix < w; ++ix ) scan[ix] = transparent;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// A coarse screen grid is back-projected and the geographic extent
// accumulated. Pole visibility is handled explicitly because near a pole the
// sampled longitude range collapses.
void EqualEarthProjection::updateBoundingBox() {
	_mapBoundingBox.reset();
	if ( _width <= 0 || _height <= 0 || _scale <= 0.0 )
		return;

	const double centerLon = _visibleCenter.x() * 180.0;
	const int    step      = std::max(1, std::min(_width, _height) / 64);

	bool   have = false;
	double west = 0.0, east = 0.0, north = 0.0, south = 0.0;

	QPointF g;
	for ( int y = 0; y < _height; y += step ) {
		for ( int x = 0; x < _width; x += step ) {
			if ( !unproject(g, QPoint(x, y)) )
				continue;

			// Longitude relative to the centre, in [-180, 180].
			const double dLon = Geo::GeoCoordinate::distanceLon(g.x(), centerLon);

			if ( !have ) {
				west = east = dLon;
				north = south = g.y();
				have = true;
			}
			else {
				if ( dLon  < west  ) west  = dLon;
				if ( dLon  > east  ) east  = dLon;
				if ( g.y() > north ) north = g.y();
				if ( g.y() < south ) south = g.y();
			}
		}
	}

	if ( !have ) {
		// Should not happen (screen centre is always on the map), fallback.
		_mapBoundingBox.west  = -180.0; _mapBoundingBox.east  = 180.0;
		_mapBoundingBox.south =  -90.0; _mapBoundingBox.north =  90.0;
		return;
	}

	QPoint p;
	project(p, QPointF(0.0,  90.0));
	const bool northPole = p.x() >= 0 && p.x() < _width &&
	                       p.y() >= 0 && p.y() < _height;
	project(p, QPointF(0.0, -90.0));
	const bool southPole = p.x() >= 0 && p.x() < _width &&
	                       p.y() >= 0 && p.y() < _height;

	if ( northPole ) north =  90.0;
	if ( southPole ) south = -90.0;

	if ( northPole || southPole || (east - west) >= 359.0 ) {
		_mapBoundingBox.west = -180.0;
		_mapBoundingBox.east =  180.0;
	}
	else {
		_mapBoundingBox.west = Geo::GeoCoordinate::normalizeLon(west + centerLon);
		_mapBoundingBox.east = Geo::GeoCoordinate::normalizeLon(east + centerLon);
	}

	_mapBoundingBox.north = std::min( 90.0, north);
	_mapBoundingBox.south = std::max(-90.0, south);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}
}
}
