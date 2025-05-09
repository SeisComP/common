#include "loc.h"


void sc_locsat_denuis(LOCSAT_Data *data, const int nd, const int np, double *dmean, int *inerr) {
	int m, n;
	double amean[4], asum, dacc;

	*inerr = 0;
	*dmean = 0.0;
	asum = 0.0;

	for ( m = 0; m < np; m++ ) {
		amean[m] = 0.0;
	}

	// Compute weighted sums
	for ( n = 0; n < nd; n++ ) {
		dacc = 1.0 / (data[n].dsd2 * data[n].dsd2);
		if ( data[n].idtyp == 1 ) {
			for ( m = 1; m < np; ++m ) {
				amean[m] = amean[m] + dacc * data[n].at[m];
			}
			asum = asum + dacc;
			*dmean = *dmean + dacc * data[n].resid2;
		}
	}

	// Convert amean and dmean to weighted means if valid arrival-time
	// data is available.  Also de-mean data and rows of at[].
	// amean[m] is the weighted mean of m'th column of travel-time
	// system matrix (before denuisancing).
	if ( asum > 0.0 ) {
		for ( m = 1; m < np; m++ ) {
			amean[m] = amean[m] / asum;
		}

		*dmean = *dmean / asum;

		for ( n = 0; n < nd; n++ ) {
			if ( data[n].idtyp == 1 ) {
				for ( m = 1; m < np; m++ ) {
					data[n].at[m] = data[n].at[m] - amean[m];
				}
				data[n].resid2 = data[n].resid2 - *dmean;
			}
		}
	}
	else {
		*inerr = 1;
	}
}
