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


#ifndef SEISCOMP_MATH_FILTER_RCA_H
#define SEISCOMP_MATH_FILTER_RCA_H

#include <vector>
#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
class Average : public InPlaceFilter<TYPE> {
	public:
		Average(double timeSpan /*sec*/ = 1.0, double fsamp = 0.0);

	public:
		void setLength(double timeSpan);

		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		// apply filter to data vector **in*place**
		void apply(int n, TYPE *inout) override;
		InPlaceFilter<TYPE> *clone() const override;

		// Resets the filter values
		void reset();

	private:
		double _timeSpan;
		double _fsamp;
		double _oocount;
		int _sampleCount;
		int _index;
		double _lastSum;
		bool _firstSample;
		std::vector<TYPE> _buffer;
};


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif

