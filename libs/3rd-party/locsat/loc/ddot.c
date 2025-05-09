#include <locsat/loc.h>


double sc_locsat_ddot(const int n, double *dx, const int incx, double *dy, const int incy) {
	int i;
	double dot;
	int ix, iy;

	dot = 0.;

	if ( n <= 0 ) {
		return 0.;
	}

	ix = 0;
	iy = 0;

	if ( incx < 0 ) {
		ix = (1 - n) * incx;
	}
	if ( incy < 0 ) {
		iy = (1 - n) * incy;
	}
	for ( i = 0; i < n; ++i ) {
		dot += dx[ix] * dy[iy];
		ix += incx;
		iy += incy;
	}

	return dot;
}
