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


#include <seiscomp/processing/amplitudes/Mjma.h>
#include <seiscomp/math/mean.h>
#include <seiscomp/math/filter/seismometers.h>
#include <seiscomp/math/restitution/fft.h>

#include <cmath>


using namespace Seiscomp::Math;

namespace Seiscomp {

namespace Processing {


REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_Mjma, "Mjma");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_Mjma::AmplitudeProcessor_Mjma()
: AmplitudeProcessor("Mjma") {
	setSignalEnd("min(R / 3 + 30, 150)");
	setMinSNR(0);
	setMaxDist(20);
	setMaxDepth(80);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeProcessor_Mjma::initFilter(double fsamp) {
	if ( !_enableResponses ) {
		AmplitudeProcessor::setFilter(
			new Filtering::IIR::Seismometer5secFilter<double>(Velocity)
		);
	}
	else
		AmplitudeProcessor::setFilter(nullptr);

	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_Mjma::deconvolveData(Response *resp,
                                             DoubleArray &data,
                                             int numberOfIntegrations) {
	if ( numberOfIntegrations < -1 )
		return false;

	Math::Restitution::FFT::TransferFunctionPtr tf =
		resp->getTransferFunction(numberOfIntegrations < 0 ? 0 : numberOfIntegrations);

	if ( tf == nullptr )
		return false;

	Math::SeismometerResponse::Seismometer5sec paz(numberOfIntegrations < 0 ? Math::Displacement : Math::Velocity);
	Math::Restitution::FFT::PolesAndZeros seis5sec(paz);

	Math::Restitution::FFT::TransferFunctionPtr cascade =
		*tf / seis5sec;

	// Remove linear trend
	double m,n;
	Math::Statistics::computeLinearTrend(data.size(), data.typedData(), m, n);
	Math::Statistics::detrend(data.size(), data.typedData(), m, n);

	return Math::Restitution::transformFFT(data.size(), data.typedData(),
	                                         _stream.fsamp, cascade.get(),
	                                         _config.respTaper, _config.respMinFreq, _config.respMaxFreq);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_Mjma::computeAmplitude(
		const DoubleArray &data,
		size_t i1, size_t i2,
		size_t si1, size_t si2,
		double offset,AmplitudeIndex *dt,
		AmplitudeValue *amplitude,
		double *period, double *snr)
{
	double amax;

	size_t imax = find_absmax(data.size(), data.typedData(), si1, si2, offset);
	amax = fabs(data[imax] - offset);
	dt->index = imax;

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	*period = -1;

	amplitude->value = amax;

	if ( _streamConfig[_usedComponent].gain != 0.0 )
		amplitude->value /= _streamConfig[_usedComponent].gain;
	else {
		setStatus(MissingGain, 0.0);
		return false;
	}

	// - convert to micrometer
	amplitude->value *= 1E06;

	// - estimate peak-to-peak from absmax amplitude
	amplitude->value *= 2.0;

	amplitude->value = std::abs(amplitude->value);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
