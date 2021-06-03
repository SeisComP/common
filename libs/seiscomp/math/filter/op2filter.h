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


#ifndef SEISCOMP_MATH_OP2FILTER
#define SEISCOMP_MATH_OP2FILTER


#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE, template <class T> class OPERATION>
class Op2Filter : public InPlaceFilter<TYPE> {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Op2Filter(InPlaceFilter<TYPE> *op1, InPlaceFilter<TYPE> *op2, double fsamp=0.0);
		~Op2Filter();


	// ------------------------------------------------------------------
	//  Derived filter interface
	// ------------------------------------------------------------------
	public:
		void apply(int n, TYPE *inout) override;

		void setStartTime(const Core::Time &time) override;
		void setStreamID(const std::string &net,
		                         const std::string &sta,
		                         const std::string &loc,
		                         const std::string &cha) override;
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, double const *params) override;

		InPlaceFilter<TYPE> *clone() const override;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		InPlaceFilter<TYPE> *_op1;
		InPlaceFilter<TYPE> *_op2;
};


#include <seiscomp/math/filter/op2filter.ipp>


} // namespace Seiscomp::Math::Filter
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif
