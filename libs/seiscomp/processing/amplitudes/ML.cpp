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


#define SEISCOMP_COMPONENT AmplitudeML

#include <seiscomp/logging/log.h>
#include <seiscomp/processing/regions.h>
#include <seiscomp/processing/amplitudes/ML.h>
#include <seiscomp/math/mean.h>
#include <seiscomp/math/filter/chainfilter.h>
#include <seiscomp/math/filter/seismometers.h>
#include <seiscomp/math/restitution/fft.h>
#include <seiscomp/geo/featureset.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/config/config.h>

#include "iaspei.h"

#include <cmath>


using namespace std;
using namespace Seiscomp::Math;


namespace Seiscomp {
namespace Processing {


namespace {


bool computePeak2Peak(const double *data, size_t npts,
                      double &amplitude, double &period, double &index) {
	if ( npts <= 3 ) return false;

	// This is a port of a Perl code from NRCAN
	int ipeak_save = -1; // If > 0 indicates that at least 2 peaks or troughs
	                     // have already been found.
	                     // Stores position of 1st of pair of peak/trough
	                     // for largest amp found so far.

	// vel indicates up or down direction of signal
	// as part of search for peaks and troughs.
	int ipeak = -1;  // will be > 0 and indicate position of peak or trough.
	                 // initialize direction in most cases. (a nonzero vel is wanted.)
	double vel = data[2]-data[1];
	for ( size_t isamp = 2; isamp < npts-1; ++isamp ) {
		double vel2 = data[isamp+1]-data[isamp];
		if ( vel2*vel < 0.0 ) {
			// have found a peak or trough at $isamp.
			if ( ipeak >= 0 ) {
				// have found consecutive peak and trough.
				double amp_temp = 0.5 * fabs(data[isamp] - data[ipeak]);
				if ( ipeak_save < 0 || amp_temp > amplitude ) {
					// Save this as the largest so far.
					amplitude = amp_temp;

					// The period will be converted to seconds in
					// AmplitudeProcessor::process. Here we return the period
					// in indexes
					period = 2.0 * (isamp-ipeak);
					ipeak_save = ipeak;
				}
			}

			// store location of current peak
			ipeak = isamp;
			vel = vel2;
		}
		else {
			// re-initialize direction in case where first few samples equal.
			// This will only happen before first peak is found.
			if ( vel == 0 )
				vel = data[isamp+1]-data[isamp];
		}
	}

	if ( ipeak_save < 0 )
		// No amplitude found
		return false;

	// not really time of maximum
	index = ipeak_save;

	return true;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AbstractAmplitudeProcessor_ML::AbstractAmplitudeProcessor_ML(
	const std::string &type
)
: AmplitudeProcessor(type) {
	setDefaultConfiguration();
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AbstractAmplitudeProcessor_ML::initFilter(double fsamp) {
	Filter *preFilter{nullptr};

	if ( !_preFilter.empty() ) {
		string error;
		preFilter = Filter::Create(_preFilter, &error);
		if ( !preFilter ) {
			SEISCOMP_ERROR("Wrong filter: %s", error.c_str());
			setStatus(ConfigurationError, 10);
			return;
		}
	}

	if ( !_enableResponses ) {
		Filter *waFilter{nullptr};

		if ( _applyWA ) {
			waFilter  = new Filtering::IIR::WoodAndersonFilter<double>(
				Velocity,
				_config.woodAndersonResponse
			);
		}

		if ( preFilter && waFilter ) {
			auto chain = new Filtering::ChainFilter<double>;
			chain->add(preFilter);
			chain->add(waFilter);
			setFilter(chain);
		}
		else if ( preFilter ) {
			setFilter(preFilter);
		}
		else {
			setFilter(waFilter);
		}
	}
	else {
		setFilter(preFilter);
	}

	AmplitudeProcessor::initFilter(fsamp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int AbstractAmplitudeProcessor_ML::capabilities() const {
	return MeasureType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor::IDList
AbstractAmplitudeProcessor_ML::capabilityParameters(Capability cap) const {
	if ( cap == MeasureType ) {
		IDList params;
		params.push_back("AbsMax");
		params.push_back("MinMax");
		params.push_back("PeakTrough");
		return params;
	}

	return AmplitudeProcessor::capabilityParameters(cap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AbstractAmplitudeProcessor_ML::setParameter(Capability cap, const std::string &value) {
	if ( cap == MeasureType ) {
		if ( value == "AbsMax" ) {
			_amplitudeMeasureType = AbsMax;
			return true;
		}
		else if ( value == "MinMax" ) {
			_amplitudeMeasureType = MinMax;
			return true;
		}
		else if ( value == "PeakTrough" ) {
			_amplitudeMeasureType = PeakTrough;
			return true;
		}

		return false;
	}

	return AmplitudeProcessor::setParameter(cap, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string AbstractAmplitudeProcessor_ML::parameter(Capability cap) const {
	if ( cap == MeasureType ) {
		switch ( _amplitudeMeasureType ) {
			case AbsMax:
				return "AbsMax";
			case MinMax:
				return "MinMax";
			case PeakTrough:
				return "PeakTrough";
			default:
				break;
		}
	}

	return AmplitudeProcessor::parameter(cap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AbstractAmplitudeProcessor_ML::setup(const Settings &settings) {
	setDefaultConfiguration();

	if ( !AmplitudeProcessor::setup(settings) ) return false;

	bool absMax = true;

	// amplitude pre-processing
	try {
		_preFilter = settings.getString("amplitudes." + _type + ".preFilter");
	}
	catch ( ... ) {}

	try {
		_applyWA = settings.getBool("amplitudes." + _type + ".applyWoodAnderson");
	}
	catch ( ... ) {}

	if ( settings.getValue(absMax, "amplitudes." + _type + ".absMax") ) {
		_amplitudeMeasureType = absMax ? AbsMax : MinMax;
	}
	else {
		std::string measureType;
		if ( settings.getValue(measureType, "amplitudes." + _type + ".measureType") ) {
			if ( !setParameter(MeasureType, measureType) ) {
				SEISCOMP_ERROR("%s.%s.%s.%s: invalid amplitude measure type: %s",
				               settings.networkCode.c_str(),
				               settings.stationCode.c_str(),
				               settings.locationCode.c_str(),
				               settings.channelCode.c_str(),
				               measureType.c_str());
				return false;
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AbstractAmplitudeProcessor_ML::deconvolveData(Response *resp,
                                                   DoubleArray &data,
                                                   int numberOfIntegrations) {
	if ( numberOfIntegrations < -1 )
		return false;

	Math::Restitution::FFT::TransferFunctionPtr instrumentResponse =
		resp->getTransferFunction(numberOfIntegrations < 0 ? 0 : numberOfIntegrations);

	if ( !instrumentResponse )
		return false;

	Math::Restitution::FFT::TransferFunctionPtr tf;
	Math::SeismometerResponse::WoodAnderson paz(numberOfIntegrations < 0 ? Math::Displacement : Math::Velocity,
	                                            _config.woodAndersonResponse);
	Math::Restitution::FFT::PolesAndZeros woodAnderson(paz);

	if ( _applyWA ) {
		tf = *instrumentResponse / woodAnderson;
	}
	else {
		tf = instrumentResponse;
	}

	// Remove linear trend
	double m, n;
	Math::Statistics::computeLinearTrend(data.size(), data.typedData(), m, n);
	Math::Statistics::detrend(data.size(), data.typedData(), m, n);

	return Math::Restitution::transformFFT(data.size(), data.typedData(),
	                                       _stream.fsamp, tf.get(),
	                                       _config.respTaper, _config.respMinFreq,
	                                       _config.respMaxFreq);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AbstractAmplitudeProcessor_ML::computeAmplitude(
		const DoubleArray &data,
		size_t, size_t,
		size_t si1, size_t si2,
		double offset,
		AmplitudeIndex *dt, AmplitudeValue *amplitude,
		double *period, double *snr) {
	double amax;

	*period = -1;

	switch ( _amplitudeMeasureType ) {
		case AbsMax:
		{
			size_t imax = find_absmax(data.size(), data.typedData(), si1, si2, offset);
			amax = fabs(data[imax] - offset);
			dt->index = imax;
			break;
		}

		case MinMax:
		{
			int lmin, lmax;
			find_minmax(lmin, lmax, data.size(), data.typedData(), si1, si2, offset);
			amax = (data[lmax] - data[lmin]) * 0.5;
			dt->index = (lmin+lmax)*0.5;
			dt->begin = lmin - dt->index;
			dt->end = lmax - dt->index;
			break;
		}

		case PeakTrough:
			if ( _config.iaspeiAmplitudes ) {
				IASPEI::AmplitudePeriodMeasurement amp;

				auto offset = Math::Statistics::mean(si2 - si1, data.typedData() + si1);
				if ( !IASPEI::measureAmplitudePeriod(data.impl(), offset, si1, si2, amp) ) {
					return false;
				}

				amax = abs(data[amp.ip2p1] - data[amp.ip2p2]) * 0.5;
				dt->index = IASPEI::findZeroCrossing(data.impl(), offset, amp.ip2p1, amp.ip2p2);
				if ( dt->index < 0 ) {
					dt->index = (amp.ip2p1 + amp.ip2p2) * 0.5;
				}
				dt->begin = amp.ip2p1 - dt->index;
				dt->end = amp.ip2p2 - dt->index;
				*period = (dt->end - dt->begin) * 2;
			}
			else {
				if ( !computePeak2Peak(data.typedData() + si1, si2 - si1, amax, *period, dt->index) ) {
					return false;
				}

				dt->index += si1;
				dt->begin = 0;
				dt->end = *period * 0.5;
			}

			break;

		default:
			return false;
	}

	if ( *_noiseAmplitude == 0. )
		*snr = 1000000.0;
	else
		*snr = amax / *_noiseAmplitude;

	if ( *snr < _config.snrMin ) {
		setStatus(LowSNR, *snr);
		return false;
	}

	amplitude->value = amax;

	if ( _streamConfig[_usedComponent].gain != 0.0 )
		amplitude->value /= _streamConfig[_usedComponent].gain;
	else {
		setStatus(MissingGain, 0.0);
		return false;
	}

	// - convert to millimeter
	amplitude->value *= 1E03;
	amplitude->value = std::abs(amplitude->value);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AbstractAmplitudeProcessor_ML::setDefaultConfiguration() {
	// Default settings
	setSignalEnd("min(R / 3 + 30, 150)");
	setMinSNR(0);
	// Maximum distance is 8 degrees
	setMaxDist(8);
	// Maximum depth is 80 km
	setMaxDepth(80);

	_amplitudeMeasureType = AbsMax;
	_preFilter = string();
	_applyWA = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
