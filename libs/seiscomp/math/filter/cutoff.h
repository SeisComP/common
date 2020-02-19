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


#ifndef _SEISCOMP_MATH_FILTER_CUTOFF_H_
#define _SEISCOMP_MATH_FILTER_CUTOFF_H_

#include<vector>
#include<seiscomp/math/filter.h>


namespace Seiscomp
{
namespace Math
{
namespace Filtering
{

template<typename TYPE>
class CutOff : public InPlaceFilter<TYPE> {
	public:
		CutOff(TYPE threshold = 0);

	public:
		virtual void setSamplingFrequency(double fsamp);
		virtual int setParameters(int n, const double *params);

		// apply filter to data vector **in*place**
		virtual void apply(int n, TYPE *inout);
		virtual InPlaceFilter<TYPE>* clone() const;

	private:
		TYPE _threshold;
		TYPE _samples[2];
		int  _outstanding;
};


} // namespace Seiscomp::Math::Filtering

} // namespace Seiscomp::Math

} // namespace Seiscomp

#endif

