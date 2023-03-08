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

#ifndef SEISCOMP_MATH_FILTER_IIR_RESTITUTION_H_INCLUDED
#define SEISCOMP_MATH_FILTER_IIR_RESTITUTION_H_INCLUDED

#include <seiscomp/math/filter/biquad.h>
#include <math.h>
#include <string>

namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {

// From the seismometer eigenperiod T0 and damping parameter h
// compute the coefficients c0, c1 and c2, which are the same
// as in Kanamori and Rivera (2008). Note that this also works
// for h > 1.
bool coefficients_from_T0_h(
	double fsamp, double gain, double T0, double h,
	double &c0, double &c1, double &c2);


// If h > 1, both poles are on the real axis rather than two complex
// conjugate poles. But as coefficients_from_T0_h() also works for that
// case, coefficients_from_T1_T2() is considered OBSOLETE.
bool coefficients_from_T1_T2(
	double fsamp, double gain, double T1, double T2,
	double &c0, double &c1, double &c2);


// Time-domain restitution filter
//
// This restitution requires the seismometer to be described by
//
// - corner period T0,
// - damping parameter h,
// - gain.
//
// For instance, an STS-2 seismometer has T0=120 and h=0.707.
//
// The algorithm follows Kanamori and Rivera (2008).
//
template<typename TYPE>
class RestitutionFilter : public BiquadCascade<TYPE> {

	public:
		RestitutionFilter(double T0=1, double h=1, double gain=1);
		~RestitutionFilter();

	public:
		// Specify a bandpass parameters to stabilize the restitution.
		//
		// An order of 4 is fine and that also seems to be what
		// Kanamori and Rivera (2008) have used.
		void setBandpass(int order, double fmin, double fmax);

		// Specify the sampling frequency of the data
		void setSamplingFrequency(double fsamp) override;

		// Set the coefficients as computed using either of the
		// two methods described above. Note that the coefficients
		// depend on the sampling frequency of the data so we need
		// to specify the same sampling frequency
		void setCoefficients(double c0, double c1, double c2);

		void setParameters(double T0, double h, double gain);

		// Needed for the filter language
		virtual int setParameters(int n, const double *params);

	public:
		virtual void reset() {}
		virtual std::string info() const;

		// Apply filter to data in place
		// virtual void apply(int n, TYPE *inout);

	protected:
		// To be called just before the data are processed.
		virtual void init();

	protected:
		// configuration
		double T0;
		double h;
		double gain;
		double _fsamp;

	private:
		double c0, c1, c2;

		// bandpass configuration
		int bp_order;
		double bp_fmin, bp_fmax;
};

} // namespace Seiscomp::Math::Filtering::IIR
} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif
