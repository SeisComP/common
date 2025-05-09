#include <math.h>
#include <locsat/loc.h>


void sc_locsat_drotg(double *da, double *db, double *c, double *s) {
	double d1, d2;
	double r, scale, z, roe;

	roe = *db;
	if ( abs(*da) > abs(*db) ) {
		roe = *da;
	}
	scale = abs(*da) + abs(*db);
	if ( scale != 0. ) {
		/* Computing 2nd power */
		d1 = *da / scale;
		/* Computing 2nd power */
		d2 = *db / scale;
		r = scale * sqrt(d1 * d1 + d2 * d2);
		r = dsign(1., roe) * r;
		*c = *da / r;
		*s = *db / r;
	}
	else {
		*c = 1.;
		*s = 0.;
		r = 0.;
	}

	z = 1.;
	if ( abs(*da) > abs(*db) ) {
		z = *s;
	}
	if ( abs(*db) >= abs(*da) && *c != 0. ) {
		z = 1. / *c;
	}
	*da = r;
	*db = z;
}
