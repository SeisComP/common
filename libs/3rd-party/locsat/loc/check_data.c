#include <stdio.h>
#include <string.h>
#include "loc.h"


void sc_locsat_check_data(
	LOCSAT_Data *data, const int num_data,
	const LOCSAT_Site *stations, const int num_sta,
	const char * const *phase_type, const int num_phase_types
) {
	const int sta_len = sizeof(((LOCSAT_Data*)0)->sta);
	const int phase_len = sizeof(((LOCSAT_Data*)0)->phase_type);
	int i, j, n, len_d_p_t, len_pt, longer;

	for ( n = 0; n < num_data; n++ ) {
		data[n].sta_index = -1;
		data[n].ipwav = -1;
		data[n].idtyp = 0;

		/* Check that the datum's station is valid */

		for ( i = 0; i < num_sta; i++ ) {
			if ( !strncmp(data[n].sta, stations[i].sta, sta_len) ) {
				data[n].err_code = 0;
				data[n].sta_index = i;

				for ( len_d_p_t = 0; len_d_p_t < phase_len; len_d_p_t++ ) {
					if ( data[n].phase_type[len_d_p_t] == ' ' ) {
						break;
					}
				}

				for ( j = 0; j < num_phase_types; j++ ) {
					len_pt = strlen(phase_type[j]);

					if ( len_d_p_t > len_pt ) {
						longer = len_d_p_t;
					}
					else {
						longer = len_pt;
					}

					if ( !strncmp(data[n].phase_type, phase_type[j], longer) ) {
						data[n].ipwav = j;

						/* Check for valid datum type */

						if ( data[n].type == 't' ) {
							data[n].idtyp = 1;
						}
						else if ( data[n].type == 'a' ) {
							data[n].idtyp = 2;
						}
						else if ( data[n].type == 's' ) {
							data[n].idtyp = 3;
						}
						else {
							data[n].idtyp = 0;
							data[n].err_code = 3;
						}
						if ( data[n].std_err < 0.0 ) {
							data[n].err_code = 4;
						}
						goto done;
					}
				}

				/* The phase ID is invalid ! */

				if ( data[n].type == 't' ) {
					data[n].idtyp = 1;
					data[n].err_code = 2;
					goto done;
				}
				else if ( data[n].type == 's' ) {
					data[n].idtyp = 3;
					data[n].err_code = 2;
				}
				else if ( data[n].type == 'a' ) {
					data[n].idtyp = 2;
				}
				else {
					data[n].idtyp = 0;
					data[n].err_code = 3;
				}
				/* Check that datum's a priori
				   standard deviation is vaild */

				if ( data[n].std_err < 0.0 ) {
					data[n].err_code = 4;
				}
				goto done;
			}
			else {
				data[n].err_code = 1;
			}
		}
done:;
	}
}
