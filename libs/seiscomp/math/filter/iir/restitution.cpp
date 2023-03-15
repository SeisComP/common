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

#include <seiscomp/math/filter/iir/restitution.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>

#include <seiscomp/math/filter/iir/butterworth.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {


bool coefficients_from_T0_h(
	double fsamp, double gain, double T0, double h,
	double &c0, double &c1, double &c2)
{
	// from Kanamori and Rivera (2008)
	double w0 = 2*M_PI/T0;
	double dt = 1./fsamp;
	double q  = 1./(gain*dt);

	c0 = q;
	c1 = -2*(1+h*w0*dt)*q;
	c2 = (1+2*h*w0*dt+(w0*dt)*(w0*dt))*q;

	return true;
}


bool coefficients_from_T1_T2(
	double fsamp, double gain, double T1, double T2,
	double &c0, double &c1, double &c2)
{
	double w1 = 2*M_PI/T1, w2 = 2*M_PI/T2;
	double dt = 1./fsamp;
	double q  = 1./(gain*dt);

	c0 = q;
	c1 = -2*(1+0.5*(w1+w2)*dt)*q;
	c2 = (1+(w1+w2)*dt+w1*w2*dt*dt)*q;

	return true;
}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// load the template class definitions
#include<seiscomp/math/filter/iir/restitution.ipp>

INSTANTIATE_INPLACE_FILTER(RestitutionFilter, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(RestitutionFilter, "Restitution");


} // namespace IIR
} // namespace Filtering
} // namespace Math
} // namespace Seiscomp
