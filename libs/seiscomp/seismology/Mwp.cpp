/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/



#include <math.h>
#include <seiscomp/seismology/magnitudes.h>

#define DELTA_MIN 5.
#define DELTA_MAX 105.

// degrees to radians factor
#define D2R 0.01745329251994329509
#define EARTH_RADIUS 6370998.685023

namespace Seiscomp {
namespace Magnitudes {

bool compute_Mwp(double amplitude, double delta, double &Mwp, double offset,double slope,double alpha,double rho,double fp)
{
	if ( (delta < DELTA_MIN) || (delta > DELTA_MAX) ) {
		return false;
	}

	double r=(D2R*delta)*EARTH_RADIUS; // convert delta to meters
	double momfac=4*M_PI*rho*pow(alpha,3.)*r/fp;
	double M0,Mw;

	M0=amplitude*momfac;
	Mw=(log10(M0)-9.1)/1.5;
	Mwp=(Mw-offset)/slope;

	return true;
}

}
}

