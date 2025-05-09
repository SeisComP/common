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


#ifndef LOCSAT_UTILS_H
#define LOCSAT_UTILS_H


#include <stdint.h>
#include <math.h>


typedef int32_t boolean;
#define TRUE 1
#define FALSE 0


#ifndef min
	#define min(a,b)        ((a) <= (b) ? (a) : (b))
	#define max(a,b)        ((a) >= (b) ? (a) : (b))
	#define dmin(a,b)       (double)min(a,b)
	#define dmax(a,b)       (double)max(a,b)
	#define abs(x)          ((x) >= 0 ? (x) : -(x))
	#define dabs(x)         (double)abs(x)
	#define dsign(x, y)     ((y) >= 0 ? abs(x) : -abs(x))
	#define nint(x)         (int)(x >= 0 ? floor(x + .5) : -floor(.5 - x))
#endif


#ifndef rad2deg
#define rad2deg(d) ((d) * 57.2957795)
// WANTED: #define rad2deg(d) (180.0*(d)/M_PI)
#endif
#ifndef deg2rad
#define deg2rad(d) ((d) * 1.0/57.2957795)
// WANTED: #define deg2rad(d) (M_PI*(d)/180.0)
#endif


#endif
