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


#include <seiscomp/math/filter.h>

namespace Seiscomp
{

namespace Math
{

namespace Filtering
{

	
// returns the next power of 2 which is greater or equal n
long next_power_of_2 (long n)
{
	int i = 1;

	if (n<=0)   return 0;
	while (i<n) i<<=1;
	return i;
}

template<typename TYPE>
void cosRamp(std::vector<TYPE> &ramp, TYPE f1, TYPE f2) {
	int n=ramp.size();
	double df = 0.5*(f2-f1), x=M_PI/n;
	for(int i=0; i<n; i++)
		ramp[i] = f1 + (TYPE)(df*(1-cos(i*x)));
}


template void cosRamp<float>(std::vector<float> &ramp, float f1, float f2);
template void cosRamp<double>(std::vector<double> &ramp, double f1, double f2);


}	// namespace Seiscomp::Math::Filtering

}	// namespace Seiscomp::Math

}	// namespace Seiscomp
