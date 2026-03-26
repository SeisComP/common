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
	double lowFreq,
	double highFreq,
	double staWindow,
	double ltaWindow,
	double fsamp
)
: _lowFreq(lowFreq)
, _highFreq(highFreq)
, _staWindow(staWindow)
, _ltaWindow(ltaWindow)
, _numSTA(0)
, _numLTA(0)
, _fsamp(fsamp)
, _initialized(false)
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::setFrequencies(double lowFreq, double highFreq) {
	_lowFreq = lowFreq;
	_highFreq = highFreq;
	_initialized = false; // Need to recompute sample counts
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
void FilterPickerCF<TYPE>::setWindows(double staWindow, double ltaWindow) {
	_staWindow = staWindow;
	_ltaWindow = ltaWindow;
	_initialized = false; // Need to recompute sample counts
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
	_initialized = false; // Will be set to true when parameters are validated
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename TYPE>
int FilterPickerCF<TYPE>::setParameters(int n, const double *params) {
	// Expected parameters:
	// params[0]: lowFreq
	// params[1]: highFreq
	// params[2]: staWindow
	// params[3]: ltaWindow

	if (n < 4) {
		SEISCOMP_ERROR("FilterPickerCF requires 4 parameters: lowFreq, highFreq, staWindow, ltaWindow");
		return -1;
	}

	if (params[0] <= 0 || params[1] <= 0) {
		SEISCOMP_ERROR("Frequencies must be positive: lowFreq=%f, highFreq=%f", params[0], params[1]);
		return -1;
	}

	if (params[0] >= params[1]) {
		SEISCOMP_ERROR("lowFreq (%f) must be less than highFreq (%f)", params[0], params[1]);
		return -2;
	}

	if (params[2] <= 0 || params[3] <= 0) {
		SEISCOMP_ERROR("Windows must be positive: staWindow=%f, ltaWindow=%f", params[2], params[3]);
		return -3;
	}

	if (params[2] >= params[3]) {
		SEISCOMP_ERROR("staWindow (%f) should be less than ltaWindow (%f)", params[2], params[3]);
		return -4;
	}

	_lowFreq = params[0];
	_highFreq = params[1];
	_staWindow = params[2];
	_ltaWindow = params[3];

	return 0; // Success
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
		_lowFreq, _highFreq, _staWindow, _ltaWindow, _fsamp
	);
	return filter;
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

	// Apply short moving average smoother (fixed window, not % of trace)
	// Use 5-10 samples to preserve onset characteristics
	int windowSize = std::min(10, std::max(3, n / 100)); // 1% of trace, min 3, max 10
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
			cf[i] = static_cast<TYPE>(1.0); // Default to 1 when LTA is zero
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
		// Check frequency range relative to Nyquist
		double nyquist = _fsamp / 2.0;
		if (_lowFreq >= nyquist || _highFreq > nyquist) {
			SEISCOMP_ERROR("Frequencies exceed Nyquist limit (fsamp=%f Hz)", _fsamp);
			return;
		}

		// Convert time windows to sample counts
		_numSTA = std::max(1, static_cast<int>(_staWindow * _fsamp));
		_numLTA = std::max(_numSTA + 1, static_cast<int>(_ltaWindow * _fsamp));

		_initialized = true;
	}

	if (!_initialized) {
		SEISCOMP_ERROR("FilterPickerCF not initialized: check sampling frequency");
		return;
	}

	// Convert input to vector for processing
	std::vector<TYPE> inputData(data, data + ndata);

	// Step 1: Apply bandpass filter
	// Use 4-pole Butterworth bandpass
	Seiscomp::Math::Filtering::IIR::ButterworthBandpass<TYPE> bpFilter(4, _lowFreq, _highFreq, _fsamp);
	bpFilter.apply(ndata, inputData.data());

	// Step 2: Compute envelope from filtered data
	std::vector<TYPE> envelope;
	computeEnvelope(inputData, envelope);

	// Step 3: Compute characteristic function (STA/LTA of envelope)
	std::vector<TYPE> cf;
	computeCF(envelope, cf);

	// Copy result back to input array (in-place modification)
	for (int i = 0; i < ndata; ++i) {
		data[i] = cf[i];
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// Explicit template instantiations
INSTANTIATE_INPLACE_FILTER(FilterPickerCF, SC_SYSTEM_CORE_API);

// Register the filter with the factory
REGISTER_INPLACE_FILTER(FilterPickerCF, "FILTERPICKERCF");


} // namespace Filtering
} // namespace Math
} // namespace Seiscomp
