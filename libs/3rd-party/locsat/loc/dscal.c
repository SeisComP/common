#include <locsat/loc.h>


void sc_locsat_dscal(const int n, double da, double *dx, const int incx) {
	int i, m, nincx, mp1;

	/* Parameter adjustments */
	--dx;

	/* Function Body */
	if ( n <= 0 ) {
		return;
	}
	if ( incx == 1 ) {
		goto L1010;
	}
	/* Code for increment not equal to 1 */
	nincx = n * incx;
	for ( i = 1; incx < 0 ? i >= nincx : i <= nincx; i += incx ) {
		dx[i] = da * dx[i];
	}
	return;
/* Code for increment equal to 1 */
L1010:
	m = n % 5;
	if ( m == 0 ) {
		goto L1030;
	}
	for ( i = 1; i <= m; ++i ) {
		dx[i] = da * dx[i];
	}
	if ( n < 5 ) {
		return;
	}
L1030:
	mp1 = m + 1;
	for ( i = mp1; i <= n; i += 5 ) {
		dx[i] = da * dx[i];
		dx[i + 1] = da * dx[i + 1];
		dx[i + 2] = da * dx[i + 2];
		dx[i + 3] = da * dx[i + 3];
		dx[i + 4] = da * dx[i + 4];
	}
}
