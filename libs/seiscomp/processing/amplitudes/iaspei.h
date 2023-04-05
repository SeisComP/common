#ifndef SEISCOMP_PROCESSING_IASPEI_H_INCLUDED
#define SEISCOMP_PROCESSING_IASPEI_H_INCLUDED

#include <vector>

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
// seismogram trace, peak-to-adjacent- trough or trough-to-adjacent-peak,
// where peak and trough are separated by one crossing of the zero-line: the
// measurement is sometimes described as "one-half peak-to-peak amplitude."
// None of the magnitude formulas presented in this article are intended to
// be used with the full peak-to-trough deflection as the amplitude. The
// periods are to be measured as twice the time-intervals separating the
// peak and adjacent-trough from which the amplitudes are measured.


struct AmplitudePeriodMeasurement
{
	// Amplitudes
	//
	// Note that "peak" here refers to both maxima and minima and
	// thus can also refer to a trough. So peak-to-peak is either
	// peak-to-trough or trough-to-peak. Just to avoid confusion.
	//
	// - az2p Max. zero-to-peak amplitude found at index iz2p
	// - ap2p1 first peak of the max. peak-to-peak amplitude pair
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
	//
	//   (ip2p2 - ip2p1)*2
	//
	//   To be divided by the sampling rate in order to get the period
	//   in seconds.
	//
	//
	// - One half of the max. peak-to-peak amplitude in data units:
	//
	//   (ap2p1 + ap2p2)/2
	//
	//
	// - Time of measurement as offset from first sample of the middle
	//   of the two peaks:
	//
	//   (ip2p1 + ip2p2)/2
	//
	//   To be divided by the sampling rate in order to get the time
	//   in seconds.
};


// Compute amplitude and dominant period for the given data vector.
//
// The data may have an offset to be removed during the amplitude measurement.
// If dealing with demeaned data the offset should be zero.
//
// We only work on the time window between samples istart and iend.
//
// A reference to a struct AmplitudePeriodMeasurement is speciefied, where the
// result will be written to. See above for details.
bool measureAmplitudePeriod(
	const std::vector<double> &data,
	double offset,
	std::size_t istart,
	std::size_t iend,
	AmplitudePeriodMeasurement &measurement);


// Interface for raw pointer.
bool measureAmplitudePeriod(
	std::size_t ndata,
	const double *data,
	double offset,
	std::size_t istart,
	std::size_t iend,
	AmplitudePeriodMeasurement &measurement);


// Compute the WWSSN-SP displacement amplitude response needed for the
// mb amplitude correction.
//
// The response is based on the poles and zeros specified in the IASPEI
// Magnitude Working Group Recommendations.
double wwssnspAmplitudeResponse(double frequency_hz);

} // namespace IASPEI

} // namespace Processing

} // namespace Seiscomp

#endif
