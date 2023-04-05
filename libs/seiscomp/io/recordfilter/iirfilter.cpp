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


#define SEISCOMP_COMPONENT RecordFilter_IIR

#include <seiscomp/logging/log.h>
#include <seiscomp/io/recordfilter/iirfilter.h>


namespace Seiscomp {
namespace IO {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

template <typename T>
Array::DataType dataType() {
	throw Core::TypeConversionException("RecordFilter::IIR: wrong data type");
}


template <>
Array::DataType dataType<float>() {
	return Array::FLOAT;
}


template <>
Array::DataType dataType<double>() {
	return Array::DOUBLE;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordIIRFilter<T>::RecordIIRFilter(InplaceFilterType *filter)
: _filter(filter) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordIIRFilter<T>::~RecordIIRFilter() {
	if ( _filter != nullptr )
		delete _filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordIIRFilter<T> &
RecordIIRFilter<T>::operator=(RecordIIRFilter<T>::InplaceFilterType *f) {
	setIIR(f);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordIIRFilter<T>::setIIR(Math::Filtering::InPlaceFilter<T> *f) {
	if ( _filter != nullptr )
		delete _filter;

	_lastEndTime = Core::Time();
	_filter = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
Record *RecordIIRFilter<T>::feed(const Record *rec) {
	if ( rec ) {
		const Array *data = rec->data();
		if ( data == nullptr ) return nullptr;

		TypedArray<T> *tdata = (TypedArray<T>*)data->copy(dataType<T>());
		if ( tdata == nullptr ) return nullptr;

		// Copy the record and assign the data
		GenericRecord *out = new GenericRecord(*rec);
		out->setData(tdata);

		if ( apply(out) )
			return out;
		else {
			delete out;
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool RecordIIRFilter<T>::apply(GenericRecord *rec) {
	// No filter is no error
	if ( _filter == nullptr ) return true;

	if ( rec->dataType() != dataType<T>() ) {
		throw Core::TypeConversionException("RecordFilter::IIR: wrong data type");
	}

	if ( _lastEndTime.valid() ) {
		// If the sampling frequency changed, reset the filter
		if ( _samplingFrequency != rec->samplingFrequency() ) {
			SEISCOMP_WARNING("[%s] sps change (%f != %f): reset filter",
			                 rec->streamID().c_str(), _samplingFrequency,
			                 rec->samplingFrequency());
			reset();
		}
		else {
			Core::TimeSpan diff = rec->startTime() - _lastEndTime;
			// Overlap or gap does not matter, we need to reset the filter
			// for non-continuous records
			if ( fabs(diff) > (0.5/_samplingFrequency) ) {
//				SEISCOMP_DEBUG("[%s] discontinuity of %fs: reset filter",
//				               rec->streamID().c_str(), (double)diff);
				reset();
			}
		}
	}

	if ( !_lastEndTime.valid() ) {
		// First call after construction or reset: initialize
		_samplingFrequency = rec->samplingFrequency();
		try {
			_filter->setSamplingFrequency(_samplingFrequency);
			_filter->setStreamID(rec->networkCode(), rec->stationCode(), rec->locationCode(), rec->channelCode());
			_lastError = std::string();
		}
		catch ( std::exception &e ) {
			SEISCOMP_WARNING("[%s] apply %f sps: %s",
			                 rec->streamID().c_str(), _samplingFrequency,
			                 e.what());
			_lastError = e.what();
			return false;
		}
	}

	TypedArray<T> *data = (TypedArray<T>*)rec->data();
	_filter->apply(*data);
	_lastEndTime = rec->endTime();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
Record *RecordIIRFilter<T>::flush() {
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordIIRFilter<T>::reset() {
	// Clone the filter
	if ( _filter ) {
		InplaceFilterType *tmp = _filter;
		_filter = _filter->clone();
		delete tmp;
	}

	// Reset last end time
	_lastEndTime = Core::Time();
	_lastError = std::string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
RecordFilterInterface* RecordIIRFilter<T>::clone() const {
	return new RecordIIRFilter<T>(_filter != nullptr?_filter->clone():nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template class SC_SYSTEM_CORE_API RecordIIRFilter<float>; \
template class SC_SYSTEM_CORE_API RecordIIRFilter<double>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
