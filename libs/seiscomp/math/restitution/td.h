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

#ifndef SEISCOMP_MATH_RESTITUTION_TD_H
#define SEISCOMP_MATH_RESTITUTION_TD_H

#include <math.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/datamodel/waveformstreamid.h>
#include <seiscomp/datamodel/responsepaz.h>
#include <seiscomp/math/filter.h>
#include <seiscomp/math/filter/butterworth.h>

namespace Seiscomp {
namespace Math {
namespace Restitution {

// subroutines to compute parameters for the recursive filter

// Form 1: from seismometer eigenperiod T0 and damping parameter h
bool coefficients_from_T0_h(double fsamp, double gain, double T0, double h, double *c0, double *c1, double *c2);

// Form 2: from the two seismometer eigenperiods T1 and T2
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


// This is a filter that does not alter the data except correcting the
// gain and optionally apply a bandpass filter.
// 
// The intended use case is to to simply pass on raw displacement to the
// processing instead of applying one of the above restitution filters.
template<typename TYPE>
class TimeDomainNullFilter: public TimeDomain<TYPE> {
	public:
		TimeDomainNullFilter(double gain, double fsamp=0);

		void setBandpass(int order, double fmin, double fmax);
		virtual std::string print() const;

		Filtering::InPlaceFilter<TYPE>* clone() const;

		void apply(int n, TYPE *inout);

	protected:
		virtual void init();
};



// This is a filter that holds a reference to one of the above. In other
// words, it is a wrapper filter that is initialized later.
//
// The typical use case for this is when we want to instantiate a filter
// before we know the stream. E.g. in a filter chain, where we know the
// sampling frequency, time and stream ID only when the data arrive.

template<typename TYPE>
class TimeDomainGeneric : public Filtering::InPlaceFilter<TYPE> {
	public:
		TimeDomainGeneric();
		TimeDomainGeneric(const TimeDomainGeneric &other);
		~TimeDomainGeneric();

		// InPlaceFilter interface, to be used *strictly* in
		// the following order:
		//
		// 1. setStartTime
		// 2. setStreamID
		// 3. setSamplingFrequency
		virtual void setStartTime(const Core::Time &time);

		virtual void setStreamID(
			const std::string &net,
			const std::string &sta,
			const std::string &loc,
			const std::string &cha);

		virtual void setSamplingFrequency(double fsamp);

		void setBandpass(int order, double fmin, double fmax);
		virtual std::string print() const;

		Filtering::InPlaceFilter<TYPE>* clone() const;

		void apply(int n, TYPE *inout);

		virtual int setParameters(int n, const double *params) {
			return 0;
		}
	protected:
		virtual void init();

	private:
		// This is the actual filter
		TimeDomain<TYPE> *filter;
	
		// Ensure that filter has been initialized.
		// If not the case, an error is generated and no
		// filtering can take place.
		bool initialized() const;

		Core::Time time;
		std::string net, sta, loc, cha;
		double fsamp;
};


const Seiscomp::DataModel::ResponsePAZ*
findResponsePAZ(
	const Seiscomp::DataModel::WaveformStreamID &wfid,
	const Seiscomp::Core::Time &time);

} // namespace Seiscomp::Math::Restitution
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif
