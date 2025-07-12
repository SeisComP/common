#include <locsat/interp.h>


void sc_locsat_fixhol(const int n, const float *x, const float *f, const float fbad,
                      int *ms, float *xs, float *fs) {
	int i;

	*ms = 0;

	if (n <= 0) {
		return;
	}

	// Set up first point
	xs[0] = x[0];
	fs[0] = f[0];

	// Do the rest
	for (i = 1; i < n; ++i) {
		if (f[i] != fbad) {
			if (fs[*ms] != fbad) {
				if (x[i] == xs[*ms]) {
					if (f[i] == fs[*ms]) {
						continue;
					}
				}
			}
			else {
				if (*ms > 0) {
					++(*ms);
				}

				xs[*ms] = x[i];
				fs[*ms] = fbad;
			}

			++(*ms);
			xs[*ms] = x[i];
			fs[*ms] = f[i];
		}
		else {
			if (fs[*ms] != fbad) {
				if (*ms > 0) {
					if (fs[*ms - 1] == fbad) {
						/* Computing MAX */
						*ms = max(0, *ms - 1);
						continue;
					}
				}

				++(*ms);
				xs[*ms] = xs[*ms - 1];
				fs[*ms] = fbad;
			}
		}

		while ((*ms > 1) && (xs[*ms] == xs[*ms - 2])) {
			fs[*ms - 1] = fs[*ms];
			--(*ms);
		}
	}

	++(*ms);
}
