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


#define SEISCOMP_COMPONENT AICPicker


#include <seiscomp/logging/log.h>
#include <seiscomp/processing/picker/araic.h>
#include <seiscomp/io/records/mseedrecord.h>

#include <fstream>


using namespace std;


namespace Seiscomp {
namespace Processing {


REGISTER_POSTPICKPROCESSOR(ARAICPicker, "AIC");


namespace {


template<typename TYPE>
static double
maeda_aic_snr_const(int n, const TYPE *data, int onset, int margin) {
	// expects a properly filtered and demeaned trace
	double snr = 0, noise = 0, signal = 0;

	for ( int i = margin; i < onset; ++i ) {
		noise += data[i] * data[i];
	}

	noise = sqrt(noise / (onset-margin));

	for ( int i = onset; i < n-margin; ++i ) {
		double a = fabs(data[i]);
		if ( a > signal ) {
			signal = a;
		}
	}

	snr = 0.707 * signal / noise;

	return snr;
}


//
// AIC repicker using the simple non-AR algorithm of Maeda (1985),
// see paper of Zhang et al. (2003) in BSSA
//

template<typename TYPE>
void maeda_aic(int n, TYPE *data, int &kmin, double &snr, int margin=10) {
	// expects a properly filtered and demeaned trace

	// square trace in place
	for ( int i=0; i<n; ++i ) {
		data[i] = data[i]*data[i];
	}

	// windowed sum for variance computation
	double sumwin1 = 0, sumwin2 = 0, minaic = 0;
	int imin = margin, imax = n-margin;
	for ( int i = 0; i < n; ++i ) {
		if ( i < imin )
			sumwin1 += data[i];
		else
			sumwin2 += data[i];
	}

	for ( int k = imin; k < imax; ++k ) {
		double var1 = sumwin1/(k-1),
		       var2 = sumwin2/(n-k-1);
		double aic = k*log10(var1) + (n-k-1)*log10(var2);

		sumwin1 += data[k];
		sumwin2 -= data[k];

		data[k] = aic;
		if ( k == imin ) minaic = aic;
		if ( aic < minaic ) {
			minaic = aic;
			kmin = k;
//			snr = sqrt(var2/var1);
		}
	}

	double maxaic = data[imin] > data[imax-1] ? data[imin] : data[imax-1];

	for ( int k = 0; k < imin; ++k ) {
		data[k] = data[n-k-1] = maxaic;
	}
	snr = maeda_aic_snr_const(n, data, kmin, margin);
}


template<typename TYPE>
void maeda_aic_const(int n, const TYPE *data, int &kmin, double &snr, int margin=10) {
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


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ARAICPicker::ARAICPicker() : _dumpTraces(false) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ARAICPicker::ARAICPicker(const Core::Time& trigger)
 : Picker(trigger), _dumpTraces(false) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ARAICPicker::~ARAICPicker() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ARAICPicker::setup(const Settings &settings) {
	if ( !Picker::setup(settings) ) return false;

	settings.getValue(_config.noiseBegin, "picker.AIC.noiseBegin");
	settings.getValue(_config.signalBegin, "picker.AIC.signalBegin");
	settings.getValue(_config.signalEnd, "picker.AIC.signalEnd");
	settings.getValue(_config.snrMin, "picker.AIC.minSNR");
	settings.getValue(_dumpTraces, "picker.AIC.dump");

	settings.getValue(_filter, "picker.AIC.filter");

	if ( !_filter.empty() ) {
		string error;
		Core::SmartPointer<Filter> tmp = Filter::Create(_filter, &error);
		if ( !tmp ) {
			SEISCOMP_ERROR("failed to create filter '%s': %s",
			               _filter.c_str(), error.c_str());
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &ARAICPicker::methodID() const {
	static string method = "AIC";
	return method;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &ARAICPicker::filterID() const {
	return _filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ARAICPicker::calculatePick(int n, const double *data,
                                int signalStartIdx, int signalEndIdx,
                                int &triggerIdx, int &lowerUncertainty,
                                int &upperUncertainty, double &snr,
                                OPT(Polarity) &polarity) {
	Core::SmartPointer<Filter> filter = _filter.empty()?nullptr:Filter::Create(_filter);
	if ( filter ) {
		SEISCOMP_DEBUG("AIC: created filter %s", _filter.c_str());
		try {
			filter->setSamplingFrequency(_stream.fsamp);
		}
		catch ( std::exception &e ) {
			SEISCOMP_WARNING("Init AIC filter for fsamp = %f sps: %s",
			                 _stream.fsamp, e.what());
			return false;
		}
	}

	if ( signalEndIdx <= 0 ) return false;

	double average = 0;
	int n2 = (signalEndIdx-signalStartIdx)/2; // use only first half of seismogram
	// FIXME somewhat hackish but better than nothing
	for ( int i = 0; i < n2; ++i )
		average += data[signalStartIdx+i];
	average /= n2;

	vector<double> tmp(signalEndIdx);
	for ( int i = 0; i < signalEndIdx; ++i ) tmp[i] = data[i]-average;

	if ( _dumpTraces && _stream.lastRecord ) {
		GenericRecord gen(*_stream.lastRecord);
		gen.setStartTime(dataTimeWindow().startTime() + Core::TimeSpan(signalStartIdx/_stream.fsamp));
		gen.setData(tmp.size() - signalStartIdx, &tmp[signalStartIdx], Array::DOUBLE);
		gen.setLocationCode("AC");

		std::ofstream ofs;
		ofs.open((_stream.lastRecord->streamID() + _trigger.iso() + ".mseed").c_str());
		IO::MSeedRecord(gen).write(ofs);
		ofs.close();
	}

	if ( filter ) {
		filter->apply(tmp);

		if ( _dumpTraces && _stream.lastRecord ) {
			GenericRecord gen(*_stream.lastRecord);
			gen.setStartTime(dataTimeWindow().startTime() + Core::TimeSpan(signalStartIdx/_stream.fsamp));
			gen.setLocationCode("AF");
			gen.setData(tmp.size() - signalStartIdx, &tmp[signalStartIdx], Array::DOUBLE);

			std::ofstream ofs;
			ofs.open((_stream.lastRecord->streamID() + _trigger.iso() + "-filter.mseed").c_str());
			IO::MSeedRecord(gen).write(ofs);
			ofs.close();
		}
	}

	triggerIdx = -1;
	snr = -1;
	maeda_aic_const(signalEndIdx-signalStartIdx, &tmp[signalStartIdx], triggerIdx, snr);
	triggerIdx += signalStartIdx;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
