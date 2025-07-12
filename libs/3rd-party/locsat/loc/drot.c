#include <locsat/loc.h>


void sc_locsat_drot(const int n, double *dx, const int incx, double *dy, const int incy, const double c, const double s) {
	int i;
	double dtemp;
	int ix, iy;

	--dy;
	--dx;

	if ( n <= 0 ) {
		return;
	}
	if ( incx == 1 && incy == 1 ) {
		for ( i = 1; i <= n; ++i ) {
			dtemp = c * dx[i] + s * dy[i];
			dy[i] = c * dy[i] - s * dx[i];
			dx[i] = dtemp;
		}
	}
	else {
		ix = 1;
		iy = 1;
		if ( incx < 0 ) {
			ix = (-n + 1) * incx + 1;
		}
		if ( incy < 0 ) {
			iy = (-n + 1) * incy + 1;
		}
		for ( i = 0; i < n; ++i ) {
			dtemp = c * dx[ix] + s * dy[iy];
			dy[iy] = c * dy[iy] - s * dx[ix];
			dx[ix] = dtemp;
			ix += incx;
			iy += incy;
		}
	}
}
