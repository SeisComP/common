/******************************************************************************
 * Copyright 1990 Science Applications International Corporation.
 *
 * There was no particular license attached to the origin source code. The
 * sources have been modified by gempa GmbH to prefix structures with LOCSAT_.
 ******************************************************************************/


#ifndef LOC_PARAMS_H
#define LOC_PARAMS_H


#include <locsat/assoc.h>
#include <locsat/arrival.h>


/*
 * The Locator_params structure will be used to pass parameter values from
 * the Locator GUI, LocSAT, ARS and ESAL to locate_event().  locate_event()
 * will then use these parameters for it's call to the locatation algorithm,
 * LocSAT0.
 */
typedef struct {
	int	   num_dof;	/* 9999    - number of degrees of freedom    */
	float  est_std_error;	/* 1.0     - estimate of data std error      */
	float  conf_level;	/* 0.9     - confidence level    	     */
	float  damp;		/* -1.0    - damping (-1.0 means no damping) */
	int	   max_iterations;	/* 20      - limit iterations to convergence */
	char   fix_depth;	/* true    - use fixed depth ?               */
	float  fixing_depth;	/* 0.0     - fixing depth value              */
	float  lat_init;	/* modifiable - initial latitude             */
	float  lon_init;	/* modifiable - initial longitude            */
	float  depth_init;	/* modifiable - initial depth                */
	int	   use_location;	/* true    - use current origin data ?       */
	char   verbose;	/* true    - verbose output of data ?        */
	char  *prefix;		/* NULL    - dir name & prefix of tt tables  */
} LOCSAT_Params;


typedef struct {
	char        *dir;
	int          num_phases;
	const char **phases;
	int          len_dir;
	int          lentbd;
	int          lentbz;
	float       *tbd;
	float       *tbz;
	float       *tbtt;
	int         *ntbd;
	int         *ntbz;
} LOCSAT_TTT;


/*
 * The LOCSAT_Errors structure will be used to pass data from locate_event()
 * to the caller. The applications will then use the values to report errors
 * that may have occurred during the location calculation called by
 * locate_event() and performed by location algoritm.
 */
typedef struct {
	int	arid;
	int	time;
	int	az;
	int	slow;
} LOCSAT_Errors;


/*
 * State of one iteration of the inversion, recorded after the least squares
 * step has been computed but before the hypocenter is perturbed.
 */
typedef struct {
	int    iteration;	/* iteration number, starting with 0         */
	int    num_data;	/* number of defining data used              */
	int    num_params;	/* number of free parameters (3 or 4)        */
	float  lat;		/* trial latitude (deg)                      */
	float  lon;		/* trial longitude (deg)                     */
	float  depth;		/* trial depth (km)                          */
	float  torg;		/* trial origin time relative to first arr.  */
	double unwt_rms;	/* RMS of the raw residuals                  */
	double wt_rms;		/* RMS of the residuals normalized by std err */
	double cnvgtst;		/* convergence test value                    */
	double dxnorm;		/* norm of the hypocenter perturbation (km)  */
	double condition;	/* condition number of the system matrix     */
	float  sighat;		/* a posteriori standard error               */
} LOCSAT_Iteration;


/*
 * Epicenter importances of the time, azimuth and slowness datum of one
 * observation. -1 means the datum was absent or not defining.
 */
typedef struct {
	float time;
	float az;
	float slow;
} LOCSAT_Importance;


/*
 * Optional diagnostics filled by sc_locsat_locate_event. The caller owns
 * the arrays and sets their capacities; NULL arrays are skipped.
 */
typedef struct {
	LOCSAT_Iteration  *iterations;	/* in: array, out: filled entries   */
	int                max_iterations;	/* in: capacity of iterations      */
	int                num_iterations;	/* out: number of entries written  */
	LOCSAT_Importance *importances;	/* in: array of num_obs entries     */
	double             rank;		/* effective rank of the matrix     */
	double             condition[2];	/* true and effective condition num */
	float              sighat;		/* a posteriori standard error      */
	float              snssd;		/* normalized sample std deviation  */
	int                ndf;		/* degrees of freedom of sighat     */
	int                num_params;	/* number of free parameters        */
	int                num_data;	/* number of defining data used     */
	int                niter;		/* number of iterations performed   */
} LOCSAT_Diagnostics;


typedef struct {
	char  phase_type[sizeof(((LOCSAT_Assoc*)0)->phase)];
	char  sta[sizeof(((LOCSAT_Arrival*)0)->sta)];
	char  type;
	char  defining;
	float obs;
	float std_err;
	int   obs_data_index;
	float residual;
	int   sta_index;
	int   err_code;
	int   ipwav;
	int   idtyp;

	// Inversion block
	float  epimp; // Epicenter importance
	double resid2;
	double resid3;
	double dsd2;
	int    idtyp2;
	double at[4];
	int    ip0;
} LOCSAT_Data;


#endif
