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


#include <seiscomp/processing/amplitudes/iaspei.h>

#include <vector>
#include <complex>
#include <cmath>

#include <seiscomp/math/filter/seismometers.h>


namespace Seiscomp {
namespace Processing {
namespace IASPEI {


#include <seiscomp/math/minmax.ipp>


using namespace std;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool measureAmplitudePeriod(
	size_t n,
	const double *f,
	double offset,
	size_t istart,
	size_t iend,
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
	vector<size_t> zero_crossings;
	double previous { f[istart] - offset };
	for ( size_t i = istart + 1; i < iend; ++i ) {
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
	vector<size_t> peaks;
	size_t nz { zero_crossings.size() };
	for ( size_t k = 1; k < nz; ++k ) {
		// The casts will never hurt in practice but should be
		// removed after a revision of find_absmax().
		int i1 = static_cast<int>(zero_crossings[k-1]);
		int i2 = static_cast<int>(zero_crossings[k]);
		size_t peak = find_absmax(static_cast<int>(n), f, i1, i2, offset);
		peaks.push_back(peak);
	}

	// Keep track of the largest p2p amplitude...
	double amax { 0 };
	// ... and the respective index
	size_t kmax { 0 };

	size_t np = peaks.size();
	for ( size_t k = 1; k < np; ++k ) {
		size_t i1 { peaks[k - 1] };
		size_t i2 { peaks[k] };

		// New p2p maximum?
		double a { abs(f[i1] - f[i2]) };
		if ( a > amax ) {
			kmax = k;
			amax = a;
		}
	}

	// The z2p peak is the larger of the two consequtive peaks
	// comprising the maximum p2p amplitude.
	size_t ip2p1 { peaks[kmax - 1] };
	size_t ip2p2 { peaks[kmax] };
	double ap2p1 { abs(f[ip2p1] - offset) };
	double ap2p2 { abs(f[ip2p2] - offset) };

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
	const vector<double> &data,
	double offset,
	size_t istart,
	size_t iend,
	AmplitudePeriodMeasurement &measurement)
{
	const double *f { &data[0] };
	size_t n { data.size() };

	return measureAmplitudePeriod(n, f, offset, istart, iend, measurement);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double findZeroCrossing(std::size_t n, const double *f, double offset,
                        std::size_t istart, std::size_t iend) {
	double previous { f[istart] - offset };

	for ( size_t i = istart + 1; i < iend; ++i ) {
		double current { f[i] - offset };
		if ( current * previous <= 0 && previous != 0 ) {
			return i - 1 + abs(previous) / (abs(current) + abs(previous));
		}

		previous = current;
	}

	return -1.0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double findZeroCrossing(
	const vector<double> &data,
	double offset,
	std::size_t istart,
	std::size_t iend
) {
	return findZeroCrossing(data.size(), &data[0], offset, istart, iend);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double wwssnspAmplitudeResponse(double frequency_hz) {
	using namespace Seiscomp::Math;
	static Filtering::IIR::WWSSN_SP_Filter<double> wwssnsp(Displacement);
	static double twopi { M_PI+M_PI };

	const complex<double> s(0, twopi * frequency_hz);
	complex<double> r { wwssnsp.norm };
	for ( const auto &p : wwssnsp.poles ) {
		r = r / (s - p);
	}
	for ( const auto &z : wwssnsp.zeros ) {
		r = r * (s - z);
	}
	return abs(r);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace IASPEI
} // namespace Processing
} // namespace Seiscomp
