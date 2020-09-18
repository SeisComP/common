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


#include "bartlett.h"


namespace Seiscomp {
namespace Math {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename TYPE>
void BartlettWindow<TYPE>::process(int n, TYPE *inout, double left, double right) const {
	TYPE a0, a1;

	// Left side
	TYPE n2 = n * left;
	if ( n2 > n ) n2 = n;
	int in2 = int(n2);
	int w = in2 * 2;

	if ( w > 1 ) {
		a0 = 2.0 / (w-1);
		a1 = (w-1)*0.5;

		for ( int i = 0; i < in2; ++i ) {
			inout[i] *= 1-fabs((i-a1))*a0;
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
		a0 = 2.0 / (w-1);
		a1 = (w-1)*0.5;

		for ( int i = 0; i < in2; ++i ) {
			inout[n-in2+i] *= 1-fabs(in2+i-a1)*a0;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API BartlettWindow<float>;
template class SC_SYSTEM_CORE_API BartlettWindow<double>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
