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


#include <math.h>
#include <seiscomp/math/filter/bpenv.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static void checkParameters(double centerFrequency, double bandwidthOctaves, int order) {
	if ( centerFrequency <= 0.  )
		throw Core::ValueException("Filter center frequency must be positive");
	if ( bandwidthOctaves <= 0. )
		throw Core::ValueException("Filter band width must be positive");
	if ( order < 0 )
		throw Core::ValueException("Filter order must be non-negative");
	if ( order > 10 )
		throw Core::ValueException("Filter order > 10 makes no sense");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
BandPassEnvelope<T>::BandPassEnvelope(double centerFrequency, double bandwidthOctaves, int order, double fsamp)
: _centerFrequency(centerFrequency)
, _bandwidthOctaves(bandwidthOctaves)
, _order(order)
, _samplingFrequency(0)
, _K(0), _yp(0), _bp(nullptr), _afterReset(true)
{
	checkParameters(_centerFrequency, _bandwidthOctaves, _order);
	setSamplingFrequency(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BandPassEnvelope<T>::reset() {
	double q = std::sqrt(std::pow(2, _bandwidthOctaves));
	double fmin = _centerFrequency/q;
	double fmax = _centerFrequency*q;

	_bp = new IIR::ButterworthBandpass<T>(_order, fmin, fmax, _samplingFrequency);

	_K = 0.5*_samplingFrequency/(M_PI*_centerFrequency);
	_K = _K*_K;

	_yp = 0;

	_afterReset = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BandPassEnvelope<T>::setSamplingFrequency(double fsamp) {
	if ( _samplingFrequency == fsamp )
		return;

	_samplingFrequency = fsamp;

	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
int BandPassEnvelope<T>::setParameters(int n, const double *params) {
	switch ( n ) {
	case 3:
		_order = (int) params[2];
	case 2:
		_bandwidthOctaves = params[1];
	case 1:
		_centerFrequency = params[0];
		break;
	default:
		return 3;
	}

	checkParameters(_centerFrequency, _bandwidthOctaves, _order);
	reset();

	return n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
InPlaceFilter<T>* BandPassEnvelope<T>::clone() const {
	return new BandPassEnvelope<T>(
		_centerFrequency, _bandwidthOctaves, _order, _samplingFrequency);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BandPassEnvelope<T>::apply(int ndata, T *data) {
	_bp->apply(ndata, data);

	if ( _afterReset && ndata > 0 ) {
		_yp = data[0];
		_afterReset = false;
	}

	for ( int i = 0; i < ndata; i++ ) {
		double y { data[i] };
		double dy { y - _yp };
		data[i] = std::sqrt(y*y + _K*dy*dy);
		_yp = y;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



INSTANTIATE_INPLACE_FILTER(BandPassEnvelope, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(BandPassEnvelope, "BPENV");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
