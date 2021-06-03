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


#ifndef SEISCOMP_MATH_CHAINFILTER
#define SEISCOMP_MATH_CHAINFILTER


#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
class ChainFilter : public InPlaceFilter<TYPE> {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		ChainFilter();
		~ChainFilter();


	// ------------------------------------------------------------------
	//  Interface
	// ------------------------------------------------------------------
	public:
		//! Adds a filter to the chain. The ownership of the filter to
		//! be added goes the ChainFilter instance which will delete
		//! the filter.
		bool add(InPlaceFilter<TYPE> *filter);

		//! Removes and deletes the filter at a certain position in
		//! the chain.
		bool remove(size_t pos);

		//! Removes the filter at position pos and returns it. The
		//! filter instance will not be deleted and the ownership goes
		//! to the caller.
		InPlaceFilter<TYPE>* take(size_t pos);

		//! Returns the index of a filter in the chain or -1 if not
		//! found
		size_t indexOf(InPlaceFilter<TYPE> *filter) const;

		//! Returns the number of filters in the chain
		size_t filterCount() const;


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
		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		typedef std::vector<InPlaceFilter<TYPE>*> FilterChain;
		FilterChain _filters;
};


} // namespace Seiscomp::Math::Filter
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif
