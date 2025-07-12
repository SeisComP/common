/******************************************************************************
 * Copyright 1991 Science Applications International Corporation.
 * Modifications and fixes gempa GmbH in 2025
 *
 * Note: there was no particular license attached to the origin source code.
 * gempa GmbH modified the source code in the following way:
 * - Translation from Fortran to plain C
 * - Prefix of function names with sc_locsat_
 * - Remove printing of debug information
 * - Get rid of static variables and make all code reentrant
 ******************************************************************************/


#ifndef LOCSAT_LOC_H
#define LOCSAT_LOC_H


#include <locsat/arrival.h>
#include <locsat/assoc.h>
#include <locsat/origin.h>
#include <locsat/origerr.h>
#include <locsat/site.h>
#include <locsat/loc_params.h>
#include <locsat/utils.h>


#define LOCSAT_NoError 0
#define LOCSAT_GLerror1 1
#define LOCSAT_GLerror2 2
#define LOCSAT_GLerror3 3
#define LOCSAT_GLerror4 4
#define LOCSAT_GLerror5 5
#define LOCSAT_GLerror6 6
#define LOCSAT_GLerror7 7
#define LOCSAT_GLerror8 8
#define LOCSAT_GLerror9 9
#define LOCSAT_GLerror10 10
#define LOCSAT_GLerror11 11
#define LOCSAT_TTerror1 12
#define LOCSAT_TTerror2 13
#define LOCSAT_TTerror3 14
#define LOCSAT_TTerror4 15
#define LOCSAT_TTerror5 16
#define LOCSAT_TTerror6 17


void sc_locsat_init_ttt(LOCSAT_TTT *ttt);
void sc_locsat_free_ttt(LOCSAT_TTT *ttt);

/**
 * @brief Reads the travel time tables.
 * @param ttt The instance to be populated.
 * @param dir The directory which holds the tables including the file prefix
 * @param verbose Verbosity flag.
 * @return Error code
 */
int sc_locsat_setup_tttables(LOCSAT_TTT *ttt, const char *dir, int verbose);
int sc_locsat_find_phase(const LOCSAT_TTT *ttt, const char *phase);

double sc_locsat_compute_ttime(
	const LOCSAT_TTT *ttt,
	double distance, double depth, const char *phase, int extrapolate,
	double *rdtdd, double *rdtdh, int *errorflag
);

/**
 * @brief Search input data file for valid seismic data.
 *
 * Check seismic data for valid station, wave type, a priori standard
 * deviation, and datum type (i.e., Slowness, Azimuth, or Travel-time).
 *
 * Discriminate between valid and invalid data for use
 * in determining a solution.  The valid data of interest include:
 * slowness, azimuth and travel-times; valid station ID's, correct
 * wave and datum types; and a proper a priori standard deviation.
 *
 * Indexing
 * i = 0, num_sta-1; j = 0, num_phase_types-1; n = 0, num_data-1;
 *
 * @param[in] data Data for n'th datum.
 * @param[in] data_std_err Standard deviation in value of n'th datum.
 * @param[in] num_data Number of data.
 * @param[in] stations Name of i'th acceptible station.
 * @param[in] num_sta Number of stations.
 * @param[in] phase_type Name of j'th acceptible wave.
 * @param[in] num_phase_types Number of wave-types allowed.
 * @param[in] sta_len Length of staid element.
 * @param[in] phase_len Length of phase_type element.
 * @param[out] sta_index Station index for n'th observation.
 * @param[out] ipwav Wave index for n'th observation.
 * @param[out] idtyp Type code for n'th observation
 *                   = 0, Data type unknown
 *                   = 1, Arrival time datum
 *                   = 2, Azimuth datum
 *                   = 3, Slowness datum
 * @param[ouz] data_error_code Error code for n'th observation
 *                             = 0, No problem, datum is valid
 *                             = 1, No station information for this datum
 *                             = 2, No travel-time table for this datum
 *                             = 3, Data type unknown
 *                             = 4, Standard deviation <= 0.0 for this datum
 */
void sc_locsat_check_data(
	LOCSAT_Data *data, const int num_data,
	const LOCSAT_Site *stations, const int num_sta,
	const char *const *phase_type, const int num_phase_types
);

int sc_locsat_ttcal(
	const float zfoc, const float radius, const float delta, const float azi,
	const int maxtbd, const int maxtbz,
	const int *ntbd, const int *ntbz, const float *tbd, const float *tbz, const float *tbtt,
	float *dcalx, double *atx, int *iterr
);

float sc_locsat_elpcor(const char *phid, float del, float z, float azi, float ecolat);

/**
 * @brief Determine event error ellipsoid and normalized confidence regions.
 *
 * Given the covariance matrix of the hypocentral estimate,
 * calculate the error ellipsoid and confidence regions from the
 * appropriate marginal variances with F-distribution factors ignored.
 * These are normalized in the sense that they are scaled to a
 * particular confidence probability, thereby making the marginal
 * variances justified.
 *
 * The following parameters are described:
 *  1. The three-dimensional confidence ellipsoid of the hypocenter,
 *  as determined from the 3x3 variance matrix of the hypocenter
 *  (marginal w.r.t. origin time)
 *  2. The two-dimensional confidence ellipse of the epicenter, as
 *  determined from the 2x2 variance matrix of the epicenter
 *  (marginal w.r.t. origin time and depth)
 *  3. The one-dimensional confidence interval of focal depth, as
 *  determined from the scalar variance of the focal depth
 *  (marginal w.r.t. origin time and epicenter)
 *
 * @note Three-dimensional ellipsoid parameters not yet inplemented. Output
 *       variables hy[...] are currently returned as zero.
 * @param[in] np Number of hypocentral parameters; either 2 or 3
 *               The parameters in order are defined to be:
 *                  1. Position along the local East direction (km),
 *                  2. Position along the local North direction (km),
 *                  3. Position along the local Up direction (km);
 *                  (i.e., minus depth).
 * @param[in] covar Parameter variance matrix (covariance of j'th and k'th parameters).
 *                  If np = 2, then covar(j,3) and covar(3,k) are assumed to be zero.
 *                  Leading dimension is 3.
 * @param[out] hymaj Length of major semi-axis of hypocenter confidence ellipsoid.
 * @param[out] hymid Length of middle semi-axis of hypocenter confidence ellipsoid.
 * @param[out] hymin Length of minor semi-axis of hypocenter confidence ellipsoid.
 * @param[out] hystr Strike of major semi-axis of hypocenter confidence ellipsoid
 *                   (degrees clockwise from north).
 * @param[out] hyplu Plunge of major semi-axis of hypocenter confidence ellipsoid
 *                   (degrees downward from horizontal).
 * @param[out] hyrak Direction of middle semi-axis of hypocenter confidence ellipsoid
 *                   (degrees clockwise from up, when looking down-plunge).
 * @param[out] epmaj Length of major semi-axis of epicenter confidence ellipse.
 * @param[out] epmin Length of minor semi-axis of epicenter confidence ellipse.
 * @param[out] epstr Strike of major semi-axis of epicenter confidence ellipse
 *                   (degrees clockwise from north).
 * @param[out] zfint Length of focal depth confidence semi-interval.
 * @param[out] stt time-time parameter covariance element.
 * @param[out] stx time-lon parameter covariance element.
 * @param[out] sty time-lat parameter covariance element.
 * @param[out] sxx time-depth parameter covariance element.
 * @param[out] sxy lon-lon parameter covariance element.
 * @param[out] syy lon-lat parameter covariance element.
 * @param[out] stz lon-depth parameter covariance element.
 * @param[out] sxz lat-lat parameter covariance element.
 * @param[out] syz lat-depth parameter covariance element.
 * @param[out] szz depth-depth parameter covariance element.
 */
void sc_locsat_ellips(
	int *np, double covar[][4],
	double *hymaj, double *hymid, double *hymin, double *hystr, double *hyplu, double *hyrak, double *epmaj, double *epmin,
	float *epstr, double *zfint,
	float *stt, float *stx, float *sty, float *sxx, float *sxy, float *syy, float *stz, float *sxz, float *syz, float *szz
);

/**
 * @brief Remove variance-weighted mean from travel-time data and system matrix.
 *
 * Denuisance data and system matrix with respect to a baseline parameter.
 * This boils downs to removing a variance-weighted mean from the travel-time
 * data and from each column of the travel-time system (derivative; sensitivity)
 * matrix.
 *
 * @param[in] data Data array
 * @param[in] nd Number of data
 * @param[in] np Number of parameters
 * @param[out] dmean Weighted mean of travel-time data (pre-denuisanced)
 * @param[out] inerr 0 == OK, 1 == No valid arrival-time data, asum = 0
 */
void sc_locsat_denuis(LOCSAT_Data *data, const int nd, const int np, double *dmean, int *inerr);

/**
 * @brief Compute azimuthal partial derivatives for a fixed hypocenter.
 *
 * Given a distance and azimuth from a fixed event
 * hypocenter to a station, azcal_() computes azimuthal partial
 * derivatives as determined from pre-processed f-k analysis.
 * Currently assumes a constant radius Earth (i.e., it ignores
 * ellipicity of the Earth).
 *
 * @param[in] radius Radius of Earth (km)
 * @param[in] delta Distance from the event to the station (deg)
 * @param[in] azi Forward-azimuth from event to station (deg)
 * @param[in] baz Back-azimuth from the event to the station (deg)
 * @param[out] dcalx Azimuth data computed from alat and alon (deg)
 * @param[out] atx Partial derivatives (deg/km)
 */
void sc_locsat_azcal(float radius, float delta, float azi, float baz, float *dcalx, double atx[4]);

/**
 * @brief Compute the dot product of two vectors.
 *
 * Given a two vectors dx() and dy() of length n, this
 * routine forms their dot product and returns as a scalar in ddot.
 *
 * @param[in] n Length of vector
 * @param[in] dx First vector
 * @param[in] incx x-storage increment counter (= 1, if entire loop accessed)
 * @param[in] dy Second vector
 * @param[in] incy y-storage increment counter (= 1, if entire loop accessed)
 * @return Scalar dot product of dx() and dy()
 */
double sc_locsat_ddot(const int n, double *dx, const int incx, double *dy, const int incy);

/**
 * @brief Apply a plane rotation.
 *
 * Given two vectors dx() and dy() of length n, simply apply a plane rotation
 * and return in the same two vectors.
 *
 * @param n Length of vector
 * @param[inout] dx Original unrotated vector
 * @param incx x-increment loop counter (= 1, if entire loop accessed)
 * @param[inout] dy Original unrotated vector
 * @param incy y-increment loop counter (= 1, if entire loop accessed)
 * @param c
 * @param s
 */
void sc_locsat_drot(const int n, double *dx, const int incx, double *dy, const int incy, const double c, const double s);

/**
 * @brief Construct a Givens plane rotation.
 *
 * Construct a Givens plane rotation, a scalar, da and db, at a time.
 * Called in SVD routine, sc_locsat_dsvdc(), prior to application of normal
 * plane rotation (subr. sc_locsat_drot()).
 */
void sc_locsat_drotg(double *da, double *db, double *c, double *s);

/**
 * @brief Compute the Euclidean norm of a vector.
 *
 * Given a vector dx() of length n, compute the Euclidean norm and return as
 * a scalar in dnrm2.
 *
 * @param[in] n Length of vector; If n <= 0, return n = 0; else n = 1
 * @param[in] dx Vector of interest
 * @param[in] incx x-storage increment counter (= 1, if entire loop accessed)
 * @return Scalar Eucildean norm of dx
 */
double sc_locsat_dnrm2(const int n, double *dx, const int incx);

/**
 * @brief Interchange two vectors.
 */
void sc_locsat_dswap(const int n, double *dx, const int incx, double *dy, const int incy);

/**
 * @brief Scale a vector by a constant.
 * @param[in] n Length of vector
 * @param[in] da Scalar constant ultimately multiplied to dx
 * @param[inout] dx Vector to which scalar is multiplied
 * @param[in] incx x-increment loop counter (= 1, if entire loop accessed)
 */
void sc_locsat_dscal(const int n, double da, double *dx, const int incx);

/**
 * @brief Multiply constant to vector and add another vector.
 *
 * Given a vector dx() of length n, multiply a constant,
 * da, then add to vector dy(). Typically used in applying
 * transformations, often in conjunction with subroutine sc_locsat_ddot().
 *
 * @param[in] n Length of vector
 * @param[in] da Scalar constant ultimately multiplied to dx()
 * @param[in] dx Vector to which constant is multiplied
 * @param[in] incx x-increment loop counter (= 1, if entire loop accessed)
 * @param[inout] dy Vector added to dx()
 * @param[in] incy y-increment loop counter (= 1, if entire loop accessed)
 */
void sc_locsat_daxpy(const int n, const double da, double *dx, const int incx, double *dy, const int incy);

/**
 * @brief Perform singular value decomposition.
 *
 * Decompose an arbitrary matrix into its left and right singular
 * vectors along with their singular values via singular value
 * decomposition.
 *
 * dsvdc is designed to reduce a real*8 NxP matrix x by orthogonal
 * transformations u and v to diagonal form.
 * The diagonal elements s(i) are the singular values of x. The columns of u
 * are the corresponding left singular vectors, and the columns of v the right
 * singular vectors.
 *
 * @param x Contains the matrix whose singular value decomposition is to be computed.
 * @param ldx Leading dimension of x.
 * @param n The number of columns (parameters) of x.
 * @param p The number of rows (data) of x.
 * @param s Singular values in descrending order of magnitude,
 *          where the array size is min(n+1,p).
 * @param e Ordinarily contains zeros. However see the discussion of info for
 *          exceptions.
 * @param u Matrix of left singular vectors, where ldu >= n.
 *          If joba == 1 then k == n, if joba >= 2, then
 *          k == min(n,p). u is not referenced if joba == 0.
 *          If n <= p or if joba == 2, then u may be indentified with x in
 *          the subroutine call.
 * @param ldu Leading dimension of u.
 * @param v Matrix of right singular vectors, where ldv >= p.
 *          v is not referenced if job == 0. If p <= n, then
 *          v may be identified with x in subroutine call.
 * @param ldv Leading dimension of v.
 * @param work Scratch array.
 * @param job Control the computation of the singular vectors.
 * @param info The singular values (and their corresponding singular
 *             vectors) s(info+1), s(info+2),..., s(m) are correct
 *             (here m = min(n,p)).  Thus, if info == 0, all the
 *             singular values and their vectors are correct. In
 *             any event, the matrix b = trans(u)*x*v is the
 *             bidiagonal matrix with the elements of s on its
 *             diagonal and the elements of e on its super-diagonal
 *             (trans(u) is the transpose of u). Thus the singular
 *             values of x and b are the same.
 */
void sc_locsat_dsvdc(
	double *x,
	const int ldx, const int n, const int p,
	double *s, double *e, double *u,
	const int ldu, double *v, const int ldv,
	double *work, int *job, int *info
);

void sc_locsat_solve_via_svd(
	LOCSAT_Data *data, const int nd, const int np,
	const int icov, float damp, double *cnvgtst, double *condit,
	double *xsol, double covar[][4], double *rank, int *ierr
);

/**
 * @brief Make an F-distribution test for M parameters and N degrees of freedom.
 * @param[in] m Number of parameters; Must be 1, 2, or 3.
 * @param[in] n Degrees of freedom;	Must be greater than 1.
 * @param[in] p Confidence level; Must be 0.90.
 * @param[inout] x Argument making Snedecor's F-distribution test for M
 *                 parameters and N degrees of freedom equal to P (i.e., F(X) = P).
 *                 x is	done with a crude table and interpolation and is accurate
 *                 only to about +/- 0.01.
 */
void sc_locsat_fstatx(int m, int n, float p, double *x);

/**
 * @brief Compute horizontal slownesses and partial derivatives.
 *
 * Slownesses and their partials are determined via inter/extrapolation of
 * pre-calculated curves. A point in a hole is rejected.
 * @param[in] zfoc Depth.
 * @param[in] radius Radius of Earth (km).
 * @param[in] delta Distance from the event to the station (deg).
 * @param[in] azi Forward-azimuth from event to station (deg).
 * @param[in] maxtbd Maximum dimension of i'th position in tbd(), tbtt().
 * @param[in] maxtbz Maximum dimension of j'th position in tbz(), tbtt().
 * @param[in] ntbd Number of distance samples in tables.
 * @param[in] ntbz Number of depth samples in tables.
 * @param[in] tbd Distance samples for tables (deg).
 * @param[in] tbz Depth samples in tables (km).
 * @param[in] tbtt Travel-time tables (sec).
 * @param[out] dcalx Calculated slownesses (sec/deg).
 * @param[out] atx Partial derivatives (sec/km/deg).
 * @param[out] iterr Error code for n'th observation
 *             =  0, No problem, normal interpolation
 *             = 11, Distance-depth point (x0,z0) in hole of T-T curve
 *             = 12, x0 < x(1)
 *             = 13, x0 > x(max)
 *             = 14, z0 < z(1)
 *             = 15, z0 > z(max)
 *             = 16, x0 < x(1) and z0 < z(1)
 *             = 17, x0 > x(max) and z0 < z(1)
 *             = 18, x0 < x(1) and z0 > z(max)
 *             = 19, x0 > x(max) and z0 > z(max)
 *             If any of these codes are negative (e.g., iderr = -17), then,
 *             the datum was used to compute the event location.
 */
void sc_locsat_slocal(const float zfoc,
                      const float radius, const float delta, const float azi,
                      const int maxtbd, const int maxtbz,
                      const int *ntbd, const int *ntbz,
                      const float *tbd, const float *tbz, const float *tbtt,
                      float *dcalx, double *atx, int *iterr
                      );

/**
 * @brief Make a first-cut educated guess at the epicentral location.
 *
 * This subroutine takes a first-cut at the epicentral location using one of
 * the following techniques, in the order tried:
 * 1. Attempts to compute an initial location using S-P times for the closest
 *    station and the best-determined defining azimuth for that station (based
 *    on the smallest S.D.).
 * 2. Attempts to compute an intial location from various combinations of
 *    S-P times and P-wave arrival times. The searches are preformed in the
 *    following order of importance:
 *    A. Uses the 3 least S-P times and finds a common crossing point.
 *    B. Uses the smallest P-wave travel time between the common crossing
 *       points of 2 S-P times.
 *    C. Uses a single S-P time to obtain a crude origin time and then find the
 *       nearest crossing points from two P-wave arrival times.
 * 3. Attempts to compute an initial location from various combinations of
 *    azimuth and P-wave arrival time data. When the difference between
 *    azimuths is < 10 deg., then the intersection will be poorly determined,
 *    and is therefore, ignored. The searches are preformed in the following
 *    order of importance:
 *    A. Computes the intersection of 2 great circles given 2 stations with
 *       azimuth data from those points. The location formed using the
 *       best-defining azimuths from those stations closest to the location is
 *       then chosen.
 *    B. Minimizes two P-wave arrival times constrained by a single azimuth
 *       datum, preferably one with an arrival time.
 * 4. Attempts to obtain an initial epicentral location based on an approximate
 *    minimization procedure using three or more P-wave arrival times as data.
 * 5. Attempts to compute an initial location using a P slowness and the
 *    best-determined azimuth datum at the station with the P-slowness with the
 *    smallest standard error between some bounds.
 * 6. Looks for the closest station based of arrival times and uses an
 *    associated azimuth (if available).
 * 7. Looks for a station with a good slowness (based on the slowness datum
 *    with the smallest a priori data standard error) and places the initial
 *    location near that station.
 * 8. It looks for a station with an azimuth and places the initial location
 *    near that station at the associated azimuth.
 *
 * @param[in] ttt Travel time table.
 * @param[in] data Observation data.
 * @param[in] ndata Number of observations.
 * @param[in] stations Station information.
 * @param[in] nsta Number of stations.
 * @param[out] lat First cut at event latitude (deg).
 * @param[out] lon First cut at event longitude (deg).
 * @param[out] ierr Error flag: 0 == No error, 1, Too few data to get initial location.
 */
void sc_locsat_hypcut(
	LOCSAT_TTT *ttt, LOCSAT_Data *data, const int ndata,
	LOCSAT_Site *stations, const int nsta,
	float *lat, float *lon, int *ierr
);

/**
 * @brief Compute a hypocentral location.
 *
 * Computes event locations, confidence bounds, residuals and importances using
 * arrival time, azimuths and slowness measurements from stations at regional
 * and teleseismic distances.
 *
 * Hypocenter inversion (event location), done as an iterative non-linear
 * least squares inversion of travel-time, azimuth and slowness data.
 *
 * @param[in] ttt Travel time table.
 * @param[in] data Observation data.
 * @param[in] ndata Number of observations.
 * @param[in] stations Station information.
 * @param[in] nsta Number of stations.
 * @param[in] alat0 Initial educated guess of event latitude (deg).
 * @param[in] alon0 Initial educated guess of event longitude (deg).
 * @param[in] zfoc0 Initial educated guess of event focal depth (km).
 * @param[in] sig0 Prior estimate of data standard error.
 * @param[in] ndf0 Number of degrees of freedom in sig0.
 * @param[in] pconf Confidence probability for confidence regions (0.0 - 1.0).
 * @param[in] radius Radius of the earth (km).
 * @param[in] damp Percent damping relative to largest singular value, if < 0.0,
 *       [in]      only damp when condition number > million.
 * @param[in] maxit Maximum number of iterations allowed in inversion.
 * @param[in] fxdflg Flag for constraining focal depth.
 *                   n: Focal depth is a free parameter in inversion,
 *                   y: Focal depth is constrained to equal zfoc0
 * @param[out] alat Final estimate of event latitude (deg).
 * @param[out] alon Final estimate of event longitude (deg).
 * @param[out] zfoc Final estimate of event focal depth (km).
 * @param[out] torg Final estimate of event origin time (sec).
 * @param[out] sighat Final estimate of data standard error.
 * @param[out] snssd Normalized sample standard deviation.
 * @param[out] ndf Number of degrees of freedom in sighat.
 * @param[out] epmaj Length of semi-major axis of confidence ellipse on epicenter (km).
 * @param[out] epmin Length of semi-minor axis of confidence ellipse on epicenter (km).
 * @param[out] epstr Strike of semi-major axis of confidence ellipse on epicenter (deg).
 * @param[out] zfint Length of confidence semi-interval on focal depth (km)
                     = < 0.0, if fxdflg = 'y' or depth was fixed by program
                     due to convergence problem.
 * @param[out] toint Length of confidence semi-interval on origin time (sec).
 * @param[out] sxx (Parameter covariance diagonal element).
 * @param[out] syy (Parameter covariance diagonal element).
 * @param[out] szz (Parameter covariance diagonal element).
 * @param[out] stt (Parameter covariance diagonal element).
 * @param[out] sxy (Parameter covariance element).
 * @param[out] sxz (Parameter covariance element).
 * @param[out] syz (Parameter covariance element).
 * @param[out] stx (Parameter covariance element).
 * @param[out] sty (Parameter covariance element).
 * @param[out] stz (Parameter covariance element).
 * @param[out] rank Effective rank of the sensitivity matrix.
 * @param[out] niter Number of iterations performed in inversion.
 * @param[out] nd
 * @param[out] ierr Error flag
 *             0 = No error
 *             1 = Maximum number of iterations exhausted
 *             2 = Iteration diverged
 *             3 = Too few usable data to constrain any parameters
 *             4 = Too few usable data to constrain origin time,
 *                 however, a valid location was obtained
 *             5 = Insufficient data for a solution
 *             6 = SVD routine cannot decompose matrix
 */
void sc_locsat_hypinv(
	LOCSAT_TTT *ttt,
	LOCSAT_Data *data, const int ndata,
	LOCSAT_Site *stations, const int nsta,
	const float alat0, const float alon0, const float zfoc0,
	const float sig0, const int ndf0, const float pconf,
	const float radius, const float damp, const int maxit,
	char fxdflg, float *alat, float *alon, float *zfoc,
	float *torg, float *sighat, float *snssd, int *ndf, float *epmaj,
	float *epmin, float *epstr, float *zfint, float *toint, float *sxx,
	float *syy, float *szz, float *stt, float *sxy, float *sxz, float *syz,
	float *stx, float *sty, float *stz, double *rank,
	int *niter, int *nd, int *ierr
);


void sc_locsat(
	LOCSAT_TTT *ttt,
	LOCSAT_Data *data, const int ndata,
	LOCSAT_Site *stations, const int nsta,
	float alat0, float alon0, const float zfoc0,
	const float sig0, const int ndf0, const float pconf, const float damp,
	const int maxit, const char fxdflg,
	float *alat, float *alon, float *zfoc,
	float *torg, float *sighat, float *snssd,
	int *ndf, float *epmaj, float *epmin, float *epstr, float *zfint,
	float *toint, float *sxx, float *syy, float *szz, float *stt, float *sxy,
	float *sxz, float *syz, float *stx, float *sty, float *stz,
	int *niter, int *ierr
);


int sc_locsat_locate_event(
	LOCSAT_TTT *ttt,
	LOCSAT_Site *sites, int num_sites,
	LOCSAT_Arrival *arrival, LOCSAT_Assoc *assoc,
	LOCSAT_Origin *origin, LOCSAT_Origerr *origerr,
	LOCSAT_Params *locator_params,
	LOCSAT_Errors *locator_errors, int num_obs
);


#endif
