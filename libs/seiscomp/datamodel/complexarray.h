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


#ifndef SEISCOMP_DATAMODEL_COMPLEXARRAY_H
#define SEISCOMP_DATAMODEL_COMPLEXARRAY_H


#include <complex>
#include <vector>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ComplexArray);


class SC_SYSTEM_CORE_API ComplexArray : public Core::BaseObject {
	DECLARE_SC_CLASS(ComplexArray)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ComplexArray();

		//! Copy constructor
		ComplexArray(const ComplexArray& other);

		//! Destructor
		~ComplexArray() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ComplexArray& operator=(const ComplexArray& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const ComplexArray& other) const;
		bool operator!=(const ComplexArray& other) const;

		//! Wrapper that calls operator==
		bool equal(const ComplexArray& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setContent(const std::vector< std::complex<double> >&);
		const std::vector< std::complex<double> >& content() const;
		std::vector< std::complex<double> >& content();


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::vector< std::complex<double> > _content;
};


}
}


#endif
