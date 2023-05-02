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


#define OUT_OF_RANGE { \
	std::string message { "out of range in " + std::string(__FILE__) + ":" + std::to_string(__LINE__) }; \
	throw std::runtime_error(message); \
}

#include <seiscomp/math/minmax.ipp>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudePeriodMeasurement
measureAmplitudePeriod(
	std::size_t n,
	const double *f,
	double offset,
	std::size_t istart,
	std::size_t iend,
	std::size_t pmin,
	std::size_t pmax)
{
	using std::size_t, std::abs;

	AmplitudePeriodMeasurement m;

	if ( istart > iend || iend > n )
		OUT_OF_RANGE;

	// Maximum peak-to-peak amplitude seen so far.
	double amax { 0 };
	// Keep track of last two consecutive amplitude extrema ...
	double ap2p1 { 0 };
	double ap2p2 { 0 };
	// ... and the indices at which they occurred:
	size_t ip2p1 { 0 };
	size_t ip2p2 { 0 };

	// A full peak-to-peak measurement requires 3 zero crossings.
	// We therefore keep track of the three latest zero crossing
	//indices:
	//
	// first zero crossing
	size_t z1 { 0 };
	// second zero crossing
	size_t z2 { 0 };
	// third (latest) zero crossing
	size_t z3 { 0 };

	double previous { f[istart] - offset };

	for ( size_t i = istart + 1; i < iend; i++ ) {
		double current { f[i] - offset };
		double a { abs(current) };

		// Check if we have just crossed zero.
		bool zero_crossing { current*previous <= 0 && previous != 0 };
		if ( zero_crossing ) {
			// Rotate zero crossings.
			z1 = z2;
			z2 = z3;
			z3 = i;

			// If
			// - there were at least 3 zero crossings (i.e. z1 > 0)
			//   and
			// - peak-to-peak amplitude has increased
			//   and
			// - the signal period is within the specified limits
			// then we update the measurement.
			if ( ap2p1 + ap2p2 > amax  &&  z1 > 0 ) {

				// Is the period within the specified limits?
				size_t p { 2 * (ip2p2 - ip2p1) };
				if ( pmin <= p && p <= pmax ) {
					// Update measurement
					m.ip2p1 = ip2p1;
					m.ip2p2 = ip2p2;

					m.ap2p1 = ap2p1;
					m.ap2p2 = ap2p2;

					m.iz2p = ap2p2 > ap2p1 ? ip2p2 : ip2p1;
					m.az2p = ap2p2 > ap2p1 ? ap2p2 : ap2p1;

					m.success = true;

					// Update max. peak-to-peak amplitude
					amax = ap2p1 + ap2p2;
				}
			}

			ip2p1 = ip2p2;
			ip2p2 = i;
			ap2p1 = ap2p2;
			ap2p2 = a;
		}

		// Keep track of amplitude in current half cycle
		if ( a > ap2p2 ) {
			ip2p2 = i;
			ap2p2 = a;
		}

		previous = current;
	}

	return m;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudePeriodMeasurement
measureAmplitudePeriod(
	const std::vector<double> &data,
	double offset,
	std::size_t istart,
	std::size_t iend,
	std::size_t pmin,
	std::size_t pmax)
{
	const double *f { &data[0] };
	std::size_t n { data.size() };

	return measureAmplitudePeriod(
			n, f, offset, istart, iend, pmin, pmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double wwssnspAmplitudeResponse(double frequency_hz) {
	using namespace Seiscomp::Math;
	static Filtering::IIR::WWSSN_SP_Filter<double> wwssnsp(Displacement);
	static double twopi { M_PI+M_PI };

	const std::complex<double> s(0, twopi * frequency_hz);
	std::complex<double> r { wwssnsp.norm };
	for ( const std::complex<double> &p : wwssnsp.poles ) {
		r = r / (s - p);
	}
	for ( const std::complex<double> &z : wwssnsp.zeros ) {
		r = r * (s - z);
	}
	return std::abs(r);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double wwssnlpAmplitudeResponse(double frequency_hz) {
	using namespace Seiscomp::Math;
	static Filtering::IIR::WWSSN_LP_Filter<double> wwssnlp(Displacement);
	static double twopi { M_PI+M_PI };

	const std::complex<double> s(0, twopi * frequency_hz);
	std::complex<double> r { wwssnlp.norm };
	for ( const std::complex<double> &p : wwssnlp.poles ) {
		r = r / (s - p);
	}
	for ( const std::complex<double> &z : wwssnlp.zeros ) {
		r = r * (s - z);
	}
	return std::abs(r);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace IASPEI

} // namespace Processing

} // namespace Seiscomp

