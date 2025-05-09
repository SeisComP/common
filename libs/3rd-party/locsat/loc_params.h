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
