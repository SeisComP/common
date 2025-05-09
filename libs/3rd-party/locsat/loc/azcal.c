#include <math.h>
#include <locsat/loc.h>


#define DEG_TO_RAD 0.017453293

void sc_locsat_azcal(float radius, float delta, float azi, float baz, float *dcalx, double atx[4]) {
	double azir, rt;

	azir = azi * DEG_TO_RAD;
	rt = sin(delta * DEG_TO_RAD) * (radius) * DEG_TO_RAD;
	if ( rt == 0.0 ) {
		rt = 0.0001;
	}

	/* Calculated azimuth in degrees */
	*dcalx = baz;

	/* Partial derivatives w.r.t. origin time (time; sec) = 1.0 */
	atx[0] = 0.0;

	/* Partial derivatives w.r.t. East (deg/km) */
	atx[1] = -cos(azir) / rt;

	/* Partial derivatives w.r.t. North (deg/km) */
	atx[2] = sin(azir) / rt;

	/* Partial derivatives w.r.t. Up (depth; km) = 0.0 */
	atx[3] = 0.0;
}
