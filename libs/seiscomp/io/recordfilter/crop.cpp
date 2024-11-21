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


#define SEISCOMP_COMPONENT CROP

#include "crop.h"

#include <seiscomp/logging/log.h>
#include <seiscomp/core/genericrecord.h>

#include <cstring>


using namespace std;
using namespace Seiscomp;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


void copy(DoubleArray &dst, int iofs, vector<double> &src, int front) {
	int chunkSize = src.size()-front;
	memcpy(dst.typedData()+iofs, src.data()+front, chunkSize*sizeof(double));
	memcpy(dst.typedData()+iofs+chunkSize, src.data(), front*sizeof(double));
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace IO {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Cropper::~Cropper() {
	cleanup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Cropper::setWindowLength(double length) {
	if ( length < 0 ) {
		return false;
	}

	_windowLength = length;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Cropper::setWindowOverlap(double overlap) {
	if ( overlap >= 1 ) {
		return false;
	}

	_timeStep = _windowLength * (1 - overlap);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Cropper::setNoAlign(bool noalign) {
	_noalign = noalign;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Cropper::cleanup() {
	if ( _buffer ) {
		delete _buffer;
		_buffer = nullptr;
	}

	for ( auto rec : _nextRecords ) {
		delete rec;
	}

	_nextRecords.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Cropper::init(const Record *rec) {
	_buffer->sampleRate = rec->samplingFrequency();
	_buffer->dt = 1.0 / _buffer->sampleRate;
	_buffer->buffer.resize(_buffer->sampleRate*_windowLength);
	_buffer->tmpOffset = 0;
	_buffer->tmp.resize(_buffer->buffer.size() + _buffer->tmpOffset * 2);
	_buffer->tmp.fill(0.0);
	_buffer->reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *Cropper::feed(const Record *rec) {
	if ( rec ) {
		if ( !_buffer ) {
			_buffer = new CropBuffer;
			init(rec);
		}
		else {
			// Sample rate changed? Check new settings and reset
			// the spec calculation.
			if ( _buffer->sampleRate != rec->samplingFrequency() ) {
				_buffer->reset();
				init(rec);
			}
		}

		crop(rec);
	}

	return flush();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *Cropper::flush() {
	if ( _nextRecords.empty() ) {
		return nullptr;
	}

	auto front = _nextRecords.front();
	_nextRecords.pop_front();
	return front;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Cropper::reset() {
	cleanup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordFilterInterface *Cropper::clone() const {
	auto cropper = new Cropper();
	cropper->_windowLength = _windowLength;
	cropper->_timeStep = _timeStep;
	cropper->_noalign = _noalign;
	return cropper;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Cropper::crop(const Record *rec) {
	Core::Time endTime;
	try {
		endTime = rec->endTime();
	}
	catch ( ... ) {
		SEISCOMP_WARNING("[crop] %s: invalid end time -> ignoring",
		                 rec->streamID().c_str());
		return;
	}

	if ( _buffer->lastEndTime.valid() ) {
		double diff = (rec->startTime() - _buffer->lastEndTime).length();
		if ( fabs(diff) > _buffer->dt*0.5 ) {
			SEISCOMP_DEBUG("[crop] %s: gap/overlap of %f secs -> reset processing",
			               rec->streamID().c_str(), diff);
			_buffer->reset();
		}
	}

	_buffer->lastEndTime = endTime;

	ArrayPtr tmp_ar;
	const DoubleArray *ar = DoubleArray::ConstCast(rec->data());
	if ( ar == nullptr ) {
		tmp_ar = rec->data()->copy(Array::DOUBLE);
		ar = DoubleArray::ConstCast(tmp_ar);
		if ( !ar ) {
			SEISCOMP_ERROR("[crop] internal error: doubles expected");
			return;
		}
	}

	size_t data_len = (size_t)ar->size();
	const double *data = ar->typedData();
	double *buffer = &_buffer->buffer[0];

	if ( _buffer->missingSamples > 0 ) {
		size_t toCopy = std::min(_buffer->missingSamples, data_len);
		memcpy(buffer + _buffer->buffer.size() - _buffer->missingSamples,
		       data, toCopy*sizeof(double));
		data += toCopy;
		data_len -= toCopy;
		_buffer->missingSamples -= toCopy;

		if ( !_buffer->startTime.valid() ) {
			_buffer->startTime = rec->startTime();

			// align to timestep if not requested otherwise
			if ( !_noalign ) {
				double mod = fmod(_buffer->startTime.epoch(), _timeStep);
				double skip = _timeStep - mod;
				_buffer->samplesToSkip = int(skip*_buffer->sampleRate+0.5);

				Core::Time nextStep(floor(_buffer->startTime.epoch() / _timeStep + (_buffer->samplesToSkip > 0 ? 1 : 0)) * _timeStep + 5E-7);
				_buffer->startTime = nextStep - Core::TimeSpan(_buffer->samplesToSkip * _buffer->dt + 5E-7);
			}
		}

		// Still samples missing and no more data available, return
		if ( _buffer->missingSamples > 0 ) {
			return;
		}
	}

	do {
		if ( _buffer->samplesToSkip == 0 ) {
			Core::Time startTime;

			// Calculate spectrum from ringbuffer
			startTime = _buffer->startTime;

			// Copy data
			copy(_buffer->tmp, _buffer->tmpOffset, _buffer->buffer, _buffer->front);

			GenericRecord *out = new GenericRecord(
				rec->networkCode(), rec->stationCode(),
				rec->locationCode(), rec->channelCode(),
				startTime, _buffer->sampleRate
			);
			out->setData(_buffer->tmp.clone());

			_nextRecords.push_back(out);

			// Still need to wait until N samples have been fed.
			_buffer->samplesToSkip = _buffer->sampleRate * _timeStep + 0.5;
		}

		size_t num_samples = std::min(_buffer->samplesToSkip, data_len);

		size_t chunk_size = std::min(num_samples, _buffer->buffer.size()-_buffer->front);
		memcpy(buffer + _buffer->front, data, chunk_size * sizeof(double));

		data += chunk_size;

		// Split chunks
		if ( chunk_size < num_samples ) {
			chunk_size = num_samples - chunk_size;

			memcpy(buffer, data, chunk_size*sizeof(double));

			_buffer->front = chunk_size;

			data += chunk_size;
		}
		else {
			_buffer->front += chunk_size;
			if ( _buffer->front >= _buffer->buffer.size() )
				_buffer->front -= _buffer->buffer.size();
		}

		_buffer->startTime += Core::TimeSpan(_buffer->dt * num_samples + 5E-7);
		_buffer->samplesToSkip -= num_samples;

		data_len -= num_samples;
	}
	while ( data_len > 0 );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
