#include <locsat/loc.h>
#include <stdint.h>


void sc_locsat_dsvdc(
	double *x,
	const int ldx, const int n, const int p,
	double *s, double *e, double *u,
	const int ldu, double *v, const int ldv,
	double *work, int *job, int *info
) {
	int x_dim1, x_offset, u_dim1, u_offset, v_dim1, v_offset, i__1, i__2, i__3;
	double d__1, d__2, d__3, d__4, d__5, d__6, d__7;

	int kase;
	int jobu, iter;
	double test;
	int nctp1;
	double b, c__;
	int nrtp1;
	double f, g;
	int i, j, k, l = 0, m;
	double t, scale;
	double shift;
	int maxit;
	boolean wantu, wantv;
	double t1, ztest, el;
	int kk;
	double cs;
	int ll, mm, ls = 0;
	double sl;
	int lu;
	double sm, sn;
	int lm1, mm1, lp1, mp1, nct, ncu, lls, nrt;
	double emm1, smm1;

	/* Parameter adjustments */
	x_dim1 = ldx;
	x_offset = 1 + x_dim1 * 1;
	x -= x_offset;
	--s;
	--e;
	u_dim1 = ldu;
	u_offset = 1 + u_dim1 * 1;
	u -= u_offset;
	v_dim1 = ldv;
	v_offset = 1 + v_dim1 * 1;
	v -= v_offset;
	--work;

	/* Function Body */
	maxit = 30;
	/*     Determine what is to be computed */
	wantu = FALSE;
	wantv = FALSE;
	jobu = *job % 100 / 10;
	ncu = n;
	if ( jobu > 1 ) {
		ncu = min(n, p);
	}
	if ( jobu != 0 ) {
		wantu = TRUE;
	}
	if ( *job % 10 != 0 ) {
		wantv = TRUE;
	}
	/*     Reduce x to bidiagonal form, storing the diagonal elements in s */
	/*     and the super-diagonal elements in e() */
	*info = 0;
	/* Computing MIN */
	nct = min(n - 1, p);
	/* Computing MAX */
	/* Computing MIN */
	i__3 = p - 2;
	i__1 = 0, i__2 = min(i__3, n);
	nrt = max(i__1, i__2);
	lu = max(nct, nrt);
	if ( lu < 1 ) {
		goto L1140;
	}
	i__1 = lu;
	for ( l = 1; l <= i__1; ++l ) {
		lp1 = l + 1;
		if ( l > nct ) {
			goto L1010;
		}
		/*        Compute the transformation for the l-th column and place the
		 */
		/*        l-th diagonal in s(l) */
		s[l] = sc_locsat_dnrm2(n - l + 1, &x[l + l * x_dim1], 1);
		if ( s[l] == 0. ) {
			goto L1000;
		}
		if ( x[l + l * x_dim1] != 0. ) {
			s[l] = dsign(s[l], x[l + l * x_dim1]);
		}
		i__2 = n - l + 1;
		d__1 = 1. / s[l];
		sc_locsat_dscal(i__2, d__1, &x[l + l * x_dim1], 1);
		x[l + l * x_dim1] += 1.;
L1000:
		s[l] = -s[l];
L1010:
		if ( p < lp1 ) {
			goto L1040;
		}
		i__2 = p;
		for ( j = lp1; j <= i__2; ++j ) {
			if ( l > nct ) {
				goto L1020;
			}
			if ( s[l] == 0. ) {
				goto L1020;
			}
			/*           Apply the transformation */
			i__3 = n - l + 1;
			t = -sc_locsat_ddot(
				i__3, &x[l + l * x_dim1], 1, &x[l + j * x_dim1], 1
			) / x[l + l * x_dim1];
			sc_locsat_daxpy(n - l + 1, t, &x[l + l * x_dim1], 1, &x[l + j * x_dim1], 1);
/*           Place the l-th row of x into  e for the subsequent */
/*           calculation of the row transformation. */
L1020:
			e[j] = x[l + j * x_dim1];
		}
L1040:
		if ( !wantu || l > nct ) {
			goto L1060;
		}
		/*        Place the transformation in u() for subsequent back
		 * multiplication */
		i__2 = n;
		for ( i = l; i <= i__2; ++i ) {
			u[i + l * u_dim1] = x[i + l * x_dim1];
		}
L1060:
		if ( l > nrt ) {
			goto L1130;
		}
		/* 	 Compute the l-th row transformation and place the l-th */
		/* 	 super-diagonal in e(l) */
		e[l] = sc_locsat_dnrm2(p - l, &e[lp1], 1);
		if ( e[l] == 0. ) {
			goto L1070;
		}
		if ( e[lp1] != 0. ) {
			e[l] = dsign(e[l], e[lp1]);
		}
		i__2 = p - l;
		d__1 = 1. / e[l];
		sc_locsat_dscal(i__2, d__1, &e[lp1], 1);
		e[lp1] += 1.;
L1070:
		e[l] = -e[l];
		if ( lp1 > n || e[l] == 0. ) {
			goto L1110;
		}
		/*        Apply the transformation */
		for ( i = lp1; i <= n; ++i ) {
			work[i] = 0.;
		}
		for ( j = lp1; j <= p; ++j ) {
			sc_locsat_daxpy(n - l, e[j], &x[lp1 + j * x_dim1], 1, &work[lp1], 1);
		}
		for ( j = lp1; j <= p; ++j ) {
			sc_locsat_daxpy(n - l, -e[j] / e[lp1], &work[lp1], 1, &x[lp1 + j * x_dim1], 1);
		}
L1110:
		if ( !wantv ) {
			goto L1130;
		}
		/* 	 Place the transformation in v() for subsequent back multiplication
		 */
		i__2 = p;
		for ( i = lp1; i <= i__2; ++i ) {
			/* L1120: */
			v[i + l * v_dim1] = e[i];
		}
L1130:;
	}
/*     Set up the final bidiagonal matrix or order m */
L1140:
	/* Computing MIN */
	i__1 = p, i__2 = n + 1;
	m = min(i__1, i__2);
	nctp1 = nct + 1;
	nrtp1 = nrt + 1;
	if ( nct < p ) {
		s[nctp1] = x[nctp1 + nctp1 * x_dim1];
	}
	if ( n < m ) {
		s[m] = 0.;
	}
	if ( nrtp1 < m ) {
		e[nrtp1] = x[nrtp1 + m * x_dim1];
	}
	e[m] = 0.;
	/*     If required, generate u() */
	if ( !wantu ) {
		goto L1240;
	}
	if ( ncu < nctp1 ) {
		goto L1170;
	}
	i__1 = ncu;
	for ( j = nctp1; j <= i__1; ++j ) {
		i__2 = n;
		for ( i = 1; i <= i__2; ++i ) {
			/* L1150: */
			u[i + j * u_dim1] = 0.;
		}
		u[j + j * u_dim1] = 1.;
		/* L1160: */
	}
L1170:
	if ( nct < 1 ) {
		goto L1240;
	}
	i__1 = nct;
	for ( ll = 1; ll <= i__1; ++ll ) {
		l = nct - ll + 1;
		if ( s[l] == 0. ) {
			goto L1210;
		}
		lp1 = l + 1;
		if ( ncu < lp1 ) {
			goto L1190;
		}
		for ( j = lp1; j <= ncu; ++j ) {
			t = -sc_locsat_ddot(
				n - l + 1, &u[l + l * u_dim1], 1, &u[l + j * u_dim1], 1
			) / u[l + l * u_dim1];
			sc_locsat_daxpy(n - l + 1, t, &u[l + l * u_dim1], 1, &u[l + j * u_dim1], 1);
		}
L1190:
		sc_locsat_dscal(n - l + 1, -1., &u[l + l * u_dim1], 1);
		u[l + l * u_dim1] += 1.;
		lm1 = l - 1;
		if ( lm1 < 1 ) {
			goto L1230;
		}
		i__2 = lm1;
		for ( i = 1; i <= i__2; ++i ) {
			/* L1200: */
			u[i + l * u_dim1] = 0.;
		}
		goto L1230;
L1210:
		i__2 = n;
		for ( i = 1; i <= i__2; ++i ) {
			/* L1220: */
			u[i + l * u_dim1] = 0.;
		}
		u[l + l * u_dim1] = 1.;
L1230:;
	}
/*     If it is required, generate v() */
L1240:
	if ( !wantv ) {
		goto L1290;
	}
	for ( ll = 1; ll <= p; ++ll ) {
		l = p - ll + 1;
		lp1 = l + 1;
		if ( l > nrt ) {
			goto L1260;
		}
		if ( e[l] == 0. ) {
			goto L1260;
		}
		for ( j = lp1; j <= p; ++j ) {
			i__3 = p - l;
			t = -sc_locsat_ddot(
				i__3, &v[lp1 + l * v_dim1], 1, &v[lp1 + j * v_dim1], 1
			) / v[lp1 + l * v_dim1];
			sc_locsat_daxpy(p - l, t, &v[lp1 + l * v_dim1], 1, &v[lp1 + j * v_dim1], 1);
			/* L1250: */
		}
L1260:
		for ( i = 1; i <= p; ++i ) {
			/* L1270: */
			v[i + l * v_dim1] = 0.;
		}
		v[l + l * v_dim1] = 1.;
		/* L1280: */
	}
/*     Main iteration loop for the singular values */
L1290:
	mm = m;
	iter = 0;
/*     Quit if all the singular values have been found */
/*     Exit */
L1300:
	if ( m == 0 ) {
		goto L9900;
	}
	/*     If too many iterations have been performed, set flag and return */
	if ( iter < maxit ) {
		goto L1310;
	}
	*info = m;
	/*     Exit */
	goto L9900;
/*     This section of the program inspects for negligible elements in */
/*     the s and e arrays.  On completion the variables kase and l are */
/*     set as follows. */
/* 	kase = 1	if s(m) and e(l-1) are negligible and l.lt.m */
/* 	kase = 2	if s(l) is negligible and l.lt.m */
/* 	kase = 3	if e(l-1) is negligible, l.lt.m, and */
/* 			s(l), ..., s(m) are not negligible (qr step) */
/* 	kase = 4	if e(m-1) is negligible (convergence) */
L1310:
	i__1 = m;
	for ( ll = 1; ll <= i__1; ++ll ) {
		l = m - ll;
		/*        Exit */
		if ( l == 0 ) {
			goto L1330;
		}
		test = (d__1 = s[l], abs(d__1)) + (d__2 = s[l + 1], abs(d__2));
		ztest = test + (d__1 = e[l], abs(d__1));
		if ( ztest != test ) {
			goto L1320;
		}
		e[l] = 0.;
		/*        Exit */
		goto L1330;
L1320:;
	}
L1330:
	if ( l != m - 1 ) {
		goto L1340;
	}
	kase = 4;
	goto L1390;
L1340:
	lp1 = l + 1;
	mp1 = m + 1;
	i__1 = mp1;
	for ( lls = lp1; lls <= i__1; ++lls ) {
		ls = m - lls + lp1;
		/*        Exit */
		if ( ls == l ) {
			goto L1360;
		}
		test = 0.;
		if ( ls != m ) {
			test += (d__1 = e[ls], abs(d__1));
		}
		if ( ls != l + 1 ) {
			test += (d__1 = e[ls - 1], abs(d__1));
		}
		ztest = test + (d__1 = s[ls], abs(d__1));
		if ( ztest != test ) {
			goto L1350;
		}
		s[ls] = 0.;
		/*           Exit */
		goto L1360;
L1350:;
	}
L1360:
	if ( ls != l ) {
		goto L1370;
	}
	kase = 3;
	goto L1390;
L1370:
	if ( ls != m ) {
		goto L1380;
	}
	kase = 1;
	goto L1390;
L1380:
	kase = 2;
	l = ls;
L1390:
	++l;
	/*     Perform the task indicated by kase */
	switch ( (int)kase ) {
		case 1:
			goto L1400;
		case 2:
			goto L1430;
		case 3:
			goto L1450;
		case 4:
			goto L1480;
	}
/*     Deflate negligible s(m) */
L1400:
	mm1 = m - 1;
	f = e[m - 1];
	e[m - 1] = 0.;
	i__1 = mm1;
	for ( kk = l; kk <= i__1; ++kk ) {
		k = mm1 - kk + l;
		t1 = s[k];
		sc_locsat_drotg(&t1, &f, &cs, &sn);
		s[k] = t1;
		if ( k == l ) {
			goto L1410;
		}
		f = -sn * e[k - 1];
		e[k - 1] = cs * e[k - 1];
L1410:
		if ( wantv ) {
			sc_locsat_drot(p, &v[k * v_dim1 + 1], 1, &v[m * v_dim1 + 1], 1, cs, sn);
		}
		/* L1420: */
	}
	goto L1300;
/*     Split at negligible s(l) */
L1430:
	f = e[l - 1];
	e[l - 1] = 0.;
	i__1 = m;
	for ( k = l; k <= i__1; ++k ) {
		t1 = s[k];
		sc_locsat_drotg(&t1, &f, &cs, &sn);
		s[k] = t1;
		f = -sn * e[k];
		e[k] = cs * e[k];
		if ( wantu ) {
			sc_locsat_drot(n, &u[k * u_dim1 + 1], 1, &u[(l - 1) * u_dim1 + 1], 1, cs, sn);
		}
	}
	goto L1300;
/*     Perform one QR step */
/*     Calculate the shift */
L1450:
	/* Computing MAX */
	d__6 = (d__1 = s[m], abs(d__1)), d__7 = (d__2 = s[m - 1], abs(d__2)),
	d__6 = max(d__6, d__7), d__7 = (d__3 = e[m - 1], abs(d__3)),
	d__6 = max(d__6, d__7), d__7 = (d__4 = s[l], abs(d__4)),
	d__6 = max(d__6, d__7), d__7 = (d__5 = e[l], abs(d__5));
	scale = max(d__6, d__7);
	sm = s[m] / scale;
	smm1 = s[m - 1] / scale;
	emm1 = e[m - 1] / scale;
	sl = s[l] / scale;
	el = e[l] / scale;
	/* Computing 2nd power */
	d__1 = emm1;
	b = ((smm1 + sm) * (smm1 - sm) + d__1 * d__1) / 2.;
	/* Computing 2nd power */
	d__1 = sm * emm1;
	c__ = d__1 * d__1;
	shift = 0.;
	if ( b == 0. && c__ == 0. ) {
		goto L1460;
	}
	/* Computing 2nd power */
	d__1 = b;
	shift = sqrt(d__1 * d__1 + c__);
	if ( b < 0. ) {
		shift = -shift;
	}
	shift = c__ / (b + shift);
L1460:
	f = (sl + sm) * (sl - sm) - shift;
	g = sl * el;
	/*     Chase zeros */
	mm1 = m - 1;
	i__1 = mm1;
	for ( k = l; k <= i__1; ++k ) {
		sc_locsat_drotg(&f, &g, &cs, &sn);
		if ( k != l ) {
			e[k - 1] = f;
		}
		f = cs * s[k] + sn * e[k];
		e[k] = cs * e[k] - sn * s[k];
		g = sn * s[k + 1];
		s[k + 1] = cs * s[k + 1];
		if ( wantv ) {
			sc_locsat_drot(p, &v[k * v_dim1 + 1], 1, &v[(k + 1) * v_dim1 + 1], 1, cs, sn);
		}
		sc_locsat_drotg(&f, &g, &cs, &sn);
		s[k] = f;
		f = cs * e[k] + sn * s[k + 1];
		s[k + 1] = -sn * e[k] + cs * s[k + 1];
		g = sn * e[k + 1];
		e[k + 1] = cs * e[k + 1];
		if ( wantu && k < n ) {
			sc_locsat_drot(n, &u[k * u_dim1 + 1], 1, &u[(k + 1) * u_dim1 + 1], 1, cs, sn);
		}
	}
	e[m - 1] = f;
	++iter;
	goto L1300;
/*     Convergence */
/*     Make the singular value  positive */
L1480:
	if ( s[l] >= 0. ) {
		goto L1490;
	}
	s[l] = -s[l];
	if ( wantv ) {
		sc_locsat_dscal(p, -1., &v[l * v_dim1 + 1], 1);
	}
/*     Order the singular value */
L1490:
	if ( l == mm ) {
		goto L1500;
	}
	/*     Exit */
	if ( s[l] >= s[l + 1] ) {
		goto L1500;
	}
	t = s[l];
	s[l] = s[l + 1];
	s[l + 1] = t;
	if ( wantv && l < p ) {
		sc_locsat_dswap(p, &v[l * v_dim1 + 1], 1, &v[(l + 1) * v_dim1 + 1], 1);
	}
	if ( wantu && l < n ) {
		sc_locsat_dswap(n, &u[l * u_dim1 + 1], 1, &u[(l + 1) * u_dim1 + 1], 1);
	}
	++l;
	goto L1490;
L1500:
	iter = 0;
	--m;
	goto L1300;
L9900:
	return;
}
