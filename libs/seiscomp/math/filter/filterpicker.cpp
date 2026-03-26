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

#define SEISCOMP_COMPONENT FilterPickerCF

#include <seiscomp/math/filter/filterpicker.h>
#include <seiscomp/math/filter/butterworth.h>
#include <seiscomp/logging/log.h>
#include <cmath>
#include <algorithm>
#include <numeric>


using namespace std;


namespace Seiscomp {
namespace Math {
namespace Filtering {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
FilterPickerCF<TYPE>::FilterPickerCF(
	int numBands,
	double minFreq,
	double maxFreq,
	double fsamp
)
: _numBands(numBands)
, _minFreq(minFreq)
, _maxFreq(maxFreq)
, _staWindow(0.5)
, _ltaWindow(10.0)
, _numSTA(0)
, _numLTA(0)
, _fsamp(fsamp)
, _initialized(false)
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::setNumBands(int numBands) {
	_numBands = numBands;
	_initialized = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::setFrequencies(double minFreq, double maxFreq) {
	_minFreq = minFreq;
	_maxFreq = maxFreq;
	_initialized = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::setSamplingFrequency(double fsamp) {
	if (fsamp <= 0) {
		SEISCOMP_ERROR("Invalid sampling frequency: %f", fsamp);
		_initialized = false;
		return;
	}

	_fsamp = fsamp;
	_initialized = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
int FilterPickerCF<TYPE>::setParameters(int n, const double *params) {
	// Expected parameters:
	// params[0]: numBands
	// params[1]: minFreq
	// params[2]: maxFreq

	if (n < 3) {
		SEISCOMP_ERROR("FilterPickerCF requires 3 parameters: numBands, minFreq, maxFreq");
		return -1;
	}

	if ((int)params[0] < 1) {
		SEISCOMP_ERROR("numBands must be >= 1, got %d", (int)params[0]);
		return -1;
	}

	if (params[1] <= 0 || params[2] <= 0) {
		SEISCOMP_ERROR("Frequencies must be positive: minFreq=%f, maxFreq=%f", params[1], params[2]);
		return -2;
	}

	if (params[1] >= params[2]) {
		SEISCOMP_ERROR("minFreq (%f) must be less than maxFreq (%f)", params[1], params[2]);
		return -3;
	}

	_numBands = (int)params[0];
	_minFreq = params[1];
	_maxFreq = params[2];

	return 3; // Success - return number of parameters used
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::reset() {
	_initialized = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
InPlaceFilter<TYPE>* FilterPickerCF<TYPE>::clone() const {
	FilterPickerCF<TYPE> *filter = new FilterPickerCF<TYPE>(
		_numBands, _minFreq, _maxFreq, _fsamp
	);
	return filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::initFilterBank() {
	_filterBands.clear();
	_filterBands.reserve(_numBands);

	double nyquist = _fsamp / 2.0;
	double fmin = max(_minFreq, 0.1);  // Ensure minimum frequency
	double fmax = min(_maxFreq, nyquist * 0.95);  // Stay below Nyquist

	if (_numBands == 1) {
		// Single band
		_filterBands.push_back({fmin, fmax});
	} else {
		// Multiple bands with logarithmic spacing
		double logMin = log10(fmin);
		double logMax = log10(fmax);
		double logStep = (logMax - logMin) / _numBands;

		for (int i = 0; i < _numBands; ++i) {
			double bandLow = pow(10.0, logMin + i * logStep);
			double bandHigh = pow(10.0, logMin + (i + 1) * logStep);
			_filterBands.push_back({bandLow, bandHigh});
		}
	}

	_initialized = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::computeEnvelope(
	const std::vector<TYPE>& data,
	std::vector<TYPE>& envelope
) {
	int n = static_cast<int>(data.size());
	envelope.resize(n);

	// Compute absolute value for envelope
	for (int i = 0; i < n; ++i) {
		envelope[i] = std::fabs(data[i]);
	}

	// Apply short moving average smoother
	int windowSize = std::min(10, std::max(3, n / 100));
	if (windowSize % 2 == 0) windowSize++;

	int halfWindow = windowSize / 2;
	for (int i = 0; i < n; ++i) {
		TYPE sum = 0;
		int count = 0;
		for (int j = std::max(0, i - halfWindow); j <= std::min(n - 1, i + halfWindow); ++j) {
			sum += envelope[j];
			count++;
		}
		envelope[i] = sum / count;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::computeCF(
	const std::vector<TYPE>& envelope,
	std::vector<TYPE>& cf
) {
	int n = static_cast<int>(envelope.size());
	cf.resize(n);

	// Compute running sums for STA and LTA
	std::vector<TYPE> sta(n, 0);
	std::vector<TYPE> lta(n, 0);

	for (int i = 0; i < n; ++i) {
		// Short-term average
		TYPE staSum = 0;
		int staCount = 0;
		for (int j = std::max(0, i - _numSTA + 1); j <= i; ++j) {
			staSum += envelope[j];
			staCount++;
		}
		sta[i] = staSum / staCount;

		// Long-term average
		TYPE ltaSum = 0;
		int ltaCount = 0;
		for (int j = std::max(0, i - _numLTA + 1); j <= i; ++j) {
			ltaSum += envelope[j];
			ltaCount++;
		}
		lta[i] = ltaSum / ltaCount;
	}

	// Compute CF as STA/LTA ratio with regularization
	const TYPE epsilon = 1e-10;
	for (int i = 0; i < n; ++i) {
		if (lta[i] > epsilon) {
			cf[i] = sta[i] / lta[i];
		} else {
			cf[i] = static_cast<TYPE>(1.0);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::apply(int ndata, TYPE *data) {
	if (ndata <= 0 || data == nullptr) {
		return;
	}

	// Validate and initialize parameters if needed
	if (!_initialized && _fsamp > 0) {
		double nyquist = _fsamp / 2.0;
		if (_minFreq >= nyquist || _maxFreq > nyquist) {
			SEISCOMP_ERROR("Frequencies exceed Nyquist limit (fsamp=%f Hz)", _fsamp);
			return;
		}

		// Convert time windows to sample counts
		_numSTA = std::max(1, static_cast<int>(_staWindow * _fsamp));
		_numLTA = std::max(_numSTA + 1, static_cast<int>(_ltaWindow * _fsamp));

		initFilterBank();
	}

	if (!_initialized) {
		SEISCOMP_ERROR("FilterPickerCF not initialized: check sampling frequency");
		return;
	}

	// Convert input to vector for processing
	std::vector<TYPE> inputData(data, data + ndata);

	// Compute CF for each band and take maximum
	std::vector<TYPE> maxCF(ndata, 0.0);

	for (size_t b = 0; b < _filterBands.size(); ++b) {
		double lowFreq = _filterBands[b].first;
		double highFreq = _filterBands[b].second;

		// Copy input data for this band
		std::vector<TYPE> bandData = inputData;

		// Apply bandpass filter (4-pole Butterworth)
		Seiscomp::Math::Filtering::IIR::ButterworthBandpass<TYPE> bpFilter(
			4, lowFreq, highFreq, _fsamp);
		bpFilter.apply(ndata, bandData.data());

		// Compute envelope
		std::vector<TYPE> envelope;
		computeEnvelope(bandData, envelope);

		// Compute CF for this band
		std::vector<TYPE> bandCF;
		computeCF(envelope, bandCF);

		// Take maximum across all bands
		for (int i = 0; i < ndata; ++i) {
			maxCF[i] = std::max(maxCF[i], bandCF[i]);
		}
	}

	// Copy result back to input array (in-place modification)
	for (int i = 0; i < ndata; ++i) {
		data[i] = maxCF[i];
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// Explicit template instantiations
INSTANTIATE_INPLACE_FILTER(FilterPickerCF, SC_SYSTEM_CORE_API);

// Register the filter with the factory (multiple names)
REGISTER_INPLACE_FILTER(FilterPickerCF, "FILTERPICKERCF");
REGISTER_INPLACE_FILTER(FilterPickerCF, "FP");  // Short name


} // namespace Filtering
} // namespace Math
} // namespace Seiscomp
