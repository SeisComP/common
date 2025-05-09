#include <locsat/loc.h>
#include <locsat/geog.h>

#include <math.h>
#include <stdint.h>
#include <string.h>


void sc_locsat_hypcut(
	LOCSAT_TTT *ttt,
	LOCSAT_Data *data, const int ndata,
	LOCSAT_Site *stations, const int nsta,
	float *alat0, float *alon0, int *ierr
) {
	// Taken from IASPEI Seismoint Tables of B.L.N. Kennett (1991)
	static const double distance[19] = {
		0.,   10.,  20.,  30.,  40.,  50.,  60.,
		70.,  80.,  90.,  100., 110., 120., 130.,
		140., 150., 160., 170., 180.
	};
	static const double slowness[19] = {
		19.17, 13.7, 10.9, 8.85, 8.3,  7.6,  6.88,
		6.15,  5.4,  4.66, 4.44, 1.96, 1.91, 1.88,
		1.79,  1.57, 1.14, .59,  .01
	};
	static const double sminusp[12] = {
		0.,     114.2,  226.76, 300.,   367.49, 432.64,
		494.45, 552.32, 605.82, 654.47, 695.75, 734.6
	};

	int i__1, i__2;
	double d1;

	double bestazcross;
	double sminusptime, torg = 0.0;
	double fmaxtime, delcross, smallest;
	int itimeyet;
	double crosslat[12];
	double crosslon[12], dist1 = 0.0;
	int isminusp;
	int i, j, k, n;
	double tcalc, delta;
	int icerr, iazim;
	double a1, a2;
	int i1 = 0, i2, ierrx, islow, n1, n2;
	double alat0x;
	double alon0x, dist1x, dist2x;
	int icompr, ic2;
	double azisav = 0.0;
	int icross;
	int i1s, i2s;
	double baz, azi, tmp;
	int iusesta;
	double useazim, res1 = 0.0, res2 = 0.0, sta1, sta2, sta3, sta4, useslow;

	// Travel time table
	const int *ntbd = ttt->ntbd;
	const float *tbd = ttt->tbd;
	const float *tbtt = ttt->tbtt;

	for ( i = 0; i < nsta; ++i ) {
		stations[i].azimsd = 40.;
		stations[i].slowsd = 19.16;
		stations[i].bestslow = (float)-888888.;
		stations[i].bestazim = (float)-888888.;
		stations[i].goodcompr = FALSE;
		stations[i].goodsminusp = FALSE;
		stations[i].goodazim = FALSE;
		stations[i].goodslow = FALSE;
	}

	*ierr = 0;
	itimeyet = 0;
	bestazcross = 80.;
	fmaxtime = (float)888888.;

	// Load valid P-wave and S-wave arrival times, P-slownesses and azimuths
	// (use P-type phase or phase with the smallest azimuth standard
	// deviation) at each station.
	for ( n = 0; n < ndata; ++n ) {
		LOCSAT_Data *d = data + n;
		if ( d->err_code == 0 && d->defining == 'd' ) {
			i = d->sta_index;
			LOCSAT_Site *sta = stations + i;

			// ---- Arrival times ----
			if ( d->idtyp == 1 ) {
				if ( itimeyet == 0 ) {
					fmaxtime = d->obs + (float)888888.;
					for ( j = 0; j < nsta; ++j ) {
						stations[j].comprtime = fmaxtime;
						stations[j].sheartime = fmaxtime;
					}
					itimeyet = 1;
				}

				// Load valid S and P times into arrays for each i'th station
				if ( (!strcmp(d->phase_type, "P") ||
				      !strcmp(d->phase_type, "Pn") ||
				      !strcmp(d->phase_type, "Pg") ||
				      !strcmp(d->phase_type, "Pb") ) &&
				     d->obs < sta->comprtime ) {
					sta->comprtime = d->obs;
					sta->iwave = d->ipwav;
				}
				else if ( (!strcmp(d->phase_type, "S") ||
				           !strcmp(d->phase_type, "Sn") ||
				           !strcmp(d->phase_type, "Sb") ||
				           !strcmp(d->phase_type, "Lg") ||
				           !strcmp(d->phase_type, "Sg")) &&
				          d->obs < sta->sheartime ) {
					sta->sheartime = d->obs;
				}
			}
			// ---- Azimuths ----
			else if ( d->idtyp == 2 && d->std_err > (float)0. ) {
				if ( d->std_err < sta->azimsd ) {
					sta->azimsd = d->std_err;
					sta->bestazim = d->obs;
				}
			}
			// ---- Slownesses ----
			else if ( d->idtyp == 3 ) {
				if ( d->phase_type[0] == 'P' && d->std_err > (float)0. &&
				     d->obs > .02 && d->obs < 19.16 ) {
					if ( d->std_err < sta->slowsd ) {
						sta->slowsd = d->std_err;
						sta->bestslow = d->obs;
					}
				}
			}
		}
	}

	// Define good (valid) P-wave and S-wave arrival times, S-P times,
	// azimuths and slownesses.  Load these valid data in the order()
	// array and save their station index in the index() array.
	icompr = 0;
	isminusp = 0;
	iazim = 0;
	islow = 0;
	for ( i = 0; i < nsta; ++i ) {
		LOCSAT_Site *sta = stations + i;
		sta->goodcompr = sta->comprtime < fmaxtime;
		if ( stations[i].goodcompr ) {
			++icompr;
			stations[icompr - 1].indexcompr = i + 1;
			stations[icompr - 1].ordercompr = sta->comprtime;
		}
		sta->goodsminusp = sta->goodcompr &&
		                              sta->sheartime < fmaxtime &&
		                              sta->sheartime > sta->comprtime;
		if ( sta->goodsminusp ) {
			++isminusp;
			stations[isminusp - 1].indexsminusp = i + 1;
			stations[isminusp - 1].ordersminusp = sta->sheartime - sta->comprtime;
		}
		sta->goodazim = sta->bestazim >= (float)-180. &&
		                           sta->bestazim <= (float)360.;
		if ( sta->goodazim ) {
			++iazim;
			stations[iazim - 1].indexdsd = i + 1;
			stations[iazim - 1].orderdsd = sta->azimsd;
		}
		sta->goodslow = sta->bestslow > .02 && sta->bestslow < 19.16;
		if ( sta->goodslow ) {
			++islow;
			stations[islow - 1].indexdsd2 = i + 1;
			stations[islow - 1].orderdsd2 = sta->slowsd;
		}
	}

	// Sort P-wave arrival times in descending order (i.e., earliest
	// arrival-times first)
	if ( icompr > 0 ) {
		for ( i = (icompr + 1) / 2; i >= 1; --i ) {
			for ( j = 0; j < icompr - i; ++j ) {
				if ( stations[j].ordercompr > stations[j + i].ordercompr ) {
					tmp = stations[j].ordercompr;
					n = stations[j].indexcompr;
					stations[j].ordercompr = stations[j + i].ordercompr;
					stations[j].indexcompr = stations[j + i].indexcompr;
					stations[j + i].ordercompr = tmp;
					stations[j + i].indexcompr = n;
				}
			}
		}
	}

	// Sort S-P times in descending order (i.e., earliest times first)
	if ( isminusp > 0 ) {
		for ( i = (isminusp + 1) / 2; i >= 1; --i ) {
			for ( j = 0; j < isminusp - i; ++j ) {
				if ( stations[j].ordersminusp > stations[j + i].ordersminusp ) {
					tmp = stations[j].ordersminusp;
					n = stations[j].indexsminusp;
					stations[j].ordersminusp = stations[j + i].ordersminusp;
					stations[j].indexsminusp = stations[j + i].indexsminusp;
					stations[j + i].ordersminusp = tmp;
					stations[j + i].indexsminusp = n;
				}
			}
		}
	}

	// Sort azimuths according to their data standard errors in increasing
	// order (i.e., smallest azimuthal standard errors first)
	if ( iazim > 0 ) {
		for ( i = (iazim + 1) / 2; i >= 1; --i ) {
			for ( j = 0; j < iazim - i; ++j ) {
				if ( stations[j].orderdsd > stations[j + i].orderdsd ) {
					tmp = stations[j].orderdsd;
					n = stations[j].indexdsd;
					stations[j].orderdsd = stations[j + i].orderdsd;
					stations[j].indexdsd = stations[j + i].indexdsd;
					stations[j + i].orderdsd = tmp;
					stations[j + i].indexdsd = n;
				}
			}
		}
	}

	// Sort slownesses in increasing order (i.e., largest slowness is
	// nearest the event and often can be quite diagnostic)
	if ( islow > 0 ) {
		for ( i = (islow + 1) / 2; i >= 1; --i ) {
			for ( j = 0; j < islow - i; ++j ) {
				if ( stations[j].orderdsd2 > stations[j + i].orderdsd2 ) {
					tmp = stations[j].orderdsd2;
					n = stations[j].indexdsd2;
					stations[j].orderdsd2 = stations[j + i].orderdsd2;
					stations[j].indexdsd2 = stations[j + i].indexdsd2;
					stations[j + i].orderdsd2 = tmp;
					stations[j + i].indexdsd2 = n;
				}
			}
		}
	}

	// Find closest station with smallest S-P time and an azimuth.
	// Compute the location from the S-P time and azimuth.
	// First and preferred search procedure !
	if ( isminusp < 1 ) {
		goto L1280;
	}

	for ( i = 0; i < isminusp; ++i ) {
		n = stations[i].indexsminusp - 1;
		if ( stations[n].goodazim ) {
			sminusptime = stations[i].ordersminusp;
			// Interpolate slowness to get distance.
			for ( j = 0; j < 11; ++j ) {
				if ( sminusptime > sminusp[j] &&
				     sminusptime <= sminusp[j + 1] ) {
					double dis = (distance[j + 1] - distance[j]) *
					             (sminusptime - sminusp[j]) /
					             (sminusp[j + 1] - sminusp[j]) +
					             distance[j];
					useazim = stations[n].bestazim;
					// Done.  Let's go find a lat./lon. pair!
					sc_locsat_latlon2(
						stations[n].lat, stations[n].lon,
						dis, useazim, &a1, &a2
					);
					*alat0 = a1;
					*alon0 = a2;

					return;
				}
			}
		}
	}

	// Look here for multiple S-P times and compute distances
	if ( icompr < 2 ) {
		goto L1280;
	}

	for ( i = 0; i < isminusp; ++i ) {
		sminusptime = stations[i].ordersminusp;
		// Interpolate slowness(es) to get distance(s).
		for ( j = 0; j < 11; ++j ) {
			if ( sminusptime > sminusp[j] && sminusptime <= sminusp[j + 1] ) {
				stations[i].dis = (distance[j + 1] - distance[j]) *
				                      (sminusptime - sminusp[j]) /
				                      (sminusp[j + 1] - sminusp[j]) +
				                      distance[j];
			}
		}
	}

	// Compute the approximate origin time
	j = stations[0].indexsminusp;
	n = stations[j - 1].iwave;

	if ( tbd[n * ttt->lentbd] > stations[0].dis ) {
		goto L1130;
	}

	i__1 = ntbd[n];
	for ( i = 0; i < i__1; ++i ) {
		if ( tbd[i + n * ttt->lentbd] > stations[0].dis ) {
			tmp = (stations[0].dis - tbd[i - 1 + n * ttt->lentbd]) /
			      (tbd[i + n * ttt->lentbd] - tbd[i - 1 + n * ttt->lentbd]);
			tcalc = tbtt[i - 1 + n * ttt->lentbz * ttt->lentbd] +
			        tmp * (tbtt[i + n * ttt->lentbz * ttt->lentbd] -
			               tbtt[i - 1 + n * ttt->lentbz * ttt->lentbd]);
			torg = stations[j - 1].ordercompr - tcalc;
			break;
		}
	}

L1130:
	icross = 0;
	icerr = 0;
	if ( isminusp > 1 ) {
		for ( i1 = 2; i1 <= isminusp; ++i1 ) {
			i__2 = i1 - 1;
			for ( i2 = 1; i2 <= i__2; ++i2 ) {
				n1 = stations[i1 - 1].indexsminusp;
				n2 = stations[i2 - 1].indexsminusp;
				++icross;
				ic2 = icross << 1;
				sta1 = stations[n2 - 1].lat;
				sta2 = stations[n2 - 1].lon;
				sta3 = stations[n1 - 1].lat;
				sta4 = stations[n1 - 1].lon;
				sc_locsat_crossings(
					&sta1, &sta2, &sta3, &sta4,
					&stations[i2 - 1].dis, &stations[i1 - 1].dis,
					&crosslat[ic2 - 2], &crosslon[ic2 - 2], &crosslat[ic2 - 1],
					&crosslon[ic2 - 1], &icerr
				);
				if ( icerr > 0 ) {
					--icross;
					goto L1150;
				}
				if ( icross > 1 ) {
					if ( n2 != i1s && n2 != i2s ) {
						i2s = n2;
					}
					else {
						i2s = n1;
					}
				}
				else {
					i1s = n2;
				}
				// Find the best crossing from 3 S-P times ?
				if ( icross > 1 ) {
					smallest = (float)888888.;
					for ( i = 0; i < 2; ++i ) {
						for ( j = 2; j < 4; ++j ) {
							sc_locsat_distaz2(
								crosslat[i], crosslon[i],
								crosslat[j], crosslon[j],
								&delcross, &azi, &baz
							);
							if ( delcross < smallest ) {
								// K.S. 1-Dec-97, abort
								// here because of illegal code */
								// i1 = i
								// i2 = j
								*ierr = 1;
								return;
							}
						}
					}
					d1 = smallest / (float)2.;
					sc_locsat_latlon2(
						crosslat[i1 - 1], crosslon[i1 - 1], d1, azisav,
						&a1, &a2
					);
					*alat0 = a1;
					*alon0 = a2;
					// Done. Exit routine.
					return;
				}
L1150:;
			}
		}
		if ( icross == 0 || icompr < 3 ) {
			goto L1230;
		}
		// Use 2 S-P times and the shortest independent arrival time
		// to determine initial location
		for ( i = 0; i < icompr; ++i ) {
			j = stations[i].indexcompr;
			if ( j != stations[0].indexsminusp && j != stations[1].indexsminusp ) {
				goto L1180;
			}
		}
L1180:
		n = stations[j - 1].iwave;
		// Calculate theoretical travel times
		sc_locsat_distaz2(stations[j - 1].lat, stations[j - 1].lon, *crosslat, *crosslon, &stations->dis, &azi, &baz);
		i__1 = ntbd[n];
		for ( i = 0; i < i__1; ++i ) {
			if ( tbd[i + n * ttt->lentbd] > stations[0].dis ) {
				tmp = (stations[0].dis - tbd[i - 1 + n * ttt->lentbd]) /
				      (tbd[i + n * ttt->lentbd] - tbd[i - 1 + n * ttt->lentbd]);
				tcalc = tbtt[i - 1 + n * ttt->lentbz * ttt->lentbd] +
				        tmp * (tbtt[i + n * ttt->lentbz * ttt->lentbd] -
				               tbtt[i - 1 + n * ttt->lentbz * ttt->lentbd]);
				res1 = (d1 = stations[j - 1].comprtime - torg - tcalc, abs(d1));
				goto L1200;
			}
		}
L1200:
		sc_locsat_distaz2(stations[j - 1].lat, stations[j - 1].lon, crosslat[1], crosslon[1], &stations[1].dis, &azi, &baz);
		i__1 = ntbd[n];
		for ( i = 0; i < i__1; ++i ) {
			if ( tbd[i + n * ttt->lentbd] > stations[1].dis ) {
				tmp = (stations[1].dis - tbd[i - 1 + n * ttt->lentbd]) /
				      (tbd[i + n * ttt->lentbd] - tbd[i - 1 + n * ttt->lentbd]);
				tcalc = tbtt[i - 1 + n * ttt->lentbz * ttt->lentbd] +
				        tmp * (tbtt[i + n * ttt->lentbz * ttt->lentbd] -
				               tbtt[i - 1 + n * ttt->lentbz * ttt->lentbd]);
				res2 = (d1 = stations[j - 1].comprtime - torg - tcalc, abs(d1));
				goto L1220;
			}
		}
		// Choose travel time with the smallest residual
L1220:
		if ( res1 < res2 ) {
			*alat0 = crosslat[0];
			*alon0 = crosslon[0];
		}
		else {
			*alat0 = crosslat[1];
			*alon0 = crosslon[1];
		}
		// Done.  Exit routine.
		return;
	}
	else {
		// Determine initial location using 1 S-P time and 2 nearest
		// independent arrival times
L1230:
		icross = 0;
		n1 = stations[0].indexsminusp;
		for ( k = 1; k <= icompr; ++k ) {
			j = stations[k - 1].indexcompr;
			if ( j == n1 ) {
				goto L1270;
			}
			++icross;
			n = stations[j - 1].iwave;
			// Calculate distance from station to the origin and then
			// find the crossing points
			tcalc = stations[k - 1].ordercompr - torg;
			if ( tbtt[n * ttt->lentbz * ttt->lentbd] > tcalc ) {
				goto L1270;
			}
			i__2 = ntbd[n];
			for ( i = 0; i < i__2; ++i ) {
				if ( tbtt[i + n * ttt->lentbz * ttt->lentbd] > tcalc ) {
					tmp = (tcalc -
					       tbtt[i - 1 + n * ttt->lentbz * ttt->lentbd]) /
					      (tbtt[i + n * ttt->lentbz * ttt->lentbd] -
					       tbtt[i - 1 + n * ttt->lentbz * ttt->lentbd]);
					dist1 = tbd[i - 1 + n * ttt->lentbd] +
					        tmp * (tbd[i + n * ttt->lentbd] -
					               tbd[i - 1 + n * ttt->lentbd]);
					break;
				}
			}
L1250:
			icerr = 0;
			ic2 = icross << 1;
			sta1 = stations[n1 - 1].lat;
			sta2 = stations[n1 - 1].lon;
			sta3 = stations[j - 1].lat;
			sta4 = stations[j - 1].lon;
			sc_locsat_crossings(
				&sta1, &sta2, &sta3, &sta4, &stations[0].dis, &dist1,
				&crosslat[ic2 - 2], &crosslon[ic2 - 2],
				&crosslat[ic2 - 1], &crosslon[ic2 - 1],
				&icerr
			);
			if ( icerr < 1 ) {
				stations[icross].dis = dist1;
				if ( icross > 1 ) {
					smallest = (float)888888.;
					for ( i = 1; i <= 2; ++i ) {
						for ( j = 3; j <= 4; ++j ) {
							sc_locsat_distaz2(
								crosslat[i - 1], crosslon[i - 1],
								crosslat[j - 1], crosslon[j - 1],
								&delcross, &azi, &baz
							);
							if ( delcross < smallest ) {
								smallest = delcross;
								azisav = azi;
								i1 = i;
							}
						}
					}
					d1 = smallest / (float)2.;
					sc_locsat_latlon2(
					    crosslat[i1 - 1], crosslon[i1 - 1], d1, azisav,
					    &a1, &a2
					);
					*alat0 = a1;
					*alon0 = a2;
					// Done!  Exit routine!
					return;
				}
			}
			else {
				--icross;
			}
L1270:;
		}
	}
	// Find crossing point of 2 defining azimuths.  Take the location
	// that is closest (on average) to the 2 stations
	// Second and next important search procedure !
L1280:
	if ( iazim > 1 ) {
		for ( i1 = 2; i1 <= iazim; ++i1 ) {
			i__2 = i1 - 1;
			for ( i2 = 1; i2 <= i__2; ++i2 ) {
				n1 = stations[i1 - 1].indexdsd;
				n2 = stations[i2 - 1].indexdsd;
				// If the difference between 2 azimuths is less
				// than 10 deg., then the confidence one might put into the
				// computed crossing point would be quite uncertain, so ignore!
				tmp = (d1 = stations[n1 - 1].bestazim - stations[n2 - 1].bestazim, abs(d1));

				if ( tmp > (float)350. ) {
					tmp = (float)360. - tmp;
				}

				if ( tmp < (float)10. ) {
					goto L1290;
				}

				sta1 = stations[n1 - 1].lat;
				sta2 = stations[n1 - 1].lon;
				sta3 = stations[n2 - 1].lat;
				sta4 = stations[n2 - 1].lon;
				sc_locsat_azcros2(
					sta1, sta2, stations[n1 - 1].bestazim,
					sta3, sta4, stations[n2 - 1].bestazim,
					&dist1x, &dist2x, &alat0x, &alon0x,
					&ierrx
				);

				if ( ierrx == 0 ) {
					delta = (dist1x + dist2x) * (float).5;
					// dsdwt = 1.0/dsd(n1)*dsd(n2)
					// delwt = 1.0/delta
					// totwt = dsdwt*delwt
					if ( delta < bestazcross ) {
						bestazcross = delta;
						*alat0 = alat0x;
						*alon0 = alon0x;
						dist1 = dist1x;
					}
				}
L1290:;
			}
		}
		if ( bestazcross < 80. ) {
			// Done!  Exit routine!
			return;
		}
	}
	/*     Look here for 1 azimuth and the 2 closest arrvial times */
	/*     if (iazim.eq.1 .and. icompr.gt.1) then */
	/*        n1 = indexdsd(1) */
	/*        Preferably with an arrival time from the azimuth datum */
	/*        do 1310 i = 1, icompr */
	/*           if (n1.eq.indexcompr(i)) then */

	/*           end if */
	/* 1310    continue */
	/*     end if */
	/*     Find station with slowness and azimuth, then compute the location */
	/*     from these data */
	if ( islow > 0 && iazim > 0 ) {
		iusesta = stations[0].indexdsd2;
		useslow = stations[iusesta - 1].bestslow;
		useazim = stations[iusesta - 1].bestazim;
		for ( j = 1; j <= 18; ++j ) {
			if ( useslow < slowness[j - 1] && useslow >= slowness[j] ) {
				stations[0].dis = (distance[j] - distance[j - 1]) *
				                  (useslow - slowness[j - 1]) /
				                  (slowness[j] - slowness[j - 1]) +
				                  distance[j - 1];
				// Done.  Let's go find a lat./lon. pair !
				sta1 = stations[iusesta - 1].lat;
				sta2 = stations[iusesta - 1].lon;
				sc_locsat_latlon2(sta1, sta2, stations[0].dis, useazim, &a1, &a2);
				*alat0 = a1;
				*alon0 = a2;
				return;
			}
		}
	}
	// # Look here for 3 closest arrvial times
	// Use point near station with earlist arrival time.  Use the azimuth
	// at that station, if there is one.  Probably will become unneccesary!
	if ( icompr > 0 ) {
		iusesta = stations[0].indexcompr;
		stations[0].dis = 5.;
		if ( stations[iusesta - 1].goodazim ) {
			useazim = stations[iusesta - 1].bestazim;
		}
		else {
			useazim = 0.;
		}
		// Use best determied azimuth
	}
	else if ( iazim > 0 ) {
		iusesta = stations[0].indexdsd;
		stations[0].dis = 5.;
		if ( stations[iusesta - 1].goodazim ) {
			useazim = stations[iusesta - 1].bestazim;
		}
		else {
			useazim = 0.;
		}
		// Finally try station with the best slowness (as defined by the
		// smallest s.d.) and the default azimuth
	}
	else if ( islow > 0 ) {
		iusesta = stations[0].indexdsd2;
		useslow = stations[iusesta - 1].bestslow;
		for ( j = 1; j <= 18; ++j ) {
			if ( useslow < slowness[j - 1] && useslow >= slowness[j] ) {
				stations[0].dis = (distance[j] - distance[j - 1]) *
				                  (useslow - slowness[j - 1]) /
				                  (slowness[j] - slowness[j - 1]) +
				                  distance[j - 1];
			}
		}
		useazim = 0.;
		// Bail out!
	}
	else {
		*ierr = 1;
		return;
	}

	// Done !
	sc_locsat_latlon2(
		stations[iusesta - 1].lat, stations[iusesta - 1].lon,
		stations[0].dis, useazim, &a1, &a2
	);

	*alat0 = a1;
	*alon0 = a2;
}
