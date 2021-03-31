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


#define SEISCOMP_COMPONENT Resample

#include <seiscomp/logging/log.h>
#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/recordfilter/resample.h>
#include <seiscomp/io/recordstream/resample.h>

#include <string.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::IO;
using namespace Seiscomp::RecordStream;


REGISTER_RECORDSTREAM(Resample, "resample");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Resample::Resample() {
	// Default target rate is 1Hz
	_debug = false;
	_targetRate = 1.0;
	_fp = 0.7;
	_fs = 0.9;
	_coeffScale = 10;
	_lanczosKernelWidth = 3;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Resample::~Resample() {
	close();
	cleanup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Resample::setSource(const string &source) {
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
	if ( pos == string::npos )
		service = name;
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
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid rate parameter value");
					}

					if ( rate <= 0 ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid rate parameter value");
					}

					_targetRate = rate;
				}
				else if ( name == "fp") {
					double fp;
					if ( !Core::fromString(fp, value) ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid fp parameter value");
					}

					if ( fp <= 0 ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid fp parameter value");
					}

					_fp = fp;
				}
				else if ( name == "fs") {
					double fs;
					if ( !Core::fromString(fs, value) ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid fs parameter value");
					}

					if ( fs <= 0 ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid fs parameter value");
					}

					_fs = fs;
				}
				else if ( name == "cs") {
					int cs;
					if ( !Core::fromString(cs, value) ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid cs parameter value");
					}

					if ( cs <= 0 ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid cs parameter value");
					}

					_coeffScale = cs;
				}
				else if ( name == "lw") {
					int lw;

					if ( !Core::fromString(lw, value) ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a numerical value",
						               name.c_str());
						throw RecordStreamException("invalid lw parameter value");
					}

					if ( lw <= 0 ) {
						SEISCOMP_ERROR("Invalid resample value for '%s': expected a positive value",
						               name.c_str());
						throw RecordStreamException("invalid lw parameter value");
					}

					_lanczosKernelWidth = lw;
				}
				else if ( name == "debug") {
					_debug = true;
				}
			}
		}
	}

	_source = Create(service.c_str());
	if ( !_source ) {
		SEISCOMP_ERROR("Unable to create proxy service: %s",
		               service.c_str());
		return false;
	}

	if ( !_source->setSource(addr) ) {
		SEISCOMP_ERROR("Failed to set proxy source: %s", addr.data());
		return false;
	}

	_source->setDataType(Array::DOUBLE);
	_source->setDataHint(Record::DATA_ONLY);

	if ( 500/_coeffScale < 2 ) {
		SEISCOMP_ERROR("Unable to compute filter stages with given cs");
		return false;
	}

	// Set the resampler record filter for the demuxer
	_demuxer.setFilter(new IO::RecordResampler<double>(_targetRate, _fp, _fs, _coeffScale, _lanczosKernelWidth));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Resample::setRecordType(const char *type) {
	if ( _source ) return _source->setRecordType(type);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Resample::addStream(const string &net, const string &sta,
                         const string &loc, const string &cha) {
	return _source->addStream(net, sta, loc, cha);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Resample::addStream(const string &net, const string &sta,
                         const string &loc, const string &cha,
                         const Seiscomp::Core::Time &stime,
                         const Seiscomp::Core::Time &etime) {
	return _source->addStream(net, sta, loc, cha, stime, etime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Resample::setStartTime(const Seiscomp::Core::Time &stime) {
	return _source->setStartTime(stime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Resample::setEndTime(const Seiscomp::Core::Time &etime) {
	return _source->setEndTime(etime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Resample::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	return _source->setTimeWindow(w);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Resample::setTimeout(int seconds) {
	return _source->setTimeout(seconds);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Resample::close() {
	if ( _source ) {
		SEISCOMP_DEBUG("Closing proxy source");
		_source->close();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Resample::cleanup() {
	_demuxer = RecordDemuxFilter();

	OutputQueue::iterator it = _queue.begin();
	for ( ; it != _queue.end(); ++it )
		delete *it;

	_source = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Resample::push(Record *rec) {
	Record *out_rec = _demuxer.feed(rec);
	GenericRecord *out = GenericRecord::Cast(out_rec);

	/*
	if ( _debug ) {
		GenericRecord *pt = new GenericRecord(*rec);
		pt->setData(rec->data()->clone());
		_queue.push_back(pt);
	}
	*/

	if ( out != nullptr ) {
		if ( _debug ) out->setLocationCode("RS");
		_queue.push_back(out);
	}
	else if ( out_rec != nullptr )
		delete out_rec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *Resample::next() {
	if ( !_source ) {
		SEISCOMP_ERROR("[resample] no source defined");
		return nullptr;
	}

	while ( true ) {
		if ( !_queue.empty() ) {
			GenericRecord *r = _queue.front();
			_queue.pop_front();
			r->setDataType(_dataType);
			r->setHint(_hint);

			if ( r->data()->dataType() != r->dataType() )
				r->setData(r->data()->copy(r->dataType()));

			return r;
		}

		RecordPtr pms = _source->next();
		if ( pms )
			push(pms.get());
		else
			break;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
