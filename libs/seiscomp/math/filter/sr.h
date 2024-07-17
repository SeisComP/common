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


#ifndef SEISCOMP_MATH_FILTER_SR_H
#define SEISCOMP_MATH_FILTER_SR_H

#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename T>
class SamplingRate : public InPlaceFilter<T> {
	public:
		SamplingRate() = default;

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		void apply(int n, T *inout) override;

		InPlaceFilter<T> *clone() const override;

	private:
		T _fsamp{0};
};


template<typename T>
class SamplingTime : public InPlaceFilter<T> {
	public:
		SamplingTime() = default;

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		void apply(int n, T *inout) override;

		InPlaceFilter<T> *clone() const override;

	private:
		T _dt{0};
};


}
}
}


#endif

