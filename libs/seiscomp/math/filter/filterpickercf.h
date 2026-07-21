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


#ifndef SEISCOMP_MATH_FILTER_FILTERPICKERCF_H
#define SEISCOMP_MATH_FILTER_FILTERPICKERCF_H


#include <seiscomp/math/filter.h>
#include <seiscomp/math/filter/butterworth.h>
#include <vector>


namespace Seiscomp {
namespace Math {
namespace Filtering {


/**
 * @brief FilterPicker Characteristic Function InplaceFilter
 *
 * Based on the FilterPicker algorithm by Lomax et al. (2012)
 * Reference: Lomax, A., Satriano, C., & Vassallo, M. (2012).
 * Automatic picker developments and optimization: FilterPicker - a robust,
 * broadband picker for real-time seismic monitoring and earthquake early-warning.
 * Seismological Research Letters, 83(3), 531-540.
 *
 * This filter computes the characteristic function of the FilterPicker algorithm.
 * It operates on multiple frequency bands and outputs the maximum envelope across
 * all bands. The output can be used by pickers (e.g., AIC, STA/LTA) for phase detection.
 *
 * The filter works incrementally, maintaining state between apply() calls.
 * This is crucial for proper integration with SeisComP's filter chains and
 * continuous data processing.
 *
 * INCREMENTAL PROCESSING:
 * The filter produces identical results whether processing samples in bulk or
 * one sample at a time. This is achieved by:
 * - IIR filters (Butterworth) maintain internal state (v1, v2 memory)
 * - Envelope smoothing buffer maintains state across apply() calls
 * - All operations are sample-by-sample with no look-ahead
 *
 * Usage:
 * - As a pre-filter for scautopick: picker.filter = "FP(numBands,minFreq,maxFreq)"
 * - In filter chains: "BP(0.5,10)+FP(5,0.5,20)"
 * - With AIC picker: picker = AIC, picker.filter = "FP(5,0.5,20)"
 * - With STA/LTA picker: picker = STALTA, picker.filter = "FP(5,0.5,20)"
 *
 * Parameters:
 * - numBands: Number of frequency bands (default: 5)
 * - minFreq: Minimum frequency in Hz (default: 0.5)
 * - maxFreq: Maximum frequency in Hz (default: 20.0)
 *
 * The filter outputs the maximum envelope across all frequency bands.
 * The picker (AIC, STA/LTA, etc.) performs the actual detection on this output.
 */
template<typename TYPE>
class FilterPickerCF : public InPlaceFilter<TYPE> {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Structure to hold filter bank configuration
		 */
		struct FilterBankConfig {
			double centerFreq;      //!< Center frequency in Hz
			double lowFreq;         //!< Low cutoff frequency in Hz
			double highFreq;        //!< High cutoff frequency in Hz
			int    poles;           //!< Number of filter poles
		};

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		FilterPickerCF();

		//! D'tor
		~FilterPickerCF() override;


	// ----------------------------------------------------------------------
	//  InplaceFilter interface
	// ----------------------------------------------------------------------
	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;
		void apply(int n, TYPE *inout) override;
		InPlaceFilter<TYPE>* clone() const override;


	// ----------------------------------------------------------------------
	//  Public methods
	// ----------------------------------------------------------------------
	public:
		//! Reset the filter state
		void reset();

		//! Set filter parameters
		void setFilterParams(int numBands, double minFreq, double maxFreq);

		//! Get the number of frequency bands
		int numBands() const { return _numBands; }

		//! Get filter bank configuration
		const std::vector<FilterBankConfig>& filterBanks() const { return _filterBandsConfig; }


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		/**
		 * @brief Initialize the filter bank
		 */
		void initFilterBank();


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		std::vector<FilterBankConfig> _filterBandsConfig;  //!< Filter bank configuration

		// Algorithm parameters
		int    _numBands{5};        //!< Number of frequency bands
		double _minFreq{0.5};       //!< Minimum frequency in Hz
		double _maxFreq{20.0};      //!< Maximum frequency in Hz

		// Sampling rate
		double _fsamp{0.0};         //!< Sampling frequency in Hz

		// Filter state for each band
		std::vector<Math::Filtering::IIR::ButterworthBandpass<TYPE>*> _filters;

		// Envelope smoothing buffer
		std::vector<TYPE> _envelopeBuffer; //!< Recent envelope values
		size_t _envelopeBufferPos{0};      //!< Position in envelope buffer
		int  _envelopeWindowSize{10};      //!< Envelope smoothing window
};


} // namespace Seiscomp::Math::Filter
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif // SEISCOMP_MATH_FILTER_FILTERPICKERCF_H
