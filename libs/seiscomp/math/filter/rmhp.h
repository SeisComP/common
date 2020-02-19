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


#ifndef _SEISCOMP_FILTERING_RMHP_H_
#define _SEISCOMP_FILTERING_RMHP_H_

#include<vector>

#include<seiscomp/math/filter.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
class RunningMean : public InPlaceFilter<TYPE> {
	public:
		RunningMean(double windowLength=0, double fsamp=0.0);
		~RunningMean() {}


	public:
		void setLength(double windowLength) {
			_windowLength = windowLength;
		}

		// apply filter to data vector **in*place**
		void apply(int n, TYPE *inout);
		virtual InPlaceFilter<TYPE>* clone() const;

		// resets the filter, i.e. erases the filter memory
		void reset() {}

		virtual void setSamplingFrequency(double fsamp) {
			_samplingFrequency = fsamp;
			_windowLengthI = int(_windowLength * _samplingFrequency);
		}

		virtual int setParameters(int n, const double *params);


	protected:
		double _windowLength,  _samplingFrequency;
		int    _windowLengthI, _sampleCount;
		double _average;
};


template<typename TYPE>
class RunningMeanHighPass : public RunningMean<TYPE> {
	public:
		RunningMeanHighPass(double windowLength=0, double fsamp=0.0);
		~RunningMeanHighPass() {}


	public:
		// apply filter to data vector **in*place**
		void apply(int n, TYPE *inout);
		virtual InPlaceFilter<TYPE>* clone() const;
};


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif
