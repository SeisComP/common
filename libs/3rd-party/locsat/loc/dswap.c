#include <locsat/loc.h>


void sc_locsat_dswap(const int n, double *dx, const int incx, double *dy, const int incy) {
	int i__1;
	int i__, m;
	double dtemp;
	int ix, iy, mp1;

	--dy;
	--dx;

	if ( n <= 0 ) {
		return;
	}
	if ( incx == 1 && incy == 1 ) {
		goto L1010;
	}

	ix = 1;
	iy = 1;
	if ( incx < 0 ) {
		ix = (-n + 1) * incx + 1;
	}
	if ( incy < 0 ) {
		iy = (-n + 1) * incy + 1;
	}
	i__1 = n;
	for ( i__ = 1; i__ <= i__1; ++i__ ) {
		dtemp = dx[ix];
		dx[ix] = dy[iy];
		dy[iy] = dtemp;
		ix += incx;
		iy += incy;
	}
	return;

L1010:
	m = n % 3;
	if ( m == 0 ) {
		goto L1030;
	}
	i__1 = m;
	for ( i__ = 1; i__ <= i__1; ++i__ ) {
		dtemp = dx[i__];
		dx[i__] = dy[i__];
		dy[i__] = dtemp;
	}
	if ( n < 3 ) {
		return;
	}
L1030:
	mp1 = m + 1;
	i__1 = n;
	for ( i__ = mp1; i__ <= i__1; i__ += 3 ) {
		dtemp = dx[i__];
		dx[i__] = dy[i__];
		dy[i__] = dtemp;
		dtemp = dx[i__ + 1];
		dx[i__ + 1] = dy[i__ + 1];
		dy[i__ + 1] = dtemp;
		dtemp = dx[i__ + 2];
		dx[i__ + 2] = dy[i__ + 2];
		dy[i__ + 2] = dtemp;
	}
}
