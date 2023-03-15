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


#include <iostream>
#include <seiscomp/math/filter/seismometerresponse.h>
#include <seiscomp/core/exceptions.h>


using namespace std;


namespace Seiscomp {
namespace Math {


namespace SeismometerResponse {


PolesAndZeros::PolesAndZeros() {}


PolesAndZeros::PolesAndZeros(const Poles &p, const Zeros &z, double n)
: poles(p), zeros(z), norm(n) {}


PolesAndZeros::PolesAndZeros(const PolesAndZeros &other)
: poles(other.poles), zeros(other.zeros), norm(other.norm) {}


WoodAnderson::WoodAnderson(GroundMotion input, Config config) {
	poles.clear();
	zeros.clear();

	double p_abs = 2*M_PI/config.T0;
	double p_re  = config.h*p_abs;
	double p_im  = sqrt(p_abs*p_abs-p_re*p_re);
	poles.push_back( Pole(-p_re, -p_im));
	poles.push_back( Pole(-p_re, +p_im));
	norm = config.gain;

	switch(input) {
		case Displacement: zeros.push_back( 0 );
		case Velocity:     zeros.push_back( 0 );
		case Acceleration: break;
	}
}


Seismometer5sec::Seismometer5sec(GroundMotion input) {
	poles.clear();
	zeros.clear();

	// Poles from Seismic Handler
	poles.push_back( Pole(-0.88857, -0.88857) );
	poles.push_back( Pole(-0.88857, +0.88857) );

	norm = 1.;

	switch(input) {
		case Displacement: zeros.push_back( 0 );
		case Velocity:     zeros.push_back( 0 );
		case Acceleration: break;
	}
}


} // namespace SeismometerResponse
} // namespace Seiscomp::Math
} // namespace Seiscomp
