#include <locsat/interp.h>
#include <locsat/loc.h>


void sc_locsat_slocal(
	const float zfoc, const float radius, const float delta, const float azi,
	const int maxtbd, const int maxtbz,
	const int ntbd, const int ntbz, const float *tbd, const float *tbz, const float *tbtt,
	float *dcalx, double *atx, int *iterr
) {
	int tbtt_dim1, tbtt_offset;
	int imin, jmin, imax, jmax;
	float tbds[4], dtdz;
	double azir;
	int iext, jext;
	float tbzs[4], slow;
	int i, j;
	int ihole, ileft, jleft, idist;
	float dsldz, ttime;
	int itotd;
	float tbsls[16]; // was [4][4]
	int itotz, jz, nz, idepth;
	double cosazi;
	float dcross;
	double sinazi;
	float dslddel;
	int ibad;

	// Parameter adjustments
	--tbd;
	tbtt_dim1 = maxtbd;
	tbtt_offset = 1 + tbtt_dim1 * 1;
	tbtt -= tbtt_offset;
	--tbz;
	--atx;

	ihole = 0;
	idist = 0;
	idepth = 0;

	// Form arrays holding distances and depths around point of interest
	sc_locsat_brack(ntbd, &tbd[1], delta, &ileft);

	// Computing MAX
	imin = max(1, ileft - 1);
	// Computing MIN
	imax = min(ntbd, ileft + 2);

	itotd = 0;
	for ( i = imin; i <= imax; ++i, ++itotd ) {
		tbds[itotd] = tbd[i];
	}
	sc_locsat_brack(ntbz, &tbz[1], zfoc, &jleft);

	// Computing MAX
	jmin = max(1, jleft - 1);
	// Computing MIN
	jmax = min(ntbz, jleft + 2);

	itotz = 0;
	for ( j = jmin; j <= jmax; ++j, ++itotz ) {
		tbzs[itotz] = tbz[j];
	}

	// Compute travel time and horizontal slownesses for each point in
	// arrays around the point of interest
	// Find relevant range of table depths
	for ( i = 0; i < itotd; ++i ) {
		for ( j = 0; j < itotz; ++j ) {
			sc_locsat_brack(ntbz, &tbz[1], zfoc, &ileft);

			// Computing MAX
			jz = max(1, ileft - 1);
			// Computing MIN
			nz = min(ntbz, ileft + 2) - jz + 1;

			// Return travel time and create a mini-table of partials
			sc_locsat_holint2(
				0, ntbd, nz, &tbd[1], &tbz[jz],
				&tbtt[jz * tbtt_dim1 + 1], maxtbd, -1.f,
				tbds[i], tbzs[j],
				&ttime, &slow, &dtdz, &dcross, &iext, &jext, &ibad
			);

			if ( ibad != 0 ) {
				ihole = ibad;
			}
			if ( iext != 0 ) {
				idist = iext;
			}
			if ( jext != 0 ) {
				idepth = jext;
			}

			tbsls[j * 4 + i] = slow;
		}
	}
	// Compute slowness and partials at point of interest from mini-table
	/*
	printf("$ %f %f %d %d %d ", delta, zfoc, do_extrap, itotd, itotz);
	printf("[%f, %f, %f, %f] [%f, %f, %f, %f] [",
	       tbds[0], tbds[1], tbds[2], tbds[3],
	       tbzs[0], tbzs[1], tbzs[2], tbzs[3]);
	for ( int i = 0; i < 16; ++i ) {
		if ( i ) {
			printf(", ");
		}
		printf("%f", tbsls[i]);
	}
	printf("]\n");
	*/
	sc_locsat_holint2(
		0, itotd, itotz, tbds, tbzs, tbsls, 4, -1.f,
		delta, zfoc,
		&slow, &dslddel, &dsldz, &dcross, &iext, &jext, &ibad
	);
	if ( ihole != 0 ) {
		ibad = ihole;
	}
	if ( idist != 0 ) {
		iext = idist;
	}
	if ( idepth != 0 ) {
		jext = idepth;
	}

	// Interpolate point in hole of curve -- Value no good
	if ( ibad != 0 ) {
		// Interpolate point less than first distance point in curve
		*iterr = 11;
	}
	else if ( (iext < 0) && (jext == 0) ) {
		// Interpolate point greater than last distance point in curve
		*iterr = 12;
	}
	else if ( (iext > 0) && (jext == 0) ) {
		// Interpolate point less than first depth point in curve
		*iterr = 13;
	}
	else if ( (iext == 0) && (jext < 0) ) {
		// Interpolate point greater than last depth point in curve
		*iterr = 14;
	}
	else if ( (iext == 0) && (jext > 0) ) {
		// Interpolate point less than first distance point in curve and less
		// than first depth point in curve
		*iterr = 15;
	}
	else if ( (iext < 0) && (jext < 0) ) {
		// Interpolate point greater than last distance point in curve and
		// less than first depth point in curve
		*iterr = 16;
	}
	else if ( (iext > 0) && (jext < 0) ) {
		// Interpolate point less than first distance point in curve and
		// greater than first depth point in curve
		*iterr = 17;
	}
	else if ( (iext < 0) && (jext > 0) ) {
		// Interpolate point greater than last distance point in curve and
		// greater than first depth point in curve
		*iterr = 18;
	}
	else if ( (iext > 0) && (jext > 0) ) {
		// Reset error code to 0 if valid table interpolation
		*iterr = 19;
	}
	else {
		*iterr = 0;
	}

	// Compute partial derivatives if point is not in a hole
	*dcalx = slow;
	dslddel /= (radius - zfoc) * (float).017453293;
	if ( ibad == 0 ) {
		azir = azi * (float).017453293;
		sinazi = sin(azir);
		cosazi = cos(azir);
		// Axis 1
		atx[1] = (float)0.;
		// Axis 2 points east
		atx[2] = -dslddel * sinazi;
		// Axis 3 points north
		atx[3] = -dslddel * cosazi;
		// Axis 4 points up
		atx[4] = -dsldz;
	}
}
