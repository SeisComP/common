#include <locsat/loc.h>
#include <stdio.h>


/* NAME */
/* 	locsat0 -- Locate events using Slowness, Azimuth and Time data. */
/* FILE */
/* 	locsat0.f */
/* SYNOPSIS */
/* 	Computes event locations, confidence bounds, residuals and */
/* 	importances using arrival time, azimuths and slowness */
/* 	measurements from stations at regional and teleseismic */
/* 	distances. */
/* DESCRIPTION */
/* 	Subroutine.  Information on travel-time tables, stations, */
/* 	detections and location parameters are passed to and from */
/* 	LocSAT via the argument list.  The phase and station names */
/* 	given for each datum (data_sta_id,data_phase_type) must match those in the */
/* 	lists of acceptable phases and stations (staid,wavid). */
/* 	---- Indexing ---- */
/* 	i = 1, nsta;	j = 1, nwav;	k = 1, ntbd(j);	m = 1, ntbz(j); */
/* 	n = 1, ndata; */
/* 	---- On entry ---- */
/* 	ndata:	Number of data */
/* 	nsta:	Number of stations in network */
/* 	nwav:	Number of phases in list */
/* 	maxtbd:	Maximum dimension of k'th position in tbd(), tbtt() */
/* 	maxtbz:	Maximum dimension of m'th position in tbz(), tbtt() */
/* 	data_sta_id(n):	Name of station for n'th datum */
/* 	data_phase_type(n):	Name of phase for n'th datum */
/* 	data_type(n):	Data type for n'th datum (time, azim, slow) */
/* 	data_defining(n):	Arrival usage */
/* 			  = d: Defining, used in location */
/* 			  = n: Could be defining, but not used in location */
/* 			  = a: Not to be used in location */
/* 	data_obs_data(n):	Value of n'th datum (sec, deg, sec/deg) */
/* 	data_std_err(n): 	Standard deviation in value of n'th datum */
/* 	data_arrival_id_index(n):	Arrival ID for datum */
/* 	staid(i):	List of all acceptible station names */
/* 	stalat(i):	Station latitudes  (deg) */
/* 	stalon(i):	Station longitudes (deg) */
/* 	wavid(j):	List of all acceptible phases for arrival time and */
/* 		        slowness data (these are the suffixes for the */
/* 			travel-time tables */
/* 	ntbd(j):	Number of distance samples in travel-time tables */
/* 	ntbz(j):	Number of depth samples in travel-time tables */
/* 	tbd(k,j):	Distance to k'th lat/lon travel-time node (deg) */
/* 	tbz(m,j):	Depth to m'th travel-time node from Earths surface (km) */
/* 	tbtt(k,m,j):	Travel-time of k'th lat/lon and m'th depth nodes (sec) */
/* 	alat0:  	Initial guess of event latitude (deg) */
/* 	alon0:  	Initial guess of event longitude (deg) */
/* 			[If alat0 or alon0 non-possible values (e.g. */
/* 			abs (lat or lon) > 90 or 180), subroutine hypcut0 */
/* 			will estimate a starting location.] */
/* 	zfoc0:  	Initial guess of event focal depth (km) */
/* 			[WARNING: alat0, alon0, zfoc0 may be changed on */
/* 			output from this subroutine if the initial guess */
/* 			was changed during processing] */
/* 	sig0:   	Prior estimate of data standard error */
/* 	ndf0:   	Number of degrees of freedom in sig0 */
/* 	pconf:  	Confidence probability for confidence regions (0.0-1.0) */
/* 			[WARNING: subroutine fstat.f only accepts .9 for now] */
/* 	azwt:   Weight applied to azimuth data and partials */
/* 	        (default = 1.0) */
/* 	damp:   Percent damping relative to largest singular value, */
/* 	        if < 0.0, only damp when condition number > million */
/* 	maxit:  Maximum number of iterations allowed in inversion */
/* 	prtflg: = y, Verbose printout */
/* 	        = n, None. */
/* 	fxdflg: = n, Focal depth is a free parameter in inversion */
/* 	        = y, Focal depth is constrained to equal zfoc0 */
/* 	---- On return ---- */
/* 	alat:	Final estimate of event latitude (deg) */
/* 	alon:	Final estimate of event longitude (deg) */
/* 	zfoc:	Final estimate of event focal depth (km) */
/* 	torg:	Final estimate of event origin time (sec) */
/* 	sighat:	Final estimate of data standard error */
/* 	snssd:	Normalized sample standard deviation */
/* 	ndf:	Number of degrees of freedom in sighat */
/* 	epmaj:	Length of semi-major axis of confidence ellipse on */
/* 		epicenter (km) */
/* 	epmin:	Length of semi-minor axis of confidence ellipse on */
/* 		epicenter (km) */
/* 	epstr:	Strike of semi-major axis of confidence ellipse on */
/* 		epicenter (deg) */
/* 	zfint:	Length of confidence semi-interval on focal depth (km) */
/* 		= < 0.0 if fxdflg = y' or depth was fixed by program due */
/* 		to convergence problem */
/* 	toint:	Length of confidence semi-interval on origin time (sec) */
/* 	rank:	Effective rank of the sensitivity matrix */
/* 	niter:	Total number of iterations performed during inversion */
/* 	sxx:	(Parameter covariance element) */
/* 	syy:	(Parameter covariance element) */
/* 	szz:	(Parameter covariance element) */
/* 		= < 0.0 if fxdflg = y' or depth was fixed by program due */
/* 		to convergence problem */
/* 	stt:	(Parameter covariance element) */
/* 	sxy:	(Parameter covariance element) */
/* 	sxz:	(Parameter covariance element) */
/* 	syz:	(Parameter covariance element) */
/* 	stx:	(Parameter covariance element) */
/* 	sty:	(Parameter covariance element) */
/* 	stz:	(Parameter covariance element) */
/* 	stadel(i):	Distance from epicenter to i'th station (deg) */
/* 	staazi(i):	Azimuth from epicenter to i'th station (deg) */
/* 	stabaz(i):	Back-azimuth from epicenter to i'th station (deg) */
/* 	data_importances(n):	Epicenter importance of n'th datum */
/* 	data_zfimp(n):	Depth importance of n'th datum */
/* 	data_residual(n):	Residual (obs-calc) for n'th datum (sec, deg, sec/deg) */
/* 	data_sta_index(n):	Station index for n'th observation */
/* 	data_err_code(n):	Error code for n'th observation */
/* 			=  0, No problem, normal interpolation */
/* 			=  1, No station information for datum */
/* 			=  2, No travel-time tables for datum */
/* 			=  3, Data type unknown */
/* 			=  4, S.D <= 0.0 for datum */
/* 			= 11, Distance-depth point (x0,z0) in hole of T-T curve */
/* 			= 12, x0 < x(1) */
/* 			= 13, x0 > x(max) */
/* 			= 14, z0 < z(1) */
/* 			= 15, z0 > z(max) */
/* 			= 16, x0 < x(1) and z0 < z(1) */
/* 			= 17, x0 > x(max) and z0 < z(1) */
/* 			= 18, x0 < x(1) and z0 > z(max) */
/* 			= 19, x0 > x(max) and z0 > z(max) */
/* 	[NOTE:	If any of these codes is .le. 0 (e.g. data_err_code = -17), */
/* 		then, the datum was used to compute event location] */
/* 	ierr:	Error flag; */
/* 		  = 0,	No error */
/* 		  = 1,	Maximum number of iterations exhausted */
/* 		  = 2,	Iteration diverged */
/* 		  = 3,	Too few usable data */
/* 		  = 4,	Too few usable data to constrain origin time, */
/* 			however, a valid location was obtained */
/*                 = 5,	Insufficient data for a solution */
/*                 = 6,	SVD routine cannot decompose matrix */
/* 	---- Subroutines called ---- */
/* 	Local */
/* 		check_data_:	Review and quality check of data */
/* 		hypcut0:	Compute first initial guess hypocenter */
/* 		hypinv0:	Compute location */
/* 		index_array	Sort in ascending order by index */
/* 	---- Functions called ---- */
/* DIAGNOSTICS */
/* 	Complains when input data are bad ... */
/* FILES */
/* 	Open an output file to a specified unit number or standard out. */
/* NOTES */
/* 	Remember to add time-offset variable and remove data_zfimp() from */
/* 	arguments in the calling subroutine. */
/* SEE ALSO */
/* 	Bratt and Bache (1988) Locating events with a sparse network of */
/* 	regional arrays, BSSA, 78, 780-798. */
/* AUTHORS */
/* 	Steve Bratt, December 1988. */
/* 	Walter Nagy, November 1990. */
/* Subroutine */
void sc_locsat(
	LOCSAT_TTT *ttt,
	LOCSAT_Data *data, const int ndata,
	LOCSAT_Site *stations, const int nsta,
	float alat0, float alon0, const float zfoc0,
	const float sig0, const int ndf0, const float pconf,
	const float damp, const int maxit, const char fxdflg,
	float *alat, float *alon, float *zfoc,
	float *torg, float *sighat, float *snssd,
	int *ndf, float *epmaj, float *epmin, float *epstr, float *zfint,
	float *toint, float *sxx, float *syy, float *szz, float *stt, float *sxy,
	float *sxz, float *syz, float *stx, float *sty, float *stz,
	int *niter, int *ierr
) {
	double rank;
	int i, n;
	int nd;

	int *ntbd = ttt->ntbd;
	int *ntbz = ttt->ntbz;

	/* Function Body */
	if ( ndata < 3 ) {
		// do not output error message which as it is described by error code
		//	s_wsfe(&io___1);
		//	do_fio(&c__1, " Insufficient data for a solution: # of Data = ", (
		//		int)47);
		//	do_fio(&c__1, (char *)&nd, (int)sizeof(int));
		//	e_wsfe();
		*ierr = 5;
		return;
	}

	// Check for valid data and load station, wave and data type pointers
	sc_locsat_check_data(data, ndata, stations, nsta, ttt->phases, ttt->num_phases);

	// Check that each travel-time curve contains valid data. For arrival
	// time or slowness data with empty curves, set iderr = 2
	for ( n = 0; n < ndata; ++n ) {
		if ( (data[n].idtyp == 1 || data[n].idtyp == 3) &&
		     (ntbd[data[n].ipwav] <= 0 || ntbz[data[n].ipwav] <= 0) ) {
			data[n].err_code = 2;
		}
	}

	// Compute initial first-cut guess location
	if ( dabs(alat0) > (float)90. || dabs(alon0) > (float)180. ) {
		sc_locsat_hypcut(
			ttt, data, ndata, stations, nsta,
			&alat0, &alon0, ierr
		);
		if ( *ierr > 0 ) {
			return;
		}
	}

	*sighat = (float)-1.;
	*zfint = (float)-1.;
	*toint = (float)-1.;
	for ( i = 0; i < ndata; ++i ) {
		data[i].epimp = (float)-1.;
	}

	sc_locsat_hypinv(
		ttt, data, ndata, stations, nsta,
		alat0, alon0, zfoc0, sig0, ndf0, pconf, LOCSAT_EARTH_RADIUS,
		damp, maxit,
		fxdflg, alat, alon, zfoc, torg, sighat, snssd,
		ndf, epmaj, epmin, epstr, zfint, toint, sxx, syy, szz, stt, sxy, sxz,
		syz, stx, sty, stz,  &rank, niter, &nd, ierr
	);
}
