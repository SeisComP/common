#include <locsat/interp.h>


void sc_locsat_holint(int n, const float *x, const float *f, const float fbad, const float x0,
                      float *f0, float *fp0, int *iext, int *ibad) {
	int imin, imax, nuse;
	int ileft;
	float fh[6];
	int nh;
	float xh[6];

	sc_locsat_brack(n, x, x0, &ileft);
	// Computing MAX
	imin = max(1, ileft - 1);
	// Computing MIN
	imax = min(n, ileft + 2);
	nuse = imax - imin + 1;

	sc_locsat_fixhol(nuse, &x[imin - 1], &f[imin - 1], fbad, &nh, xh, fh);

	// Interpolate fixed function
	if ( nh <= 1 ) {
		*f0 = fh[0];
		*fp0 = 0.f;
	}
	else {
		sc_locsat_quaint(nh, xh, fh, x0, f0, fp0, iext);
	}

	// Now check if interpolation point is in a hole
	*ibad = *f0 == fbad && *fp0 == 0.f ? 1 : 0;
}
