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


#ifndef SEISCOMP_FILTERING_FILTERPICKER_H
#define SEISCOMP_FILTERING_FILTERPICKER_H


#include <seiscomp/math/filter.h>
#include <vector>
#include <cmath>
#include <algorithm>


namespace Seiscomp {
namespace Math {
namespace Filtering {


/**
 * @brief FilterPicker Characteristic Function InPlaceFilter
 *
 * Computes the FilterPicker characteristic function (CF) for seismic traces.
 * The CF emphasizes phase onsets by computing a ratio of short-term to
 * long-term averages of the filtered trace envelope.
 *
 * Based on the FilterPicker algorithm by Lomax et al. (2012):
 * Lomax, A., Satriano, C., & Vassallo, M. (2012).
 * Automatic picker developments and optimization: FilterPicker - a robust,
 * broadband picker for real-time seismic monitoring and earthquake early-warning.
 * Seismological Research Letters, 83(3), 531-540.
 *
 * This implementation operates on a single frequency band. For broadband
 * processing, use multiple instances with different frequency bands and
 * combine the outputs (e.g., using maximum or weighted average).
 *
 * @tparam TYPE Data type (float or double)
 */
template<typename TYPE>
class FilterPickerCF : public InPlaceFilter<TYPE> {
	public:
		/**
		 * @brief Construct a new FilterPicker CF filter
		 *
		 * @param lowFreq Low cutoff frequency in Hz (default: 1.0)
		 * @param highFreq High cutoff frequency in Hz (default: 10.0)
		 * @param staWindow Short-term average window in seconds (default: 0.5)
		 * @param ltaWindow Long-term average window in seconds (default: 5.0)
		 * @param fsamp Sampling frequency in Hz (default: 1.0)
		 */
		FilterPickerCF(
			double lowFreq = 1.0,
			double highFreq = 10.0,
			double staWindow = 0.5,
			double ltaWindow = 5.0,
			double fsamp = 1.0
		);

	public:
		/**
		 * @brief Apply the FilterPicker CF in place to the data
		 *
		 * The input data is replaced with the characteristic function values.
		 * The CF is computed as:
		 * 1. Apply bandpass filter to input
		 * 2. Compute envelope (absolute value + smoothing)
		 * 3. Compute STA/LTA ratio of envelope
		 *
		 * @param ndata Number of data samples
		 * @param data Input/output data array (modified in place)
		 */
		void apply(int ndata, TYPE *data) override;

		/**
		 * @brief Reset the filter state
		 *
		 * Clears internal buffers and resets the sample counter.
		 * Call this before processing a new trace.
		 */
		void reset();

		/**
		 * @brief Set the sampling frequency in Hz
		 *
		 * @param fsamp Sampling frequency
		 */
		void setSamplingFrequency(double fsamp) override;

		/**
		 * @brief Set the filter parameters
		 *
		 * Parameters:
		 * - params[0]: Low cutoff frequency (Hz)
		 * - params[1]: High cutoff frequency (Hz)
		 * - params[2]: STA window (seconds)
		 * - params[3]: LTA window (seconds)
		 *
		 * @param n Number of parameters provided
		 * @param params Parameter array
		 * @return 0 on success, negative value on error (abs(return)-1 is error position)
		 */
		int setParameters(int n, const double *params) override;

		/**
		 * @brief Create a clone of this filter
		 *
		 * @return InPlaceFilter<TYPE>* New filter instance with same parameters
		 */
		InPlaceFilter<TYPE>* clone() const override;

		/**
		 * @brief Set the bandpass filter frequencies
		 *
		 * @param lowFreq Low cutoff frequency in Hz
		 * @param highFreq High cutoff frequency in Hz
		 */
		void setFrequencies(double lowFreq, double highFreq);

		/**
		 * @brief Set the STA and LTA window lengths
		 *
		 * @param staWindow Short-term average window in seconds
		 * @param ltaWindow Long-term average window in seconds
		 */
		void setWindows(double staWindow, double ltaWindow);

	protected:
		/**
		 * @brief Compute envelope of the signal
		 *
		 * Uses absolute value with short moving average smoothing.
		 *
		 * @param data Input signal
		 * @param envelope Output envelope (same size as input)
		 */
		void computeEnvelope(const std::vector<TYPE>& data, std::vector<TYPE>& envelope);

		/**
		 * @brief Compute STA/LTA characteristic function
		 *
		 * @param envelope Input envelope
		 * @param cf Output characteristic function (same size as input)
		 */
		void computeCF(const std::vector<TYPE>& envelope, std::vector<TYPE>& cf);

	protected:
		// Filter parameters
		double _lowFreq;       ///< Low cutoff frequency in Hz
		double _highFreq;      ///< High cutoff frequency in Hz
		double _staWindow;     ///< STA window in seconds
		double _ltaWindow;     ///< LTA window in seconds

		// Derived parameters
		int _numSTA;           ///< STA window in samples
		int _numLTA;           ///< LTA window in samples
		double _fsamp;         ///< Sampling frequency in Hz

		// Internal state
		bool _initialized;     ///< Whether filter parameters are valid
};


} // namespace Filtering
} // namespace Math
} // namespace Seiscomp


#endif // SEISCOMP_FILTERING_FILTERPICKER_H
