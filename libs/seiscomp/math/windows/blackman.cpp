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


#include "blackman.h"


namespace Seiscomp {
namespace Math {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename TYPE>
BlackmanWindow<TYPE>::BlackmanWindow(TYPE alpha) : _alpha(alpha) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename TYPE>
void BlackmanWindow<TYPE>::process(int n, TYPE *inout, double left, double right) const {
	TYPE a0 = (1 - _alpha) * 0.5,
	     a1 = 0.5,
	     a2 = _alpha * 0.5;
	TYPE o;

	// Left side
	TYPE n2 = n * left;
	if ( n2 > n ) n2 = n;
	int in2 = int(n2);
	int w = in2 * 2;

	if ( w > 1 ) {
		o = 1.0 / (w - 1);
		for ( int i = 0; i < in2; ++i ) {
			inout[i] *= a0 - a1*cos(2*M_PI*i*o) + a2*cos(4*M_PI*i*o);
		}
	}

	// Right side
	if ( left != right ) {
		n2 = n * right;
		if ( n2 > n ) n2 = n;
		in2 = int(n2);
		w = in2 * 2;
	}

	if ( w > 1 ) {
		o = 1.0 / (w - 1);
		for ( int i = 0; i < in2; ++i ) {
			inout[n-in2+i] *= a0 - a1*cos(2*M_PI*(in2+i)*o) + a2*cos(4*M_PI*(in2+i)*o);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API BlackmanWindow<float>;
template class SC_SYSTEM_CORE_API BlackmanWindow<double>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
