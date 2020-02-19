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


#include <seiscomp/math/filter/const.h>

namespace Seiscomp
{
namespace Math
{
namespace Filtering
{

template<typename T>
ConstFilter<T>::ConstFilter(T c) : _const(c) {}


template<typename T>
void ConstFilter<T>::setSamplingFrequency(double fsamp) {}


template<typename T>
int ConstFilter<T>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;
	_const = (T)params[0];
	return n;
}


template<typename T>
void ConstFilter<T>::apply(int n, T *inout) {
	for ( int i = 0; i < n; ++i )
		inout[i] = _const;
}


template<typename T>
InPlaceFilter<T>* ConstFilter<T>::clone() const {
	return new ConstFilter<T>(_const);
}


INSTANTIATE_INPLACE_FILTER(ConstFilter, SC_SYSTEM_CORE_API);


}
}
}
