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

#ifndef SEIS_SIGNAL_TDRESTITUTION_H
#define SEIS_SIGNAL_TDRESTITUTION_H

#include <math.h>
#include <seiscomp/math/filter.h>
#include <seiscomp/math/filter/butterworth.h>

namespace Seiscomp {
namespace Math {
namespace Restitution {

// subroutines to compute parameters for the recursive filter

// from seismometer eigenperiod T0 and damping parameter h bool 
bool coefficients_from_T0_h(double fsamp, double gain, double T0, double h, double *c0, double *c1, double *c2);

// from the two seismometer eigenperiods T1 and T2
bool coefficients_from_T1_T2(double fsamp, double gain, double T1, double T2, double *c0, double *c1, double *c2);

template<typename TYPE>
class TimeDomain : public Filtering::InPlaceFilter<TYPE> {
	public:
		TimeDomain();
		~TimeDomain();

		// configuration
		void setBandpass(int order, double fmin, double fmax);
		void setSamplingFrequency(double fsamp);
		void setCoefficients(double c0, double c1, double c2);

		virtual int setParameters(int n, const double *params);

		virtual void reset() {}
		virtual void apply(int n, TYPE *inout);
		virtual std::string print() const;

	protected:
		virtual void init();

	protected:
		// configuration
		double c0, c1, c2;
		double fsamp, dt;
		double gain;

	private:
		// filter configuration
		int order;
		double fmin, fmax;
		// temp variables
		double y0, y1, y2, a1, a2;
		double cumsum1, cumsum2;

		Filtering::IIR::ButterworthHighLowpass<TYPE> *bandpass;
};

template<typename TYPE>
class TimeDomain_from_T0_h: public TimeDomain<TYPE> {
	public:
		TimeDomain_from_T0_h(double T0, double h, double gain, double fsamp=0);

		void setBandpass(int order, double fmin, double fmax);
		virtual std::string print() const;

		Filtering::InPlaceFilter<TYPE>* clone() const;

	protected:
		virtual void init();
	
	private:
		// configuration
		double T0, h;
};

template<typename TYPE>
class TimeDomain_from_T1_T2: public TimeDomain<TYPE> {
	public:
		TimeDomain_from_T1_T2(double T1, double T2, double gain, double fsamp=0);

		void setBandpass(int order, double fmin, double fmax);
		virtual std::string print() const;

		Filtering::InPlaceFilter<TYPE>* clone() const;

	protected:
		virtual void init();

	private:
		// configuration
		double T1, T2;
};


} // namespace Seiscomp::Math::Restitution
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif
