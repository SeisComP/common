#include <locsat/interp.h>
#include <locsat/loc.h>


/* NAME */
/* 	ttcal0 -- Compute travel times and their partial derivatives. */
/* FILE */
/* 	ttcal0.f */
/* SYNOPSIS */
/* 	Compute travel times and their partial derivatives for a fixed */
/* 	hypocenter. */
/* DESCRIPTION */
/* 	Subroutine.  Travel times and partials are determined via inter/ */
/* 	extrapolation of pre-calculated travel-time curves.  A point in a */
/* 	hole is rejected. */
/* 	---- On entry ---- */
/* 	radius:		Radius of Earth (km) */
/* 	delta:		Distance from the event to the station (deg) */
/* 	azi:		Forward-azimuth from event to station (deg) */
/* 	zfoc:		Event focal depth (km below sea level) */
/* 	maxtbd:		Maximum dimension of i'th position in tbd(), tbtt() */
/* 	maxtbz:		Maximum dimension of j'th position in tbz(), tbtt() */
/* 	ntbd:		Number of distance samples in tables */
/* 	ntbz:		Number of depth samples in tables */
/* 	tbd(i):		Distance samples for tables (deg) */
/* 	tbz(j):		Depth samples in tables (km) */
/* 	tbtt(i,j):	Travel-time tables (sec) */
/* 	---- On return ---- */
/* 	dcalx:	Calculated slownesses (sec/deg) */
/* 	atx(4):	Partial derivatives (sec/km/deg) */
/* 	iterr:	Error code for n'th observation */
/* 		=  0, No problem, normal interpolation */
/* 		= 11, Distance-depth point (x0,z0) in hole of T-T curve */
/* 		= 12, x0 < x(1) */
/* 		= 13, x0 > x(max) */
/* 		= 14, z0 < z(1) */
/* 		= 15, z0 > z(max) */
/* 		= 16, x0 < x(1) and z0 < z(1) */
/* 		= 17, x0 > x(max) and z0 < z(1) */
/* 		= 18, x0 < x(1) and z0 > z(max) */
/* 		= 19, x0 > x(max) and z0 > z(max) */
/* 	[NOTE:	If any of these codes are negative (e.g., iderr = -17), */
/* 		then, the datum was used to compute the event location] */
/* 	---- Subroutines called ---- */
/* 	From libinterp */
/* 		brack:		Bracket travel-time data via bisection */
/* 		holint2:	Quadratic interpolation function */
/* DIAGNOSTICS */
/* 	Currently assumes a constant radius Earth (i.e., it ignores */
/* 	ellipicity of the Earth. */
/* NOTES */
/* 	Future plans include dealing with ellipicity and higher-order */
/* 	derivatives. */
/* SEE ALSO */
/* 	Subroutines azcal0, and slocal0 are parallel routines for computing */
/* 	azimuthal and slowness partial derivatives, respectively. */
/* AUTHOR */
/* 	Steve Bratt, December 1988. */
int sc_locsat_ttcal(
	const float zfoc, const float radius, const float delta, const float azi,
	const int maxtbd, const int maxtbz,
	const int *ntbd, const int *ntbz, const float *tbd, const float *tbz, const float *tbtt,
	float *dcalx,
	double *atx, int *iterr
) {
	int tbtt_dim1, tbtt_offset, i__1, i__2;

	float dtdz;
	double azir;
	int iext, jext;
	int ileft, do_extrap;
	float dtddel;
	int jz, nz;
	double cosazi;
	float dcross;
	double sinazi;
	double pd12;
	int ibad;

	--tbd;
	tbtt_dim1 = maxtbd;
	tbtt_offset = 1 + tbtt_dim1 * 1;
	tbtt -= tbtt_offset;
	--tbz;

	do_extrap = 0;

	// Find relevant range of table depths
	sc_locsat_brack(*ntbz, &tbz[1], zfoc, &ileft);

	// Computing MAX
	i__1 = 1, i__2 = ileft - 1;
	jz = max(i__1, i__2);

	// Computing MIN
	i__1 = *ntbz, i__2 = ileft + 2;
	nz = min(i__1, i__2) - jz + 1;

	// HOLIN2 performs bivariate interpolation
	sc_locsat_holint2(
		do_extrap, *ntbd, nz,
		&tbd[1], &tbz[jz], &tbtt[jz * tbtt_dim1 + 1], maxtbd,
		-1.f, delta, zfoc, dcalx,
		&dtddel, &dtdz, &dcross, &iext, &jext, &ibad
	);

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
		// Interpolate point less than first distance point in curve and
		// less than first depth point in curve
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

	// Compute partial derivatives if point is not in a hole.
	if ( ibad == 0 ) {
		azir = azi * (float).017453293;
		sinazi = sin(azir);
		cosazi = cos(azir);
		pd12 = dtddel / ((radius - zfoc) * (float).017453293);
		atx[0] = (float)1.; // Axis 1
		atx[1] = -pd12 * sinazi; // Axis 2 points east
		atx[2] = -pd12 * cosazi; // Axis 3 points north
		atx[3] = -dtdz; // Axis 4 points up
	}

	return 0;
}
