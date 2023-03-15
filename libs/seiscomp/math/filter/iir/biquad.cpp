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


#include <math.h>
#include <vector>
#include <ostream>
#include <iostream>

#include<seiscomp/math/filter/iir/biquad.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {


// load the template class definitions
#include<seiscomp/math/filter/iir/biquad.ipp>


BiquadCoefficients::BiquadCoefficients(double b0, double b1, double b2,
                                       double a0, double a1, double a2) {
	set(b0, b1, b2, a0, a1, a2);
}


BiquadCoefficients::BiquadCoefficients(BiquadCoefficients const &bq)
	: b0(bq.b0), b1(bq.b1), b2(bq.b2), a0(bq.a0), a1(bq.a1), a2(bq.a2) {}


void BiquadCoefficients::set(double b0, double b1, double b2,
                             double a0, double a1, double a2) {
	this->b0 = b0;
	this->b1 = b1;
	this->b2 = b2;
	this->a0 = a0;
	this->a1 = a1;
	this->a2 = a2;
}


std::ostream &operator<<(std::ostream &os, const BiquadCoefficients &biq) {
	os << "b: " << biq.b0 << ", " << biq.b1 << ", " << biq.b2 << std::endl
	   << "a: " << biq.a0 << ", " << biq.a1 << ", " << biq.a2 << std::endl;
	return os;
}


template class SC_SYSTEM_CORE_API Biquad<float>;
template class SC_SYSTEM_CORE_API Biquad<double>;

template class SC_SYSTEM_CORE_API BiquadFilter<float>;
template class SC_SYSTEM_CORE_API BiquadFilter<double>;


} // namespace Seiscomp::Math::Filtering::IIR
} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
