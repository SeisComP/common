#include <locsat/interp.h>
#include <assert.h>


void sc_locsat_holint2(int do_extrapolate,
                       int nx, int ny, const float *x, const float *y,
                       const float *func, const int ldf,
                       const float fbad, const float x0, const float y0,
                       float *f0, float *fx0, float *fy0, float *fxy0,
                       int *ix, int *iy, int *ihole) {
	#define MAX_SUBGRID_X 4
	#define MAX_SUBGRID_Z 4

	int f_dim1, f_offset;
	int ichk;
	float hold;
	int imin, jmin, imax, jmax, iext, jext, muse, nuse;
	float dist_min, dist_max;
	int i, j, k;
	int ileft, jleft, js;
	float tt_min, tt_max;
	float f0s[MAX_SUBGRID_Z];
	int extrap_in_hole;
	float vel;
	int min_idx, max_idx;
	float subgrid[MAX_SUBGRID_X * MAX_SUBGRID_Z], fx0s[MAX_SUBGRID_Z];
	int num_samples, extrap_distance, ibad;

	f_dim1 = ldf;
	f_offset = 1 + f_dim1 * 1;
	func -= f_offset;

	iext = 0;
	jext = 0;
	ibad = 0;
	*ihole = 0;
	*ix = 0;
	*iy = 0;
	num_samples = ldf;

	// Should we extrapolate ?
	if ( (x0 > x[nx - 1] || x0 < x[0]) && do_extrapolate != 1 ) {
		*f0 = -1.f;
		return;
	}

	// Bracket x0 -- Find 4 relevant x() samples, or as many as needed
	sc_locsat_brack(nx, x, x0, &ileft);

	// Computing MAX
	imin = ileft - 1;
	if ( imin < 1 ) {
		imin = 1;
	}

	// Computing MIN
	imax = ileft + 2;
	if ( imax > nx ) {
		imax = nx;
	}
	muse = imax - imin + 1;

	assert(muse <= 4);

	// Do the same for y()
	sc_locsat_brack(ny, y, y0, &jleft);

	// Computing MAX
	jmin = jleft - 1;
	if ( jmin < 1 ) {
		jmin = 1;
	}

	// Computing MIN
	jmax = jleft + 2;
	if ( jmax > ny ) {
		jmax = ny;
	}

	nuse = jmax - jmin + 1;

	assert(nuse <= 4);

	// Fill in subgrid with valid times where available and fill-in empty
	// parts of the desired curve with linearly extrapolated values.  For
	// travel-time tables x(i) contains the distance elements, while y(j)
	// holds the depth samples.
	for ( j = jmin; j <= jmax; ++j ) {
		for ( i = imin; i <= imax; ++i ) {
			if ( func[i + j * f_dim1] == -1.f ) {
				if ( do_extrapolate != 1 ) {
					*f0 = -1.f;
					return;
				}

				extrap_in_hole = *ihole;
				extrap_distance = *ix;

				for ( min_idx = 1; min_idx <= num_samples; ++min_idx ) {
					if ( func[min_idx + j * f_dim1] != -1.f )
						break;
				}

				dist_min = x[min_idx - 1];
				ichk = 0;

				for ( max_idx = min_idx; max_idx <= num_samples; ++max_idx ) {
					if ( func[max_idx + j * f_dim1] == -1.f ) {
						if ( ichk == 0 ) {
							ichk = max_idx - 1;
						}
					}
					else if ( max_idx == num_samples ) {
						ichk = max_idx;
					}
					else {
						ichk = 0;
					}
				}

				max_idx = ichk;
				dist_max = x[max_idx - 1];

				// Off the high end ?
				if ( x[i - 1] > dist_max ) {
					for ( k = max_idx; k >= 1; --k ) {
						if ( dist_max - x[k - 1] >= 5.f )
							break;
					}

					tt_max = func[max_idx + j * f_dim1];
					vel = (dist_max - x[k - 1]) / (tt_max - func[k + j * f_dim1]);

					if ( dist_max <= 110.f && x[i - 1] > 110.f ) {
						hold = (110.f - dist_max) / vel + tt_max + 238.f;
						vel *= 2.4f;
						subgrid[(j - jmin) * MAX_SUBGRID_X + i - imin] = (x[i -1] - 110.f) / vel + hold;
					}
					else {
						subgrid[(j - jmin) * MAX_SUBGRID_X + i - imin] = (x[i - 1] - dist_max) / vel + tt_max;
					}

					extrap_distance = 1;
					// Off the low end ?
				}
				else if ( x[i - 1] < dist_min ) {
					for (k = min_idx; k <= num_samples; ++k) {
						if (x[k - 1] - dist_min >= 5.f)
							break;
					}

					tt_min = func[min_idx + j * f_dim1];
					vel = (x[k - 1] - dist_min) / (func[k + j * f_dim1] - tt_min);
					subgrid[(j - jmin) * MAX_SUBGRID_X + i - imin] = tt_min - (dist_min - x[i - 1]) / vel;
					extrap_distance = -1;
					// In a hole ?
				}
				else {
					for (k = max_idx; k >= 1; --k) {
						if (x[k - 1] < x[i - 1]) {
							if (func[k + j * f_dim1] != -1.f) {
								dist_max = x[k -1];
								max_idx = k;
								break;
							}
						}
					}

					for (k = max_idx; k >= 1; --k) {
						if (dist_max - x[k - 1] >= 5.f)
							break;
					}

					tt_max = func[max_idx + j * f_dim1];
					vel = (dist_max - x[k - 1]) / (tt_max - func[k + j * f_dim1]);
					subgrid[(j - jmin) * MAX_SUBGRID_X + i - imin] = (x[i - 1] - dist_max) / vel + tt_max;
					extrap_in_hole = 1;
				}

				*ihole = extrap_in_hole;
				*ix = extrap_distance;
			}
			else
				subgrid[(j - jmin) * MAX_SUBGRID_X + i - imin] = func[i + j * f_dim1];
		}
	}

	if ( y0 > y[jmax - 1] )
		*iy = 1;

	// Now interpolate to (x0, y(j)), j = jmin, jmax)
	for ( j = jmin; j <= jmax; ++j ) {
		js = j - jmin + 1;
		sc_locsat_holint(
			muse, &x[imin - 1], &subgrid[(j - jmin) * MAX_SUBGRID_X], fbad, x0,
			&f0s[js - 1], &fx0s[js - 1], &iext, &ibad
		);
	}

	// Now interpolate to (x0,y0)
	sc_locsat_holint(nuse, &y[jmin - 1], f0s, fbad, y0, f0, fy0, &jext, &ibad);
	sc_locsat_quaint(nuse, &y[jmin - 1], fx0s, y0, fx0, fxy0, &jext);

	iext = *ix;
	jext = *iy;
	ibad = *ihole;
}

