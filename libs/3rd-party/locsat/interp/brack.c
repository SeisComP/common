#include <locsat/interp.h>


void sc_locsat_brack(const int n, const float *x, const float x0, int *ileft) {
	int imid, i, iright;

	*ileft = 0;
	iright = n + 1;

	while ( 1 ) {
		imid = (*ileft + iright) / 2;
		if (imid == *ileft) {
			return;
		}
		else if (x0 < x[imid - 1]) {
			iright = imid;
		}
		else if (x0 > x[imid - 1]) {
			*ileft = imid;
		}
		else
			break;
	}

	// Special case: The point x(imid) found to equal x0.  Find bracket
	// [x(ileft),x(ileft+1)], such that x(ileft+1) > x(ileft).
	for (i = imid; i < n; ++i) {
		if (x[i] > x0) {
			*ileft = i;
			return;
		}
	}

	for (i = imid - 1; i >= 1; --i) {
		if (x[i - 1] < x0) {
			*ileft = i;
			return;
		}
	}

	*ileft = 0;
}
