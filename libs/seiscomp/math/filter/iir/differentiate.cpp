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


#include <seiscomp/math/filter/iir/differentiate.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template <typename T>
IIRDifferentiate<T>::IIRDifferentiate(double fsamp) : _fsamp(fsamp) {
	setSamplingFrequency(fsamp);
}


template <typename T>
IIRDifferentiate<T>::IIRDifferentiate(const IIRDifferentiate<T> &other) {
	_fsamp = 0.0;
	reset();
}


template <typename T>
void IIRDifferentiate<T>::reset() {
	_v1 = 0;
	_init = false;
}


template <typename T>
void IIRDifferentiate<T>::setSamplingFrequency(double fsamp) {
	if ( fsamp == _fsamp) return;

	_fsamp = fsamp;
	reset();
}


template <typename T>
int IIRDifferentiate<T>::setParameters(int n, const double *params) {
	if ( n != 0 ) return 0;
	return n;
}


template <typename T>
void IIRDifferentiate<T>::apply(int n, T *inout) {
	for ( int i = 0;  i < n; ++i ) {
		T v = inout[i];
		if ( !_init ) {
			inout[i] = 0;
			_init = true;
		}
		else
			inout[i] = (v-_v1)*_fsamp;
		_v1 = v;
	}
}


template <typename T>
InPlaceFilter<T> *IIRDifferentiate<T>::clone() const {
	return new IIRDifferentiate<T>(_fsamp);
}


INSTANTIATE_INPLACE_FILTER(IIRDifferentiate, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(IIRDifferentiate, "DIFF");


}
}
}
