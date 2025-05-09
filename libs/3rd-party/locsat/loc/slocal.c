#include <locsat/interp.h>
#include <locsat/loc.h>


void sc_locsat_slocal(
	const float zfoc, const float radius, const float delta, const float azi,
	const int maxtbd, const int maxtbz,
	const int *ntbd, const int *ntbz, const float *tbd, const float *tbz, const float *tbtt,
	float *dcalx, double *atx, int *iterr
) {
	/* System generated locals */
	int tbtt_dim1, tbtt_offset, i__1, i__2, i__3, i__4;

	/* Local variables */
	int imin, jmin, imax, jmax;
	float tbds[4], dtdz;
	double azir;
	int iext, jext;
	float tbzs[4], slow;
	int i, j;
	int ihole, ileft, jleft, idist;
	float dsldz, ttime;
	int itotd;
	float tbsls[16] /* was [4][4] */;
	int do_extrap, itotz, jz, nz, idepth;
	double cosazi;
	float dcross;
	double sinazi;
	float dslddel;
	int ibad;

	/* Parameter adjustments */
	--tbd;
	tbtt_dim1 = maxtbd;
	tbtt_offset = 1 + tbtt_dim1 * 1;
	tbtt -= tbtt_offset;
	--tbz;
	--atx;

	/* Function Body */
	ihole = 0;
	idist = 0;
	idepth = 0;
	/*     Permit extrapolation */
	do_extrap = 0;
	/*     Form arrays holding distances and depths around point of interest */
	sc_locsat_brack(*ntbd, &tbd[1], delta, &ileft);
	/* Computing MAX */
	i__1 = 1, i__2 = ileft - 1;
	imin = max(i__1, i__2);
	/* Computing MIN */
	i__1 = *ntbd, i__2 = ileft + 2;
	imax = min(i__1, i__2);
	itotd = 0;
	i__1 = imax;
	for ( i = imin; i <= i__1; ++i ) {
		++itotd;
		tbds[itotd - 1] = tbd[i];
		/* L1000: */
	}
	sc_locsat_brack(*ntbz, &tbz[1], zfoc, &jleft);
	/* Computing MAX */
	i__1 = 1, i__2 = jleft - 1;
	jmin = max(i__1, i__2);
	/* Computing MIN */
	i__1 = *ntbz, i__2 = jleft + 2;
	jmax = min(i__1, i__2);
	itotz = 0;
	i__1 = jmax;
	for ( j = jmin; j <= i__1; ++j ) {
		++itotz;
		tbzs[itotz - 1] = tbz[j];
		/* L1010: */
	}
	/*     Compute travel time and horizontal slownesses for each point in */
	/*     arrays around the point of interest */
	/*     Find relevant range of table depths */
	i__1 = itotd;
	for ( i = 1; i <= i__1; ++i ) {
		i__2 = itotz;
		for ( j = 1; j <= i__2; ++j ) {
			sc_locsat_brack(*ntbz, &tbz[1], zfoc, &ileft);
			/* Computing MAX */
			i__3 = 1, i__4 = ileft - 1;
			jz = max(i__3, i__4);
			/* Computing MIN */
			i__3 = *ntbz, i__4 = ileft + 2;
			nz = min(i__3, i__4) - jz + 1;
			/*           Return travel time and create a mini-table of partials
			 */
			sc_locsat_holint2(
				do_extrap, *ntbd, nz, &tbd[1], &tbz[jz],
				&tbtt[jz * tbtt_dim1 + 1], maxtbd, -1.f,
				tbds[i - 1], tbzs[j - 1],
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
			/*           if (ibad.ne.0) then */
			/*              tbsls(i,j) = -1.0 */
			/*           else */
			tbsls[i + (j << 2) - 5] = slow;
			/*           end if */
			/* L1020: */
		}
		/* L1030: */
	}
	/*     Compute slowness and partials at point of interest from mini-table */
	sc_locsat_holint2(
		do_extrap, itotd, itotz, tbds, tbzs, tbsls, 4, -1.f,
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
	/*     Interpolate point in hole of curve -- Value no good */
	if ( ibad != 0 ) {
		*iterr = 11;
		/*     Interpolate point less than first distance point in curve */
	}
	else if ( iext < 0 && jext == 0 ) {
		*iterr = 12;
		/*     Interpolate point greater than last distance point in curve */
	}
	else if ( iext > 0 && jext == 0 ) {
		*iterr = 13;
		/*     Interpolate point less than first depth point in curve */
	}
	else if ( iext == 0 && jext < 0 ) {
		*iterr = 14;
		/*     Interpolate point greater than last depth point in curve */
	}
	else if ( iext == 0 && jext > 0 ) {
		*iterr = 15;
		/*     Interpolate point less than first distance point in curve and
		 * less */
		/*     than first depth point in curve */
	}
	else if ( iext < 0 && jext < 0 ) {
		*iterr = 16;
		/*     Interpolate point greater than last distance point in curve and
		 * less */
		/*     than first depth point in curve */
	}
	else if ( iext > 0 && jext < 0 ) {
		*iterr = 17;
		/*     Interpolate point less than first distance point in curve and */
		/*     greater than first depth point in curve */
	}
	else if ( iext < 0 && jext > 0 ) {
		*iterr = 18;
		/*     Interpolate point greater than last distance point in curve and
		 */
		/*     greater than first depth point in curve */
	}
	else if ( iext > 0 && jext > 0 ) {
		*iterr = 19;
		/*     Reset error code to 0 if valid table interpolation */
	}
	else {
		*iterr = 0;
	}
	/*     Compute partial derivatives if point is not in a hole */
	*dcalx = slow;
	dslddel /= (radius - zfoc) * (float).017453293;
	if ( ibad == 0 ) {
		azir = azi * (float).017453293;
		sinazi = sin(azir);
		cosazi = cos(azir);
		/*        Axis 1 */
		atx[1] = (float)0.;
		/*        Axis 2 points east */
		atx[2] = -dslddel * sinazi;
		/*        Axis 3 points north */
		atx[3] = -dslddel * cosazi;
		/*        Axis 4 points up */
		atx[4] = -dsldz;
	}
} /* slocal0_ */
