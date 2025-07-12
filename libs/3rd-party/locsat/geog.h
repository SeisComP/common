/******************************************************************************
 * Copyright 1991 Science Applications International Corporation.
 * Modifications and fixes gempa GmbH in 2025
 *
 * Note: there was no particular license attached to the origin source code.
 * gempa GmbH modified the source code in the following way:
 * - Translation from Fortran to plain C
 * - Prefix of function names with sc_locsat_
 * - Remove printing of debug information
 * - Get rid of static variables and make all code reentrant
 ******************************************************************************/


#ifndef LOCSAT_GEOG_H
#define LOCSAT_GEOG_H


#include <locsat/utils.h>


/**
 * @brief Calculate angular distance, azimuth and backazimuth between two
 *        points on a sphere.
 * All arguments are in degrees. Latitude, longitude and delta are
 * geocentric.  Latitude is zero at equator and positive North.
 * Longitude is positive toward the East azi is measured clockwise
 * from local North.
 * Azi and baz are positive and measured clockwise from local north.
 * @param[in] alat1 Latitude of point 1.
 * @param[in] alon1 Longitude of point 1.
 * @param[in] alat2 Latitude of point 2.
 * @param[in] alon2 Longitude of point 2.
 * @param[out] delta Angular distance between points 1 and 2.
 * @param[out] azi Azimuth from north of point 2 w.r.t. point 1.
 * @param[out] baz Azimuth from north of point 1 w.r.t. point 2.
 */
void sc_locsat_distaz2(double alat1, double alon1, double alat2, double alon2,
                       double *delta, double *azi, double *baz);


/**
 * @brief Find a point on a sphere which is a given distance and azimuth
 *        away from another point.
 * All arguments are in degrees.
 * Latitude, longitude and DELTA are geocentric.
 * Latitude is zero at equator and positive north.
 * Longitude is positive toward the east.
 * AZI is measured clockwise from local north.
 * @param[in] alat1 Latitude of point 1.
 * @param[in] alon1 Longitude of point 1
 * @param[in] delta Angular distance between points 1 and 2.
 * @param[in] azi Azimuth from north of point 2 w.r.t. point 1.
 * @param[out] alat2 Latitude of point 2.
 * @param[out] alon2 Longitude of point 2.
 */
void sc_locsat_latlon2(double alat1, double alon1, double delta, double azi, double *alat2, double *alon2);

/**
 * @brief Determine crossing points of two small circles.
 * Calculate the latitude and longitude crossing points of two
 * geographical small circles.
 *
 * Compute the latitude and longitude crossing points
 * from two geographical small circles using non-Naperian
 * trigonometric relations.
 *
 * @param[in] olat1 Latitudinal center of smaller circle (deg)
 * @param[in] olon1 Longitudinal center of smaller circle (deg)
 * @param[in] olat2 Latitudinal center of larger  circle (deg)
 * @param[in] olon2 Longitudinal center of larger  circle (deg)
 * @param[in] rsmall Radius of smaller circle (deg)
 * @param[in] rlarge Radius of larger circle (deg)
 * @param[out] xlat1 First latitudinal  crossing of 2 small circles (deg)
 * @param[out] xlon1 First longitudinal crossing of 2 small circles (deg)
 * @param[out] xlat2 Second latitudinal crossing of 2 small circles (deg)
 * @param[out] xlon2 Second longitudinal crossing of 2 small circles (deg)
 * @param[out] icerr 0 == OK, 1 == No crossing points exist
 */
void sc_locsat_crossings(double *olat1, double *olon1, double *olat2, double *olon2,
                         double *rsmall, double *rlarge,
                         double *xlat1, double *xlon1,
                         double *xlat2, double *xlon2, int *icerr);

/**
 * @brief Given the locations of two reference points and two azimuths of
 *        great circles passing through those points, the
 *        program computes the location of the crossing of two great circles
 *        and the distances from two reference points.
 * @param[in] alat1 Latitude of reference point 1.
 * @param[in] alon1 Longitude of reference point 1.
 * @param[in] aza Azimuth of great circles passing through point 1.
 * @param[in] alat2 Latitude of reference point 2.
 * @param[in] alon2 Longitude of reference point 2.
 * @param[in] azb Azimuth of great circles passing through point 2.
 * @param[out] dista Distance from points 1 to point great circles cross.
 * @param[out] distb Distance from points 2 to point great circles cross.
 * @param[out] alat Latitude of crossing point.
 * @param[out] alon Longitude of crossing point.
 * @param[out] ierr 0 all O.K., = 1 lines do not cross within reasonable distance.
 */
void sc_locsat_azcros2(
	double alat1, double alon1, double aza,
	double alat2, double alon2, double azb,
	double *dista, double *distb,
	double *alat, double *alon,
	int *ierr
);


#endif
