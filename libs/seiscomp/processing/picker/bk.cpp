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


#define SEISCOMP_COMPONENT Picker

#include <seiscomp/logging/log.h>
#include <seiscomp/math/filter/butterworth.h>
#include <seiscomp/processing/picker/bk.h>


#include "./bk_private.cpp"


using namespace std;


namespace Seiscomp {
namespace Processing {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_POSTPICKPROCESSOR(BKPicker, "BK");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BKPicker::setup(const Settings &settings){
	if ( !Picker::setup(settings) ) {
		return false;
	}

	// do config stuff here

	try {
		_config.signalBegin = settings.getDouble("picker.BK.signalBegin");
		SEISCOMP_DEBUG("signalBegin read from config: %f",_config.signalBegin);
	}
	catch (...) {
		_config.signalBegin = -20;
		SEISCOMP_DEBUG("signalBegin not read from config, defaults to: %f",_config.signalBegin);
	}

	try {
		_config.signalEnd = settings.getDouble("picker.BK.signalEnd");
	}
	catch (...) {
		_config.signalEnd = 80;
	}

	try {
		_config.noiseBegin = settings.getDouble("picker.BK.noiseBegin");
		SEISCOMP_DEBUG("noiseBegin read from config: %f",_config.noiseBegin);
	}
	catch (...) {
		_config.noiseBegin = _config.signalBegin;
		SEISCOMP_DEBUG("noiseBegin not read from config, defaults to signalBegin: %f", _config.noiseBegin);
	}

	try {
		filterType = settings.getString("picker.BK.filterType");
		SEISCOMP_DEBUG("filter type from config: %s",filterType.c_str());
	}
	catch (...) {
		filterType = "BP"; // default is Bandpass Filter
		SEISCOMP_DEBUG("filter type default: %s",filterType.c_str());
	}

	try {
		filterPoles = settings.getInt("picker.BK.filterPoles");
		SEISCOMP_DEBUG("filterPoles from config: %d",filterPoles);
	}
	catch (...) {
		filterPoles = 2; // defaults to 2 poles
		SEISCOMP_DEBUG("filterPoles default: %d",filterPoles);
	}

	// bandpass lower cutoff freq. in Hz
	try {
		f1 = settings.getDouble("picker.BK.f1");
		SEISCOMP_DEBUG("f1 from config: %f",f1);
	}
	catch (...) {
		f1 =  5;
		SEISCOMP_DEBUG("f1 default: %f",f1);
	}

	// bandpass upper cutoff freq. in Hz
	try {
		f2 = settings.getDouble("picker.BK.f2");
		SEISCOMP_DEBUG("f2 from config: %f",f2);
	}
	catch (...) {
		f2 = 20;
		SEISCOMP_DEBUG("f2 default: %f",f2);
	}

	// thresholds
	try {
		thrshl1 = settings.getDouble("picker.BK.thrshl1");
		SEISCOMP_DEBUG("thrshl1 from config: %f",thrshl1);
	}
	catch (...) {
		thrshl1 = 10;
		SEISCOMP_DEBUG("thrshl1 default: %f",thrshl1);
	}

	try {
		thrshl2 = settings.getDouble("picker.BK.thrshl2");
		SEISCOMP_DEBUG("thrshl2 from config: %f",thrshl2);
	}
	catch (...) {
		thrshl2 = 20;
		SEISCOMP_DEBUG("thrshl2 default: %f",thrshl2);
	}

	try {
		debugOutput = settings.getBool("picker.BK.debug")?1:0;
	}
	catch ( ... ) {
		debugOutput = 0;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// bk_wrapper by Stefan Heimers 2010
//
// bk_wrapper replaces the maeda_aic() algorithm of the gfz picker
// using ppick() from picker_ppick.cpp which was converted
// from Manfred Baers picker_ppick.f
void BKPicker::bk_wrapper(int n, double *data, int &kmin, double &snr,
                          double samplespersec){
	double *reltrc=data; // timeseries as floating data, possibly filtered
	double *trace=data;  // again ??? TODO: check
	int npts=n;  // number of datapoints in the timeseries

	//  if dtime exceeds tdownmax, the trigger is examined for validity
	int tdownmax= samplespersec * (1/f1 + 1/f2)/2; // for bandpass
	//  tdownmax= samplespersec * 1/f1; // for highpass
	// tdownmax= samplespersec * 1/f2;  // for lowpass
	// tdownmax = samplespersec; // default

	// min nr of samples for itrm to be accepted as a pick */
	int tupevent= samplespersec * 1/f1; // for bandpass
	// tupevent= samplespersec * 1/f1; // for highpass
	// tupevent= samplespersec * 1; // for lowpass
	// tupevent = samplespersec; // default

	int ipkflg;
	int uptime;
	int *ptime = &kmin;   // sample number of parrival
	int pamp;
	int pamptime;
	int preptime;
	int prepamp;
	int prepamptime;
	int ifrst;
	int noise;
	int noisetime;
	int signal; // maximum signal amplitude
	int signaltime; // sample number of signal amplitude
	int nanf;
	int nend;
	int skip=0; // number of samples of data[] to skip. if skip==0 it's 3 * samplespersec
	int prset=0; // ?
	int pickduration = n / (int)samplespersec;
	char pfm;

	ppick(reltrc, trace, npts, &thrshl1, &thrshl2, &tdownmax, &tupevent,
	      &ipkflg, &uptime, ptime, &pamp, &pamptime,
	      &preptime, &prepamp, &prepamptime,
	      &ifrst, &noise, &noisetime, &signal,
	      &signaltime, &debugOutput, &nanf, &nend,
	      &skip, &prset, &pickduration,
	      &samplespersec, &pfm);

	snr = (double)signal / (double)noise;
	SEISCOMP_DEBUG(" bk_wrapper() signal: %d noise: %d  snr: %f",signal,noise,snr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BKPicker::BKPicker() {
	setMinSNR(0.);
	// Setup defaults
	_config.signalBegin = -20;
	_config.signalEnd = 80;
	filterType = "BP"; // default is Bandpass Filter
	filterPoles = 2; // defaults to 2 poles
	f1 =  5;
	f2 = 20;
	thrshl1 = 10;
	thrshl2 = 20;
	debugOutput = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BKPicker::BKPicker(const Core::Time& trigger)
: Picker(trigger) {
	BKPicker();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BKPicker::~BKPicker() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &BKPicker::methodID() const {
	static string method = "BK";
	return method;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &BKPicker::filterID() const {
	return usedFilter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BKPicker::calculatePick(int ndata, const double *data,
                             int signalStartIndex, int signalEndIndex,
                             int &onsetIndex, int &lowerUncertainty,
                             int &upperUncertainty, double &snr,
                             OPT(Polarity) &polarity) {
	// Initially, onsetIndex contains the index of the triggering sample.
	const int     n = signalEndIndex - signalStartIndex;
	const double *f = data+signalStartIndex;

	if ( n <= 10 ) {
		return false;
	}

	// Here we assume that the first third of the seismogram contains only noise.	
	//int nnoise = n/3; // FIXME: somewhat hackish
	int nnoise = static_cast<int>(0.8 * _stream.fsamp * abs(_config.signalBegin)); // is this better?

	// determine offset
	double offset = 0;
	for ( int i = 0; i < nnoise; ++i ) {
		offset += f[i];
	}
	offset /= nnoise;

	vector<double> tmp(n);
	for ( int i = 0; i < n; ++i ) {
		tmp[i] = f[i]-offset;
	}

	usedFilter = "";

	if ( filterType == "BP" ) {
		SEISCOMP_DEBUG("Applying Bandpass: poles: %d, f1: %f, f2: %f",filterPoles,f1,f2);
		// Set the filter string to be returned in filterID
		usedFilter = "BW(" + Core::toString(filterPoles) + "," + Core::toString(f1) + "," + Core::toString(f2) + ")";
		try {
			Math::Filtering::IIR::ButterworthBandpass<double> f(filterPoles, f1, f2, _stream.fsamp);
			static_cast<Math::Filtering::InPlaceFilter<double>*>(&f)->apply(tmp);
		}
		catch ( exception &e ) {
			SEISCOMP_ERROR("Error while filtering: %s", e.what());
			setStatus(Error, 2);
			return false;
		}
	}
	else{
		SEISCOMP_ERROR("Filter %s is not implemented", filterType.c_str());
	}

	int onset = onsetIndex-signalStartIndex;
	bk_wrapper(n, tmp.data(), onset, snr, _stream.fsamp);

	if ( onset == -1 ) {
		return false;
	}

	SEISCOMP_INFO("BKPicker::calculatePick n=%d fs=%g sb=%g se=%g offs=%g    %d -> onset=%d",
	              n, _stream.fsamp, _config.signalBegin, _config.signalEnd,
	              offset, onsetIndex-signalStartIndex, onset);

	onsetIndex = onset + signalStartIndex;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
