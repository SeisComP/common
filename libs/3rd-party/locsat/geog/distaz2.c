#include <locsat/geog.h>
#include <math.h>


void sc_locsat_distaz2(double alat1, double alon1, double alat2, double alon2, double *delta, double *azi, double *baz) {
	double clat1, clat2, cdlon, cdel;
	double rlat1, rlat2, rdlon, slat1, slat2, sdlon;
	double xazi, xbaz, yazi, ybaz;

	/*  changed for ellipticity of earth
	 *   changed use of *alat1 and *alat2
	 */

	if ( (alat1 == alat2) && (alon1 == alon2) ) {
		*delta = 0.0;
		*azi = 0.0;
		*baz = 180.0;
		return;
	}

	double esq, alat3, alat4;

	esq = (1.0 - 1.0 / 298.25) * (1.0 - 1.0 / 298.25);
	alat3 = rad2deg(atan(tan(deg2rad(alat1)) * esq));
	alat4 = rad2deg(atan(tan(deg2rad(alat2)) * esq));

	rlat1 = deg2rad(alat3);
	rlat2 = deg2rad(alat4);
	rdlon = deg2rad(alon2 - alon1);

	clat1 = cos(rlat1);
	clat2 = cos(rlat2);
	slat1 = sin(rlat1);
	slat2 = sin(rlat2);
	cdlon = cos(rdlon);
	sdlon = sin(rdlon);

	cdel = slat1 * slat2 + clat1 * clat2 * cdlon;
	cdel = (cdel <  1.0) ? cdel :  1.0;
	cdel = (cdel > -1.0) ? cdel : -1.0;
	yazi = sdlon * clat2;
	xazi = clat1 * slat2 - slat1 * clat2 * cdlon;
	ybaz = -sdlon * clat1;
	xbaz = clat2 * slat1 - slat2 * clat1 * cdlon;

	*delta = rad2deg(acos(cdel));
	*azi   = rad2deg(atan2(yazi, xazi));
	*baz   = rad2deg(atan2(ybaz, xbaz));

	if ( *azi < 0.0 ) {
		*azi += 360.0;
	}
	if ( *baz < 0.0 ) {
		*baz += 360.0;
	}
}
