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


#define SEISCOMP_COMPONENT CutOff

#include <math.h>

#include <seiscomp/math/filter/cutoff.h>
#include <seiscomp/core/exceptions.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


namespace _private {

class FilterException : public Seiscomp::Core::GeneralException {
	public:
		FilterException() : GeneralException("filter exception") {}
		FilterException(std::string what) : GeneralException(what) {}
};


}


template<typename TYPE>
CutOff<TYPE>::CutOff(TYPE threshold) : _threshold(threshold), _outstanding(2) {}


template<typename TYPE>
void CutOff<TYPE>::apply(int n, TYPE *inout) {
	if ( _threshold <= 0.0 )
		throw _private::FilterException("Threshold not initialized");

	for ( int i = 0; i < n; ++i ) {
		if ( _outstanding == 2 ) {
			_samples[0] = _samples[1] = inout[i];
			--_outstanding;
			continue;
		}
		else if ( _outstanding == 1 ) {
			_samples[1] = inout[i];
			--_outstanding;
		}

		TYPE s = inout[i];

		if ( fabs(_samples[0] - _samples[1]) > _threshold ) {
			inout[i] = (_samples[0] + inout[i]) / 2;
			_samples[1] = inout[i];
		}
		else
			inout[i] = _samples[1];

		_samples[0] = _samples[1];
		_samples[1] = s;
	}
}


template<typename TYPE>
void CutOff<TYPE>::setSamplingFrequency(double fsamp) {}


template<typename TYPE>
InPlaceFilter<TYPE> *CutOff<TYPE>::clone() const {
	return new CutOff<TYPE>(_threshold);
}


template<typename TYPE>
int CutOff<TYPE>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;
	if ( params[0] <= 0 )
		return -1;

	_threshold = params[0];
	return n;
}


INSTANTIATE_INPLACE_FILTER(CutOff, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(CutOff, "CUTOFF");


} // namespace Filtering
} // namespace Math
} // namespace Seiscomp
