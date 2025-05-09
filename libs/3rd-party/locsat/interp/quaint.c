#include <locsat/interp.h>


void sc_locsat_quaint(const int n, const float *x, const float *f, const float x0,
                      float *f0, float *fp0, int *iext) {
	int ileft;
	float fpdev, f1, f2, f3;
	int i1, i2, i3, i4;
	float f4, x1, x2, x3, x4, fpdev2, fpdev3, h12, h23, h34, s12, s23, s34;
	float fp2, fp3, fac;

	// Binary search for samples bounding x0

	sc_locsat_brack(n, x, x0, &ileft);

	// x0 < x(1)
	if ( ileft < 1 ) {
		if ( x[1] > x[0] ) {
			*fp0 = (f[1] - f[0]) / (x[1] - x[0]);
		}
		else {
			*fp0 = 0.f;
		}
		*f0 = f[0] + *fp0 * (x0 - x[0]);
		*iext = -1;
		return;
	}

	// x0 > x(n)
	if ( ileft >= n ) {
		if ( x[n - 1] > x[n - 2] ) {
			*fp0 = (f[n - 1] - f[n - 2]) / (x[n - 1] - x[n - 2]);
		}
		else {
			*fp0 = 0.f;
		}

		*f0 = f[n - 1] + *fp0 * (x0 - x[n - 1]);
		*iext = 1;
		return;
	}

	// Normal case
	// Define points 1..4, such that x1 <= x2 <= x0 <= x3 <= x4 ----
	// If necessary, make x1 = x2 or x3 = x4
	// Computing MAX
	i1 = max(0, ileft - 2);
	i2 = ileft - 1;
	i3 = ileft;
	// Computing MIN
	i4 = min(n - 1, ileft + 1);

	x1 = x[i1];
	x2 = x[i2];
	x3 = x[i3];
	x4 = x[i4];
	f1 = f[i1];
	f2 = f[i2];
	f3 = f[i3];
	f4 = f[i4];

	// Find widths of three intervals
	// Note 'brack' guarantees x(ileft) < x(ileft+1), and thus h23 > 0
	h12 = x2 - x1;
	h23 = x3 - x2;
	h34 = x4 - x3;

	// Set finite-difference derivative in center interval
	s23 = (f3 - f2) / h23;

	// Assign a function derivative to point 2; call it fp2.  The derivative
	// of the parabola fitting points 1, 2 and 3 c (evaluated at x2) is used,
	// howvever, if h12 is zero, s23 is used.
	if ( h12 > 0.f ) {
		s12 = (f2 - f1) / h12;
		fp2 = (s23 * h12 + s12 * h23) / (h12 + h23);
	}
	else {
		fp2 = s23;
	}

	// Assign a function derivative to point 3; call it fp3.  The derivative
	// of the parabola fitting points 2, 3 and 4 (evaluated at x3) is used,
	// howvever, if h34 is zero, s23 is used.
	if ( h34 > 0.f ) {
		s34 = (f4 - f3) / h34;
		fp3 = (s23 * h34 + s34 * h23) / (h34 + h23);
	}
	else {
		fp3 = s23;
	}

	// Adjust fp2 and fp3 such that they average to s23, but neither gets
	// farther from s23
	fpdev2 = s23 - fp2;
	fpdev3 = fp3 - s23;
	if ( fpdev2 * fpdev3 <= 0.f ) {
		fpdev = 0.f;
	}
	else if ( fpdev2 < 0.f ) {
		// Computing MIN
		fpdev = -dmin(-fpdev2, -fpdev3);
	}
	else {
		fpdev = dmin(fpdev2, fpdev3);
	}

	// Adjust derivatives such that Hermite cubic interpolant is monotonic
	if ( s23 != 0.f ) {
		fac = dabs(fpdev / s23);
		if ( fac > 1.f ) {
			fpdev /= fac;
		}
	}

	fp2 = s23 - fpdev;
	fp3 = s23 + fpdev;

	// Now do a straight Hermite cubic interpolation bewteen points 2 and 3
	sc_locsat_hermit(x2, x3, f2, f3, fp2, fp3, x0, f0, fp0);
	*iext = 0;
}
