/***************************************************************************
 * Copyright (C) Donavin Liebgott                                                *
 * All rights reserved.                                                    *
 * Contact: Donavin Liebgott (donavinliebgott@gmail.com)                             *
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


#define SEISCOMP_COMPONENT FilterPicker

#include <seiscomp/logging/log.h>
#include <seiscomp/math/filter/butterworth.h>
#include <seiscomp/math/filter/filterpicker.h>
#include "filterpicker.h"
#include <cmath>
#include <algorithm>
#include <numeric>


using namespace std;


namespace Seiscomp {
namespace Processing {


// Register the FilterPicker plugin
REGISTER_POSTPICKPROCESSOR(FilterPicker, "FilterPicker");


namespace {


/**
 * @brief Compute the envelope of a signal using the Hilbert transform approximation
 *
 * @param data Input signal
 * @return Envelope of the signal
 */
vector<double> computeEnvelope(const vector<double>& data) {
	int n = static_cast<int>(data.size());
	vector<double> envelope(n);

	// Simple envelope computation using absolute value
	// For a more accurate envelope, use the Hilbert transform
	for (int i = 0; i < n; ++i) {
		envelope[i] = fabs(data[i]);
	}

	// Apply a short moving average smoother (fixed window, not % of trace)
	// Use 5-10 samples to preserve onset characteristics
	int windowSize = min(10, max(3, n / 100)); // 1% of trace, min 3, max 10
	if (windowSize % 2 == 0) windowSize++;

	int halfWindow = windowSize / 2;
	for (int i = 0; i < n; ++i) {
		double sum = 0;
		int count = 0;
		for (int j = max(0, i - halfWindow); j <= min(n - 1, i + halfWindow); ++j) {
			sum += envelope[j];
			count++;
		}
		envelope[i] = sum / count;
	}

	return envelope;
}


/**
 * @brief Compute the characteristic function for a filtered trace
 *
 * The characteristic function emphasizes the onset of seismic phases
 * by computing a ratio of short-term to long-term averages of the envelope.
 *
 * @param envelope Envelope of the filtered trace
 * @param staWindow Short-term average window in samples
 * @param ltaWindow Long-term average window in samples
 * @return Characteristic function values
 */
vector<double> computeCF(const vector<double>& envelope, int staWindow, int ltaWindow) {
	int n = static_cast<int>(envelope.size());
	vector<double> cf(n, 0.0);

	// Compute running sums for STA and LTA
	vector<double> sta(n, 0.0);
	vector<double> lta(n, 0.0);

	for (int i = 0; i < n; ++i) {
		// Short-term average
		double staSum = 0;
		int staCount = 0;
		for (int j = max(0, i - staWindow + 1); j <= i; ++j) {
			staSum += envelope[j];
			staCount++;
		}
		sta[i] = staSum / staCount;

		// Long-term average
		double ltaSum = 0;
		int ltaCount = 0;
		for (int j = max(0, i - ltaWindow + 1); j <= i; ++j) {
			ltaSum += envelope[j];
			ltaCount++;
		}
		lta[i] = ltaSum / ltaCount;
	}

	// Compute CF as STA/LTA ratio with regularization
	double cfSum = 0.0;
	double cfMax = 0.0;
	for (int i = 0; i < n; ++i) {
		if (lta[i] > 1e-10) {
			cf[i] = sta[i] / lta[i];
		} else {
			cf[i] = 1.0; // Default to 1 when LTA is zero
		}
		cfSum += cf[i];
		cfMax = max(cfMax, cf[i]);
	}

	return cf;
}


} // anonymous namespace


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FilterPicker::FilterPicker()
: _numBands(5), _minFreq(0.5), _maxFreq(20.0),
  _windowSize(2), _threshold(2.0), _thresholdOff(1.5),
  _useAdaptiveThreshold(false),
  _lastFsamp(0.0) {
	// Filter bank will be initialized in calculatePick() when we have valid fsamp
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FilterPicker::FilterPicker(const Core::Time& trigger)
: Picker(trigger),
  _numBands(5), _minFreq(0.5), _maxFreq(20.0),
  _windowSize(2), _threshold(2.0), _thresholdOff(1.5),
  _useAdaptiveThreshold(false),
  _lastFsamp(0.0) {
	// Filter bank will be initialized in calculatePick() when we have valid fsamp
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FilterPicker::~FilterPicker() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterPicker::reset() {
	Picker::reset();
	_filterBandsConfig.clear();
	_lastFsamp = 0.0;
	// Filter bank will be re-initialized in calculatePick() when we have valid fsamp
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FilterPicker::setup(const Settings &settings) {
	if (!Picker::setup(settings)) {
		return false;
	}

	// Read FilterPicker-specific configuration parameters

	// Number of frequency bands
	try {
		_numBands = settings.getInt("picker.FilterPicker.numBands");
		SEISCOMP_DEBUG("FilterPicker: numBands = %d", _numBands);
	} catch (...) {
		_numBands = 5; // Default: 5 bands
		SEISCOMP_DEBUG("FilterPicker: numBands default = %d", _numBands);
	}

	// Minimum frequency
	try {
		_minFreq = settings.getDouble("picker.FilterPicker.minFreq");
		SEISCOMP_DEBUG("FilterPicker: minFreq = %.2f Hz", _minFreq);
	} catch (...) {
		_minFreq = 0.5; // Default: 0.5 Hz
		SEISCOMP_DEBUG("FilterPicker: minFreq default = %.2f Hz", _minFreq);
	}

	// Maximum frequency
	try {
		_maxFreq = settings.getDouble("picker.FilterPicker.maxFreq");
		SEISCOMP_DEBUG("FilterPicker: maxFreq = %.2f Hz", _maxFreq);
	} catch (...) {
		_maxFreq = 20.0; // Default: 20 Hz
		SEISCOMP_DEBUG("FilterPicker: maxFreq default = %.2f Hz", _maxFreq);
	}

	// Integration window size (in seconds)
	try {
		_windowSize = settings.getInt("picker.FilterPicker.windowSize");
		SEISCOMP_DEBUG("FilterPicker: windowSize = %d s", _windowSize);
	} catch (...) {
		_windowSize = 2; // Default: 2 seconds
		SEISCOMP_DEBUG("FilterPicker: windowSize default = %d s", _windowSize);
	}

	// Detection threshold
	try {
		_threshold = settings.getDouble("picker.FilterPicker.threshold");
		SEISCOMP_DEBUG("FilterPicker: threshold = %.2f", _threshold);
	} catch (...) {
		_threshold = 2.0; // Default: 2.0 (CF ratio)
		SEISCOMP_DEBUG("FilterPicker: threshold default = %.2f", _threshold);
	}

	// Threshold off (for reset)
	try {
		_thresholdOff = settings.getDouble("picker.FilterPicker.thresholdOff");
		SEISCOMP_DEBUG("FilterPicker: thresholdOff = %.2f", _thresholdOff);
	} catch (...) {
		_thresholdOff = 1.5; // Default: 1.5
		SEISCOMP_DEBUG("FilterPicker: thresholdOff default = %.2f", _thresholdOff);
	}

	// Adaptive threshold
	try {
		_useAdaptiveThreshold = settings.getBool("picker.FilterPicker.adaptiveThreshold");
		SEISCOMP_DEBUG("FilterPicker: adaptiveThreshold = %s",
		               _useAdaptiveThreshold ? "true" : "false");
	} catch (...) {
		_useAdaptiveThreshold = true; // Default: enabled
		SEISCOMP_DEBUG("FilterPicker: adaptiveThreshold default = true");
	}

	// Configuration from parent class (noise/signal windows, SNR)
	try {
		_config.noiseBegin = settings.getDouble("picker.FilterPicker.noiseBegin");
	} catch (...) {
		_config.noiseBegin = -10.0;
	}

	try {
		_config.signalBegin = settings.getDouble("picker.FilterPicker.signalBegin");
	} catch (...) {
		_config.signalBegin = -5.0;
	}

	try {
		_config.signalEnd = settings.getDouble("picker.FilterPicker.signalEnd");
	} catch (...) {
		_config.signalEnd = 20.0;
	}

	try {
		_config.snrMin = settings.getDouble("picker.FilterPicker.minSNR");
	} catch (...) {
		_config.snrMin = 2.0;
	}

	// Reinitialize filter bank with new parameters
	initFilterBank();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &FilterPicker::methodID() const {
	static string method = "FilterPicker";
	return method;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &FilterPicker::filterID() const {
	return _filterString;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterPicker::initFilterBank() {
	_filterBandsConfig.clear();
	_filterString.clear();

	if (_numBands <= 0) {
		SEISCOMP_WARNING("FilterPicker: numBands must be positive, using default");
		_numBands = 5;
	}

	// Compute Nyquist frequency and adjust max frequency if needed
	double nyquist = _stream.fsamp / 2.0;
	double effectiveMaxFreq = min(_maxFreq, nyquist * 0.95); // Use 95% of Nyquist for safety margin

	if (_minFreq <= 0 || effectiveMaxFreq <= _minFreq) {
		SEISCOMP_WARNING("FilterPicker: invalid frequency range (fsamp=%.2f Hz), using adjusted defaults",
		                 _stream.fsamp);
		_minFreq = 0.5;
		effectiveMaxFreq = min(20.0, nyquist * 0.95);
	}

	// Create logarithmically spaced frequency bands
	// This follows the FilterPicker design for broadband coverage
	double logMinFreq = log10(_minFreq);
	double logMaxFreq = log10(effectiveMaxFreq);
	double logStep = (logMaxFreq - logMinFreq) / _numBands;

	ostringstream filterStr;
	filterStr << "FilterPicker[" << _numBands << " bands: " << _minFreq << "-" << effectiveMaxFreq << " Hz]";

	for (int i = 0; i < _numBands; ++i) {
		FilterBankConfig config;

		// Center frequency (logarithmic spacing)
		config.centerFreq = pow(10.0, logMinFreq + (i + 0.5) * logStep);

		// Band edges (approximately 1/3 octave bandwidth)
		double bandwidth = pow(10.0, logStep / 2.0);
		config.lowFreq = config.centerFreq / bandwidth;
		config.highFreq = config.centerFreq * bandwidth;

		// Clamp to min/max frequencies and Nyquist
		config.lowFreq = max(config.lowFreq, _minFreq);
		config.highFreq = min(config.highFreq, effectiveMaxFreq);

		// Ensure lowFreq < highFreq after clamping
		if (config.lowFreq >= config.highFreq) {
			config.lowFreq = config.highFreq * 0.8;
		}

		// Number of poles (typically 2-4 for Butterworth)
		config.poles = 2;

		_filterBandsConfig.push_back(config);

		SEISCOMP_DEBUG("FilterPicker band %d: %.2f-%.2f Hz (center: %.2f Hz)",
		               i, config.lowFreq, config.highFreq, config.centerFreq);
	}

	_filterString = filterStr.str();
	SEISCOMP_INFO("FilterPicker initialized with %d frequency bands (fsamp=%.2f Hz, Nyquist=%.2f Hz)",
	              _numBands, _stream.fsamp, nyquist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<vector<double>> FilterPicker::applyFilterBank(const double *data, int n) {
	vector<vector<double>> filteredTraces(_numBands);

	for (int i = 0; i < _numBands; ++i) {
		const FilterBankConfig& config = _filterBandsConfig[i];

		// Copy input data
		vector<double> trace(data, data + n);

		// Apply Butterworth bandpass filter
		try {
			Math::Filtering::IIR::ButterworthBandpass<double> filter(
				config.poles, config.lowFreq, config.highFreq, _stream.fsamp);
			filter.apply(n, trace.data());
		} catch (exception& e) {
			SEISCOMP_WARNING("FilterPicker: Filter band %d failed: %s", i, e.what());
			// Fill with zeros on failure
			trace.assign(n, 0.0);
		}

		// Debug: check filter output
		static int filterDebug = 0;
		if (filterDebug < 5) {
			double traceMax = 0.0, traceSum = 0.0;
			for (int j = 0; j < n; ++j) {
				traceMax = max(traceMax, fabs(trace[j]));
				traceSum += fabs(trace[j]);
			}
			SEISCOMP_DEBUG("Filter band %d (%.2f-%.2f Hz): outMax=%.6f, outAvg=%.6f",
			               i, config.lowFreq, config.highFreq, traceMax, traceSum/n);
			filterDebug++;
		}

		filteredTraces[i] = trace;
	}

	return filteredTraces;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FilterPicker::CharacteristicFunction FilterPicker::computeCharacteristicFunction(
	const vector<double>& filtered) {

	CharacteristicFunction cf;
	int n = static_cast<int>(filtered.size());

	cf.filtered = filtered;

	// Use the FilterPickerCF InPlaceFilter to compute the characteristic function
	// This provides a reusable CF computation that can be used independently
	Math::Filtering::FilterPickerCF<double> cfFilter(
		0.5, 10.0,  // Frequency range (CF is computed on already-filtered data)
		0.5, 10.0,  // STA/LTA windows in seconds
		_stream.fsamp
	);

	// Copy filtered data for in-place modification
	vector<double> cfData = filtered;
	cfFilter.apply(n, cfData.data());
	cf.values = cfData;

	// Compute integral and find maximum
	cf.integral = 0.0;
	cf.maxVal = 0.0;
	cf.maxIdx = -1;

	for (int i = 0; i < n; ++i) {
		cf.integral += cf.values[i];
		if (cf.values[i] > cf.maxVal) {
			cf.maxVal = cf.values[i];
			cf.maxIdx = i;
		}
	}

	// Debug: check if filtered data has signal
	static int debugCount = 0;
	if (debugCount < 5) {
		// Compute stats on filtered data
		double filteredMax = 0.0, filteredSum = 0.0;
		for (int i = 0; i < n; ++i) {
			filteredMax = max(filteredMax, fabs(filtered[i]));
			filteredSum += fabs(filtered[i]);
		}
		SEISCOMP_DEBUG("CF debug: filtMax=%.6f, filtAvg=%.6f, cfMax=%.4f, cfAvg=%.4f",
		               filteredMax, filteredSum/n, cf.maxVal, cf.integral/n);
		debugCount++;
	}

	return cf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double FilterPicker::computeCFIntegral(
	const vector<CharacteristicFunction>& cfs, int startIdx, int windowSize) {

	if (cfs.empty() || startIdx < 0) return 0.0;

	int n = static_cast<int>(cfs[0].values.size());
	if (startIdx >= n) return 0.0;

	int endIdx = min(startIdx + windowSize, n);
	double maxIntegral = 0.0;

	// Compute integral of maximum CF across all bands
	for (int i = startIdx; i < endIdx; ++i) {
		double maxCF = 0.0;
		for (const auto& cf : cfs) {
			if (i < static_cast<int>(cf.values.size())) {
				maxCF = max(maxCF, cf.values[i]);
			}
		}
		maxIntegral += maxCF;
	}

	// Debug: log first few calls to understand the issue
	static int debugCount = 0;
	if (debugCount < 5) {
		SEISCOMP_DEBUG("CFIntegral: startIdx=%d, windowSize=%d, n=%d, result=%.3f, cfs[0].size=%zu",
		               startIdx, windowSize, n, maxIntegral, cfs[0].values.size());
		debugCount++;
	}

	return maxIntegral;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FilterPicker::findOnset(const vector<CharacteristicFunction>& cfs,
                              double threshold, int windowSize,
                              int &onsetIdx, double &snr) {
	if (cfs.empty()) return false;

	int n = static_cast<int>(cfs[0].values.size());
	if (n < 10) return false;

	// windowSize is already in samples (passed from calculatePick)
	int windowSamples = max(1, windowSize);

	// Always use adaptive thresholding based on noise level
	// Use first third of trace as noise reference
	int noiseStart = 0;
	int noiseEnd = max(10, min(n / 3, static_cast<int>(n / 10.0))); // First 10% or at least 10 samples
	
	double noiseSum = 0.0;
	int noiseCount = 0;
	double noiseMax = 0.0;

	for (int i = noiseStart; i < noiseEnd; ++i) {
		double maxCF = 0.0;
		for (const auto& cf : cfs) {
			if (i < static_cast<int>(cf.values.size())) {
				maxCF = max(maxCF, cf.values[i]);
			}
		}
		noiseSum += maxCF;
		noiseMax = max(noiseMax, maxCF);
		noiseCount++;
	}

	double noiseMean = noiseCount > 0 ? noiseSum / noiseCount : 1.0;
	double noiseStd = 0.0;
	
	// Compute noise standard deviation
	for (int i = noiseStart; i < noiseEnd; ++i) {
		double maxCF = 0.0;
		for (const auto& cf : cfs) {
			if (i < static_cast<int>(cf.values.size())) {
				maxCF = max(maxCF, cf.values[i]);
			}
		}
		double diff = maxCF - noiseMean;
		noiseStd += diff * diff;
	}
	noiseStd = noiseCount > 1 ? sqrt(noiseStd / (noiseCount - 1)) : noiseMean;

	// Use adaptive threshold: based on noise statistics
	// The threshold is compared against the MAX CF value in the window, not the integral
	// So we use point-wise threshold, not integrated
	double adaptiveThreshold = max(threshold * noiseMean, noiseMean + threshold * noiseStd);

	// Ensure minimum threshold (CF ratio typically 2-10 for good onsets)
	adaptiveThreshold = max(adaptiveThreshold, 1.5);

	SEISCOMP_DEBUG("FilterPicker: noise stats - mean=%.3f, std=%.3f, max=%.3f, threshold=%.3f",
	               noiseMean, noiseStd, noiseMax, adaptiveThreshold);
	SEISCOMP_DEBUG("FilterPicker: n=%d, windowSamples=%d, loop end=%d, cfs.size=%zu",
	               n, windowSamples, n - windowSamples, cfs.size());

	// Scan for onset: find first point where MAX CF (not integral) exceeds threshold
	int maxCFIdx = -1;
	double maxCFVal = 0.0;
	int scanCount = 0;

	for (int i = 0; i < n - windowSamples; ++i) {
		// Get max CF value across all bands at this sample
		double maxCF = 0.0;
		for (const auto& cf : cfs) {
			if (i < static_cast<int>(cf.values.size())) {
				maxCF = max(maxCF, cf.values[i]);
			}
		}
		scanCount++;

		// Debug first few iterations
		if (i < 3) {
			SEISCOMP_DEBUG("findOnset: i=%d, maxCF=%.3f", i, maxCF);
		}

		if (maxCF >= adaptiveThreshold) {
			onsetIdx = i;

			// Compute SNR as ratio of max signal to noise level
			double signalMax = 0.0;

			// Signal window (after onset)
			int signalEnd = min(i + windowSamples * 2, n);
			for (int j = i; j < signalEnd; ++j) {
				for (const auto& cf : cfs) {
					if (j < static_cast<int>(cf.values.size())) {
						signalMax = max(signalMax, cf.values[j]);
					}
				}
			}

			snr = noiseMax > 1e-10 ? signalMax / noiseMax : 0.0;

			SEISCOMP_DEBUG("FilterPicker: onset found at index %d, SNR = %.2f", onsetIdx, snr);
			return true;
		}

		// Track maximum CF value for fallback
		if (maxCF > maxCFVal) {
			maxCFVal = maxCF;
			maxCFIdx = i;
		}
	}

	// Fallback: if no threshold crossing found, use the maximum CF point
	// if it's significantly above noise level
	if (maxCFIdx >= 0 && maxCFVal > adaptiveThreshold * 0.5) {
		onsetIdx = maxCFIdx;

		// Compute SNR at this point
		double signalMax = 0.0;
		int signalEnd = min(maxCFIdx + windowSamples * 2, n);
		for (int j = maxCFIdx; j < signalEnd; ++j) {
			for (const auto& cf : cfs) {
				if (j < static_cast<int>(cf.values.size())) {
					signalMax = max(signalMax, cf.values[j]);
				}
			}
		}
		snr = noiseMax > 1e-10 ? signalMax / noiseMax : 0.0;

		SEISCOMP_DEBUG("FilterPicker: onset found (fallback) at index %d, SNR = %.2f", onsetIdx, snr);
		return true;
	}

	SEISCOMP_DEBUG("FilterPicker: no onset found (maxCF=%.3f, threshold=%.3f)",
	               maxCFVal, adaptiveThreshold);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FilterPicker::estimateUncertainty(const vector<CharacteristicFunction>& cfs,
                                        int onsetIdx, int &lowerUnc, int &upperUnc) {
	if (cfs.empty() || onsetIdx < 0) {
		lowerUnc = -1;
		upperUnc = -1;
		return;
	}

	int n = static_cast<int>(cfs[0].values.size());

	// Find the rise time of the characteristic function
	// Uncertainty is related to how sharp the onset is

	// Get the maximum CF value at onset
	double maxCF = 0.0;
	for (const auto& cf : cfs) {
		if (onsetIdx < static_cast<int>(cf.values.size())) {
			maxCF = max(maxCF, cf.values[onsetIdx]);
		}
	}

	// Find points where CF reaches 10% and 90% of maximum
	int idx10 = onsetIdx;
	int idx90 = onsetIdx;

	double threshold10 = 0.1 * maxCF;
	double threshold90 = 0.9 * maxCF;

	// Search backward for 10% point
	for (int i = onsetIdx; i >= 0; --i) {
		double cf = 0.0;
		for (const auto& c : cfs) {
			if (i < static_cast<int>(c.values.size())) {
				cf = max(cf, c.values[i]);
			}
		}
		if (cf <= threshold10) {
			idx10 = i;
			break;
		}
	}

	// Search forward for 90% point
	for (int i = onsetIdx; i < n; ++i) {
		double cf = 0.0;
		for (const auto& c : cfs) {
			if (i < static_cast<int>(c.values.size())) {
				cf = max(cf, c.values[i]);
			}
		}
		if (cf >= threshold90) {
			idx90 = i;
			break;
		}
	}

	// Uncertainty is proportional to rise time
	int riseTime = idx90 - idx10;
	lowerUnc = max(1, riseTime / 2);
	upperUnc = max(1, riseTime / 2);

	SEISCOMP_DEBUG("FilterPicker: uncertainty estimated: lower=%d, upper=%d samples",
	               lowerUnc, upperUnc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Picker::Polarity FilterPicker::determinePolarity(const double *data, int onsetIdx, double fsamp) {
	if (onsetIdx < 0 || onsetIdx >= static_cast<int>(_stream.fsamp * _config.signalEnd)) {
		return UNDECIDABLE;
	}

	// Look at the first few samples after onset to determine polarity
	int windowSize = max(1, static_cast<int>(fsamp * 0.1)); // 100 ms window
	double sum = 0.0;

	for (int i = onsetIdx; i < min(onsetIdx + windowSize, static_cast<int>(_stream.fsamp * _config.signalEnd)); ++i) {
		sum += data[i];
	}

	double mean = sum / windowSize;

	if (mean > 1e-10) {
		return POSITIVE;
	} else if (mean < -1e-10) {
		return NEGATIVE;
	}

	return UNDECIDABLE;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FilterPicker::calculatePick(int n, const double *data,
                                  int signalStartIdx, int signalEndIdx,
                                  int &triggerIdx, int &lowerUncertainty,
                                  int &upperUncertainty, double &snr,
                                  OPT(Polarity) &polarity) {

	SEISCOMP_DEBUG("FilterPicker::calculatePick: n=%d, signalStart=%d, signalEnd=%d, fsamp=%.2f",
	               n, signalStartIdx, signalEndIdx, _stream.fsamp);

	if (n <= 10) {
		SEISCOMP_INFO("FilterPicker: not enough data (n=%d)", n);
		return false;
	}

	// Check if we have a valid sampling rate
	if (_stream.fsamp <= 0) {
		SEISCOMP_WARNING("FilterPicker: invalid sampling rate (%.2f Hz), cannot pick", _stream.fsamp);
		return false;
	}

	// Initialize filter bank if not yet done or if sampling rate changed
	if (_filterBandsConfig.empty() || _stream.fsamp != _lastFsamp) {
		_lastFsamp = _stream.fsamp;
		initFilterBank();
	}

	// Demean the data using the noise window
	int nnoise = max(1, static_cast<int>(n / 3));
	double offset = 0.0;
	for (int i = 0; i < nnoise; ++i) {
		offset += data[i];
	}
	offset /= nnoise;

	// Create demeaned copy
	vector<double> demeaned(signalEndIdx - signalStartIdx);
	for (size_t i = 0; i < demeaned.size(); ++i) {
		demeaned[i] = data[signalStartIdx + i] - offset;
	}

	// Apply filter bank
	vector<vector<double>> filteredTraces = applyFilterBank(demeaned.data(),
	                                                         static_cast<int>(demeaned.size()));

	// Compute characteristic functions for all bands
	vector<CharacteristicFunction> cfs;
	cfs.reserve(_numBands);

	for (const auto& filtered : filteredTraces) {
		cfs.push_back(computeCharacteristicFunction(filtered));
	}

	// Find onset
	int windowSamples = static_cast<int>(_windowSize * _stream.fsamp);
	int onsetIdx = -1;
	snr = -1.0;

	if (!findOnset(cfs, _threshold, windowSamples, onsetIdx, snr)) {
		SEISCOMP_INFO("FilterPicker: no onset found");
		return false;
	}

	// Check SNR threshold
	if (snr < _config.snrMin) {
		SEISCOMP_INFO("FilterPicker: SNR %.2f below threshold %.2f", snr, _config.snrMin);
		return false;
	}

	// Convert to absolute index
	triggerIdx = onsetIdx + signalStartIdx;

	// Estimate uncertainty
	estimateUncertainty(cfs, onsetIdx, lowerUncertainty, upperUncertainty);

	// Determine polarity
	polarity = determinePolarity(demeaned.data(), onsetIdx, _stream.fsamp);

	SEISCOMP_INFO("FilterPicker: pick found at index %d (relative: %d), SNR=%.2f, "
	              "uncertainty=[%d,%d] samples",
	              triggerIdx, onsetIdx, snr, lowerUncertainty, upperUncertainty);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Processing
} // namespace Seiscomp
