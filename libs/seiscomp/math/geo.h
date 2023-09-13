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


#ifndef SEISCOMP_MATH_GEO
#define SEISCOMP_MATH_GEO

#include <seiscomp/core.h>
#include <seiscomp/math/coord.h>
#include <vector>

namespace Seiscomp
{

namespace Math
{

namespace Geo
{

/**
 * For two points (lat1, lon1) and (lat2, lon2),
 * the angular distance in degrees is returned.
 */
SC_SYSTEM_CORE_API
double delta(double lat1, double lon1, double lat2, double lon2);

/**
 * For two points (lat1, lon1) and (lat2, lon2),
 * the angular distance 'dist' in degrees,
 * the azimuth 'azi1' (azimuth of point 2 seen from point 1) and
 * the azimuth 'azi2' (azimuth of point 1 seen from point 2)
 * are computed.
 */
SC_SYSTEM_CORE_API
void delazi(double lat1, double lon1, double lat2, double lon2,
            double *out_dist = nullptr, double *out_azi1 = nullptr,
            double *out_azi2 = nullptr);


/**
 * For two points (lat1, lon1) and (lat2, lon2),
 * the angular distance 'dist' in degrees,
 * the azimuth 'azi1' (azimuth of point 2 seen from point 1) and
 * the azimuth 'azi2' (azimuth of point 1 seen from point 2)
 * are computed.
 */
SC_SYSTEM_CORE_API
void delazi_wgs84(double lat1, double lon1, double lat2, double lon2,
                  double *out_dist, double *out_azi1, double *out_azi2);


/**
 * Computes the coordinates (lat, lon) of the point which
 * is at an azimuth of 'azi' and a distance of 'dist' as seen
 * from the point (lat0, lon0).
 *      -> lat, lon
 */
SC_SYSTEM_CORE_API
void delandaz2coord(double dist, double azi, double lat0, double lon0,
                    double *out_lat, double *out_lon);

/**
 * Compute the intersection points between two small circles.
 * Returns the number of intersection points found; 0 means circles don't
 * intersect at all (or their centers coincide), 1 means both intersection
 * points coincide, i.e. the circles just "touch" at one point, 2 is a
 * normal intersection
 *
 * @returns lat, lon of intersection points my the way of latx1, lonx1, latx2,
 * lonx2
 *
 * XXX Corrently the case 1 (circles "touch") is not implemented well. Best
 * way is to test if in case 2 the intersection points are very close.
 */
SC_SYSTEM_CORE_API
int scxsc(double lat1, double lon1, double r1,
	  double lat2, double lon2, double r2,
	  double *latx1, double *lonx1, double *latx2, double *lonx2,
	  double epsilon=0);

/**
 * "Draws" a small circle
 *
 * Around a center point (lat0, lon0), compute n equally spaced points on a
 * small circle with specified radius (in degrees).
 */
SC_SYSTEM_CORE_API
int scdraw(double lat0, double lon0, double radius,
	   int n, double *lat, double *lon);

/**
 * Definition of degree to km conversion constant is based on the
 * Mean Radius of the Three Semi-Axes as defined by WGS84
 * (https://nsgreg.nga.mil/doc/view?i=4085)
 *
 *   - Semi-major Axis (Equatorial Radius of the Earth) [a] : 6378137.0m
 *   - Flattening Factor of the Earth                 [1/f] : 298.257223563
 *   - Semi-major Axis (Polar Radius of the Earth)      [b] : 6356752.314245179m
 *       b = a * (1 - f)
 *   - Mean Radius of the Three Semi-Axes             [r_1] : 6371008.77141506m
 *       r_1 = a * (1 - f/3) = (2a + b) / 3
 *   - Kilometer per Degree                        [km_deg] : 111.195079734632km
 *       km_deg = r_1 * Pi / (180 * 1000)
 *
 *  Note:
 *    Prior to SeisComP 6 (API 16.0.0) this constant was set to
 *    111.1329149013519096
 */
#define KM_OF_DEGREE 111.195079734632


template<typename T>
T deg2km(T deg) { return deg * (T)KM_OF_DEGREE; }

template<typename T>
T km2deg(T km)  { return km / (T)KM_OF_DEGREE; }

/**
 * Converts X, Y, Z coordinates to LTP coordinates using WGS-84 constants for
 * the ellipsoid.
 *
 * For reference, The X, Y, Z coordinate system has origin at the mass center
 * of the Earth, with Z axis along the spin axis of the Earth (+ at North pole).
 * The +X axis emerges from the Earth at the equator-prime meridian
 * intersection. The +Y axis defines a right-hand coordinate system, emerging
 * from the Earth at +90 degrees longitude on the equator.

 * The Local Tangential Plane (LTP) coordinates are in latitude/longitude/
 * altitude where North latitude is + and East longitude is +. Altitude is in
 * meters above the reference ellipsoid (which may be either above or below the
 * local mean sea level).
 */
SC_SYSTEM_CORE_API
void xyz2ltp(const double x, const double y, const double z,
             double *lat, double *lon, double *alt);

/**
 * Converts LTP coordinates to X, Y, Z coordinates using WGS-84 constants for
 * the ellipsoid.
 *
 * For reference, The X, Y, Z coordinate system has origin at the mass center
 * of the Earth, with Z axis along the spin axis of the Earth (+ at North pole).
 * The +X axis emerges from the Earth at the equator-prime meridian
 * intersection. The +Y axis defines a right-hand coordinate system, emerging
 * from the Earth at +90 degrees longitude on the equator.

 * The Local Tangential Plane (LTP) coordinates are in latitude/longitude/
 * altitude where North latitude is + and East longitude is +. Altitude is in
 * meters above the reference ellipsoid (which may be either above or below the
 * local mean sea level).
 */
SC_SYSTEM_CORE_API
void ltp2xyz(double lat, double lon, double alt,
             double *x, double *y, double *z);



/**
 * Finds the nearest hotspot of an array of hotspots regarding a given
 * location.
 *
 * Returns the distance in degree and the azimuth seen from the location.
 */
SC_SYSTEM_CORE_API
const NamedCoordD*
nearestHotspot(double lat, double lon, double maxDist,
               int nCoords, const NamedCoordD* coordArray,
               double *dist, double *azi);

SC_SYSTEM_CORE_API
const NamedCoordD*
nearestHotspot(double lat, double lon, double maxDist,
               const std::vector<NamedCoordD>& coords,
               double *dist, double *azi);


SC_SYSTEM_CORE_API
const CityD*
nearestCity(double lat, double lon,
            double maxDist, double minPopulation,
            int nCities, const CityD* cityArray,
            double *dist, double *azi);

SC_SYSTEM_CORE_API
const CityD*
nearestCity(double lat, double lon,
            double maxDist, double minPopulation,
            const std::vector<CityD>& cities,
            double *dist, double *azi);

SC_SYSTEM_CORE_API
const CityD*
largestCity(double lat, double lon,
            double maxDist,
            const std::vector<CityD>& cities,
            double *dist, double *azi);


class SC_SYSTEM_CORE_API PositionInterpolator {
	public:
		/**
		 * Constructs an interpolator between lat1,lon1 and lat2,lon2.
		 * Between both positions 'steps-2' new equidistanct positions
		 * are going to be calculated.
		 * @param lat1 Latitude of position 1
		 * @param lon1 Longitude of position 1
		 * @param lat2 Latitude of position 2
		 * @param lon2 Longitude of position 2
		 * @param steps Number of steps including position 1 and position 2
		 */
		PositionInterpolator(double lat1, double lon1,
		                     double lat2, double lon2,
		                     int steps);

		/**
		 * Same as above but the number of steps depends on the
		 * distance between both positions
		 * steps = int(|pos1-pos2|*stepsDistScale)
		 * @param lat1 Latitude of position 1
		 * @param lon1 Longitude of position 1
		 * @param lat2 Latitude of position 2
		 * @param lon2 Longitude of position 2
		 * @param stepsDistScale The distance scale factor to calculate
		 *                       the number of steps
		 */
		PositionInterpolator(double lat1, double lon1,
		                     double lat2, double lon2,
		                     double stepsDistScale);

	public:
		PositionInterpolator& operator++();
		PositionInterpolator operator++(int);

		bool end() const;

		//! Returns the distance between pos1 and pos2
		//! given to constructor
		double overallDistance() const;

		//! Returns the current interpolation distance from pos1
		double distance() const;
		//! Returns the azimuth of pos2 seen from pos1
		double azimuth() const;

		//! Returns the current interpolated latitude
		double latitude() const;
		//! Returns the current interpolated longitude
		double longitude() const;

	private:
		void update();

	private:
		double _lat1, _lon1;
		double _lat2, _lon2;
		double _dist;
		double _azimuth;
		double _stepDist;
		double _currentDist;
		double _lat, _lon;

		int _nSteps;
		int _currentStep;
};


inline double PositionInterpolator::overallDistance() const {
	return _dist;
}

inline double PositionInterpolator::distance() const {
	return _currentDist;
}

inline double PositionInterpolator::azimuth() const {
	return _azimuth;
}

inline double PositionInterpolator::latitude() const {
	return _lat;
}

inline double PositionInterpolator::longitude() const {
	return _lon;
}


} // namespace Seiscomp::Math::Geo

} // namespace Seiscomp::Math

} // namespace Seiscomp

#endif
