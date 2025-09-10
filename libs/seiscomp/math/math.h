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


#ifndef SEISCOMP_MATH_MATH_H
#define SEISCOMP_MATH_MATH_H


#include <seiscomp/core.h>
#include <cmath>
#include <complex>


#ifndef deg2rad
#define deg2rad(d) (M_PI*(d)/180.0)
#endif
#ifndef rad2deg
#define rad2deg(d) (180.0*(d)/M_PI)
#endif

#ifndef HALF_PI
#define HALF_PI (M_PI/2)
#endif


#if defined(_MSC_VER)
	#include <float.h>
#endif


namespace Seiscomp::Math {


using Complex = std::complex<double>;
using Fraction = std::pair<int, int>;


#if defined(WIN32)
template <typename T>
inline bool isNaN(T v) { return _isnan(v)!=0; }
#elif defined(__SUNPRO_CC) || defined(__sun)
template <typename T>
inline bool isNaN(T v) { return isnan(v)!=0; }
#else
template <typename T>
inline bool isNaN(T v) { return std::isnan(v)!=0; }
#endif

/**
 * @brief Converts a double to a fraction containing numerator and
 * denominator. If the double value exceeds the numeric integer limits
 * numeric_limits<int>::max()/1 or numeric_limits<int>::min()/1 is returned.
 * @param value The double number to be converted.
 * @return Fraction containing numerator and denominator.
 */
Fraction double2frac(double value);


} // namespace Seiscomp::Math


#endif
