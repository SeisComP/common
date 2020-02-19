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


/*
 * Computes the semblance as measure of coherency
 * of the input records
 *
 * Algorithm after:
 *
 * Neidell, N.S. and Taner, M.T. (1971)
 * Semblance and oher coherency measures for
 * multichannel data
 * Geophysics, 36, pp. 482-497
 *
 * Another interesting paper on the topic is:
 * Kennett, B.L.N, (2000)
 * Stacking three-component seismograms
 * Geophys. J. Int., 141, pp. 263-269
 *
 * Implemented by:
 *      Joachim Saul
 *      GeoForschungsZentrum Potsdam
 *      Potsdam, Germany
 *      Email: saul@gfz-potsdam.de
 */

#include<stdlib.h>
#include<math.h>

namespace Seiscomp
{

namespace Filter
{

void	hilbert_TYPE(int ndata, TYPE *f, int direction);
int	next_power_of_2 (int n);

template<class TYPE>
void semblance (int nrec, int ndata, int nw, TYPE **f, TYPE *s)
/* Low-level semblance routine */
{
	int i, k, kk;
	TYPE *tmp, *gg, *gx;
	double norm = 1./nrec, ss, sx,
		*q = (double*) malloc(sizeof(double)*(nw+1));

	tmp = (TYPE*) calloc(2*ndata, sizeof(TYPE));
	gg  = tmp; gx = gg + ndata;

	if (nw<0) nw = 0;

	for (i=0; i<ndata; i++)
		gg[i] = gx[i] = 0.;

	for (k=0; k<nrec; k++) {
		TYPE *fk = f[k];

		for (i=0; i<ndata; i++) {
			TYPE fki = fk[i];

			gg[i] += fki;
			gx[i] += fki*fki;
		}
	}

	for (i=0; i<ndata; i++)
		gg[i] *= gg[i];

	/* Now `gg' corresponds to (SUM[over k](u_ks))^2
	 * and `gx' corresponds to  SUM[over k](u_ks^2)
	 * in Kennett (2000)
	 */
	for (kk=0; kk<=nw; kk++)
		q[kk] = (nw+1-kk) / (double)(nw+1);

	for (i=0; i<ndata; i++) {
		ss = sx = 0.;

		for (k=-nw; k<=nw; k++) {
			kk = k > 0 ? k : -k;

			if ((i+k)>=0 && (i+k)<ndata) {
				ss += q[kk]*gg[i+k];
				sx += q[kk]*gx[i+k];
			}
		}

		/* avoid division by zero */
		if (sx/ss < ss*1.E-20)
			s[i] = (TYPE) 0.;
		else	s[i] = (TYPE) (norm*ss/sx);
	}

	free(tmp); free(q);
}

template<class TYPE>
void semblance_c(int nrec, int ndata, int nw, TYPE **f, TYPE *s)
/* Low-level "complex" semblance routine */
{
	int i, k, kk;

	TYPE *tmp, *gg, *gh, *gx, *fh;
	double	norm = 1./nrec;
	double  ss, sx, *q = (double*) malloc(sizeof(double)*(nw+1));

	tmp = (TYPE*) calloc(4*ndata, sizeof(TYPE));
	gg  = tmp; gh = gg+ndata; gx = gh+ndata; fh = gx+ndata;
	
	if (nw<0) nw = 0;

	for (k=0; k<nrec; k++) {
		TYPE *fk = f[k];  /* pointer to the k-th record */
		TYPE fki, fhi;

		for (i=0; i<ndata; i++)
			fh[i] = fk[i];

			hilbert_TYPE(ndata, fh, 0);

		for (i=0; i<ndata; i++) {
			fki = fk[i];
			fhi = fh[i];

			gg[i] += fki;
			gh[i] += fhi;
			gx[i] += fki*fki + fhi*fhi;
		}
	}

	for (i=0; i<ndata; i++) {
		gg[i] *= gg[i];
		gh[i] *= gh[i];
	}

	/* Now `gg' corresponds to (SUM[over k](u_ks))^2
	 * and `gx' corresponds to  SUM[over k](u_ks^2)
	 * in Kennett (2000) */
	for (kk=0; kk<=nw; kk++)
		q[kk] = (nw+1-kk)/(double)(nw+1);

	for (i=0; i<ndata; i++) {
		ss = sx = 0.;

		for (k=-nw; k<=nw; k++) {
			kk = k > 0 ? k : -k;

			if ((i+k) >= 0 && (i+k) < ndata) {
				ss += q[kk]*(gg[i+k]+gh[i+k]);
				sx += q[kk]* gx[i+k];
			}
		}

		/* avoid division by zero */
		if (sx/ss < ss*1.E-20)
			s[i] = (TYPE) 0.;
		else	s[i] = (TYPE) (norm*ss/sx);
	}

	free(tmp); free(q);
}	

}       // namespace Seiscomp::Filter

}       // namespace Seiscomp
