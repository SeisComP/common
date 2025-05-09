#include <locsat/aesir.h>
#include <locsat/interp.h>
#include <locsat/loc.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#if WIN32
void bzero(char *s, int n) {
	memset(s, 0, n);
}
#endif

#define NULL_TIME -9999999999.999
#define STRIKE_NULL -1.0
#define STRIKE_MIN 0.0
#define STRIKE_MAX 360.0

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define VALID_TIME(x) ((x) > -9999999999.000)
#define VALID_SEAZ(x) (((x) >= 0.0) && ((x) <= STRIKE_MAX))
#define VALID_SLOW(x) ((x) >= 0.0)
#define DEG_TO_RAD 0.017453293

#define MAXTBD_ 210
#define MAXTBZ_ 50


void sc_locsat_rdtttab(
	const char *froot,                  /* Size [ca. 1024] */
	const char **phase_type_ptr,        /* Size [nwav][8] */
	int nwav,                           /* Array lengths */
	int maxtbd,                         /* Array lengths */
	int maxtbz,                         /* Array lengths */
	int *ntbd,                          /* Array lengths */
	int *ntbz,                          /* Array lengths */
	float *tbd,                         /* Size [nwav][maxtbd] */
	float *tbz,                         /* Size [nwav][maxtbz] */
	float *tbtt,                        /* Size [nwav][maxtbz][maxtbd] */
	int *ierr,                          /* Error flag */
	int verbose                         /* Verbose mode */
);


void sc_locsat_init_ttt(LOCSAT_TTT *ttt) {
	ttt->dir = NULL;
	ttt->num_phases = 0;
	ttt->phases = NULL;
	ttt->tbd = NULL;
	ttt->tbd = NULL;
	ttt->tbz = NULL;
	ttt->tbtt = NULL;
	ttt->ntbd = NULL;
	ttt->ntbz = NULL;
}

void sc_locsat_free_ttt(LOCSAT_TTT *ttt) {
	UFREE(ttt->dir);
	for ( int i = 0; i < ttt->num_phases; ++i ) {
		UFREE(ttt->phases[i]);
	}
	UFREE(ttt->phases);
	ttt->num_phases = 0;
	UFREE(ttt->tbd);
	UFREE(ttt->tbz);
	UFREE(ttt->tbtt);
	UFREE(ttt->ntbd);
	UFREE(ttt->ntbz);
}


static const char *default_phases[] = {
	"P", "Pb", "PcP", "Pg", "PKKP", "PKP", "PKPab", "PKPbc", "PKPdf", "Pn",
	"pP", "PP", "pPKPab", "pPKPbc", "pPKPdf",

	"S", "Sb", "ScP", "ScS", "Sg", "SKKP", "SKP", "SKPdf", "SKS", "SKSac",
	"SKSdf", "sP", "sPKPab", "sPKPbc", "sPKPdf", "Sn", "sS", "SS",

	"LQ", "LR", "Lg", "Rg",

	"Is", "It", "Iw"
};

static int default_phases_num = sizeof(default_phases) / sizeof(const char*);

static int setup_tttables(
	LOCSAT_TTT *ttt,
	const char *dir, const char **phase_types, int num_phase_types,
	int verbose
) {
	int ierr;
	int malloc_err = 0;

	sc_locsat_free_ttt(ttt);

	// First determine whether it is necessary to read in new tables
	if ( num_phase_types == 0 || !phase_types ) {
		return LOCSAT_TTerror1;
	}

	ttt->dir = STRALLOC(dir);
	ttt->len_dir = strlen(ttt->dir);

	// Take ownership of the input parameters
	ttt->num_phases = num_phase_types;
	ttt->phases = phase_types;

	malloc_err = 0;
	ttt->ntbd = ttt->ntbz = (int *)NULL;
	ttt->tbd = ttt->tbz = ttt->tbtt = (float *)NULL;
	ttt->lentbd = MAXTBD_;
	ttt->lentbz = MAXTBZ_;

	if ( !(ttt->ntbd = UALLOC(int, num_phase_types)) ) {
		malloc_err++;
	}

	else if ( !(ttt->ntbz = UALLOC(int, num_phase_types)) ) {
		malloc_err++;
	}

	else if ( !(ttt->tbd = UALLOC(float, ttt->lentbd * num_phase_types)) ) {
		malloc_err++;
	}

	else if ( !(ttt->tbz = UALLOC(float, ttt->lentbz * num_phase_types)) ) {
		malloc_err++;
	}

	else if ( !(ttt->tbtt = UALLOC(float, ttt->lentbd * ttt->lentbz * num_phase_types)) ) {
		malloc_err++;
	}

	if ( malloc_err ) {
		sc_locsat_free_ttt(ttt);
		return LOCSAT_TTerror6;
	}

	// Read the travel-time tables
	ttt->num_phases = num_phase_types;
	ttt->phases = phase_types;

	sc_locsat_rdtttab(
		ttt->dir, ttt->phases, ttt->num_phases,
		ttt->lentbd, ttt->lentbz,
		ttt->ntbd, ttt->ntbz, ttt->tbd,
		ttt->tbz, ttt->tbtt, &ierr, verbose
	);

	if ( ierr == 0 ) {
		return LOCSAT_NoError;
	}
	else {
		sc_locsat_free_ttt(ttt);

		if ( ierr == 1 ) {
			return LOCSAT_TTerror2;
		}
		else if ( ierr == 2 ) {
			return LOCSAT_TTerror3;
		}
		else if ( ierr == 3 ) {
			return LOCSAT_TTerror4;
		}
		else {
			return LOCSAT_TTerror5;
		}
	}
}


int sc_locsat_setup_tttables(LOCSAT_TTT *ttt, const char *dir, int verbose) {
	if ( ttt->dir && !strcmp(ttt->dir, dir) ) {
		return LOCSAT_NoError;
	}

	// First check, if there is a phase list file
	const char **phases = NULL;
	int phases_num = 0;

	char *phase_file_name = UALLOC(char, strlen(dir) + 3 + 1);
	strcpy(phase_file_name, dir);
	strcat(phase_file_name, ".ph");

	FILE *fp = fopen(phase_file_name, "r");
	free(phase_file_name);

	if ( fp ) {
		char ph[21];
		while ( fscanf(fp, "%20s", ph) == 1 ) {
			if ( !phases_num ) {
				phases = (const char **)UALLOC(char*, phases_num + 1);
			}
			else {
				phases = (const char **)UREALLOC(phases, char*, phases_num + 1);
			}

			char *tgt = UALLOC(char, strlen(ph) + 1);
			strcpy(tgt, ph);
			phases[phases_num] = tgt;
			++phases_num;
		}
		fclose(fp);
	}
	else {
		phases = (const char **)UALLOC(char*, default_phases_num);
		if ( !phases ) {
			return LOCSAT_TTerror6;
		}

		for ( int i = 0; i < default_phases_num; ++i ) {
			phases[i] = STRALLOC(default_phases[i]);
			++phases_num;
		}
	}

	return setup_tttables(ttt, dir, phases, phases_num, verbose);
}


int sc_locsat_locate_event(
	LOCSAT_TTT *ttt,
	LOCSAT_Site *sites, int num_sites,
	LOCSAT_Arrival *arrival, LOCSAT_Assoc *assoc,
	LOCSAT_Origin *origin, LOCSAT_Origerr *origerr,
	LOCSAT_Params *params, LOCSAT_Errors *errors,
	int num_obs
) {
	int num_data;

	float lat, lon, depth, torg;
	float semi_major_axis, semi_minor_axis, strike;
	float sxx, syy, szz, stt;
	float sxy, sxz, syz;
	float stx, sty, stz;
	float sighat, snssd;
	int ndf;
	float depth_error, origin_time_error;
	int niter;
	int ierr;

	float lat_init, lon_init, depth_init;

	int i, j, k;
	int max_data;
	double time_offset;
	int error_found = FALSE;

	sc_locsat_setup_tttables(ttt, params->prefix, 0);

	/* Check if travel-time tables have been read */
	if ( !ttt->num_phases ) {
		return LOCSAT_TTerror1;
	}

	// sta_cor_level = locator_params->cor_level;

	/* Check input */

	if ( (num_obs == 0) || (!arrival) ) {
		/* It is not an error to simply initialize */
		return LOCSAT_GLerror7;
	}
	if ( !assoc ) {
		return LOCSAT_GLerror8;
	}
	else if ( !origin ) {
		return LOCSAT_GLerror9;
	}
	else if ( !origerr ) {
		return LOCSAT_GLerror10;
	}

	/* Check alignment of arrival/assoc pointers */

	for ( i = 0; i < num_obs; i++ ) {
		if ( arrival[i].arid != assoc[i].arid ) {
			return LOCSAT_GLerror11;
		}
	}

	// Use current lat/lon/depth as initial guess.
	lat_init = origin->lat;
	lon_init = origin->lon;
	depth_init = origin->depth;

	// If the use location flag is zero than set the lat and lon
	// init values to something locsat0 knows to ignore
	if ( !(params->use_location) ) {
		lon_init = -999.0;
		lat_init = -999.0;
	}

	if ( params->fix_depth == 'y' ) {
		depth_init = params->fixing_depth;
	}

	// Allocate a bunch of space for data
	max_data = 3 * num_obs;

	LOCSAT_Data *data = (LOCSAT_Data*)malloc(sizeof(LOCSAT_Data) * max_data);

	for ( i = 0; (!VALID_TIME(arrival[i].time)) && i < num_obs; i++ );

	if ( i < num_obs ) { /* Find offset time */
		time_offset = arrival[i].time;
	}
	else {
		time_offset = 0.0;
	}

	for ( i = 0; i < num_obs; i++ ) {
		if ( VALID_TIME(arrival[i].time) ) {
			time_offset = MIN(time_offset, arrival[i].time);
		}
	}

	// Extract the observations from the arrival/assoc structures
	// Assume that these are stored in same order in both structures
	for ( i = 0, num_data = 0; i < num_obs; i++ ) {
		{
			memcpy(data[num_data].sta, arrival[i].sta, sizeof(((LOCSAT_Data*)0)->sta));
			data[num_data].type = 't';
			data[num_data].defining = assoc[i].timedef;
			memcpy(data[num_data].phase_type, assoc[i].phase, sizeof(((LOCSAT_Data*)0)->phase_type));

			data[num_data].obs = (float)(arrival[i].time - time_offset);
			data[num_data].std_err = arrival[i].deltim;
			data[num_data].obs_data_index = i;
			num_data++;
		}
		if ( VALID_SEAZ(arrival[i].azimuth) ) {
			memcpy(data[num_data].sta, arrival[i].sta, sizeof(((LOCSAT_Data*)0)->sta));
			data[num_data].type = 'a';
			data[num_data].defining = assoc[i].azdef;
			memcpy(data[num_data].phase_type, assoc[i].phase, sizeof(((LOCSAT_Data*)0)->phase_type));

			data[num_data].obs = arrival[i].azimuth;
			data[num_data].std_err = arrival[i].delaz;
			data[num_data].obs_data_index = i;
			num_data++;
		}
		if ( VALID_SLOW(arrival[i].slow) ) {
			memcpy(data[num_data].sta, arrival[i].sta, sizeof(((LOCSAT_Data*)0)->sta));
			data[num_data].type = 's';
			data[num_data].defining = assoc[i].slodef;
			memcpy(data[num_data].phase_type, assoc[i].phase, sizeof(((LOCSAT_Data*)0)->phase_type));

			data[num_data].obs = arrival[i].slow;
			data[num_data].std_err = arrival[i].delslo;
			data[num_data].obs_data_index = i;
			num_data++;
		}
	}

	sc_locsat(
		ttt, data, num_data, sites, num_sites,
		lat_init, lon_init,
		depth_init, params->est_std_error, params->num_dof,
		params->conf_level, params->damp,
		params->max_iterations, params->fix_depth, &lat, &lon,
		&depth, &torg, &sighat, &snssd, &ndf, &semi_major_axis,
		&semi_minor_axis, &strike, &depth_error, &origin_time_error, &sxx, &syy,
		&szz, &stt, &sxy, &sxz, &syz, &stx, &sty, &stz,
		&niter, &ierr
	);

	// Check the return codes from locsat
	if ( ierr == 0 || ierr == 4 ) {
		// Fill in origin/origerr structure
		origin->lat = lat;
		origin->lon = lon;
		origin->depth = depth;

		origerr->sdobs = sighat;
		origerr->sxx = sxx;
		origerr->syy = syy;
		origerr->szz = szz;
		origerr->sxy = sxy;
		origerr->sxz = sxz;
		origerr->syz = syz;
		origerr->stt = stt;
		origerr->stx = stx;
		origerr->sty = sty;
		origerr->stz = stz;
		origerr->smajax = semi_major_axis;
		origerr->sminax = semi_minor_axis;
		origerr->stime = origin_time_error;
		origerr->sdepth = depth_error;

		if ( strike < STRIKE_MIN && strike != STRIKE_NULL ) {
			while ( strike < STRIKE_MIN ) {
				strike += 180;
			}
		}

		origerr->strike = strike;

		// Special case for unconstained origin time
		if ( ierr == 4 ) {
			origin->time = (double)NULL_TIME;
			origerr->stt = stt;
			origerr->stx = stx;
			origerr->sty = sty;
			origerr->stz = stz;
		}
		else if ( ierr == 0 ) {
			origin->time = (double)torg + time_offset;
		}

		/*
		 * obs_data_index is an array of indexes to the assoc
		 * tuple with a maximum length of 3 (time,slow,az) *
		 * the number of assoc records.  data_err_code is an array,
		 * parallel with obs_data_index, returned by locsat0,
		 * which contains error numbers corresponding to an
		 * error_table in the Locator GUI.  data_type is a string
		 * of maximum length 3 * number of assocs (with blank
		 * spaces between each letter) indicating the data
		 * type, t-time, a-az, and s-slow.  data_type and
		 * obs_data_index are matched pairs.  The locator_error
		 * structure is allocated and initialized to zero
		 * in the Locator GUI.  If there is an error code in
		 * data_err_code the appropriate type (time,slow,az) receives
		 * the error code and the arid is assigned to the
		 * arid field of locator_errors.  Otherwise the values
		 * remain 0.  Later the Locator GUI prints out errors
		 * corresponding to the error type for each data type.
		 */
		for ( i = 0; i < num_data; i++ ) {
			j = data[i].obs_data_index;

			if ( data[i].type == 't' ) {
				if ( ierr != 4 ) {
					assoc[j].timeres = data[i].residual;
					errors[j].time = data[i].err_code;
					if ( data[i].err_code != 0 ) {
						error_found = TRUE;
						assoc[j].timedef = 'n';
					}
				}
				assoc[j].azres = -999.0;
				assoc[j].slores = -999.0;
			}

			else if ( data[i].type == 'a' ) {
				assoc[j].azres = data[i].residual;
				errors[j].az = data[i].err_code;
				if ( data[i].err_code != 0 ) {
					error_found = TRUE;
					assoc[j].azdef = 'n';
				}
			}

			else if ( data[i].type == 's' ) {
				assoc[j].slores = data[i].residual;
				errors[j].slow = data[i].err_code;
				if ( data[i].err_code != 0 ) {
					error_found = TRUE;
					assoc[j].slodef = 'n';
				}
			}
			// If there's been an error fill-in the arid
			if ( error_found ) {
				errors[j].arid = assoc[j].arid;
				error_found = FALSE;
			}
		}

		// Try (!!) to get the del's, azi's, and baz's
		for ( i = 0; i < num_data; i++ ) {
			j = data[i].obs_data_index;
			k = data[i].sta_index;
			assoc[j].delta = sites[k].distance;
			assoc[j].seaz = sites[k].backazimuth;
			assoc[j].esaz = sites[k].azimuth;
		}

		// Count number of defining phases
		for ( i = 0, j = 0; i < num_obs; i++ ) {
			if ( assoc[i].timedef == 'd' ) {
				j++;
			}
		}
		origin->ndef = j;
		origin->nass = i;
	}

	UFREE(data);

	return ierr;
}


int sc_locsat_find_phase(const LOCSAT_TTT *ttt, const char *phase) {
	int i;

	if ( !phase || !*phase ) {
		return ERR;
	}

	if ( strlen(phase) >= sizeof(((LOCSAT_Assoc*)0)->phase) ) {
		return ERR;
	}

	for ( i = 0; i < ttt->num_phases; i++ ) {
		if ( ttt->phases[i] &&
		     !strncmp(phase, ttt->phases[i], sizeof(((LOCSAT_Assoc*)0)->phase)) ) {
			break;
		}
	}

	if ( i < ttt->num_phases ) {
		return i;
	}
	else {
		return ERR;
	}
}


double sc_locsat_compute_ttime(
	const LOCSAT_TTT *ttt,
	double distance, double depth, const char *phase, int extrapolate,
	double *rdtdd, double *rdtdh, int *errorflag
) {
	int ileft, jz, nz;
	float zfoc = depth;
	float bad_sample = -1.0;
	float tcal; // Travel time to return
	float delta, dtddel, dtdz, dcross;
	int iext, jext, ibad; // Errors from holint2
	int phase_id;

	phase_id = sc_locsat_find_phase(ttt, phase);
	if ( phase_id < 0 ) {
		return -1.0;
	}

	if ( ttt->ntbz[phase_id] <= 0 ) {
		return -1.0;
	}

	delta = distance;

	sc_locsat_brack(ttt->ntbz[phase_id], &ttt->tbz[phase_id * ttt->lentbz], zfoc, &ileft);

	jz = ((ileft - 1) > 1) ? (ileft - 1) : 1;
	nz = (((ileft + 2) < ttt->ntbz[phase_id]) ? (ileft + 2) : ttt->ntbz[phase_id]) - jz + 1;

	// Indexing into two and three dimensional arrays:
	//
	// &tbz[phase_id][jz - 1] == &tbz[phase_id * maxtbz + jz - 1]
	//
	// &tbtt[phase_id][jz - 1][0] ==
	// &tbtt[(phase_id * maxtbz * maxtbd) + ((jz -1) * maxtbd)+0]

	sc_locsat_holint2(
		extrapolate, ttt->ntbd[phase_id], nz,
		&ttt->tbd[phase_id * ttt->lentbd],
		&ttt->tbz[phase_id * ttt->lentbz + jz - 1],
		&ttt->tbtt[(phase_id * ttt->lentbd * ttt->lentbz) + ((jz - 1) * ttt->lentbd)],
		ttt->lentbd, bad_sample, delta, zfoc,
		&tcal, &dtddel, &dtdz, &dcross, &iext, &jext, &ibad
	);

	*errorflag = 0;
	if ( iext != 0 || jext != 0 || ibad != 0 ) {
		*errorflag = 1;
	}

	if ( ibad == 0 ) {
		if ( rdtdd ) {
			*rdtdd = dtddel;
		}
		if ( rdtdh ) {
			*rdtdh = dtdz;
		}
	}
	else {
		if ( rdtdd ) {
			*rdtdd = 0;
		}
		if ( rdtdh ) {
			*rdtdh = 0;
		}
	}

	return tcal;
}
