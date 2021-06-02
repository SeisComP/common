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
#include <seiscomp/math/filter/rmhp.h>
#include <seiscomp/core/exceptions.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
RunningMean<TYPE>::RunningMean(double windowLength, double fsamp)
: _windowLength(windowLength), _samplingFrequency(0)
, _windowLengthI(0), _sampleCount(0)
, _average(0) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}


template<typename TYPE>
void RunningMean<TYPE>::apply(int n, TYPE *inout) {
	if ( !_windowLengthI )
		throw Core::GeneralException("RMHP: Window length is 0");

	if ( _sampleCount < _windowLengthI ) {
		// Initially compute initial average from as many samples
		// as possible, up to the specified window length
		int samplesLeft = _windowLengthI - _sampleCount;
		int navg = samplesLeft > n ? n : samplesLeft;
		for ( int i = 0; i < navg; ++i ) {
			_average = (_average*_sampleCount + inout[i]) / (_sampleCount+1);
			inout[i] = _average;
			++_sampleCount;
		}

		inout += navg;
		n -= navg;
	}

	for ( int i = 0; i < n; ++i ) {
		_average = (_average*(_windowLengthI-1) + inout[i]) / _windowLengthI;
		inout[i] = (TYPE)_average;
	}
}

template<typename TYPE>
void RunningMean<TYPE>::reset() {
	_sampleCount = 0;
	_average = 0;
}

template<typename TYPE>
void RunningMean<TYPE>::setSamplingFrequency(double fsamp) {
	if (fsamp == _samplingFrequency) return;

	_samplingFrequency = fsamp;
	_windowLengthI = int(_windowLength * _samplingFrequency);

	reset();
}

template<typename TYPE>
int RunningMean<TYPE>::setParameters(int n, const double *params) {
	// Return an error stating that the passed parameter count
	// does not match
	if ( n != 1 ) return 1;

	_windowLength = params[0];

	return n;
}

template<typename TYPE>
InPlaceFilter<TYPE>* RunningMean<TYPE>::clone() const {
	return new RunningMean<TYPE>(_windowLength, _samplingFrequency);
}



template<typename TYPE>
RunningMeanHighPass<TYPE>::RunningMeanHighPass(double windowLength, double fsamp)
: RunningMean<TYPE>(windowLength, fsamp)
{}


template<typename TYPE>
void RunningMeanHighPass<TYPE>::apply(int n, TYPE *inout) {
	if ( !RunningMean<TYPE>::_windowLengthI )
		throw Core::GeneralException("RMHP: Window length is 0");

	if ( RunningMean<TYPE>::_sampleCount < RunningMean<TYPE>::_windowLengthI ) {
		// Initially compute initial average from as many samples
		// as possible, up to the specified window length
		int samplesLeft = RunningMean<TYPE>::_windowLengthI - RunningMean<TYPE>::_sampleCount;
		int navg = samplesLeft > n ? n : samplesLeft;
		for ( int i = 0; i < navg; ++i ) {
			RunningMean<TYPE>::_average = (RunningMean<TYPE>::_average*RunningMean<TYPE>::_sampleCount + inout[i]) / (RunningMean<TYPE>::_sampleCount+1);
			inout[i] -= RunningMean<TYPE>::_average;
			++RunningMean<TYPE>::_sampleCount;
		}

		inout += navg;
		n -= navg;
	}

	for ( int i = 0; i < n; ++i ) {
		RunningMean<TYPE>::_average = (RunningMean<TYPE>::_average*(RunningMean<TYPE>::_windowLengthI-1) + inout[i]) / RunningMean<TYPE>::_windowLengthI;
		inout[i] -= (TYPE)RunningMean<TYPE>::_average;
	}	
}

template<typename TYPE>
InPlaceFilter<TYPE>* RunningMeanHighPass<TYPE>::clone() const {
	return new RunningMeanHighPass<TYPE>(RunningMean<TYPE>::_windowLength, RunningMean<TYPE>::_samplingFrequency);
}


INSTANTIATE_INPLACE_FILTER(RunningMean, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(RunningMean, "RM");

INSTANTIATE_INPLACE_FILTER(RunningMeanHighPass, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(RunningMeanHighPass, "RMHP");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
