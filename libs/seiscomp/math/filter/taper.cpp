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
#include <seiscomp/math/filter/taper.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
InitialTaper<TYPE>::InitialTaper(double taperLength, TYPE offset, double fsamp)
: _taperLength(taperLength)
, _samplingFrequency(0)
, _taperLengthI(0)
, _sampleCount(0)
, _offset(offset) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}

template<typename TYPE>
void InitialTaper<TYPE>::setSamplingFrequency(double fsamp) {
	if ( _samplingFrequency == fsamp ) return;

	_samplingFrequency = fsamp;
	_taperLengthI = int(_taperLength * _samplingFrequency);

	reset();
}


template<typename TYPE>
int InitialTaper<TYPE>::setParameters(int n, const double *params) {
	if ( (n < 1) || (n > 2) ) return 1;

	_taperLength = (int)params[0];

	if ( n > 1 )
		_offset = (TYPE)params[1];
	else
		_offset = 0;

	return n;
}

template<typename TYPE>
InPlaceFilter<TYPE>* InitialTaper<TYPE>::clone() const {
	return new InitialTaper<TYPE>(_taperLength, _offset, _samplingFrequency);
}

template<typename TYPE>
void InitialTaper<TYPE>::apply(int n, TYPE *inout) {
	if ( _sampleCount >= _taperLengthI ) return;

	for ( int i = 0; i < n && _sampleCount < _taperLengthI; ++i ) {
		double frac = double(_sampleCount++)/_taperLengthI;
		inout[i] = (TYPE)((inout[i]-_offset)*0.5*(1-cos(M_PI*frac)) + _offset);
	}
}


INSTANTIATE_INPLACE_FILTER(InitialTaper, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(InitialTaper, "ITAPER");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
