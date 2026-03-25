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


#define SEISCOMP_COMPONENT MSeed

#include <seiscomp/logging/log.h>
#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/io/records/mseed/encoder/uncompressed.h>
#include <seiscomp/io/records/mseed/encoder/steim1.h>
#include <seiscomp/io/records/mseed/encoder/steim2.h>

#include <streambuf>
#include <istream>

#include "mseedencoder.h"


#define ENCODER static_cast<MSEED::Encoder*>(_encoder.get())


namespace {


template <typename T>
struct TypedArrayBuf : std::streambuf {
	TypedArrayBuf(const Seiscomp::TypedArray<T> *base) {
		char *p(const_cast<char*>(static_cast<const char*>(base->data())));
		setg(p, p, p + base->size() * base->elementSize());
	}
};

template <typename T>
struct ITypedArrayStream : virtual TypedArrayBuf<T>, std::istream {
	ITypedArrayStream(const Seiscomp::TypedArray<T> *base)
	: TypedArrayBuf<T>(base)
	, std::istream(static_cast<std::streambuf*>(this)) {}
};


}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace IO {
namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
MSEED::Encoder *createEncoder(MSeedEncoder::CompressionType compression,
                              MSEED::Format *format, int freqn, int freqd) {
	switch ( compression ) {
		case MSeedEncoder::Identity:
			return new MSEED::Uncompressed<T>(format, freqn, freqd);
			break;
		case MSeedEncoder::Steim1:
			return new MSEED::Steim1<T>(format, freqn, freqd);
			break;
		case MSeedEncoder::Steim2:
			return new MSEED::Steim2<T>(format, freqn, freqd);
			break;
		default:
			break;
	}
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MSeedEncoder::setRecordSize(int size) {
	if ( (size < 7) || (size > 20) ) {
		return false;
	}

	_recordSize = size;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedEncoder::setIdentity() {
	_compression = Identity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedEncoder::setSteim1() {
	_compression = Steim1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedEncoder::setSteim2() {
	_compression = Steim2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedEncoder::allowFloatingPointCompression(bool f) {
	_allowFloatingPointCompression = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *MSeedEncoder::feed(const Record *rec) {
	if ( !rec || !rec->data() || !rec->sampleCount() ) {
		return pop();
	}

	auto dataType = rec->data()->dataType();

	if ( _encoder ) {
		if ( _samplingFrequency != rec->samplingFrequency() ||
		     _dataType != dataType ) {
			// Reset encoder
			flush();
			reset();
		}
		else {
			// Check gap
			auto time = ENCODER->getTime();
			if ( time != rec->startTime() ) {
				if ( !ENCODER->contiguous(rec->startTime() - time) ) {
					/*
					std::cerr << "GAP detected: " << time << " -> " << rec->startTime()
					          << ": " << (diff.count() * ENCODER->clock().freqn * 2 / ENCODER->clock().freqd / 1000000)
					          << std::endl;
					*/
					flush();
					ENCODER->setTime(rec->startTime());
				}
			}
		}
	}

	if ( !_encoder ) {
		_samplingFrequency = rec->samplingFrequency();
		_dataType = dataType;

		int freqn, freqd;
		if ( !MSEED::getFraction(freqn, freqd, _samplingFrequency) ) {
			SEISCOMP_ERROR("%s: invalid sampling rate: %f",
			               rec->streamID(), _samplingFrequency);
			return pop();
		}

		auto format = new MSEED::V2::StandardFormat(
			rec->networkCode(), rec->stationCode(),
			rec->locationCode(), rec->channelCode(),
			freqn, freqd
		);
		format->recordSize = _recordSize;

		switch ( _dataType ) {
			case Array::INT:
				_encoder = createEncoder<int32_t>(_compression, format, freqn, freqd);
				break;
			case Array::FLOAT:
				if ( _allowFloatingPointCompression ) {
					_encoder = createEncoder<float>(_compression, format, freqn, freqd);
				}
				else {
					_encoder = new MSEED::Uncompressed<float>(format, freqn, freqd);
				}
				break;
			case Array::DOUBLE:
				if ( _allowFloatingPointCompression ) {
					_encoder = createEncoder<double>(_compression, format, freqn, freqd);
				}
				else {
					_encoder = new MSEED::Uncompressed<double>(format, freqn, freqd);
				}
				break;
			default:
				delete format;
				SEISCOMP_ERROR("%s: unsupported data type: %d", static_cast<int>(rec->dataType()));
				return nullptr;
		}

		if ( !_encoder ) {
			delete format;
			return pop();
		}

		ENCODER->setTime(rec->startTime());
	}

	ENCODER->setTimingQuality(rec->timingQuality());
	ENCODER->push(rec->data()->size(), rec->data()->data());

	return pop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *MSeedEncoder::flush() {
	if ( _encoder ) {
		ENCODER->flush();
		ENCODER->reset();
	}
	return pop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MSeedEncoder::reset() {
	_samplingFrequency = -1;
	_encoder = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordFilterInterface *MSeedEncoder::clone() const {
	MSeedEncoder *tmp = new MSeedEncoder;
	tmp->_recordSize = _recordSize;
	tmp->_compression = _compression;
	tmp->_allowFloatingPointCompression = _allowFloatingPointCompression;
	tmp->reset();
	return tmp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *MSeedEncoder::pop() {
	if ( _encoder ) {
		auto buf = ENCODER->pop();
		while ( buf ) {
			ITypedArrayStream<char> is(buf.get());
			auto rec = new MSeedRecord(_dataType, Record::SAVE_RAW);
			rec->read(is);
			if ( is ) {
				return rec;
			}
			buf = ENCODER->pop();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
