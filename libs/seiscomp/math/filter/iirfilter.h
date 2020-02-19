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


#ifndef _SEISCOMP_IIRFILT_H_
#define _SEISCOMP_IIRFILT_H_

#include<vector>
#include <seiscomp/math/filter.h>


namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


template <typename T>
class IIRFilter : public InPlaceFilter<T> {
	public:
		IIRFilter();
		IIRFilter(int na, int nb, const double *a, const double *b);
		~IIRFilter();


	public:
		void setCoefficients(int na, int nb, const double *a, const double *b);


	// InPlaceFilter interface
	public:
		void setSamplingFrequency(double fsamp);
		int setParameters(int n, const double *params);

		void apply(int n, T *inout);

		InPlaceFilter<T>* clone() const;


	private:
		int _na, _nb;
		std::vector<double> _a, _b;
		std::vector<double> _lastValues;
};


}
}
}

#endif
