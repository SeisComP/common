#include <locsat/loc.h>
#include <math.h>


double sc_locsat_dnrm2(const int n, double *dx, const int incx) {
	double zero = 0.;
	double one = 1.;
	double cutlo = 8.232e-11;
	double cuthi = 1.304e19;
	double ret_val, d1;

	double xmax = 0.0;
	int next, i, j, nn;
	double hitest, sum;

	--dx;

	if ( n > 0 ) {
		goto L1000;
	}
	ret_val = zero;
	goto L1130;
L1000:
	next = 0;
	sum = zero;
	nn = n * incx;
	i = 1;
L1010:
	switch ( (int)next ) {
		case 0:
			goto L1020;
		case 1:
			goto L1030;
		case 2:
			goto L1060;
		case 3:
			goto L1070;
	}
L1020:
	if ( (d1 = dx[i], abs(d1)) > cutlo ) {
		goto L1100;
	}
	next = 1;
	xmax = zero;
/*     Phase 1.  sum is zero */
L1030:
	if ( dx[i] == zero ) {
		goto L1120;
	}
	if ( (d1 = dx[i], abs(d1)) > cutlo ) {
		goto L1100;
	}
	/*     Prepare for phase 2 */
	next = 2;
	goto L1050;
/*     Prepare for phase 4 */
L1040:
	i = j;
	next = 3;
	sum = sum / dx[i] / dx[i];
L1050:
	xmax = (d1 = dx[i], abs(d1));
	goto L1080;
/*     Phase 2.  Sum is small */
/*        Scale to avoid destructive underflow */
L1060:
	if ( (d1 = dx[i], abs(d1)) > cutlo ) {
		goto L1090;
	}
/*     Common code for phases 2 and 4 */
/*     In phase 4, sum is large, so scale to avoid overflow */
L1070:
	if ( (d1 = dx[i], abs(d1)) <= xmax ) {
		goto L1080;
	}
	/* Computing 2nd power */
	d1 = xmax / dx[i];
	sum = one + sum * (d1 * d1);
	xmax = (d1 = dx[i], abs(d1));
	goto L1120;
L1080:
	/* Computing 2nd power */
	d1 = dx[i] / xmax;
	sum += d1 * d1;
	goto L1120;
/*     Prepare for phase 3 */
L1090:
	sum = sum * xmax * xmax;
/*     For real or D.P. set hitest = cuthi/n */
/*     For complex      set hitest = cuthi/(2*n) */
L1100:
	hitest = cuthi / (float)(n);
	/*     Phase 3:  sum is mid-range, no scaling */
	for ( j = i; incx < 0 ? j >= nn : j <= nn; j += incx ) {
		if ( (d1 = dx[j], abs(d1)) >= hitest ) {
			goto L1040;
		}
		/* L1110: */
		/* Computing 2nd power */
		d1 = dx[j];
		sum += d1 * d1;
	}
	ret_val = sqrt(sum);
	goto L1130;
L1120:
	i += incx;
	if ( i <= nn ) {
		goto L1010;
	}
	/*     Compute square root and adjust for scaling */
	ret_val = xmax * sqrt(sum);
L1130:
	return ret_val;
}
