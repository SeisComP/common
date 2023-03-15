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

#define SEISCOMP_COMPONENT TimeDomainRestitution

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <math.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/math/restitution/td.h>
#include <seiscomp/math/fft.h> // for Math::Complex // FIXME
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/datamodel/inventory.h>
#include <seiscomp/datamodel/network.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/datamodel/sensorlocation.h>
#include <seiscomp/datamodel/stream.h>
#include <seiscomp/datamodel/sensor.h>
#include <seiscomp/datamodel/responsepaz.h>
#include <seiscomp/datamodel/utils.h>

namespace Seiscomp {
namespace Math {
namespace Restitution {


static const DataModel::Stream*
getStream(
	const DataModel::Inventory *inventory,
	const DataModel::WaveformStreamID &wfid,
	const Core::Time &time,
	DataModel::InventoryError *err)
{
	const DataModel::Stream *stream =
		DataModel::getStream(
			inventory,
			wfid.networkCode(),
			wfid.stationCode(),
			wfid.locationCode(),
			wfid.channelCode(),
			time,
			err);

	return stream;
}


static const DataModel::Stream*
findStream(
	const DataModel::WaveformStreamID &wfid,
	const Core::Time &time,
	DataModel::InventoryError *err)
{
	using namespace DataModel;

	const Inventory *inventory =
		Inventory::Cast(PublicObject::Find("Inventory"));
	if (inventory==nullptr) {
		SEISCOMP_ERROR("Global inventory not found");
		return nullptr;
	}

	const Stream *stream =
		getStream(
			inventory,
			wfid.networkCode(),
			wfid.stationCode(),
			wfid.locationCode(),
			wfid.channelCode(),
			time,
			err);

	return stream;
}


static std::string
dotted(const DataModel::WaveformStreamID &wfid) {
	return 
		wfid.networkCode() + "." +
		wfid.stationCode() + "." +
		wfid.locationCode()+ "." +
		wfid.channelCode();
}

enum PAZStatus {
	TD_INVALID_POLES,
	TD_FOUND_TWO_CONJUGATE_POLES,
	TD_FOUND_TWO_REAL_POLES,
	TD_FOUND_EMPTY_PAZ,
	TD_NOT_TWO_POLES
};


static PAZStatus
checkPAZ(
	const DataModel::ResponsePAZ &paz,
	Math::Complex &p1,
	Math::Complex &p2)
{
	// TODO: fine-tune criteria and thresholds

	const DataModel::ComplexArray &poles = paz.poles();
	Math::ComplexArray smallpoles;

	double limit = 0.1;

	for ( size_t i = 0; i < poles.content().size(); ++i ) {
		const Math::Complex &p = poles.content()[i];
		if ( fabs(p.real()) < 1E-06 )
			continue;

		if ( abs(p) < limit )
			smallpoles.push_back(p);
	}

	if ( smallpoles.size() != 2 )
		return TD_NOT_TWO_POLES;

	p1 = smallpoles[0];
	p2 = smallpoles[1];

	if (fabs(p1.imag())/fabs(p1.real()) < 1E-06 &&
	    fabs(p2.imag())/fabs(p2.real()) < 1E-06 ) {
		return TD_FOUND_TWO_REAL_POLES;
	}

	else if (fabs(p1.imag() + p2.imag()) < 1E-05 &&
	         fabs(p1.real() - p2.real()) < 1E-05 ) {
		return TD_FOUND_TWO_CONJUGATE_POLES;
	}

	return TD_INVALID_POLES;
}


const DataModel::ResponsePAZ*
getResponsePAZ(
	const DataModel::Inventory *inventory,
        const DataModel::WaveformStreamID &wfid,
	const Core::Time &time)
{
	using namespace DataModel;

	InventoryError err;

	const Stream *stream =
		getStream(inventory, wfid, time, &err);

	if (stream==nullptr) {
		std::string s = dotted(wfid);
		SEISCOMP_ERROR("%s: While attempting to retrieve ResponsePAZ:", s.c_str());
		SEISCOMP_ERROR("%s: %s", s.c_str(), err.toString());
		return nullptr;
	}

	const Sensor *sensor = Sensor::Find(stream->sensor());
	if (sensor==nullptr) {
		std::string s = dotted(wfid);
		SEISCOMP_ERROR("%s: While attempting to retrieve ResponsePAZ:", s.c_str());
		SEISCOMP_ERROR("%s: Sensor not found", s.c_str());
		return nullptr;
	}

	/* const */ PublicObject *response = PublicObject::Find(sensor->response());
	if (response==nullptr) {
		std::string s = dotted(wfid);
		SEISCOMP_ERROR("%s: While attempting to retrieve ResponsePAZ:", s.c_str());
		SEISCOMP_ERROR("%s: Response not found", s.c_str());
		return nullptr;
	}

	const ResponsePAZ *paz = ResponsePAZ::Cast(response);
	if (paz==nullptr) {
		std::string s = dotted(wfid);
		SEISCOMP_ERROR("%s: While attempting to retrieve ResponsePAZ:", s.c_str());
		SEISCOMP_ERROR("%s: ResponsePAZ not found", s.c_str());
		return nullptr;
	}

	return paz;
}


// Find the ResponsePAZ for specified waveform ID and time in the global
// inventory instance. Returns a pointer if found, NULL otherwise.
const DataModel::ResponsePAZ*
findResponsePAZ(
        const DataModel::WaveformStreamID &wfid,
	const Core::Time &time)
{
	using namespace DataModel;

	const Inventory *inventory =
		Inventory::Cast(PublicObject::Find("Inventory"));
	if (inventory==nullptr) {
		SEISCOMP_ERROR("Global inventory not found");
		return nullptr;
	}

	return getResponsePAZ(inventory, wfid, time);
}


bool coefficients_from_T0_h(double fsamp, double gain, double T0, double h, double *c0, double *c1, double *c2)
{
	// from Kanamori and Rivera (2008)
	double w0 = 2*M_PI/T0;
	double dt = 1./fsamp;
	double q  = 1./(gain*dt);

	*c0 = q;
	*c1 = -2*(1+h*w0*dt)*q;
	*c2 = (1+2*h*w0*dt+(w0*dt)*(w0*dt))*q;
	return true;
}


bool coefficients_from_T1_T2(double fsamp, double gain, double T1, double T2, double *c0, double *c1, double *c2)
{
	// derived by Joachim Saul
	double w1 = 2*M_PI/T1, w2 = 2*M_PI/T2;
	double dt = 1./fsamp;
	double q  = 1./(gain*dt);

	*c0 = q;
	*c1 = -2*(1+0.5*(w1+w2)*dt)*q;
	*c2 = (1+(w1+w2)*dt+w1*w2*dt*dt)*q;
	return true;
}


template<class TYPE>
TimeDomain<TYPE>::TimeDomain()
{
	bandpass = nullptr;
	fsamp = 0;
	dt = 0;
	init();
}


template<class TYPE>
TimeDomain<TYPE>::~TimeDomain()
{
	if ( bandpass != nullptr )
		delete bandpass;
}


template<class TYPE>
void
TimeDomain<TYPE>::init()
{
	a2 = a1 = y1 = y0 = 0.;
	cumsum1 = cumsum2 = 0;
}


template<class TYPE>
int
TimeDomain<TYPE>::setParameters(int n, const double *params) {
	return 0;
}


template<class TYPE>
void
TimeDomain<TYPE>::apply(int n, TYPE *inout)
{
	for (int i=0; i<n; i++) {
		y2  = inout[i];
		a2 = a1 + c2*y2 + c1*y1 + c0*y0;
		y0 = y1;
		y1 = y2;
		inout[i] = a1 = a2;
	}

	// Apply bandpass before double integration, which according
	// to Kanamori and Rivera (2008) is numerically advantageous
	// compared to first double integration followed by bandpass
	// filtering, even though analytically equivalent.
	if (bandpass)
		bandpass->apply(n,inout);
	// else ???

	// Double-integration *after* bandpass filtering
	// (see Kanamori & Rivera, 2008)
	for (int i=0; i<n; i++) {
		// cause a delay of one sample (half a sample per integration)
		cumsum1 += inout[i]*dt;
		inout[i] = cumsum2;
		cumsum2 += cumsum1*dt;
	}

	// FIXME: We still observe about half a sample of delay compared to
	// FIXME: FFT version. This isn't going to be a big problem for most
	// FIXME: applications, but there must be an explanation or fix.
}


template<class TYPE>
void
TimeDomain<TYPE>::setBandpass(int _order, double _fmin, double _fmax)
{
	order = _order;
	fmin  = _fmin;
	fmax  = _fmax;

	bandpass = new Filtering::IIR::ButterworthHighLowpass<TYPE>(order, fmin, fmax);
	if ( fsamp>0 )
		bandpass->setSamplingFrequency(fsamp);
}


template<class TYPE>
void
TimeDomain<TYPE>::setSamplingFrequency(double _fsamp)
{
	fsamp = _fsamp;
	if ( _fsamp > 0 ) {
		dt = 1. / fsamp;
		init();
	}

	if ( bandpass && _fsamp > 0 )
		bandpass->setSamplingFrequency(_fsamp);
}

template<class TYPE>
void
TimeDomain<TYPE>::setCoefficients(double _c0, double _c1, double _c2)
{
	c0 = _c0;
	c1 = _c1;
	c2 = _c2;
}


template<class TYPE>
std::string
TimeDomain<TYPE>::print() const
{
	return info();
}


template<class TYPE>
std::string
TimeDomain<TYPE>::info() const
{
	std::stringstream tmp;
	if ( fsamp <= 0. )
		tmp << "  Not yet initialized" << std::endl;
	else {
		tmp << "  fsamp   = " <<   fsamp << std::endl;
		tmp << "  gain    = " <<    gain << std::endl;
		tmp << "  c0*gain = " << c0*gain << std::endl;
		tmp << "  c1*gain = " << c1*gain << std::endl;
		tmp << "  c2*gain = " << c2*gain << std::endl;
	}

	if (order <= 0) {
		tmp << "  bandpass not configured" << std::endl;
	}
	else {
		tmp << "  bandpass configuration" << std::endl;
		tmp << "  order   = " << order << std::endl;
		tmp << "  fmin    = " << fmin  << std::endl;
		tmp << "  fmax    = " << fmax  << std::endl;
	}
	return tmp.str();
}


template<class TYPE>
TimeDomain_from_T0_h<TYPE>::TimeDomain_from_T0_h(double T0, double h, double _gain, double fsamp)
  : T0(T0), h(h) {
	TimeDomain<TYPE>::gain=_gain;

	if ( fsamp )
		TimeDomain<TYPE>::setSamplingFrequency(fsamp);
}


template<class TYPE>
void
TimeDomain_from_T0_h<TYPE>::init()
{
	double c0, c1, c2;
	coefficients_from_T0_h(TimeDomain<TYPE>::fsamp,
				 TimeDomain<TYPE>::gain,
				 T0, h, &c0, &c1, &c2);
	TimeDomain<TYPE>::setCoefficients(c0,c1,c2);
	TimeDomain<TYPE>::init();
}


template<class TYPE>
void
TimeDomain_from_T0_h<TYPE>::setBandpass(int order, double fmin, double fmax) {
	TimeDomain<TYPE>::setBandpass(order, fmin, fmax);
}


template<class TYPE>
Filtering::InPlaceFilter<TYPE>*
TimeDomain_from_T0_h<TYPE>::clone() const {
	return new TimeDomain_from_T0_h<TYPE>(T0, h, TimeDomain<TYPE>::gain, TimeDomain<TYPE>::fsamp);
}

template<class TYPE>
std::string
TimeDomain_from_T0_h<TYPE>::info() const
{
	std::stringstream tmp;
	tmp << "TimeDomainRestitution_from_T0_h instance:" << std::endl;
	tmp << "  T0      = " << T0   << std::endl;
	tmp << "  h       = " << h    << std::endl;
	tmp << TimeDomain<TYPE>::info();
	return tmp.str();
}


template<class TYPE>
TimeDomain_from_T1_T2<TYPE>::TimeDomain_from_T1_T2(double T1, double T2, double _gain, double fsamp)
  : T1(T1), T2(T2) {
	TimeDomain<TYPE>::gain = _gain;
	if ( fsamp )
		TimeDomain<TYPE>::setSamplingFrequency(fsamp);
}


template<class TYPE>
void
TimeDomain_from_T1_T2<TYPE>::init()
{
	double c0, c1, c2;
	coefficients_from_T1_T2(TimeDomain<TYPE>::fsamp, TimeDomain<TYPE>::gain, T1, T2, &c0, &c1, &c2);

	TimeDomain<TYPE>::setCoefficients(c0,c1,c2);
	TimeDomain<TYPE>::init();
}

template<class TYPE>
void
TimeDomain_from_T1_T2<TYPE>::setBandpass(int order, double fmin, double fmax) {
	TimeDomain<TYPE>::setBandpass(order, fmin, fmax);
}


template<class TYPE>
Filtering::InPlaceFilter<TYPE>*
TimeDomain_from_T1_T2<TYPE>::clone() const {
	return new TimeDomain_from_T1_T2<TYPE>(T1, T2, TimeDomain<TYPE>::gain, TimeDomain<TYPE>::fsamp);
}


template<class TYPE>
std::string
TimeDomain_from_T1_T2<TYPE>::info() const
{
	std::stringstream tmp;
	tmp << "TimeDomainRestitution_from_T1_T2 instance:" << std::endl;
	tmp << "  T1      = " << T1   << std::endl;
	tmp << "  T2      = " << T2   << std::endl;
	tmp << TimeDomain<TYPE>::info();
	return tmp.str();
}


template<class TYPE>
TimeDomainNullFilter<TYPE>::TimeDomainNullFilter(double _gain, double fsamp) {
	TimeDomain<TYPE>::gain = _gain;
	if ( fsamp )
		TimeDomain<TYPE>::setSamplingFrequency(fsamp);
}

template<class TYPE>
void
TimeDomainNullFilter<TYPE>::init()
{
	TimeDomain<TYPE>::setCoefficients(0, 0, 0);
	TimeDomain<TYPE>::init();
}


template<class TYPE>
void
TimeDomainNullFilter<TYPE>::setBandpass(int order, double fmin, double fmax) {
	TimeDomain<TYPE>::setBandpass(order, fmin, fmax);
}


template<class TYPE>
Filtering::InPlaceFilter<TYPE>*
TimeDomainNullFilter<TYPE>::clone() const {
	return new TimeDomainNullFilter<TYPE>(TimeDomain<TYPE>::gain, TimeDomain<TYPE>::fsamp);
}


template<class TYPE>
std::string
TimeDomainNullFilter<TYPE>::info() const
{
	std::stringstream tmp;
	tmp << "TimeDomainRestitutionNullFilter instance:" << std::endl;
	tmp << TimeDomain<TYPE>::info();
	return tmp.str();
}

template<class TYPE>
void
TimeDomainNullFilter<TYPE>::apply(int n, TYPE *inout)
{
	for (int i=0; i<n; i++)
		inout[i] /= TimeDomain<TYPE>::gain;
}


template<class TYPE>
TimeDomainGeneric<TYPE>::TimeDomainGeneric()
	: filter(nullptr), order(0), fmin(0.001), fmax(10.) {
}


template<class TYPE>
TimeDomainGeneric<TYPE>::TimeDomainGeneric(const TimeDomainGeneric &other)
	: Filtering::InPlaceFilter<TYPE>(other),
	  filter(other.filter),
	  time(other.time),
	  net(other.net), sta(other.sta), loc(other.loc), cha(other.cha)
//	  order(other.order), fmin(other.fmin), fmax(other.fmax) {
{
	order = other.order;
	fmin = other.fmin;
	fmax = other.fmax;
}


template<class TYPE>
TimeDomainGeneric<TYPE>::~TimeDomainGeneric()
{
	if (filter)
		delete filter;
}


template<class TYPE>
void
TimeDomainGeneric<TYPE>::setStartTime(const Core::Time &t)
{
	time = t;
}


template<class TYPE>
void
TimeDomainGeneric<TYPE>::setStreamID(
	const std::string &n,
	const std::string &s,
	const std::string &l,
        const std::string &c)
{
	net = n;
	sta = s;
	loc = l;
	cha = c;
}


template<class TYPE>
void
TimeDomainGeneric<TYPE>::setSamplingFrequency(double fs)
{
	// We allow setting the bandpass parameter before initialization
	fsamp = fs;

	if (fsamp <= 0)
		return;

	init();

	if (filter)
		filter->setSamplingFrequency(fsamp);
}


template<class TYPE>
void
TimeDomainGeneric<TYPE>::init()
{
	using namespace DataModel;

	assert( ! net.empty() && ! sta.empty());

	WaveformStreamID wfid(net, sta, loc, cha, "");
	std::string s = dotted(wfid);

	InventoryError err;
	bool success = true;

	const Stream *stream =
		findStream(wfid, time, &err);
        if (stream == nullptr)
		success = false;

	const DataModel::ResponsePAZ *paz = findResponsePAZ(wfid, time);
        if (paz == nullptr)
		success = false;

	if ( ! success) {
		SEISCOMP_WARNING("Stream %s:", s.c_str());
		SEISCOMP_WARNING("Failed to initialize time domain restitution filter");
		return;
		// TODO: Exception
	}

	Math::Complex p1, p2; // TODO: std::complex?
	PAZStatus status = checkPAZ(*paz, p1, p2);

	success = false;
	std::string gainUnit = stream->gainUnit();
	// to upper case
	std::transform(gainUnit.begin(), gainUnit.end(), gainUnit.begin(), ::toupper);

	switch(status) {
	case TD_FOUND_EMPTY_PAZ:
		{
			// Our expectation is that this is a raw displacement
			// (e.g. GNSS) stream. We therefore additionally
			// confirm that the input unit is meters.
			// TODO: Otherwise issue a meaningful error.
			if (gainUnit == "M") {
				filter = new TimeDomainNullFilter<TYPE>(1);
				SEISCOMP_DEBUG("Stream %s:", s.c_str());
				SEISCOMP_DEBUG("Initialized time domain restitution filter"
					       "(displacement pass through)");
				if (filter)
					success = true;
			}
		}
		break;

	case TD_FOUND_TWO_REAL_POLES:
		{
			// Our expectation here is that we are dealing with velocity.
			if (gainUnit == "M/S") {
				double T1 = 2*M_PI/abs(p1);
				double T2 = 2*M_PI/abs(p2);
				filter = new TimeDomain_from_T1_T2<TYPE>(T1, T2, 1);
				SEISCOMP_DEBUG("Stream %s:", s.c_str());
				SEISCOMP_DEBUG("Initialized time domain restitution filter "
					       "for T1=%gs T2=%gs", T1, T2);
				if (filter)
					success = true;
			}
		}
		break;

	case TD_FOUND_TWO_CONJUGATE_POLES:
		{
			// Our expectation here is that we are dealing with velocity.
			if (gainUnit == "M/S") {
				double T0 = 2*M_PI/abs(p1);
				double h = -p1.real()/abs(p1);
				filter = new TimeDomain_from_T0_h<TYPE>(T0, h, 1);
				SEISCOMP_DEBUG("Stream %s:", s.c_str());
				SEISCOMP_DEBUG("Initialized time domain restitution filter "
					       "for T0=%gs h=%.6fs", T0, h);
				if (filter)
					success = true;
			}
		}
		break;

	default:
		break;
	}

	if ( ! success) {
		SEISCOMP_WARNING("Stream %s:", s.c_str());
		SEISCOMP_WARNING("Failed to initialize time domain restitution filter");
		return;
		// TODO: Exception
	}

	// make sure fmax <= 90% of Nyquist frequency
	if (fsamp > 0 && fmax > 0.45*fsamp) {
		fmax = 0.45*fsamp;
	}
	if (filter)
		filter->setBandpass(order, fmin, fmax);
}


template<class TYPE>
void
TimeDomainGeneric<TYPE>::setBandpass(int order, double fmin, double fmax)
{
	// make sure fmax <= 90% of Nyquist frequency
	if (fsamp > 0 && fmax > 0.45*fsamp) {
		fmax = 0.45*fsamp;
	}

	// We allow setting the bandpass parameter before initialization
	this->order = order;
	this->fmin = fmin;
	this->fmax = fmax;

	if ( ! initialized())
		return;

	if (filter) {
		filter->setBandpass(order, fmin, fmax);
	}
}


template<class TYPE>
int
TimeDomainGeneric<TYPE>::setParameters(int n, const double *params)
{
	switch(n) {
	case 0:
		order = 0; // deactivates the bandpass
		fmin = fmax = 0; // dummy
		break;
	case 2:
		fmin = params[0];
		fmax = params[1];
		order = 4;
		break;
	default:
		// TODO : Consider highpass
		order = 0; // deactivates the bandpass
		fmin = fmax = 0; // dummy
	}

	return n;
}


template<class TYPE>
Filtering::InPlaceFilter<TYPE>*
TimeDomainGeneric<TYPE>::clone() const {
	TimeDomainGeneric<TYPE> *copy = new TimeDomainGeneric<TYPE>(*this);
	return copy;
}


template<class TYPE>
std::string
TimeDomainGeneric<TYPE>::info() const
{
	std::stringstream tmp;
	tmp << "TimeDomainGeneric instance:" << std::endl;
	if (initialized())
		tmp << filter->info();
	else
		tmp << "uninitialized";
	return tmp.str();
}


template<class TYPE>
bool
TimeDomainGeneric<TYPE>::initialized() const
{
	if (filter == nullptr) {
		SEISCOMP_ERROR("Access to uninitialized Restitution::TimeDomainGeneric instance");
		return false;
	}
	return true;
}


template<class TYPE>
void
TimeDomainGeneric<TYPE>::apply(int n, TYPE *inout)
{
	if ( ! initialized())
		return;

	filter->apply(n, inout);
}


INSTANTIATE_INPLACE_FILTER(TimeDomainGeneric, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(TimeDomainGeneric, "TDREST");

template class SC_SYSTEM_CORE_API TimeDomain_from_T0_h<float>;
template class SC_SYSTEM_CORE_API TimeDomain_from_T0_h<double>;

template class SC_SYSTEM_CORE_API TimeDomain_from_T1_T2<float>;
template class SC_SYSTEM_CORE_API TimeDomain_from_T1_T2<double>;

template class SC_SYSTEM_CORE_API TimeDomainNullFilter<float>;
template class SC_SYSTEM_CORE_API TimeDomainNullFilter<double>;

}       // namespace Seiscomp::Math::Filtering
}       // namespace Seiscomp::Math
}       // namespace Seiscomp

