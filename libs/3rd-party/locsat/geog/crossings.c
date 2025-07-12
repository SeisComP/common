#include <locsat/geog.h>


#define	SIGN(a1, a2) ((a2) >= 0 ? -(a1) : (a1))

void sc_locsat_crossings(double *olat1, double *olon1,
                        double *olat2, double *olon2,
                        double *rsmall, double *rlarge,
                        double *xlat1, double *xlon1,
                        double *xlat2, double *xlon2, int *icerr) {
	double alpha, arg, azi, baz, s, stadist, tmp;

	*icerr = 0;

	// If small circle greater than larger circle, then correct
	if ( *rsmall > *rlarge ) {
		tmp    = *rlarge;
		*rlarge = *rsmall;
		*rsmall = tmp;
		tmp    = *olat1;
		*olat1  = *olat2;
		*olat2  = tmp;
		tmp    = *olon1;
		*olon1  = *olon2;
		*olon2  = tmp;
	}

	sc_locsat_distaz2(*olat1, *olon1, *olat2, *olon2, &stadist, &azi, &baz);

	// Test for the case of no intersection, if so, ignore

	if ( fabs(*rsmall - stadist) > *rlarge || *rsmall + stadist < *rlarge ) {
		*icerr = 1;
		return;
	}

	// To determine the angle between the two stations and their
	// intersections use non-Naperian trigonometry.
	//   The defining equation is:
	// alpha =	2 * atan (sqrt(( sin(s-stadist)*sin(s-rsmall) /
	// sin(s)*sin(s-rlarge) )) )
	// where, rsmall = Radius of 1st (smaller) circle
	//        rlarge = Radius of 2nd (largest) circle
	// and s = (rsmall + rlarge + stadist) / 2
	s   = (*rsmall + *rlarge + stadist) / 2.0;
	s   = deg2rad(s);
	arg = (sin(s - deg2rad(stadist)) * sin(s - deg2rad(*rlarge)) ) /
	      (sin(s) * sin(s - deg2rad(*rsmall)));
	if (arg < 0.0) {
		*icerr = 1;
		return;
	}

	alpha = 2.0 * atan(sqrt(arg) );
	alpha = rad2deg(alpha);

	// Now find the two intersection points from the center of the
	// larger of the two small circles
	azi = baz + alpha;
	if (fabs(azi) > 180.0) {
		azi = SIGN((360.0-fabs(azi)), azi);
	}
	sc_locsat_latlon2(*olat2, *olon2, *rlarge, azi, xlat1, xlon1);

	azi = azi - 2.0*alpha;
	if (fabs(azi) > 180.0) {
		azi = SIGN((360.0-fabs(azi)), azi);
	}
	sc_locsat_latlon2(*olat2, *olon2, *rlarge, azi, xlat2, xlon2);
}
