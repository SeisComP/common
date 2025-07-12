#include <locsat/loc.h>


void sc_locsat_daxpy(const int n, const double da, double *dx, const int incx, double *dy, const int incy) {
	int i, ix, iy;

	if ( n <= 0 ) {
		return;
	}
	if ( da == 0. ) {
		return;
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
		dy[iy] += da * dx[ix];
		ix += incx;
		iy += incy;
	}
}
