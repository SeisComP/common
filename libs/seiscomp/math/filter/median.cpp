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


#define SEISCOMP_COMPONENT Median


#include <seiscomp/logging/log.h>
#include <seiscomp/core/exceptions.h>
#include <seiscomp/math/filter/median.h>

#include <algorithm>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
Median<TYPE>::Median(double timeSpan /*sec*/, double fsamp)
: _timeSpan(timeSpan), _fsamp(0.0) {
	if ( fsamp )
		setSamplingFrequency(fsamp);
}


template<typename TYPE>
void Median<TYPE>::apply(int n, TYPE *inout) {
	if ( _fsamp == 0.0 ) {
		throw Core::GeneralException("Sample rate not initialized");
	}

	if ( n <= 0 ) return;

	if ( _sampleCount < 1 ) {
		throw std::out_of_range("Attempted computation of median for length < sample distance");
	}

	// Initialize the median buffer with the first sample
	if ( _firstSample ) {
		std::fill(_buffer.begin(), _buffer.end(), inout[0]);
		std::fill(_sorted.begin(), _sorted.end(), inout[0]);
		_firstSample = false;
	}

	size_t mid = _sampleCount / 2;
	TYPE sample;

	// The median calculation requires a sorted buffer. Sorting is an expencive
	// operation and should be performed on the entire buffer for each sample.
	// Hence, a second, sorted buffer is used which managed slightly different
	// depending on the buffer size.
	// For a buffer length of less than 10 samples it is faster to use the
	// linear std::find, change the value without changing the container size
	// and perform a subsequent sort operation on an almost sorted buffer.
	// For larger buffer sizes it is more efficient to delete the old value and
	// insert the new value at the correct postion taking advantage of the
	// binary_search routine.
	//
	// Speed up depending on buffer size compared against creation of sorted
	// copy of ring buffer of each sample:
	//
	//   size  find/sort   erase/insert
	//      3    1.41056       0.939747
	//      5    1.38352       1.09831
	//      7    1.47858       1.34695
	//     10    1.63019       1.76063
	//     15    1.98368       2.60729
	//     20    1.58768       3.86773
	//     50    1.68139       9.86892
	//    100    1.70717      20.8161
	if ( _sampleCount < 10 ) {
		for ( int i = 0; i < n; ++i ) {
			sample = inout[i];
			// before the _buffer at _index will be overriden in the next step,
			// we search for the value in the sorted buffer and replace it by
			// the current sample
			*std::find(_sorted.begin(), _sorted.end(), _buffer[_index]) = sample;

			// store sample for later use in ring buffer
			_buffer[_index++] = sample;
			if ( _index >= _sampleCount ) {
				_index = 0;
			}

			// sort vector for computing the median (which is pre-sorted except
			// for one element)
			std::sort(_sorted.begin(), _sorted.end());

			inout[i] = (_sampleCount % 2) ? _sorted[mid] :  // odd
			           (_sorted[mid-1] + _sorted[mid]) / 2; // even
		}
	}
	else {
		for ( int i = 0; i < n; ++i ) {
			sample = inout[i];

			// before the _buffer at _index will be overriden in the next step,
			// we search for the value in the sorted buffer, deleted it and
			// insert the new value at the correct position eliminated the need
			// for a sort operation
			_sorted.erase(std::lower_bound(_sorted.begin(), _sorted.end(), _buffer[_index]));
			_sorted.insert(std::upper_bound(_sorted.begin(), _sorted.end(), sample), sample);

			// store sample for later use in ring buffer
			_buffer[_index++] = inout[i];
			if ( _index >= _sampleCount ) {
				_index = 0;
			}

			inout[i] = (_sampleCount % 2) ? _sorted[mid] :  // odd
			           (_sorted[mid-1] + _sorted[mid]) / 2; // even
		}
	}


}


template<typename TYPE>
InPlaceFilter<TYPE>* Median<TYPE>::clone() const {
	return new Median<TYPE>(_timeSpan, _fsamp);
}


template<typename TYPE>
void Median<TYPE>::setLength(double timeSpan) {
	_timeSpan = timeSpan;
}


template<typename TYPE>
void Median<TYPE>::setSamplingFrequency(double fsamp) {
	if ( _fsamp == fsamp ) {
		return;
	}

	_fsamp = fsamp;
	_sampleCount = static_cast<size_t>(_fsamp * _timeSpan);
	if ( _sampleCount < 1 ) _sampleCount = 1;
	_buffer.resize(_sampleCount);
	_sorted.resize(_sampleCount);

	reset();
}


template<typename TYPE>
int Median<TYPE>::setParameters(int n, const double *params) {
	if ( n != 1 ) return 1;
	if ( params[0] <= 0 )
		return -1;

	_timeSpan = params[0];
	return n;
}


template<typename TYPE>
void Median<TYPE>::reset() {
	_firstSample = true;
	_index = 0;
}


INSTANTIATE_INPLACE_FILTER(Median, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(Median, "MEDIAN");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
