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


#ifndef SEISCOMP_FILTERING_IIR_BUTTERWORTH_H
#define SEISCOMP_FILTERING_IIR_BUTTERWORTH_H


#include <seiscomp/math/filter/biquad.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {
namespace IIR {


template<class TYPE>
class ButterworthLowpass : public BiquadCascade<TYPE> {
	public:
		ButterworthLowpass(int order = 3, double fmax = 0.7, double fsamp=0);

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;

	private:
		int _order;
		double _fmax, _fsamp;
};


template<class TYPE>
class ButterworthHighpass : public BiquadCascade<TYPE> {
	public:
		ButterworthHighpass(int order = 3, double fmin = 2.0, double fsamp=0);

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;

	private:
		int _order;
		double _fmin, _fsamp;
};


template<class TYPE>
class ButterworthBandpass : public BiquadCascade<TYPE> {
	public:
		ButterworthBandpass(int order = 3, double fmin = 0.7, double fmax = 2.0,
		                    double fsamp=0);

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;


	protected:
		// configuration
		int _order;
		double _fmin, _fmax, _fsamp;
};


template<class TYPE>
class ButterworthBandstop : public BiquadCascade<TYPE> {
	public:
		ButterworthBandstop(int order = 3, double fmin = 0.7, double fmax = 2.0,
		                    double fsamp=0);

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;

	protected:
		// configuration
		int _order;
		double _fmin, _fmax, _fsamp;
};


template<class TYPE>
class ButterworthHighLowpass : public BiquadCascade<TYPE> {
	public:
		ButterworthHighLowpass(int order = 3, double fmin = 0.7,
		                       double fmax = 2.0, double fsamp = 0);

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;

	private:
		int _order;
		double _fmin, _fmax, _fsamp;
};


} // namespace Seiscomp::Math::Filtering::IIR
} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif
