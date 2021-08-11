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


#ifndef SEISCOMP_DATAMODEL_REALPDF1D_H
#define SEISCOMP_DATAMODEL_REALPDF1D_H


#include <seiscomp/datamodel/realarray.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(RealPDF1D);


/**
 * \brief A probability density function description. It can be used
 * \brief in three
 * \brief different modes:
 * \brief 
 * \brief 1) "raw samples mode"
 * \brief 
 * \brief variable is a list of M values, no probability. The values
 * \brief represent
 * \brief samples, no binning/probabilities made.
 * \brief 
 * \brief 2) "implicitly binned PDF"
 * \brief 
 * \brief variable and probabilty arrays have length N. variable
 * \brief values to be
 * \brief interpreted as "bin centers" (or representative values),
 * \brief no bin edges given.
 * \brief 
 * \brief 3) "explicitly binned PDF"
 * \brief 
 * \brief variable has length N+1, probability has length N. variable
 * \brief values
 * \brief describe bin edges (upper bin edge is lower edge of next
 * \brief bin).
 */
class SC_SYSTEM_CORE_API RealPDF1D : public Core::BaseObject {
	DECLARE_SC_CLASS(RealPDF1D)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RealPDF1D();

		//! Copy constructor
		RealPDF1D(const RealPDF1D& other);

		//! Destructor
		~RealPDF1D() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		RealPDF1D& operator=(const RealPDF1D& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const RealPDF1D& other) const;
		bool operator!=(const RealPDF1D& other) const;

		//! Wrapper that calls operator==
		bool equal(const RealPDF1D& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! List of values
		void setVariable(const RealArray& variable);
		RealArray& variable();
		const RealArray& variable() const;

		//! List of probabilities
		void setProbability(const RealArray& probability);
		RealArray& probability();
		const RealArray& probability() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealArray _variable;
		RealArray _probability;
};


}
}


#endif
