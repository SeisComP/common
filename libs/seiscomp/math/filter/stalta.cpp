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

#include<iostream>
#include <seiscomp/core/exceptions.h>
#include <seiscomp/math/filter/stalta.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {

#define FABS(x) (((x)<0)?(-(x)):(x))

static void checkParameters(double lenSTA, double lenLTA)
{
	if (lenSTA <= 0.)
		throw Core::ValueException("STA length must be positive");
	if (lenLTA <= 0.)
		throw Core::ValueException("LTA length must be positive");
	if (lenSTA > lenLTA)
		throw Core::ValueException("STA length must not exceed LTA length");
}


template<typename TYPE>
STALTA<TYPE>::STALTA(double lenSTA, double lenLTA, double fsamp)
	: _lenSTA(lenSTA), _lenLTA(lenLTA), _sampleCount(0)
{
	checkParameters(_lenSTA, _lenLTA);
	setSamplingFrequency(fsamp);
}


template<typename TYPE>
void
STALTA<TYPE>::setSamplingFrequency(double fsamp)
{
	if (fsamp <= 0.)
		throw Core::ValueException("Sampling frequency must be positive");
	_fsamp  = fsamp;
	_numSTA = int(_lenSTA*fsamp+0.5);
	_numLTA = int(_lenLTA*fsamp+0.5);
	reset();
}


template<typename TYPE>
int
STALTA<TYPE>::setParameters(int n, const double *params)
{
	if ( n != 2 ) return 2;

	_lenSTA = params[0];
	_lenLTA = params[1];
	checkParameters(_lenSTA, _lenLTA);

	return 2;
}


template<typename TYPE>
void 
STALTA<TYPE>::reset()
{
	_sampleCount = 0;
	_STA = _LTA = 0.;
}


template<typename TYPE>
void
STALTA<TYPE>::apply(int ndata, TYPE *data)
{
	double normLTA = 1./_numLTA;
	double normSTA = 1./_numSTA;

	for (int i=0; i<ndata; i++, _sampleCount++) {

		double current = FABS(data[i]);

		if (_sampleCount < _numSTA) {
			normSTA = 1./(_sampleCount+1);
			_STA = (_STA*_sampleCount + current)*normSTA;
		}
		else
			_STA += (current - _STA)*normSTA;

		if (_sampleCount < _numLTA) {
			normLTA = 1./(_sampleCount+1);
			_LTA = (_LTA*_sampleCount + current)*normLTA;
		}
		else {
			// Normally we would expect:
			// _LTA += (current - _LTA)*normLTA;
			// But this is a little smoother and
			// with a slightly delayed LTA:
			_LTA += (_STA - _LTA)*normLTA;
		}

		// During the initial _numSTA samples, _STA and _LTA
		// are identical, so we skip the division here.
		data[i] = (TYPE) (_sampleCount < _numSTA ? 1 : _STA/_LTA);
	}
}


template <typename TYPE>
InPlaceFilter<TYPE>* STALTA<TYPE>::clone() const {
	return new STALTA<TYPE>(_lenSTA, _lenLTA, _fsamp);
}


INSTANTIATE_INPLACE_FILTER(STALTA, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(STALTA, "STALTA");





template<typename TYPE>
STALTA2<TYPE>::STALTA2(double lenSTA, double lenLTA,
		       double eventOn, double eventOff, double fsamp)
	: _lenSTA(lenSTA), _lenLTA(lenLTA), _sampleCount(0)
{
	_eventOn = eventOn;
	_eventOff = eventOff;
	_updateLTA = 1.;
	checkParameters(_lenSTA, _lenLTA);
	setSamplingFrequency(fsamp);
}


template<typename TYPE>
void
STALTA2<TYPE>::setSamplingFrequency(double fsamp)
{
	if (fsamp <= 0.)
		throw Core::ValueException("Sampling frequency must be positive");
	_fsamp  = fsamp;
	_numSTA = int(_lenSTA*_fsamp+0.5);
	_numLTA = int(_lenLTA*_fsamp+0.5);
	reset();
std::cerr << "setSamplingFrequency fsamp=" << _fsamp << std::endl;
}


template<typename TYPE>
int
STALTA2<TYPE>::setParameters(int n, const double *params)
{
	if ( n != 4 ) return 4;

	_lenSTA   = params[0];
	_lenLTA   = params[1];
	_eventOn  = params[2];
	_eventOff = params[3];
	checkParameters(_lenSTA, _lenLTA);

	return 4;
}


template<typename TYPE>
void
STALTA2<TYPE>::reset()
{
	_sampleCount = 0;
	_STA = _LTA = 1.;
	_updateLTA = 1.;
}


template<typename TYPE>
void
STALTA2<TYPE>::apply(int ndata, TYPE *data)
{
	double normLTA = 1./_numLTA;
	double normSTA = 1./_numSTA;

std::cerr << "apply fsamp=" << _fsamp << std::endl;
	for (int i=0; i<ndata; i++, _sampleCount++) {

		double current = FABS(data[i]);

		if (_sampleCount < _numSTA) {
			normSTA = 1./(_sampleCount+1);
			_STA = (_STA*_sampleCount + current)*normSTA;
		}
		else
			_STA += (current - _STA)*normSTA;

		if (_sampleCount < _numLTA) {
			normLTA = 1./(_sampleCount+1);
			_LTA = (_LTA*_sampleCount + current)*normLTA;
		}
		else {
			// Normally we would expect:
			// _LTA += (current - _LTA)*normLTA;
			// But this is a little smoother and
			// with a slightly delayed LTA:
			_LTA += (_STA - _LTA)*normLTA*_updateLTA;
		}

		// During the initial _numSTA samples, _STA and _LTA
		// are identical, so we skip the division here.
		data[i] = (TYPE) (_sampleCount < _numSTA ? 1 : _STA/_LTA);

		if ( (_updateLTA > 0.) && (data[i] > _eventOn) )
			_updateLTA = 0.;
		else if ( (_updateLTA < 1.) && (data[i] < _eventOff) )
			_updateLTA = 1.;
	}
}


template <typename TYPE>
InPlaceFilter<TYPE>* STALTA2<TYPE>::clone() const
{
	return new STALTA2<TYPE>(_lenSTA, _lenLTA, _eventOn, _eventOff, _fsamp);
}


INSTANTIATE_INPLACE_FILTER(STALTA2, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(STALTA2, "STALTA2");





template <typename TYPE>
STALTA_Classic<TYPE>::STALTA_Classic(
	double lenSTA, double lenLTA, double fsamp)
	: _lenSTA(lenSTA), _lenLTA(lenLTA), _sampleCount(0)
{
	checkParameters(_lenSTA, _lenLTA);
	setSamplingFrequency(fsamp);
}


template<typename TYPE>
void
STALTA_Classic<TYPE>::setSamplingFrequency(double fsamp)
{
	if (fsamp <= 0.)
		throw Core::ValueException("Sampling frequency must be positive");
	_fsamp  = fsamp;
	_numSTA = int(_lenSTA*fsamp+0.5);
	_numLTA = int(_lenLTA*fsamp+0.5);
	reset();
}


template<typename TYPE>
int
STALTA_Classic<TYPE>::setParameters(int n, const double *params)
{
	if ( n != 2 ) return 2;

	_lenSTA = params[0];
	_lenLTA = params[1];
	checkParameters(_lenSTA, _lenLTA);

	return 2;
}


template <typename TYPE>
void
STALTA_Classic<TYPE>::reset()
{
	_sampleCount = 0;
	_STA = _LTA = 0.;
	_sta_buffer.clear();
	_lta_buffer.clear();
	_sta_buffer.reserve(_numSTA);
	_lta_buffer.reserve(_numLTA);
}


template <typename TYPE>
void
STALTA_Classic<TYPE>::apply(int ndata, TYPE *data)
{
	for (int i=0; i<ndata; i++, _sampleCount++) {

		double current = FABS(data[i]);

		if (_sampleCount < _numSTA) {
			_STA += current;
			_sta_buffer.push_back(current);
		}
		else {
			int k = _sampleCount % _numSTA;
			// Replace the oldest by the latest.
			_STA += current - _sta_buffer[k];
			_sta_buffer[k] = current;
		}

		if (_sampleCount < _numLTA) {
			_LTA += current;
			_lta_buffer.push_back(current);
		}
		else {
			int k = _sampleCount % _numLTA;
			// Replace the oldest by the latest
			_LTA += current - _lta_buffer[k];
			_lta_buffer[k] = current;
		}

		data[i] = (TYPE) (
			(_STA/_sta_buffer.size()) /
			(_LTA/_lta_buffer.size()) );
	}
}


template <typename TYPE>
InPlaceFilter<TYPE>* STALTA_Classic<TYPE>::clone() const {
	return new STALTA_Classic<TYPE>(_lenSTA, _lenLTA, _fsamp);
}


INSTANTIATE_INPLACE_FILTER(STALTA_Classic, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(STALTA_Classic, "STALTAClassic");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

