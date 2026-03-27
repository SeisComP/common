/***************************************************************************
 * Copyright (C) Donavin Liebgott                                          *
 * All rights reserved.                                                    *
 * Contact: Donavin Liebgott (donavinliebgott@gmail.com)                   *
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


#include <cmath>
#include <algorithm>
#include <seiscomp/logging/log.h>
#include <seiscomp/math/filter/filterpickercf.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


// -----------------------------------------------------------------------------
// FilterPickerCF template implementation
// -----------------------------------------------------------------------------

template<typename TYPE>
FilterPickerCF<TYPE>::FilterPickerCF() {
	_filters.reserve(_numBands);
}


template<typename TYPE>
FilterPickerCF<TYPE>::~FilterPickerCF() {
	for (auto* filter : _filters) {
		delete filter;
	}
	_filters.clear();
}


template<typename TYPE>
void FilterPickerCF<TYPE>::reset() {
	// Reset all bandpass filters
	for (auto* filter : _filters) {
		filter->reset();
	}

	// Reset envelope buffer
	if (!_envelopeBuffer.empty()) {
		std::fill(_envelopeBuffer.begin(), _envelopeBuffer.end(), 0);
	}
	_envelopeBufferPos = 0;
}


template<typename TYPE>
void FilterPickerCF<TYPE>::setSamplingFrequency(double fsamp) {
	if (fsamp <= 0) {
		return;
	}

	_fsamp = fsamp;

	// Initialize envelope buffer
	_envelopeBuffer.resize(_envelopeWindowSize, 0);

	// Initialize filter bank
	initFilterBank();
}


template<typename TYPE>
int FilterPickerCF<TYPE>::setParameters(int n, const double *params) {
	if (n < 3) {
		return -1; // Not enough parameters
	}

	int numBands = static_cast<int>(params[0]);
	double minFreq = params[1];
	double maxFreq = params[2];

	if (numBands <= 0 || minFreq <= 0 || maxFreq <= minFreq) {
		return -2; // Invalid parameter values
	}

	setFilterParams(numBands, minFreq, maxFreq);

	return n;
}


template<typename TYPE>
void FilterPickerCF<TYPE>::setFilterParams(int numBands, double minFreq, double maxFreq) {
	_numBands = numBands;
	_minFreq = minFreq;
	_maxFreq = maxFreq;

	// Reinitialize if sampling frequency is already set
	if (_fsamp > 0) {
		setSamplingFrequency(_fsamp);
	}
}


template<typename TYPE>
void FilterPickerCF<TYPE>::initFilterBank() {
	// Clean up old filters
	for (auto* filter : _filters) {
		delete filter;
	}
	_filters.clear();
	_filterBandsConfig.clear();

	if (_numBands <= 0) {
		_numBands = 5;
	}

	// Compute Nyquist frequency and adjust max frequency
	double nyquist = _fsamp / 2.0;
	double effectiveMaxFreq = std::min(_maxFreq, nyquist * 0.95);

	if (_minFreq <= 0 || effectiveMaxFreq <= _minFreq) {
		SEISCOMP_WARNING("FilterPickerCF: invalid frequency range (fsamp=%.2f Hz), using adjusted defaults",
		                 _fsamp);
		_minFreq = 0.5;
		effectiveMaxFreq = std::min(20.0, nyquist * 0.95);
	}

	// Create logarithmically spaced frequency bands
	double logMinFreq = log10(_minFreq);
	double logMaxFreq = log10(effectiveMaxFreq);
	double logStep = (logMaxFreq - logMinFreq) / _numBands;

	for (int i = 0; i < _numBands; ++i) {
		FilterBankConfig config;

		// Center frequency (logarithmic spacing)
		config.centerFreq = pow(10.0, logMinFreq + (i + 0.5) * logStep);

		// Band edges (approximately 1/3 octave bandwidth)
		double bandwidth = pow(10.0, logStep / 2.0);
		config.lowFreq = config.centerFreq / bandwidth;
		config.highFreq = config.centerFreq * bandwidth;

		// Clamp to min/max frequencies and Nyquist
		config.lowFreq = std::max(config.lowFreq, _minFreq);
		config.highFreq = std::min(config.highFreq, effectiveMaxFreq);

		// Ensure lowFreq < highFreq after clamping
		if (config.lowFreq >= config.highFreq) {
			config.lowFreq = config.highFreq * 0.8;
		}

		config.poles = 2;
		_filterBandsConfig.push_back(config);

		// Create bandpass filter for this band
		auto* filter = new Math::Filtering::IIR::ButterworthBandpass<TYPE>(
			config.poles, config.lowFreq, config.highFreq, _fsamp);
		_filters.push_back(filter);
	}

	SEISCOMP_DEBUG("FilterPickerCF initialized with %d frequency bands (fsamp=%.2f Hz)",
	               _numBands, _fsamp);
}


template<typename TYPE>
void FilterPickerCF<TYPE>::apply(int n, TYPE *inout) {
	if (n <= 0 || inout == nullptr) {
		return;
	}

	// Check if filter bank is initialized
	if (_filters.empty() && _fsamp > 0) {
		initFilterBank();
	}

	if (_filters.empty()) {
		// Not initialized, pass through
		return;
	}

	// Process each sample incrementally
	for (int i = 0; i < n; ++i) {
		TYPE sample = inout[i];

		// Apply filter bank and get maximum envelope across all bands
		TYPE maxEnvelope = 0;

		for (size_t band = 0; band < _filters.size(); ++band) {
			// Apply bandpass filter (maintains internal state)
			TYPE filtered = sample;
			_filters[band]->apply(1, &filtered);

			// Compute envelope (absolute value)
			TYPE envelope = std::fabs(filtered);

			// Track maximum across bands (before smoothing)
			if (envelope > maxEnvelope) {
				maxEnvelope = envelope;
			}
		}

		// Apply short moving average smoothing to the maximum envelope
		// Update circular buffer
		_envelopeBuffer[_envelopeBufferPos] = maxEnvelope;
		_envelopeBufferPos = (_envelopeBufferPos + 1) % _envelopeWindowSize;

		// Compute moving average
		TYPE smoothedEnvelope = 0;
		for (int j = 0; j < _envelopeWindowSize; ++j) {
			smoothedEnvelope += _envelopeBuffer[j];
		}
		smoothedEnvelope /= _envelopeWindowSize;

		// Output the smoothed maximum envelope (characteristic function)
		// The picker (AIC, STA/LTA, etc.) will perform detection on this
		inout[i] = smoothedEnvelope;
	}
}


template<typename TYPE>
InPlaceFilter<TYPE>* FilterPickerCF<TYPE>::clone() const {
	auto* clone = new FilterPickerCF<TYPE>();
	clone->setFilterParams(_numBands, _minFreq, _maxFreq);
	if (_fsamp > 0) {
		clone->setSamplingFrequency(_fsamp);
	}
	return clone;
}


// Explicit template instantiations
template class SC_SYSTEM_CORE_API FilterPickerCF<float>;
template class SC_SYSTEM_CORE_API FilterPickerCF<double>;


// Register the FilterPickerCF InplaceFilter plugin
REGISTER_INPLACE_FILTER(FilterPickerCF, "FP");


} // namespace Seiscomp::Math::Filter
} // namespace Seiscomp::Math
} // namespace Seiscomp
