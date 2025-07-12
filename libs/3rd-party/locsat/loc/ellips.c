#include <locsat/loc.h>
#include <math.h>


#define RAD_TO_DEG 57.2957795
#define PI 3.141592654
#define TWOPI 2.0 * PI

void sc_locsat_ellips(
	int *np, double covar[][4],
	double *hymaj, double *hymid, double *hymin, double *hystr, double *hyplu, double *hyrak, double *epmaj, double *epmin,
	float *epstr, double *zfint,
	float *stt, float *stx, float *sty, float *sxx, float *sxy, float *syy, float *stz, float *sxz, float *syz, float *szz
) {
	double a2, b2, c, cc, epstrr, s, ss, sxytcs, twosxy;

	*stt = covar[0][0];
	*stx = covar[0][1];
	*sty = covar[0][2];
	*sxx = covar[1][1];
	*sxy = covar[1][2];
	*syy = covar[2][2];

	if ( *np != 4 ) {
		*stz = 0.0;
		*sxz = 0.0;
		*syz = 0.0;
		*szz = 0.0;
	}
	else {
		*stz = covar[0][3];
		*sxz = covar[1][3];
		*syz = covar[2][3];
		*szz = covar[3][3];
	}

	// Compute two-dimenstional ellipse parameters from marginal
	// epicentral variance
	twosxy = 2.0 * (*sxy);
	if ( twosxy != 0.0 ) {
		epstrr = 0.5 * atan2(twosxy, *syy - *sxx);
	}
	else {
		epstrr = 0.0;
	}
	c = cos(epstrr);
	s = sin(epstrr);
	cc = c * c;
	ss = s * s;
	sxytcs = twosxy * c * s;
	a2 = *sxx * ss + sxytcs + *syy * cc;
	b2 = *sxx * cc - sxytcs + *syy * ss;
	if ( epstrr < 0.0 ) {
		epstrr = epstrr + TWOPI;
	}
	if ( epstrr > TWOPI ) {
		epstrr = epstrr - TWOPI;
	}
	if ( epstrr > PI ) {
		epstrr = epstrr - PI;
	}
	if ( a2 < 0.0 ) {
		*epmaj = -1.0;
	}
	else {
		*epmaj = sqrt(a2);
	}
	if ( b2 < 0.0 ) {
		*epmin = -1.0;
	}
	else {
		*epmin = sqrt(b2);
	}
	*epstr = RAD_TO_DEG * epstrr;

	// Compute the one-dimensional depth confidence semi-interval
	// from marginal depth variance
	if ( *szz < 0.0 ) {
		*zfint = -1.0;
	}
	else {
		*zfint = sqrt(*szz);
	}
}
