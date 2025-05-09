#include <locsat/geog.h>


#define SIGN(a1, a2) ((a2) >= 0 ? -(a1) : (a1))

void sc_locsat_azcros2(double alat1, double alon1, double aza,
                       double alat2, double alon2, double azb,
                       double *dista, double *distb, double *alat,
                       double *alon, int *ierr) {
	double alatin, alonin, az, azi, baz, c1, c2, c3, c4, c5;
	double delta, dist, e, f, fa, fb, g, h, ra, rb, rc;

	// Find azimuth, back azimuth and radial distance between stations
	sc_locsat_distaz2(alat1, alon1, alat2, alon2, &delta, &azi, &baz);

	// Find angle measured from line between two stations to aza and azb
	fa = aza - azi;
	fb = azb - baz;
	ra = fa;
	rb = fb;

	if ( fabs(ra) > 180.0 ) {
		ra = SIGN((360.0 - fabs(ra)), ra);
	}
	if ( fabs(rb) > 180.0 ) {
		rb = SIGN((360.0 - fabs(rb)), rb);
	}

	// If the signs of ra and rb are the same, the great circles along
	// those azimuths will not cross within a "reasonable" distance.
	if ( SIGN(1.0, ra) == SIGN(1.0, rb) ) {
		*ierr = 1;
		return;
	}

	ra = fabs(ra);
	rb = fabs(rb);

	// If the sum of ra and rb is > 180., there will be no crossing
	// within a reasonable distance
	if ( (ra + rb) > 180.0 ) {
		*ierr = 1;
		return;
	}

	ra = deg2rad(ra);
	rb = deg2rad(rb);
	rc = deg2rad(delta);

	c1 = tan(0.5*rc);
	c2 = 0.5 * (ra - rb);
	c3 = 0.5 * (ra + rb);

	// Equations for solving for the distances
	f = c1 * sin(c2);
	g = sin(c3);
	h = c1 * cos(c2);
	e = cos(c3);

	c4 = atan(f/g);
	c5 = atan(h/e);

	// Compute distances (lengths of the triangle)
	*distb = rad2deg(c4 + c5);
	*dista = rad2deg(c5 - c4);

	if ( (*dista < 0.0) || (*distb < 0.0) ) {
		*ierr = 1;
		return;
	}

	if ( *dista < *distb ) {
		dist   = *dista;
		az     = aza;
		alatin = alat1;
		alonin = alon1;
	}
	else {
		dist   = *distb;
		az     = azb;
		alatin = alat2;
		alonin = alon2;
	}

	sc_locsat_latlon2(alatin, alonin, dist, az, alat, alon);
	*ierr = 0;
}
