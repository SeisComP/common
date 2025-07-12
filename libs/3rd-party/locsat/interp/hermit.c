#include <locsat/interp.h>


void sc_locsat_hermit(float x1, float x2, float y1, float y2, float yp1, float yp2,
                      float x0, float *y0, float *yp0) {
	float a, b, c, d, t, f1, f2, df, dx, fp1, fp2, sfp;

	dx = x2 - x1;
	t = (x0 - x1) / dx;

	if (t <= .5f) {
		f1 = y1;
		f2 = y2;
		fp1 = yp1;
		fp2 = yp2;
	}
	else {
		t = 1.f - t;
		dx = -dx;
		f1 = y2;
		f2 = y1;
		fp1 = yp2;
		fp2 = yp1;
	}

	fp1 *= dx;
	fp2 *= dx;
	df = f2 - f1;
	sfp = fp1 + fp2;
	a = f1;
	b = fp1;
	c = df * 3.f - sfp - fp1;
	d = df * -2.f + sfp;
	*y0 = ((d * t + c) * t + b) * t + a;
	*yp0 = ((d * 3.f * t + c * 2.f) * t + b) / dx;
}
