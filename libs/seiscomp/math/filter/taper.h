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


#ifndef SEISCOMP_FILTERING_TAPER_H
#define SEISCOMP_FILTERING_TAPER_H


#include <vector>
#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
class InitialTaper : public InPlaceFilter<TYPE> {
	public:
		InitialTaper(double taperLength=0, TYPE offset=0, double fsamp=0);
		~InitialTaper() {}

		void setLength(double taperLength, TYPE offset=0) {
			_taperLength = taperLength;
			_offset = offset;
		}

		// apply filter to data vector **in*place**
		void apply(int n, TYPE *inout) override;

		InPlaceFilter<TYPE> *clone() const override;

		void setSamplingFrequency(double fsamp) override;

		int setParameters(int n, const double *params) override;

		// resets the filter, i.e. erases the filter memory
		void reset() { _sampleCount = 0; }


	private:
		double _taperLength,  _samplingFrequency;
		int    _taperLengthI, _sampleCount;
		TYPE   _offset;
};


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif
