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


//#include"biquad.cpp"
//#include"resamp.cpp"

#include<seiscomp/math/filter/filter.h>

namespace Seiscomp
{

namespace Math
{

namespace Filtering
{
	
template int rotate(std::vector<double> &f1, std::vector<double> &f2,
			double phi);
template int decompose(std::vector<double> &f1, std::vector<double> &f2,
			double p, double vs, double sigma);
//template int decompose(std::vector<double> &f1, std::vector<double> &f2,
//		       double p, double vs, double sigma);

#include<seiscomp/math/filter/minmax.ipp>

template int minmax(std::vector<int> const &f, int i1, int i2, int *imax, int *fmax);
template int find_max(std::vector<int> const &f, int i1, int i2, int *imax, int *fmax);

//#include "seiscomp/impl/hilbert.ipp"
//#include "src/filter/hilbert.cpp"
//#include "hilbert.cpp"
//template void hilbert_transform(std::vector<double> &f1, int direction);
//template void envelope(std::vector<double> &f1);

//BiquadCascade<int> intcascade;
//Biquad<int> intbiquad;
//BiquadCascade<double> doublecascade;
//Biquad<double> doublebiquad;

} // namespace Seiscomp::Math::Filter

} // namespace Seiscomp::Math

} // namespace Seiscomp
