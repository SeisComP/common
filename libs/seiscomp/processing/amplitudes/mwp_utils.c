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


#include<math.h>
#include "mwp_utils.h"

void
Mwp_demean(int n, double *f, int i0)
{
	int i;
	double sum = 0, mean;

	for (i=0; i<i0; i++)
		sum += f[i];
	mean = sum/i0;

	for (i=0; i<n; i++)
		f[i] -= mean;
}

void
Mwp_taper(int n, double *f, int i0)
{
	int i, nn=i0/2;
	double q = M_PI/nn;
	for (i=0; i<nn; i++)
		f[i] *= 0.5*(1-cos(i*q));
}

static void
Mwp_integr(int n, double *f, int i0)
{
	int i;
	double sum = 0;

//	Mwp_demean(n, f, i0);

	for (i=0; i<n; i++) {
		sum += f[i];
		f[i] = sum;
	}
}


static void
Mwp_scale(int n, double *f, double factor)
{
	int i;

	for (i=0; i<n; i++) {
		f[i] *= factor;
	}
}

double
Mwp_SNR(int n, double *f, int i0)
{
	int i;
	double smax = 0, nmax = 0;

	for (i=0; i<i0; i++) {
		double n = fabs(f[i]);
		if (n > nmax)
			nmax = n;
	}
	for (i=i0; i<n; i++) {
		double s = fabs(f[i]);
		if (s > smax)
			smax = s;
	}

	return smax/nmax;
}


double
Mwp_amplitude(int n, double *f, int i0, int *pos)
{
	int i;
	double smax = 0;
	*pos = i0;

	for (i=i0; i<n; i++) {
		double s = fabs(f[i]);
		if (s > smax) {
			*pos = i;
			smax = s;
		}
	}

	return smax;
}


void
Mwp_double_integration(int n, double *f, int i0, double fsamp)
{
	Mwp_integr(n, f, i0);
	Mwp_integr(n, f, i0);
	Mwp_scale (n, f, 1/(fsamp*fsamp));
}
