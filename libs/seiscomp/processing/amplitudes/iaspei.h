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


#ifndef SEISCOMP_PROCESSING_IASPEI_H_INCLUDED
#define SEISCOMP_PROCESSING_IASPEI_H_INCLUDED


#include <vector>
#include <cstdint>


namespace Seiscomp {

namespace Processing {

namespace IASPEI {


// IASPEI Amplitude Measurement Procedure
//
// From:
// Summary of Magnitude Working Group Recommendations on Standard Procedures
// for Determining Earthquake Magnitudes From Digital Data - 2013 March 27
//
// Amplitudes [...] to be measured as one-half the maximum deflection of the
// seismogram trace, peak-to-adjacent-trough or trough-to-adjacent-peak,
// where peak and trough are separated by one crossing of the zero-line: the
// measurement is sometimes described as "one-half peak-to-peak amplitude."
// None of the magnitude formulas presented in this article are intended to
// be used with the full peak-to-trough deflection as the amplitude. The
// periods are to be measured as twice the time-intervals separating the
// peak and adjacent-trough from which the amplitudes are measured.


struct AmplitudePeriodMeasurement
{
	AmplitudePeriodMeasurement() : success(false) {}

	// Amplitudes
	//
	// Note that "peak" here refers to both maxima and minima and thus
	// can also refer to a trough. So peak-to-peak can refer to either
	// peak-to-trough or trough-to-peak. Just to clearify.
	//
	// - az2p Max. zero-to-peak amplitude found at index iz2p
	//
	// - ap2p1 first peak of the max. peak-to-peak amplitude pair
	//
	// - ap2p1 second peak 
	//
	// Note that az2p is the larger of ap2p1 and ap2p2.

	double az2p;
	double ap2p1;
	double ap2p2;

	// Sample indices corresponding the above peaks
	std::size_t iz2p;
	std::size_t ip2p1;
	std::size_t ip2p2;

	// From the above numbers we can now derive:
	//
	// - Period in sampling intervals:
	//   (ip2p2 - ip2p1)*2
	//
	// - Amplitude in data units:
	//   (ap2p1 + ap2p2)/2
	//
	// - Time as offset from first sample:
	//   (ip2p1 + ip2p2)/2

	// Success of the measurement, true or false
	bool success;
};


// Measure the amplitude and period in the given data array according
// to the IASPEI Magnitude Working Group Recommendations.
//
// Input is as follows:
//
// - data is the data array.
//
// - offset is the data offset to be subtracted from the data samples.
//
// - istart and iend are the indices of start and end samples, respectively.
//   They define the time window in which the measurement will take place.
//   By default the entire data array is used.
//
// - pmin and pmax are the minimum and maximum data period, respectively.
//   An amplitude is accepted only if the period (ip2p2 - ip2p1)*2 falls
//   within this period range. Note that the period must be specified in
//   samples (not seconds). The default values disable period filtering.
//
AmplitudePeriodMeasurement
measureAmplitudePeriod(
	std::size_t ndata,
	const double *data,
	double offset,
	std::size_t istart,
	std::size_t iend,
	std::size_t pmin=0,
	std::size_t pmax=SIZE_MAX);


AmplitudePeriodMeasurement
measureAmplitudePeriod(
	const std::vector<double> &data,
	double offset,
	std::size_t istart,
	std::size_t iend,
	std::size_t pmin=0,
	std::size_t pmax=SIZE_MAX);


// Compute the WWSSN-SP displacement amplitude response needed for the
// mb amplitude correction.
//
// The response is based on the poles and zeros specified in the IASPEI
// Magnitude Working Group Recommendations.
//
double wwssnspAmplitudeResponse(double frequency_hz);


// Compute the WWSSN-LP displacement amplitude response needed for the
// Ms20 amplitude correction.
//
// The response is based on the poles and zeros specified in the IASPEI
// Magnitude Working Group Recommendations.
//
double wwssnlpAmplitudeResponse(double frequency_hz);


} // namespace IASPEI

} // namespace Processing

} // namespace Seiscomp


#endif
