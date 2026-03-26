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
 * This implementation operates on multiple frequency bands and combines them
 * using the maximum CF across all bands, providing broadband phase detection.
 *
 * @tparam TYPE Data type (float or double)
 */
template<typename TYPE>
class FilterPickerCF : public InPlaceFilter<TYPE> {
	public:
		/**
		 * @brief Construct a new FilterPicker CF filter
		 *
		 * @param numBands Number of frequency bands (default: 5)
		 * @param minFreq Minimum frequency in Hz (default: 0.5)
		 * @param maxFreq Maximum frequency in Hz (default: 20.0)
		 * @param fsamp Sampling frequency in Hz (default: 1.0)
		 */
		FilterPickerCF(
			int numBands = 5,
			double minFreq = 0.5,
			double maxFreq = 20.0,
			double fsamp = 1.0
		);

	public:
		/**
		 * @brief Apply the FilterPicker CF in place to the data
		 *
		 * The input data is replaced with the characteristic function values.
		 * The CF is computed as:
		 * 1. Apply filter bank (multiple bandpass filters)
		 * 2. Compute envelope for each band
		 * 3. Compute STA/LTA CF for each band
		 * 4. Take maximum CF across all bands
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
		 * - params[0]: numBands - Number of frequency bands
		 * - params[1]: minFreq - Minimum frequency (Hz)
		 * - params[2]: maxFreq - Maximum frequency (Hz)
		 *
		 * @param n Number of parameters provided
		 * @param params Parameter array
		 * @return 4 on success (number of params used), negative value on error
		 */
		int setParameters(int n, const double *params) override;

		/**
		 * @brief Create a clone of this filter
		 *
		 * @return InPlaceFilter<TYPE>* New filter instance with same parameters
		 */
		InPlaceFilter<TYPE>* clone() const override;

		/**
		 * @brief Set the number of frequency bands
		 *
		 * @param numBands Number of bands
		 */
		void setNumBands(int numBands);

		/**
		 * @brief Set the frequency range
		 *
		 * @param minFreq Minimum frequency in Hz
		 * @param maxFreq Maximum frequency in Hz
		 */
		void setFrequencies(double minFreq, double maxFreq);

	protected:
		/**
		 * @brief Initialize the filter bank
		 *
		 * Creates bandpass filters for each frequency band
		 */
		void initFilterBank();

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
		int _numBands;         ///< Number of frequency bands
		double _minFreq;       ///< Minimum frequency in Hz
		double _maxFreq;       ///< Maximum frequency in Hz

		// Derived parameters
		double _staWindow;     ///< STA window in seconds (fixed: 0.5)
		double _ltaWindow;     ///< LTA window in seconds (fixed: 10.0)
		int _numSTA;           ///< STA window in samples
		int _numLTA;           ///< LTA window in samples
		double _fsamp;         ///< Sampling frequency in Hz

		// Internal state
		bool _initialized;     ///< Whether filter parameters are valid
		std::vector<std::pair<double, double>> _filterBands; ///< Band frequencies
};


} // namespace Filtering
} // namespace Math
} // namespace Seiscomp


#endif // SEISCOMP_FILTERING_FILTERPICKER_H
