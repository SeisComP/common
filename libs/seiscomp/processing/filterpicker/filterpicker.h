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


#ifndef SEISCOMP_PROCESSING_FILTERPICKER_H
#define SEISCOMP_PROCESSING_FILTERPICKER_H


#include <seiscomp/processing/picker.h>
#include <seiscomp/math/filter.h>
#include <seiscomp/math/filter/butterworth.h>
#include <seiscomp/math/filter/filterpickercf.h>
#include <seiscomp/core/plugin.h>
#include <vector>


namespace Seiscomp {
namespace Processing {


/**
 * @brief FilterPicker - A robust, broadband picker for real-time seismic monitoring
 * 
 * Based on the FilterPicker algorithm by Lomax et al. (2012)
 * Reference: Lomax, A., Satriano, C., & Vassallo, M. (2012). 
 * Automatic picker developments and optimization: FilterPicker - a robust, 
 * broadband picker for real-time seismic monitoring and earthquake early-warning. 
 * Seismological Research Letters, 83(3), 531-540.
 * 
 * The algorithm operates on multiple frequency bands and uses characteristic
 * functions to detect and pick seismic phases.
 */
class SC_SYSTEM_CLIENT_API FilterPicker : public Picker {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Structure to hold filter bank configuration
		 */
		struct FilterBankConfig {
			double centerFreq;      // Center frequency in Hz
			double lowFreq;         // Low cutoff frequency in Hz
			double highFreq;        // High cutoff frequency in Hz
			int    poles;           // Number of filter poles
		};

		/**
		 * @brief Structure to hold characteristic function data for a frequency band
		 */
		struct CharacteristicFunction {
			std::vector<double> values;  // CF values
			std::vector<double> filtered; // Filtered trace for this band
			double integral;              // Running integral
			double maxVal;                // Maximum value
			int    maxIdx;                // Index of maximum
		};

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		FilterPicker();
		FilterPicker(const Core::Time& trigger);

		//! D'tor
		~FilterPicker();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setup(const Settings &settings) override;

		const std::string &methodID() const override;
		const std::string &filterID() const override;

		//! Get the number of frequency bands
		int numBands() const { return _numBands; }

		//! Get filter bank configuration
		const std::vector<FilterBankConfig>& filterBanks() const { return _filterBandsConfig; }


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		//! Calculate pick using FilterPicker algorithm
		bool calculatePick(int n, const double *data,
		                   int signalStartIdx, int signalEndIdx,
		                   int &triggerIdx, int &lowerUncertainty,
		                   int &upperUncertainty, double &snr,
		                   OPT(Polarity) &polarity) override;

		//! Reset the picker state
		void reset() override;


	// ----------------------------------------------------------------------
	//  Private Interface
	// ----------------------------------------------------------------------
	private:
		/**
		 * @brief Initialize the filter bank
		 * 
		 * Creates bandpass filters for each frequency band
		 */
		void initFilterBank();

		/**
		 * @brief Apply filter bank to input data
		 * 
		 * @param data Input data
		 * @param n Number of samples
		 * @return Vector of filtered traces for each band
		 */
		std::vector<std::vector<double>> applyFilterBank(const double *data, int n);

		/**
		 * @brief Compute characteristic function for a filtered trace
		 * 
		 * The characteristic function is based on the envelope of the signal
		 * and emphasizes the onset of seismic phases.
		 * 
		 * @param filtered Filtered trace
		 * @return Characteristic function values
		 */
		CharacteristicFunction computeCharacteristicFunction(const std::vector<double>& filtered);

		/**
		 * @brief Compute the integral of the maximum characteristic function
		 * 
		 * @param cfs Vector of characteristic functions for all bands
		 * @param startIdx Start index for integration
		 * @param windowSize Size of the integration window in samples
		 * @return Integral value
		 */
		double computeCFIntegral(const std::vector<CharacteristicFunction>& cfs, 
		                         int startIdx, int windowSize);

		/**
		 * @brief Find the pick onset using the FilterPicker algorithm
		 * 
		 * @param cfs Characteristic functions for all bands
		 * @param threshold Detection threshold
		 * @param windowSize Integration window size
		 * @param onsetIdx Output: index of the onset
		 * @param snr Output: signal-to-noise ratio
		 * @return true if onset found, false otherwise
		 */
		bool findOnset(const std::vector<CharacteristicFunction>& cfs,
		               double threshold, int windowSize,
		               int &onsetIdx, double &snr);

		/**
		 * @brief Estimate pick uncertainty
		 * 
		 * @param cfs Characteristic functions
		 * @param onsetIdx Onset index
		 * @param lowerUnc Output: lower uncertainty in samples
		 * @param upperUnc Output: upper uncertainty in samples
		 */
		void estimateUncertainty(const std::vector<CharacteristicFunction>& cfs,
		                         int onsetIdx, int &lowerUnc, int &upperUnc);

		/**
		 * @brief Determine polarity of the onset
		 * 
		 * @param data Original data
		 * @param onsetIdx Onset index
		 * @param fsamp Sampling frequency
		 * @return Polarity (POSITIVE, NEGATIVE, or UNDECIDABLE)
		 */
		Polarity determinePolarity(const double *data, int onsetIdx, double fsamp);

	// ----------------------------------------------------------------------
	//  Private Members
	// ----------------------------------------------------------------------
	private:
		std::vector<FilterBankConfig> _filterBandsConfig;  // Filter bank configuration
		std::string _filterString;                    // Filter string for filterID()

		// Algorithm parameters
		int    _numBands;           // Number of frequency bands (default: 5)
		double _minFreq;            // Minimum frequency in Hz (default: 0.5)
		double _maxFreq;            // Maximum frequency in Hz (default: 20)
		int    _windowSize;         // Integration window size in seconds (default: 2)
		double _threshold;          // Detection threshold (default: 3.0)
		double _thresholdOff;       // Threshold for reset (default: 1.5)
		bool   _useAdaptiveThreshold; // Use adaptive thresholding
		
		// Sampling rate tracking
		double _lastFsamp{0.0};     // Last sampling rate used for filter bank initialization
};


} // namespace Processing
} // namespace Seiscomp


#endif // SEISCOMP_PROCESSING_FILTERPICKER_H
