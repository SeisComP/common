#include <locsat/geog.h>
#include <locsat/loc.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>


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
) {
	int i__1;
	double d__1, d__2;

	double andf, sgh12, sgh23, dxn12, dxn23, dist, xold[4];
	double step = 0.0, xsol[4], slwt;
	float azwt;
	int ntoodeep;
	double cnvghats[3], snssdden, alat2, alon2;
	int ierr0;
	double snssdnum;
	int i, k, m, n;
	double dmean, scale, delta;
	float dcalx;
	double cnvg12;
	double cnvg23;
	double covar[4][4];
	boolean divrg;
	double hyrak;
	int inerr, iterr;
	double dxmax, a1;
	int nairquake;
	double a2, hyplu;
	boolean cnvrg;
	double hystr, epmaj0, wtrms;
	double epmin0, hymaj0, hymid0;
	double hymin0, zfint0;
	double fs;
	int np;
	double condit[2];
	double sghats[3];
	char fxdsav = 0;
	int ntimes, nazims;
	double dxnorm;
	double dxnrms[3];
	int nslows;
	double fac;
	double azi;
	int nds[3];
	double atx[4], ssq, cnvgold = 0.0;
	int ndftemp;
	float correct;
	boolean ldenuis;
	double cnvgtst, sta3, sta4, sta5;

	*alat = alat0;
	*alon = alon0;
	*zfoc = zfoc0;
	*torg = (float)0.;
	*sighat = (float)-1.;
	*snssd = (float)-1.;
	*ndf = -1;
	*epmaj = (float)-1.;
	*epmin = (float)-1.;
	*epstr = (float)-1.;
	*zfint = (float)-1.;
	*toint = (float)-1.;
	*sxx = (float)-999.;
	*syy = (float)-999.;
	*szz = (float)-999.;
	*stt = (float)-999.;
	*sxy = (float)-999.;
	*sxz = (float)-999.;
	*syz = (float)-999.;
	*stx = (float)-999.;
	*sty = (float)-999.;
	*stz = (float)-999.;
	slwt = (float)1.;
	azwt = (float)1.;
	cnvrg = FALSE;
	ldenuis = FALSE;
	*niter = 0;
	*ierr = 0;
	ierr0 = 0;
	nairquake = 0;
	ntoodeep = 0;

L1020:
	for ( n = 0; n < ndata; ++n ) {
		data[n].residual = -999.f;
		data[n].resid2 = -999.f;
		data[n].at[0] = 0.f;
		data[n].at[1] = 0.f;
		data[n].at[2] = 0.f;
		data[n].at[3] = 0.f;
	}

	// Set fix-depth flag and number of parameters.  Depth is always fixed
	// during the first 2 iterations.  If depth becomes negative
	// ("airquake"), then fix the depth at 0.0 during the next iteration.
	// If several airquakes occur, then fix the depth at zero for all subsequent
	// iterations. Also fix events > 650.0 km to value 650.0 during the next
	// iteration, i.e., to the approximate depth of the deepest credible
	// earthquake -- WCN.

	// !!!MODIFICATION!!! Mathias, 2008.267 */
	//  max depth: 650 --> 750 km */
	if ( *niter < 3 ) {
		fxdsav = 'y';
	}
	else if ( nairquake > 4 ) {
		fxdsav = 'y';
		*zfoc = (float)0.;
		xsol[3] = (float)0.;
	}
	else if ( ntoodeep > 4 ) {
		fxdsav = 'y';
		*zfoc = (float)750.;
		xsol[3] = (float)0.;
	}
	else if ( *zfoc < (float)0. ) {
		++nairquake;
		*zfoc = (float)0.;
		xsol[3] = (float)0.;
	}
	else if ( *zfoc > (float)750. ) {
		++ntoodeep;
		*zfoc = (float)750.;
		xsol[3] = (float)0.;
	}
	else {
		fxdsav = fxdflg;
	}
	// How many model parameters?
	if ( fxdsav != 'y' ) {
		np = 4;
	}
	else {
		np = 3;
	}

	// Compute distance and azimuths to stations (forward problem for azimuths)
	for ( i = 0; i < nsta; ++i ) {
		sc_locsat_distaz2(*alat, *alon, stations[i].lat, stations[i].lon, &sta3, &sta4, &sta5);
		stations[i].azimuth = sta4;
		stations[i].distance = sta3;
		stations[i].backazimuth = sta5;
	}

	// Compute travel-times, slownesses and azimuths based on current
	// location hypothesis and determine partial derivatives.  Ignore
	// points with completely invalid data (i.e., iderr = 1, 2, 3).
	*nd = 0;
	ntimes = 0;
	nazims = 0;
	nslows = 0;

	for ( n = 0; n < ndata; ++n ) {
		if ( (data[n].err_code < 1) || (data[n].err_code > 3) ) {
			i = data[n].sta_index;
			k = data[n].ipwav;

			if ( data[n].idtyp == 1 ) {
				// Arrival times
				sc_locsat_ttcal(
					*zfoc, radius, stations[i].distance, stations[i].azimuth,
					ttt->lentbd, ttt->lentbz, &ttt->ntbd[k], &ttt->ntbz[k], &ttt->tbd[k * ttt->lentbd],
					&ttt->tbz[k * ttt->lentbz],
					&ttt->tbtt[(k * ttt->lentbz) * ttt->lentbd],
					&dcalx, atx, &iterr
				);

				// Ellipticity corrections for travel times including the
				// ellipticity corrections of the IASPEI 1991 Travel Time
				// Tables (Kennet, 1991).
				dcalx += sc_locsat_elpcor(data[n].phase_type,
				                          stations[i].distance, *zfoc, stations[i].azimuth,
				                          90.f - *alat);

				// Use only those data that have been interpolated (iterr = 0)
				// or have been extrapolated to depths beyond these curves
				// (iterr = 15). If the number of iterations is less than
				// minit, allow all extrapolated values.
				if ( *niter < 4 || iterr == 15 ) {
					data[n].err_code = 0;
				}
				else {
					data[n].err_code = iterr;
				}
			}
			else if ( data[n].idtyp == 2 ) {
				// Azimuths
				sc_locsat_azcal(radius, stations[i].distance, stations[i].azimuth,
				                stations[i].backazimuth, &dcalx, atx);
			}
			else if ( data[n].idtyp == 3 ) {
				// Slownesses
				sc_locsat_slocal(
					*zfoc, radius, stations[i].distance, stations[i].azimuth,
					ttt->lentbd, ttt->lentbz, &ttt->ntbd[k], &ttt->ntbz[k], &ttt->tbd[k * ttt->lentbd],
					&ttt->tbz[k * ttt->lentbz],
					&ttt->tbtt[(k * ttt->lentbz) * ttt->lentbd], &dcalx, atx,
					&iterr
				);

				// Same rules as for travel-time calculations.
				if ( *niter < 4 || iterr == 15 ) {
					data[n].err_code = 0;
				}
				else {
					data[n].err_code = iterr;
				}
			}

			// Apply station correction adjustments, if necessary
			correct = (float)0.;
			/*           if (niter.gt.2 .and. idtyp(n).eq.1) */
			/*    &         call ssscor (alat, alon, correct, 1, i, k) */
			/*           Compute residual = [observed - calculated] datum */
			/*                              + station correction */
			if ( data[n].idtyp != 1 ) {
				data[n].residual = data[n].obs - dcalx;
			}
			else {
				data[n].residual = data[n].obs - dcalx - *torg + correct;
			}

			// If the azimuth residual is > +/- 180.0 deg., change it
			// to the corresponding difference that is < +/- 180.0 deg.
			if ( (data[n].idtyp == 2) && (abs(data[n].residual) > 180.f) ) {
				data[n].residual = -dsign(360.f - abs(data[n].residual), data[n].residual);
			}

			// Load valid data and partials for defining detections
			// into arrays. Note that parameters are ordered: origin-time;
			// longitude; latitude; depth.  If depth is fixed, np = 3.
			if ( data[n].err_code < 1 && data[n].defining == 'd' ) {
				for ( m = 0; m < np; ++m ) {
					data[*nd].at[m] = atx[m];
				}
				// Array ip0 holds the original index of the n'th valid datum
				data[*nd].ip0 = n;
				data[*nd].resid2 = data[n].residual;
				data[*nd].dsd2 = data[n].std_err;
				data[*nd].idtyp2 = data[n].idtyp;
				/* Count valid data for each data type */
				if ( data[*nd].idtyp2 == 1 ) {
					++ntimes;
				}
				else if ( data[*nd].idtyp2 == 2 ) {
					++nazims;
				}
				else if ( data[*nd].idtyp2 == 3 ) {
					++nslows;
				}
				++(*nd);
			}
		}
	}

	// Quick check on array declarations
	if ( np > 4 || *nd > 9999 ) {
		return;
	}

	// Check for insufficient data
	if ( fxdsav != 'y' ) {
		if ( *nd < 4 ) {
			*ierr = 5;
			return;
		}
	}
	else {
		if ( *nd < 3 ) {
			*ierr = 5;
			return;
		}
	}

	// If initial iteration, then orthogonalize out origin-time term */
	if ( !ldenuis ) {
		sc_locsat_denuis(data, *nd, np, &dmean, &inerr);
		*torg = dmean;
		if ( inerr != 0 ) {
			*torg = (float)0.;
		}
		ldenuis = TRUE;
		goto L1020;
	}

	// Compute weighted and unweighted RMS residual (dimensionless quantities).
	// Also normalize matrix and residuals w.r.t. data standard deviations
	// and apply weights to azimuth and slowness data, as necessary.
	wtrms = (float)0.;
	for ( n = 0; n < *nd; ++n ) {
		data[n].resid3 = data[n].resid2;

		if ( data[n].idtyp2 == 1 ) {
			data[n].resid2 /= data[n].dsd2;
		}
		else if ( data[n].idtyp2 == 2 ) {
			data[n].resid2 = azwt * data[n].resid2 / data[n].dsd2;
		}
		else if ( data[n].idtyp2 == 3 ) {
			data[n].resid2 = slwt * data[n].resid2 / data[n].dsd2;
		}

		// Computing 2nd power
		d__1 = data[n].resid2;
		wtrms += d__1 * d__1;

		for ( m = 0; m < np; ++m ) {
			if ( data[n].idtyp2 == 1 ) {
				data[n].at[m] /= data[n].dsd2;
			}
			else if ( data[n].idtyp2 == 2 ) {
				data[n].at[m] = azwt * data[n].at[m] / data[n].dsd2;
			}
			else if ( data[n].idtyp2 == 3 ) {
				data[n].at[m] = slwt * data[n].at[m] / data[n].dsd2;
			}
		}
	}
	wtrms = sqrt(wtrms / *nd);
	if ( cnvrg ) {
		goto L1200;
	}
	// Determine least squares solution
	sc_locsat_solve_via_svd(
		data, *nd, np, 1, damp, &cnvgtst, condit, xsol, covar, rank, ierr
	);
	if ( *ierr == 6 ) {
		return;
	}
	// Compute number of degrees of freedom and data-variance estimate
	// ndf0 is the K of Jordan and Sverdrup (1981); Bratt and Bache (19
	// set ndf0 = 8; here we set ndf0 = 9999
	// sig0 is "not" the s-sub-k of Jordan and Sverdrup (1981);
	//      here, sig0 = 1.0
	// ndf  is the total degrees of freedom assuming a chi-squared
	//      distribution = ndf0 + [# of data + # of parameters]
	// ssq  is the numerator for the a posteriori estimate for the
	//      squared variance scale factor
	// sighat, is therefore, the actual estimate of the variance scale
	// factor (eqn. 34 of J&S, 1981), and subsequently,
	// snssd is the normalized a priori estimate for the estimated
	//      variance scale factor
	*ndf = ndf0 + *nd - np;
	ssq = ndf0 * sig0 * sig0;
	i__1 = *nd;
	for ( n = 0; n < i__1; ++n ) {
		// Computing 2nd power
		d__1 = data[n].resid2;
		ssq += d__1 * d__1;
	}
	andf = (double)(*ndf);
	if ( *ndf == 0 ) {
		andf = (float).001;
	}
	if ( (d__1 = (float)(*ndf) - ssq, abs(d__1)) < (float)1e-5 ) {
		andf = ssq;
	}
	*sighat = sqrt(ssq / andf);
	snssdnum = ssq - ndf0 * sig0 * sig0;
	snssdden = andf - ndf0;
	if ( abs(snssdden) > (float).001 && snssdnum / snssdden >= (float)0. ) {
		*snssd = sqrt(snssdnum / snssdden);
	}
	else {
		*snssd = (float)999.;
	}

	// Compute norm of hypocenter perturbations
	ssq = (float)0.;
	for ( m = 0; m < np; ++m ) {
		// Computing 2nd power
		ssq += xsol[m] * xsol[m];
	}
	dxnorm = sqrt(ssq);
	// Scale down hypocenter perturbations if they are very large.  Scale
	// down even more for lat(t)er iterations.
	dxmax = (float)1500.;
	if ( *niter < maxit / 5 + 1 ) {
		dxmax = (float)3e3;
	}
	if ( dxnorm > dxmax ) {
		scale = dxmax / dxnorm;
		for ( m = 0; m < np; ++m ) {
			xsol[m] *= scale;
		}
		dxnorm = dxmax;
	}
	// Store the convergence test information from the 2 previous iterations
	// Computing MIN
	for ( i = min(2, *niter); i >= 1; --i ) {
		cnvghats[i] = cnvghats[i - 1];
		sghats[i] = sghats[i - 1];
		dxnrms[i] = dxnrms[i - 1];
		nds[i] = nds[i - 1];
	}

	// Current convergence test information
	cnvghats[0] = cnvgtst;
	sghats[0] = *snssd;
	dxnrms[0] = dxnorm;
	nds[0] = *nd;

	// Stop iterations if number of data < number of parameters. The
	// exception is when the depth is fixed (np = 3) and we have only
	// azimuth, or one azimuth and one slowness data.  In that case
	// continue on even though it will be impossible to get an origin time.
	ndftemp = ndf0;
	if ( np == 3 && (nazims > 1 || (nazims > 0 && nslows > 0)) ) {
		ndftemp = ndf0 - 1;
	}

	// Convergence, divergence or just keep on iterating
	if ( *ndf < ndftemp ) {
		divrg = TRUE;
		cnvrg = FALSE;
		ierr0 = 1;
		*ierr = 2;
	}
	else if ( *niter < 4 ) {
		divrg = FALSE;
		cnvrg = FALSE;
	}
	else {
		if ( dxnorm > (float)0. ) {
			cnvg12 = cnvghats[0] / cnvghats[1];
			cnvg23 = cnvghats[1] / cnvghats[2];
			sgh12 = sghats[0] / sghats[1];
			sgh23 = sghats[1] / sghats[2];
			dxn12 = dxnrms[0] / dxnrms[1];
			dxn23 = dxnrms[1] / dxnrms[2];
			divrg =
			    ((sgh23 > (float)1.1 && sgh12 > sgh23) ||
			     (dxn23 > (float)1.1 && dxn12 > dxn23 && *niter > 6 &&
			      dxnorm > (float)1e3));
			cnvrg = nds[0] == nds[1] && !divrg &&
			        (sgh12 > (float).99 && sgh12 < (float)1.001) &&
			        (cnvgtst < 1e-8 || dxnorm < (float).5);
			if ( (cnvgtst < cnvgold * (float)1.01 && cnvgtst < 1e-8) ||
			     (*niter > maxit * 3 / 4 &&
			      (cnvgtst < sqrt(1e-8) ||
			       (d__1 = cnvg23 - cnvg12, abs(d__1)) < 1e-8 ||
			       (d__2 = cnvghats[0] - cnvghats[2], abs(d__2)) < (float)1e-5)
			     ) ) {
				cnvrg = TRUE;
			}
			if ( (wtrms < (float).001 || dxnrms[0] < (float).001) &&
			     *niter > 6 ) {
				cnvrg = TRUE;
			}
		}
		else {
			divrg = FALSE;
			cnvrg = TRUE;
		}
	}
	// Apply step-length weighting, if unweighted RMS residual is increasing
	if ( *niter > 6 &&
	     (cnvgtst > cnvgold || cnvghats[0] - cnvghats[2] == (float)0.) &&
	     step > (float).05 ) {
		step *= (float).5;
		if ( step != (float).5 ) {
			for ( i = 0; i < np; ++i ) {
				xsol[i] = step * xold[i];
			}
		}
		else {
			for ( i = 0; i < np; ++i ) {
				xsol[i] = step * xsol[i];
				xold[i] = xsol[i];
			}
		}
	}
	else {
		step = (float)1.;
		cnvgold = cnvgtst;
	}

	// Perturb hypocenter
	if ( xsol[1] != (float)0. || xsol[2] != (float)0. ) {
		azi = atan2(xsol[1], xsol[2]) * 57.2957795;
		// Computing 2nd power
		d__1 = xsol[1];
		// Computing 2nd power
		d__2 = xsol[2];
		dist = sqrt(d__1 * d__1 + d__2 * d__2);
		delta = dist / (radius - *zfoc) * 57.2957795;
		a1 = *alat;
		a2 = *alon;
		sc_locsat_latlon2(a1, a2, delta, azi, &alat2, &alon2);
		*alat = alat2;
		*alon = alon2;
	}
	*torg += xsol[0];
	if ( fxdsav != 'y' ) {
		*zfoc -= xsol[3];
	}

	// End of main iterative loop
	if ( cnvrg ) {
		*ierr = 0;
		if ( condit[0] > 1e4 ) {
			*ierr = 5;
			return;
		}
		goto L1020;
	}
	else if ( divrg ) {
		*ierr = 2;
		if ( ierr0 == 1 ) {
			*ierr = 3;
		}
		goto L1210;
	}
	else if ( *niter >= maxit ) {
		*ierr = 1;
	}
	else {
		++(*niter);
		goto L1020;
	}

	// Compute confidence regions
L1200:
	sc_locsat_solve_via_svd(
		data, *nd, np, 2, damp, &cnvgtst, condit, xsol, covar, rank, ierr
	);
	if ( *ierr == 6 ) {
		return;
	}
	// Compute location confidence bounds
	sc_locsat_ellips(
	    &np, covar, &hymaj0, &hymid0, &hymin0, &hystr, &hyplu, &hyrak, &epmaj0,
	    &epmin0, epstr, &zfint0, stt, stx, sty, sxx, sxy, syy, stz, sxz, syz,
	    szz
	);
	// Not currently used, so commented out (WCN)
	// call fstatx (3, ndf, pconf, fs)
	// fac   = dsqrt(3.0*fs)*sighat
	// hymaj = hymaj0*fac
	// hymid = hymid0*fac
	// hymin = hymin0*fac
	sc_locsat_fstatx(2, *ndf, pconf, &fs);
	fac = sqrt(fs * (float)2.) * *sighat;
	*epmaj = epmaj0 * fac;
	*epmin = epmin0 * fac;
	sc_locsat_fstatx(1, *ndf, pconf, &fs);
	fac = sqrt(fs) * *sighat;
	*zfint = zfint0 * fac;
	// szz   = zfint0*zfint0
	if ( *stt < (float)0. || ntimes < 1 ) {
		*toint = (float)-999.;
	}
	else {
		*toint = sqrt(*stt) * fac;
	}
	// Remove weights and standard deviations from residuals
L1210:
	i__1 = *nd;
	for ( n = 0; n < i__1; ++n ) {
		if ( data[n].idtyp2 == 1 ) {
			data[n].resid2 *= data[n].dsd2;
		}
		else if ( data[n].idtyp2 == 2 ) {
			data[n].resid2 = data[n].resid2 * data[n].dsd2 / azwt;
		}
		else if ( data[n].idtyp2 == 3 ) {
			data[n].resid2 = data[n].resid2 * data[n].dsd2 / slwt;
		}
	}

	// Place input values back into original arrays
	for ( n = *nd - 1; n >= 0; --n ) {
		data[data[n].ip0].residual = data[n].resid2;
		data[data[n].ip0].epimp = data[n].epimp;
	}

	// Don't return a depth value < 0.0
	if ( *zfoc < (float)0. ) {
		*zfoc = (float)0.;
	}

	// Correct non-defining arrival times for the origin time and then
	// load the default values into arrays for the erroneous data
	for ( n = 0; n < ndata; ++n ) {
		if ( (data[n].err_code > 0 && data[n].err_code < 4) || data[n].err_code == 11 ) {
			data[n].residual = -999.f;
			data[n].epimp = -1.f;
			data[n].defining = 'n';
		}
		else if ( data[n].idtyp == 1 && (data[n].defining != 'd' || data[n].err_code > 0) ) {
			data[n].epimp = -1.f;
			data[n].defining = 'n';
		}
		else if ( data[n].err_code > 0 ) {
			data[n].epimp = -1.f;
			data[n].defining = 'n';
		}
		if ( data[n].defining != 'd' ) {
			data[n].epimp = -1.f;
		}
	}

	if ( *toint <= -888.f ) {
		*ierr = 4;
	}
}
