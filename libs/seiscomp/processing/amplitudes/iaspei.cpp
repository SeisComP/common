#include <seiscomp/processing/amplitudes/iaspei.h>

#include <vector>
#include <complex>
#include <cmath>

#include <seiscomp/math/filter/seismometers.h>

namespace Seiscomp {

namespace Processing {

namespace IASPEI {

#include <seiscomp/math/minmax.ipp>

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool measureAmplitudePeriod(
	std::size_t n,
	const double *f,
	double offset,
	std::size_t istart,
	std::size_t iend,
	AmplitudePeriodMeasurement &measurement)
{
	// NOTE:
	// This routine scans the input data array twice, once for the zero
	// crossings and once for the peaks (incl. troughs!) in between.
	// This makes the routine easier to understand but at some
	// computational cost that in tests have shown to be irrelevant in
	// a normal processing setup.

	if ( istart >= iend || iend >= n ) {
		return false;
	}

	// Determine zero crossings.
	std::vector<std::size_t> zero_crossings;
	double previous { f[istart] - offset };
	for ( std::size_t i = istart + 1; i < iend; i++ ) {
		double current { f[i] - offset };
		if ( current*previous <= 0 && previous != 0 ) {
			// => zero crossing
			// i refers to the first sample after zero crossing
			zero_crossings.push_back(i);
		}

		previous = current;
	}

	if ( zero_crossings.size() < 3 ) {
		return false;
	}

	// Determine peaks between zero crossings.
	std::vector<std::size_t> peaks;
	std::size_t nz { zero_crossings.size() };
	for ( std::size_t k = 1; k < nz; k++ ) {
		// The casts will never hurt in practice but should be
		// removed after a revision of find_absmax().
		int i1 = static_cast<int>(zero_crossings[k-1]);
		int i2 = static_cast<int>(zero_crossings[k]);
		std::size_t peak = find_absmax(static_cast<int>(n), f, i1, i2, offset);
		peaks.push_back(peak);
	}

	// Keep track of the largest p2p amplitude...
	double amax { 0 };
	// ... and the respective index
	std::size_t kmax { 0 };

	std::size_t np = peaks.size();
	for ( std::size_t k = 1; k < np; k++ ) {
		std::size_t i1 { peaks[k - 1] };
		std::size_t i2 { peaks[k] };

		// New p2p maximum?
		double a { std::abs(f[i1] - f[i2]) };
		if ( a > amax ) {
			kmax = k;
			amax = a;
		}
	}

	// The z2p peak is the larger of the two consequtive peaks
	// comprising the maximum p2p amplitude.
	std::size_t ip2p1 { peaks[kmax - 1] };
	std::size_t ip2p2 { peaks[kmax] };
	double ap2p1 { std::abs(f[ip2p1] - offset) };
	double ap2p2 { std::abs(f[ip2p2] - offset) };

	measurement.iz2p = ap2p2 > ap2p1 ? ip2p2 : ip2p1;
	measurement.az2p = ap2p2 > ap2p1 ? ap2p2 : ap2p1;

	measurement.ip2p1 = ip2p1;
	measurement.ip2p2 = ip2p2;
	measurement.ap2p1 = ap2p1;
	measurement.ap2p2 = ap2p2;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool measureAmplitudePeriod(
	const std::vector<double> &data,
	double offset,
	std::size_t istart,
	std::size_t iend,
	AmplitudePeriodMeasurement &measurement)
{
	const double *f { &data[0] };
	std::size_t n { data.size() };

	return measureAmplitudePeriod(n, f, offset, istart, iend, measurement);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double wwssnspAmplitudeResponse(double frequency_hz)
{
	using namespace Seiscomp::Math;
	static Filtering::IIR::WWSSN_SP_Filter<double> wwssnsp(Displacement);
	static double twopi { M_PI+M_PI };

        const std::complex<double> s(0, twopi*frequency_hz);
        std::complex<double> r { wwssnsp.norm };
        for ( const std::complex<double> &p : wwssnsp.poles ) {
                r = r/(s-p);
	}
        for ( const std::complex<double> &z : wwssnsp.zeros ) {
                r = r*(s-z);
	}
        return std::abs(r);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace IASPEI

} // namespace Processing

} // namespace Seiscomp

