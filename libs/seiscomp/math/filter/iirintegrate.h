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


#ifndef SEISCOMP_IIRINTEGRATE_H
#define SEISCOMP_IIRINTEGRATE_H


#include <vector>
#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template <typename T>
class IIRIntegrate : public InPlaceFilter<T> {
	public:
		IIRIntegrate(double a = 0, double fsamp = 0);
		IIRIntegrate(const IIRIntegrate<T> &other);


	public:
		void reset();


	// InPlaceFilter interface
	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		void apply(int n, T *inout) override;

		InPlaceFilter<T> *clone() const override;

	private:
		void init(double a);

	private:
		double _ia0, _ia1, _ia2;

		double _a0, _a1, _a2;
		double _b0, _b1, _b2;
		T _v1, _v2;
};


}
}
}

#endif
