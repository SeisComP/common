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


#ifndef SEISCOMP_MATH_FILTERS_IIR_SEISMOMETERS_H
#define SEISCOMP_MATH_FILTERS_IIR_SEISMOMETERS_H


#include <complex>
#include <seiscomp/math/filter/seismometerresponse.h>
#include <seiscomp/math/filter/iir/biquad.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {

template <typename T>
class Filter : public Seiscomp::Math::Filtering::IIR::BiquadFilter<T> {
	public:
		Filter();

		Filter(const SeismometerResponse::Poles &poles,
		       const SeismometerResponse::Zeros &zeros,
		       double norm);

		Filter(const Filter &other) = default;

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		void apply(int n, T *inout) override;
	
		Math::Filtering::InPlaceFilter<T> *clone() const override;

	protected:
		// The poles and zeros that describe the seismometer response.
		SeismometerResponse::PolesAndZeros paz;
};


template <typename T>
class WWSSN_SP_Filter : public Filter<T> {
	public:
		WWSSN_SP_Filter(GroundMotion input=Velocity);
		WWSSN_SP_Filter(const WWSSN_SP_Filter &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input);
};


template <typename T>
class WWSSN_LP_Filter : public Filter<T> {
	public:
		WWSSN_LP_Filter(GroundMotion input=Velocity);
		WWSSN_LP_Filter(const WWSSN_LP_Filter &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input);
};


template <typename T>
class WoodAndersonFilter : public Filter<T> {
	public:
		WoodAndersonFilter(GroundMotion input=Velocity, SeismometerResponse::WoodAnderson::Config config = SeismometerResponse::WoodAnderson::Config());
		WoodAndersonFilter(const WoodAndersonFilter &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input,
		              SeismometerResponse::WoodAnderson::Config config = SeismometerResponse::WoodAnderson::Config());
};


template <typename T>
class GenericSeismometer : public Filter<T> {
	public:
		GenericSeismometer(double cornerPeriod=1.,
		                   GroundMotion input=Velocity);
		GenericSeismometer(const GenericSeismometer &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input);

	private:
		double _cornerPeriod;
};

template <typename T>
class Seismometer5secFilter : public Filter<T> {
	public:
		Seismometer5secFilter(GroundMotion input=Velocity);
		Seismometer5secFilter(const Seismometer5secFilter &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input);
};


}


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif
