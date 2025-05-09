/******************************************************************************
 * Copyright 1991 Science Applications International Corporation.
 * Modifications and fixes gempa GmbH in 2025
 *
 * Note: there was no particular license attached to the origin source code.
 * gempa GmbH modified the source code in the following way:
 * - Translation from Fortran to plain C
 * - Prefix of function names with sc_locsat_
 * - Remove printing of debug information
 * - Get rid of static variables and make all code reentrant
 ******************************************************************************/


#ifndef LOCSAT_INTERP_H
#define LOCSAT_INTERP_H


#include <locsat/utils.h>
#include <math.h>

/**
 * @brief Bracket an array of interpolative values.
 * Using bi-section, brack an array of interpolative values by
 * performing a binary search.
 *
 * Perform a binary search to find those elements of
 * array x() that bracket x0.  Given the array x(i), i = 1,.,N, in
 * non-decreasing order, and given the number x0, this routine finds
 * ileft from 0..n, such that (pretend x(0) = -infinity,
 * x(n+1) = +infinity):
 *  x(ileft) <= x0 <= x(ileft+1)
 *  x(ileft) < x(ileft+1)
 * Note that x() may contain duplicate values, but ileft will still
 * point to a non-zero interval.
 * @param[in] n Dimension of input vector (array), x.
 * @param[in] x One-dimensional input array of values to be bracketed
 * @param[in] x0 Value being compared against
 * @param[out] ileft Left bracketed indice on return
 */
void sc_locsat_brack(const int n, const float *x, const float x0, int *ileft);


/**
 * @brief Deal with bad values (holes) in function.
 * When "holes" in a given function are found, look for "good" samples.
 * Fix up the sampled function f(x) to allow for "holes"
 * with bad values.  A bad function value at a sample point x(i) is
 * assumed to occur when the function sample f(i) has the value fbad.
 * For interpolation purposes, the function is assumed then to be fbad
 * in the intervals surrounding x(i) up to the next "good" samples,
 * where a jump discontinuity is assumed to occur.
 * Given the original function samples -- x(i), f(i), i = 1, m -- this
 * routine creates a new set of samples -- xs(i), fs(j), j = 1, ms --
 * in which the intervals of bad values and discontinuities between
 * good and bad values are explicitly sampled.  To create a
 * discontinuity at a given point, the point is included as two
 * samples with different function values.
 * ---- Example ----
 * x =  0.0  2.0   3.0  7.0  8.5  12.0  14.5  18.0  19.0  20.5 21.5  22.0
 * f =  2.3  1.1  fbad  7.6  4.5  fbad  fbad  12.1  fbad   6.2  4.3  fbad
 * xs =  0.0  2.0   2.0  7.0  7.0   8.5   8.5  20.5 20.5  21.5  21.5
 * fs =  2.3  1.1  fbad fbad  7.6   4.5  fbad  fbad  6.2   4.3  fbad
 * @param[in] n Number of x() samples.
 * @param[in] x Sample values of x(); must be ordered.
 * @param[in] f Value of function at x(i).
 * @param[in] fbad Function value signifying that f(x) is not well-defined (bad).
 * @param[out] ms Number of new x() samples; May be as large as 1 + (4*n)/3.
 * @param[out] xs New sample values of x().
 * @param[out] fs New value of function at x(j).
 */
void sc_locsat_fixhol(const int n, const float *x, const float *f, const float fbad,
                      int *ms, float *xs, float *fs);


/**
 * @brief Two-point Hermite cubic interpolation routine.
 * A simple two-point Hermitian cubic interpolation routine.
 * Perform a Hermite cubic interpolation of function y(x)
 * bewteen two sample points.
 * @param[in] x1 Sample value of independent variable.
 * @param[in] x2 Sample value of independent variable.
 * @param[in] y1 Value of function at x1.
 * @param[in] y2 Value of function at x2.
 * @param[in] yp1 Value of derivative of function at x1.
 * @param[in] yp2 Value of derivative of function at x2.
 * @param[in] x0 Value of independent variable for interpolation.
 * @param[out] y0 Interpolated value of function at x0.
 * @param[out] yp0 Interpolated value of derivative at x0.
 */
void sc_locsat_hermit(float x1, float x2, float y1, float y2, float yp1, float yp2,
                      float x0, float *y0, float *yp0);


/**
 * @brief Monotone, quadratic interpolation routine.
 * Perform a monotone, quadratic interpolation, even when holes in the
 * data are present.
 * Monotone, quadratic interpolation of function f(x) which might have
 * holes (bad values). Bad function samples are
 * given the value fbad.
 * @note f(x) may be discontinuous. A discontinuity is presumed to occur
 *       when x(i) repeats for consecutive i.
 * @note If x0 is out of range (iext = -1 or +1), then f0 and fp0 are
 *       defined through linear extrapolation of function.
 * @note If f(i) = fbad, then the function is assumed to be fbad in the
 *       intervals on either side of x(i), and then jump discontinuously
 *       to the next good sample value.  See subroutine fixhol for details
 *       on how holes are defined.
 * @note If x0 is in a hole (ibad = 1), then f0 is returned fbad and fp0
 *       is zero.
 * @param[in] n Number of function samples.
 * @param[in] x Sample values of independent variable; Must be ordered: x(i) >= x(i-1).
 * @param[in] f Value of function at x(i).
 * @param[in] fbad Function value denoting bad sample.
 * @param[in] x0 Value of independent variable for interpolation.
 * @param[out] f0 Interpolated value of function at x0.
 * @param[out] fp0 Interpolated value of derivative at x0.
 * @param[out] iext Flag indicating whether extrapolation has occurred:
 *             =  0, No extrapolation
 *             = -1, Yes, x0 < x(1)
 *             = +1, Yes, x0 > x(N)
 * @param[out] ibad Flag indicating whether interopolation point is in a hole.
 */
void sc_locsat_holint(int n, const float *x, const float *f, const float fbad, const float x0,
                      float *f0, float *fp0, int *iext, int *ibad);


/**
 * @brief Hybrid monotone, quadratic inter/extrapolation routine.
 * Perfrom a monotone, quadratic interpolation on a function which may have
 * holes.
 * Monotone, quadratic interpolation of function f(x,y)
 * which might have holes (bad values).  Bad function samples are
 * linearly extrapolated as necessary (see below).
 * @param[in] do_extrapolate Do we want to extrapolate data (0 = TRUE; 1 = FALSE).
 * @param[in] nx Number of x() samples.
 * @param[in] ny Number of y() samples.
 * @param[in] x Sample values of x().
 * @param[in] y Sample values of y().
 * @param[in] func Value of function at (x(i), y(j)).
 * @param[in] ldf Leading dimension of array f()
 * @param[in] fbad Function value denoting bad sample.
 * @param[in] x0 Value of x() for interpolation.
 * @param[in] y0 Value of y() for interpolation
 * @param[out] f0 Interpolated value of function at (x0,y0).
 * @param[out] fx0 Interpolated value of x-derivative of function at (x0,y0).
 * @param[out] fy0 Interpolated value of y-derivative of function at (x0,y0).
 * @param[out] fxy0 Interpolated value of x-y-derivative of function at (x0,y0).
 * @param[out] ix Error flag;  0, No error;  -1, x0 < x(1);  1, x0 > x(m).
 * @param[out] iy Error flag;  0, No error;  -1, y0 < Y(1);  1, y0 > y(n).
 * @param[out] ibad Flag indicating whether interopolation point is in a hole.
 */
void sc_locsat_holint2(int do_extrapolate, int nx, int ny,
                       const float *x, const float *y, const float *func,
                       const int ldf, const float fbad,
                       const float x0, const float y0,
                       float *f0, float *fx0, float *fy0, float *fxy0,
                       int *ix, int *iy, int *ibad);


/**
 * @brief Monotone, quadratic interpolation with linear derivatives.
 * Constrain derivative to be linear during monotone, quadratic
 * interpolation.
 * Perform monotone, quadratic interpolation of function
 * f(x).  The interpolating function between two points is montone in
 * value and linear in derivative.
 * @note f(x) may be discontinuous.  A discontinuity is presumed to occur
 *       when x(i) repeats for consecutive i.
 * @note If x0 is out of range (iext = -1 or +1), then f0 and fp0 are
 *       defined through linear extrapolation of function.
 * @param[in] n Number of function samples.
 * @param[in] x Sample values of independent variable; must be ordered:
 *          x(i) >= x(i-1).
 * @param[in] f Value of function at x(i).
 * @param[in] x0 Value of independent variable for interpolation.
 * @param[out] f0 Interpolated value of function at x0.
 * @param[out] fp0 Interpolated value of derivative at x0.
 * @param[out] iext Flag indicating whether extrapolation has occurred:
 *                  =  0, No extrapolation
 *                  = -1, Yes, x0 < x(1)
 *                  = +1, Yes, x0 > x(N)
 */
void sc_locsat_quaint(const int n, const float *x, const float *f, const float x0,
                      float *f0, float *fp0, int *iext);


#endif
