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


#ifndef SEISCOMP_MATH_FILTER_MINMAX_H
#define SEISCOMP_MATH_FILTER_MINMAX_H


#include <vector>
#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


/**
 * @brief The MinMax class is the base class for either the Min or Max
 *        filter. Each output sample holds the minimum/maximum of all prior
 *        samples within the configured time window.
 */
template<typename TYPE>
class MinMax : public InPlaceFilter<TYPE> {
	public:
		MinMax(double timeSpan /*sec*/ = 1.0, double fsamp = 0.0);


	public:
		/**
		 * @brief Sets the length of the minmax time window.
		 * @param timeSpan Length in seconds
		 */
		void setLength(double timeSpan);

		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		// Resets the filter values
		void reset();


	protected:
		double            _timeSpan;
		double            _fsamp;
		int               _sampleCount;
		int               _index;
		bool              _firstSample;
		std::vector<TYPE> _buffer;
};


template<typename TYPE>
class Min : public MinMax<TYPE> {
	public:
		Min(double timeSpan /*sec*/ = 1.0, double fsamp = 0.0);

	public:
		// apply filter to data vector **in*place**
		void apply(int n, TYPE *inout) override;
		InPlaceFilter<TYPE> *clone() const override;

	private:
		TYPE _minimum;
};


template<typename TYPE>
class Max : public MinMax<TYPE> {
	public:
		Max(double timeSpan /*sec*/ = 1.0, double fsamp = 0.0);

	public:
		// apply filter to data vector **in*place**
		void apply(int n, TYPE *inout) override;
		InPlaceFilter<TYPE> *clone() const override;

	private:
		TYPE _maximum;
};


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif

