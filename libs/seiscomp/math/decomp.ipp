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

#include<vector>
#include<math.h>

#define VPVS(sigma)  sqrt((2.-2.*(sigma))/(1.-2.*(sigma)))

namespace Seiscomp
{

namespace Math
{

namespace Filtering
{

template<typename TYPE>
int rotate(std::vector<TYPE> &f1, std::vector<TYPE> &f2, double phi)
{
        double  m11 = cos(phi*M_PI/180.),   m12 = sin(phi*M_PI/180.),
                m21 = -m12,                 m22 = m11,    x, y;

	if (f1.size() != f2.size()) 
		throw AlignmentError("rotate(): vector's not aligned");

	TYPE *p1 = &f1[0], *p2 = &f2[0];
	int n = f1.size();
	
        while (n--) {
                x = (*p1)*m11 + (*p2)*m12;
                y = (*p1)*m21 + (*p2)*m22;
                (*p1++) = (TYPE) x;
                (*p2++) = (TYPE) y;
        }

        return 0;
}

template<typename TYPE>
int decompose(std::vector<TYPE> &f1, std::vector<TYPE> &f2,
	      double p, double vs, double sigma)
{
        double	vp = vs*VPVS(sigma), qa, qb;
        double	m11, m12, m21, m22, x, y;

        if (p >= 1./vp)
                return -1;

	if (f1.size() != f2.size()) 
		throw AlignmentError("decompose(): vector's not aligned");

	TYPE *p1 = &f1[0], *p2 = &f2[0];
	int n = f1.size();

        qa  = sqrt (1./(vp*vp)-p*p);
        qb  = sqrt (1./(vs*vs)-p*p);
        m11 = -(2*vs*vs*p*p-1.)/(vp*qa);
        m12 =   2.*p*vs*vs/vp;
        m21 =  -2.*p*vs;
        m22 =  (1.-2.*vs*vs*p*p)/(vs*qb);

        while (n--) {
		x = (*p1)*m11 + (*p2)*m12;
                y = (*p1)*m21 + (*p2)*m22;
                (*p1++) = (TYPE) x;
                (*p2++) = (TYPE) y;
        }

        return 0;
}

}       // namespace Seiscomp::Math::Filter

}       // namespace Seiscomp::Math

}       // namespace Seiscomp
