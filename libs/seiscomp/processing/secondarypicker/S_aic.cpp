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


#define SEISCOMP_COMPONENT SAICPicker

#include <seiscomp/logging/log.h>
#include <seiscomp/processing/operator/ncomps.h>
#include <seiscomp/math/filter.h>

#include "S_aic.h"


using namespace std;

namespace Seiscomp {

namespace Processing {

namespace {

template<typename TYPE>
double
maeda_aic_snr_const(int n, const TYPE *data, int onset, int margin) {
	// expects a properly filtered and demeaned trace
	double snr=0, noise=0, signal=0;
	for (int i=margin; i<onset; i++)
		noise += data[i]*data[i];
	noise = sqrt(noise/(onset-margin));
	for (int i=onset; i<n-margin; i++) {
		double a=fabs(data[i]);
		if (a>signal) signal=a;
	}
	snr = 0.707*signal/noise;

	return snr;
}


template<typename TYPE>
void
maeda_aic(int n, const TYPE *data, int &kmin, double &snr, int margin=10) {
	// expects a properly filtered and demeaned trace

	// windowed sum for variance computation
	double sumwin1 = 0, sumwin2 = 0, minaic = 0;
	int imin = margin, imax = n-margin;
	for ( int i = 0; i < n; ++i ) {
		TYPE squared = data[i]*data[i];
		if ( i < imin )
			sumwin1 += squared;
		else
			sumwin2 += squared;
	}

	for ( int k = imin; k < imax; ++k ) {
		double var1 = sumwin1/(k-1),
		       var2 = sumwin2/(n-k-1);
		double aic = k*log10(var1) + (n-k-1)*log10(var2);
		TYPE squared = data[k]*data[k];

		sumwin1 += squared;
		sumwin2 -= squared;

		if ( (k == imin) || (aic < minaic) ) {
			minaic = aic;
			kmin = k;
		}
	}

	snr = maeda_aic_snr_const(n, data, kmin, margin);
}

}


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SAICPicker::State::State() : aicValid(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SAICPicker::SAICPicker(const string& methodID, StreamComponent c)
	         : _methodID(methodID) {
	setUsedComponent(c);
	// Use ten seconds as noise to initialize the filter
	setNoiseStart(-10);
	// Start checking at onset time
	setSignalStart(0);
	// Check for S picks one minute after P
	setSignalEnd(60);
	setMargin(Core::TimeSpan(0,0));
	_aicConfig.threshold = 3;
	_aicConfig.timeCorr = 0;
	_aicConfig.minSNR = 15;
	_aicConfig.margin = 5.0;
	_initialized = false;
	_compFilter = nullptr;
	_saveIntermediate = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SAICPicker::~SAICPicker() {
	if ( _compFilter ) delete _compFilter;

	/*
	if ( lastRecord() ) {
		GenericRecord rec(lastRecord()->networkCode(),
		                  lastRecord()->stationCode(),
		                  "AIC",
		                  lastRecord()->channelCode(),
		                  dataTimeWindow().startTime(), lastRecord()->samplingFrequency());
		ArrayPtr data = continuousData().clone();
		rec.setData(data.get());
		IO::MSeedRecord mseed(rec);
		mseed.write(cout);
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SAICPicker::setSaveIntermediate(bool e) {
	_saveIntermediate = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &SAICPicker::methodID() const {
	return _methodID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &SAICPicker::filterID() const {
	return _aicConfig.filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SAICPicker::setAicConfig(const AICConfig &config) {
	_aicConfig = config;
	return applyConfig();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SAICPicker::applyConfig() {
	_initialized = false;
	_state = State();
	_detectionTrace.clear();

	setOperator(nullptr);

	if ( _compFilter ) {
		delete _compFilter;
		_compFilter = nullptr;
	}

	if ( !_aicConfig.filter.empty() ) {
		_compFilter = Filter::Create(_aicConfig.filter.c_str());
		if ( _compFilter == nullptr ) {
			SEISCOMP_WARNING("[S-%s]: wrong component filter definition: %s",
			                 _methodID.c_str(), _aicConfig.filter.c_str());
			return false;
		}
	}
	else
		_compFilter = nullptr;

	if ( !_aicConfig.detecFilter.empty() ) {
		Filter *filter = Filter::Create(_aicConfig.detecFilter.c_str());
		if ( filter == nullptr ) {
			SEISCOMP_WARNING("[S-%s]: wrong filter definition: %s",
			                 _methodID.c_str(), _aicConfig.detecFilter.c_str());
			return false;
		}

		setFilter(filter);
	}
	else
		setFilter(nullptr);

	WaveformOperatorPtr op( createFilterOperator(_compFilter) );
	setOperator(op.get()); 

	_initialized = true;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SAICPicker::fill(size_t n, double *samples) {
	// Disable dectection filter since we need the frequency filtered data.
	// The detection filter is applied during process
	Filter *tmp = _stream.filter;
	_stream.filter = nullptr;
	SecondaryPicker::fill(n, samples);
	// Restore detection filter
	_stream.filter = tmp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SAICPicker::process(const Record *rec, const DoubleArray &filteredData) {
	/*
	IO::MSeedRecord mseed(*rec);
	mseed.setLocationCode("AIC");
	mseed.write(cout);
	*/

	if ( !_initialized ) return;

	// The result of the L2 norm operator is stored in the vertical component
	// and already sensitivity corrected.
	size_t n = filteredData.size();
	int ni = 0;
	int si = 0;

	Core::Time noiseBegin = _trigger.onset + Core::TimeSpan(_config.noiseBegin);
	Core::Time signalBegin = _trigger.onset + Core::TimeSpan(_config.signalBegin);
	if ( rec->startTime() < noiseBegin )
		ni = (int)(double(noiseBegin - rec->startTime()) * rec->samplingFrequency());
	if ( rec->startTime() < signalBegin )
		si = (int)(double(signalBegin - rec->startTime()) * rec->samplingFrequency());

	double progress = (_config.signalEnd - _config.signalBegin) / double(dataTimeWindow().endTime() - _trigger.onset);
	if ( progress < 0 ) progress = 0;
	else if ( progress > 100 ) progress = 100;

	setStatus(InProgress, progress);

	// Active trigger but AIC time window not filled during last call?
	if ( _result.time.valid() && _aicConfig.margin > 0 ) {
		int kmin;
		double snr;
		double trigger = _result.time - _trigger.onset;
		int ti = (int)(double(_result.time - dataTimeWindow().startTime()) * _stream.fsamp);
		int ri = (int)(double(_trigger.onset - dataTimeWindow().startTime()) * _stream.fsamp);
		int ai = ti - (int)(_aicConfig.margin*_stream.fsamp);
		int ae = ti + (int)(_aicConfig.margin*_stream.fsamp);

		_state.aicStart = trigger - _aicConfig.margin;
		_state.aicEnd = trigger + _aicConfig.margin;
		if ( _state.aicStart < _config.signalBegin ) _state.aicStart = _config.signalBegin;
		if ( _state.aicEnd < _config.signalBegin ) _state.aicEnd = _config.signalBegin;

		si = ri + (int)(_config.signalBegin*_stream.fsamp);
		bool skip = ae > continuousData().size();

		if ( skip ) return;

		_state.aicValid = true;

		// Clip to signal begin
		if ( ai < si ) ai = si;

		maeda_aic(ae-ai, continuousData().typedData()+ai, kmin, snr);

		SEISCOMP_DEBUG("[S-%s] %s: AIC [%d;%d] = %d, ref: %d, si: %d, ri: %d, sb: %f",
		               _methodID.c_str(), rec->streamID().c_str(), ai, ae, kmin+ai, ti, si, ri, _config.signalBegin);

		double t = (double)(kmin+ai)/(double)continuousData().size();
		_result.time = dataTimeWindow().startTime() + Core::TimeSpan(t*dataTimeWindow().length());
		_state.pick = _result.time;
		_state.snr = snr;

		if ( snr < _aicConfig.minSNR ) {
			_initialized = false;
			SEISCOMP_DEBUG("[S-%s] %s: snr %f too low at %s, need %f",
			                _methodID.c_str(), rec->streamID().c_str(), snr,
			               _result.time.iso().c_str(), _aicConfig.minSNR);
			setStatus(LowSNR, snr);
			_result = Result();
			return;
		}

		if ( _result.time <= _trigger.onset ) {
			_initialized = false;
			SEISCOMP_DEBUG("[S-%s] %s: pick at %s is before trigger at %s: rejected",
			               _methodID.c_str(), rec->streamID().c_str(),
			               _result.time.iso().c_str(),
			               _trigger.onset.iso().c_str());
			setStatus(Terminated, 1);
			_result = Result();
			return;
		}

		_result.snr = snr;
	}

	size_t idx = si;
	//std::cerr << n << " " << idx << "  " << ni << std::endl;

	// If no pick has been found, check the data
	if ( !_result.time.valid() ) {
		if ( (si > 0) && _stream.filter ) {
			size_t m = idx>n?n:idx;
			//std::cerr << "Filter " << m-ni << " samples" << std::endl;
			for ( size_t i = ni; i < m; ++i ) {
				double fv = filteredData[i];
				_stream.filter->apply(1, &fv);
				if ( _saveIntermediate ) _detectionTrace.append(1, fv);
				//std::cout << filteredData[i] << " " << fv << std::endl;
			}
		}

		//std::cerr << "Work on " << n-idx << " samples" << std::endl;
		for ( ; idx < n; ++idx ) {
			double v = filteredData[idx];
			double fv(v);

			// Apply detection filter
			if ( _stream.filter )
				_stream.filter->apply(1, &fv);

			if ( _saveIntermediate ) _detectionTrace.append(1, fv);

			//std::cout << v << " " << fv << std::endl;

			// Trigger detected
			if ( fv >= _aicConfig.threshold ) {
				double t = (double)idx/(double)n;
				_result.time = rec->startTime() + Core::TimeSpan(rec->timeWindow().length() * t) + Core::TimeSpan(_aicConfig.timeCorr);
				_result.timeLowerUncertainty = _result.timeUpperUncertainty = -1;
				_result.snr = -1;
				_state.detection = _result.time;

				SEISCOMP_DEBUG("[S-%s] %s: detection at %s with value %f",
				               _methodID.c_str(), rec->streamID().c_str(),
				               _result.time.iso().c_str(), fv);

				if ( _aicConfig.margin > 0 ) {
					int kmin;
					double snr;
					double trigger = _result.time - _trigger.onset;
					int ti = (int)(double(_result.time - dataTimeWindow().startTime()) * _stream.fsamp);;
					int ri = (int)(double(_trigger.onset - dataTimeWindow().startTime()) * _stream.fsamp);
					int ai = ti - (int)(_aicConfig.margin*_stream.fsamp);
					int ae = ti + (int)(_aicConfig.margin*_stream.fsamp);

					_state.aicStart = trigger - _aicConfig.margin;
					_state.aicEnd = trigger + _aicConfig.margin;
					if ( _state.aicStart < _config.signalBegin ) _state.aicStart = _config.signalBegin;
					if ( _state.aicEnd < _config.signalBegin ) _state.aicEnd = _config.signalBegin;

					si = ri + (int)(_config.signalBegin*_stream.fsamp);
					if ( ae > continuousData().size() )
						return;

					_state.aicValid = true;

					// Clip to signal begin
					if ( ai < si ) ai = si;

					SEISCOMP_DEBUG("[S-%s] %s: AIC [%d;%d] = %d, ref: %d",
					               _methodID.c_str(), rec->streamID().c_str(),
					               ai, ae, kmin+ai, ti);

					maeda_aic(ae-ai, continuousData().typedData()+ai, kmin, snr);

					t = (double)(kmin+ai)/(double)continuousData().size();
					_result.time = dataTimeWindow().startTime() + Core::TimeSpan(t*dataTimeWindow().length());
					_state.pick = _result.time;
					_state.snr = snr;

					if ( snr < _aicConfig.minSNR ) {
						_result = Result();
						_initialized = false;
						SEISCOMP_DEBUG("[S-%s] %s: snr %f too low at %s, need %f",
						               _methodID.c_str(), rec->streamID().c_str(),
						               snr, _result.time.iso().c_str(),
						               _aicConfig.minSNR);
						setStatus(LowSNR, snr);
						return;
					}

					_result.snr = snr;
				}

				if ( _result.time <= _trigger.onset ) {
					_result = Result();
					_initialized = false;
					SEISCOMP_DEBUG("[S-%s] %s: pick at %s is before trigger at %s: rejected",
					               _methodID.c_str(), rec->streamID().c_str(),
					               _result.time.iso().c_str(),
					               _trigger.onset.iso().c_str());
					setStatus(Terminated, 1);
					return;
				}

				break;
			}
		}
	}

	// Time still valid, emit pick
	if ( _result.time.valid() ) {
		_result.phaseCode = "S";
		_result.record = rec;
		SEISCOMP_DEBUG("[S-%s] %s: %s pick at %s with snr=%f",_methodID.c_str(),
		               rec->streamID().c_str(), _result.phaseCode.c_str(),
		               _result.time.iso().c_str(),
		               _result.snr);
		setStatus(Finished, 100);
		_initialized = false;
		emitPick(_result);

		// Reset result
		_result = Result();

		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
