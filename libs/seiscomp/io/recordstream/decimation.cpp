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


#define SEISCOMP_COMPONENT DECIMATION

#include <seiscomp/logging/log.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/strings.h>

#include <string.h>

#include "decimation.h"
#include "remez/remez.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::IO;
using namespace Seiscomp::RecordStream;


REGISTER_RECORDSTREAM(Decimation, "dec");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


GenericRecord *createDecimatedRecord(Record *rec) {
	return new GenericRecord(rec->networkCode(), rec->stationCode(),
	                         rec->locationCode(), rec->channelCode(),
	                         rec->startTime(), rec->samplingFrequency());
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Decimation::Decimation() {
	// Default target rate is 1Hz
	_targetRate = 1;
	_fp = 0.7;
	_fs = 0.9;
	_coeffScale = 10;
	_nextRecord = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Decimation::~Decimation() {
	close();
	cleanup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::setSource(const string &source) {
	size_t pos;

	close();
	cleanup();

	pos = source.find('/');
	if ( pos == string::npos ) {
		SEISCOMP_ERROR("Invalid address, expected '/'");
		return false;
	}

	string name = source;
	string addr = name.substr(pos+1);
	name.erase(pos);

	string service;

	pos = name.find('?');
	if ( pos == string::npos ) {
		service = name;
	}
	else {
		service = name.substr(0, pos);
		name.erase(0, pos+1);

		vector<string> toks;
		Core::split(toks, name.c_str(), "&");
		if ( !toks.empty() ) {
			for ( std::vector<std::string>::iterator it = toks.begin();
			      it != toks.end(); ++it ) {
				std::string value;

				pos = it->find('=');
				if ( pos != std::string::npos ) {
					name = it->substr(0, pos);
					value = it->substr(pos+1);
				}
				else {
					name = *it;
					value = "";
				}

				if ( name == "rate" ) {
					double rate;
					if ( !Core::fromString(rate, value) ) {
						SEISCOMP_ERROR("Invalid decimation value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid rate parameter value");
					}

					if ( rate <= 0 ) {
						SEISCOMP_ERROR("Invalid decimation value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid rate parameter value");
					}

					_targetRate = rate;
				}
				else if ( name == "fp") {
					double fp;
					if ( !Core::fromString(fp, value) ) {
						SEISCOMP_ERROR("Invalid decimation value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid fp parameter value");
					}

					if ( fp <= 0 ) {
						SEISCOMP_ERROR("Invalid decimation value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid fp parameter value");
					}

					_fp = fp;
				}
				else if ( name == "fs") {
					double fs;
					if ( !Core::fromString(fs, value) ) {
						SEISCOMP_ERROR("Invalid decimation value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid fs parameter value");
					}

					if ( fs <= 0 ) {
						SEISCOMP_ERROR("Invalid decimation value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid fs parameter value");
					}

					_fs = fs;
				}
				else if ( name == "cs") {
					int cs;
					if ( !Core::fromString(cs, value) ) {
						SEISCOMP_ERROR("Invalid decimation value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid cs parameter value");
					}

					if ( cs <= 0 ) {
						SEISCOMP_ERROR("Invalid decimation value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid cs parameter value");
					}

					_coeffScale = cs;
				}
			}
		}
	}

	SEISCOMP_DEBUG("Set decimation proxy stream to %s://%s", service, addr);

	_source = Create(service.c_str());
	if ( !_source ) {
		SEISCOMP_ERROR("Unable to create proxy service: %s",
		               service.c_str());
		return false;
	}

	try {
		if ( !_source->setSource(addr) )
			return false;
	}
	catch ( ... ) {
		_source = nullptr;
		throw;
	}

	_source->setDataType(Array::DOUBLE);
	_source->setDataHint(Record::DATA_ONLY);

	_maxN = 500/_coeffScale;
	if ( _maxN < 2 ) {
		SEISCOMP_ERROR("Unable to compute filter stages with given cs");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::setRecordType(const char *type) {
	if ( _source ) return _source->setRecordType(type);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha) {
	return _source->addStream(net, sta, loc, cha);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha,
                           const OPT(Core::Time) &stime,
                           const OPT(Core::Time) &etime) {
	return _source->addStream(net, sta, loc, cha, stime, etime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::setStartTime(const OPT(Core::Time) &stime) {
	return _source->setStartTime(stime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::setEndTime(const OPT(Core::Time) &etime) {
	return _source->setEndTime(etime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::setTimeWindow(const Core::TimeWindow &w) {
	return _source->setTimeWindow(w);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::setTimeout(int seconds) {
	return _source->setTimeout(seconds);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Decimation::close() {
	if ( _source ) {
		SEISCOMP_DEBUG("Closing proxy source");
		_source->close();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Decimation::cleanup() {
	// Clean stream stages
	{
		StreamMap::iterator it;
		for ( it = _streams.begin(); it != _streams.end(); ++it )
			delete it->second;
		_streams.clear();
	}

	{
		CoefficientMap::iterator it;
		for ( it = _coefficients.begin(); it != _coefficients.end(); ++it )
			delete it->second;
		_coefficients.clear();
	}

	_source = nullptr;

	if ( _nextRecord != nullptr ) delete _nextRecord;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *Decimation::next() {
	if ( !_source ) {
		SEISCOMP_ERROR("[dec] no source defined");
		return nullptr;
	}

	while ( true ) {
		RecordPtr rec = _source->next();
		if ( rec ) {
			// If new data has been pushed to stream, return
			if ( push(rec.get()) ) {
				GenericRecord *rec = _nextRecord;
				_nextRecord = nullptr;

				if ( rec->data()->dataType() != _dataType ) {
					rec->setData(rec->data()->copy(_dataType));
				}

				return rec;
			}
		}
		else
			break;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Decimation::checkSR(Record *rec) const {
	if ( rec->samplingFrequency() <= _targetRate ) {
		SEISCOMP_DEBUG("[dec] %s: sr of %.1f <= %.1f -> pass through",
		               rec->streamID().c_str(), rec->samplingFrequency(),
		               _targetRate);
		return -1;
	}

	// Check if the N is an integer.
	double N = rec->samplingFrequency() / _targetRate;
	if ( fabs(N - int(N+0.5)) > 1E-05 ) {
		SEISCOMP_DEBUG("[dec] %s: sr/tsr = %f which is not an integer -> "
		               "pass through", rec->streamID().c_str(), N);
		return -1;
	}

	return int(N+0.5);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::initCoefficients(ResampleStage *stage) {
	CoefficientMap::iterator it = _coefficients.find(stage->N);
	if ( it != _coefficients.end() ) {
		if ( it->second == nullptr )
			// Invalid coefficients for that stage?
			return false;

		stage->coefficients = it->second;
	}
	else {
		stage->coefficients = nullptr;

		if ( stage->N > _maxN ) {
			for ( int i = _maxN; i > 1; --i ) {
				if ( stage->N % i == 0 ) {
					int nextStageN = stage->N / i;

					if ( nextStageN > _maxN ) {
						SEISCOMP_WARNING("[dec] max decimations exceeded: %d > %d",
						                 nextStageN, _maxN);
						return false;
					}

					SEISCOMP_DEBUG("[dec] clipping N=%d to %d and create sub stage",
					               stage->N, i);

					stage->N = i;
					stage->targetRate = stage->sampleRate / stage->N;

					ResampleStage *nextStage = new ResampleStage;
					nextStage->sampleRate = stage->targetRate;
					nextStage->targetRate = _targetRate;
					nextStage->N = nextStageN;
					if ( !initCoefficients(nextStage) ) {
						delete nextStage;
						return false;
					}

					stage->nextStage = nextStage;

					break;
				}
			}

			CoefficientMap::iterator it = _coefficients.find(stage->N);
			if ( it != _coefficients.end() )
				stage->coefficients = it->second;
		}

		if ( stage->coefficients == nullptr ) {
			// Create and cache coefficients for N
			int Ncoeff = stage->N*_coeffScale*2+1;

			Coefficients *coeff = new Coefficients(Ncoeff);

			double bands[4] = {0,0.5*(_fp/stage->N),0.5*(_fs/stage->N),0.5};
			double weights[2] = {1,1};
			double desired[2] = {1,0};

			if ( remez(coeff->data(), Ncoeff, 2, bands, desired, weights, BANDPASS) ) {
				SEISCOMP_WARNING("[dec] failed to build coefficients for N=%d, ignore stream", stage->N);
				delete coeff;
				_coefficients[stage->N] = nullptr;
				return false;
			}

			SEISCOMP_DEBUG("[dec] caching %d coefficents for N=%d", Ncoeff, stage->N);

			_coefficients[stage->N] = coeff;
			stage->coefficients = coeff;
		}
	}

	stage->dt = 1.0 / stage->sampleRate;
	stage->N2 = stage->coefficients->size() / 2;
	stage->buffer.resize(stage->coefficients->size());
	stage->reset();
	stage->valid = true;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Decimation::init(ResampleStage *stage, Record *rec) {
	stage->N = checkSR(rec);
	stage->passThrough = stage->N <= 0;
	stage->targetRate = _targetRate;
	stage->sampleRate = rec->samplingFrequency();
	stage->dt = 0;
	stage->valid = true;

	if ( !stage->passThrough )
		stage->valid = initCoefficients(stage);
	else
		stage->coefficients = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Decimation::push(Record *rec) {
	// Delete next record if still active which should never be the case
	if ( _nextRecord != nullptr ) {
		delete _nextRecord;
		_nextRecord = nullptr;
	}

	string id = rec->streamID();
	ResampleStage *stage;
	StreamMap::iterator it = _streams.find(id);

	if ( it == _streams.end() ) {
		std::pair<StreamMap::iterator, bool> itp;
		stage = new ResampleStage;
		itp = _streams.insert(StreamMap::value_type(id, stage));
		it = itp.first;
		init(stage, rec);
	}
	else {
		stage = it->second;

		// Sample rate changed? Check new settings and reset
		// the resampling.
		if ( stage->sampleRate != rec->samplingFrequency() ) {
			stage->reset();
			if ( stage->nextStage ) {
				delete stage->nextStage;
				stage->nextStage = nullptr;
			}

			init(stage, rec);
		}
	}

	if ( stage->passThrough )
		_nextRecord = convert(rec);
	else
		_nextRecord = resample(stage, rec);

	return _nextRecord != nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GenericRecord *Decimation::convert(Record *rec) {
	if ( rec->data() == nullptr ) return nullptr;

	GenericRecord *out;
	switch ( rec->dataType() ) {
		case Array::CHAR:
		case Array::INT:
		case Array::FLOAT:
		case Array::DOUBLE:
			out = createDecimatedRecord(rec);
			break;
		default:
			return nullptr;
	}

	ArrayPtr data = rec->data() ? rec->data()->copy(_dataType) : nullptr;
	out->setData(data.get());

	return out;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GenericRecord *Decimation::resample(ResampleStage *stage, Record *rec) {
	if ( !stage->valid )
		return nullptr;

	Core::Time endTime;
	try {
		endTime = rec->endTime();
	}
	catch ( ... ) {
		SEISCOMP_WARNING("[dec] %s: invalid end time -> ignoring",
		                 rec->streamID().c_str());
		return nullptr;
	}

	if ( stage->lastEndTime ) {
		double diff = (rec->startTime() - *stage->lastEndTime).length();
		if ( fabs(diff) > stage->dt*0.5 ) {
			if ( diff < 0 )
				// Ignore overlap
				return nullptr;

			stage->reset();
		}
	}

	stage->lastEndTime = endTime;

	ArrayPtr tmp_ar;
	const DoubleArray *ar = DoubleArray::ConstCast(rec->data());
	if ( !ar ) {
		tmp_ar = rec->data() ? rec->data()->copy(Array::DOUBLE) : nullptr;
		if ( !tmp_ar ) {
			SEISCOMP_WARNING("[dec] %s: %s ~ %s: no data -> ignoring",
			                 rec->streamID(), rec->startTime().iso(),
			                 rec->endTime().iso());
			return nullptr;
		}
		ar = DoubleArray::ConstCast(tmp_ar);
		if ( !ar ) {
			SEISCOMP_ERROR("[dec] internal error: doubles expected");
			return nullptr;
		}
	}

	size_t data_len = (size_t)ar->size();
	const double *data = ar->typedData();
	double *buffer = stage->buffer.data();

	if ( stage->missingSamples > 0 ) {
		size_t toCopy = std::min(stage->missingSamples, data_len);
		memcpy(buffer + stage->buffer.size() - stage->missingSamples,
		       data, toCopy*sizeof(double));
		data += toCopy;
		data_len -= toCopy;
		stage->missingSamples -= toCopy;

		if ( !stage->startTime ) {
			stage->startTime = rec->startTime();
		}

		// Still samples missing and no more data available, return
		if ( stage->missingSamples > 0 ) {
			return nullptr;
		}

		// Resampling can start now
		stage->samplesToSkip = 0;
	}

	// Ring buffer is filled at this point.
	DoubleArrayPtr resampled_data;
	Core::Time startTime;

	do {
		if ( stage->samplesToSkip == 0 ) {
			// Calculate scalar product of coefficients and ring buffer
			double *coeff = stage->coefficients->data();
			double sample = 0;

			for ( size_t i = stage->front; i < stage->buffer.size(); ++i ) {
				sample += buffer[i] * *(coeff++);
			}
			for ( size_t i = 0; i < stage->front; ++i ) {
				sample += buffer[i] * *(coeff++);
			}

			if ( !resampled_data ) {
				startTime = *stage->startTime + Core::TimeSpan(stage->dt * stage->N2);
				resampled_data = new DoubleArray;
			}

			resampled_data->append(1, &sample);

			// Still need to wait until N samples have been fed.
			stage->samplesToSkip = stage->N;
		}

		size_t num_samples = std::min(stage->samplesToSkip, data_len);

		size_t chunk_size = std::min(num_samples, stage->buffer.size()-stage->front);
		memcpy(buffer + stage->front, data, chunk_size*sizeof(double));

		data += chunk_size;

		// Split chunks
		if ( chunk_size < num_samples ) {
			chunk_size = num_samples - chunk_size;

			memcpy(buffer, data, chunk_size*sizeof(double));

			stage->front = chunk_size;

			data += chunk_size;
		}
		else {
			stage->front += chunk_size;
			if ( stage->front >= stage->buffer.size() )
				stage->front -= stage->buffer.size();
		}

		*stage->startTime += Core::TimeSpan(stage->dt * num_samples);
		stage->samplesToSkip -= num_samples;
		data_len -= num_samples;
	}
	while ( data_len > 0 );

	// Create the record and push it
	if ( resampled_data ) {
		GenericRecord *grec;
		grec = new GenericRecord(rec->networkCode(), rec->stationCode(),
		                         rec->locationCode(), rec->channelCode(),
		                         startTime, stage->targetRate);
		grec->setData(resampled_data.get());

		if ( stage->nextStage ) {
			GenericRecord *nrec = resample(stage->nextStage, grec);
			delete grec;
			grec = nrec;
		}

		return grec;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
