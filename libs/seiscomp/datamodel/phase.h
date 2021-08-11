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


#ifndef SEISCOMP_DATAMODEL_PHASE_H
#define SEISCOMP_DATAMODEL_PHASE_H


#include <string>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Phase);


/**
 * \brief Generic and extensible phase description that currently
 * \brief contains the phase code only.
 */
class SC_SYSTEM_CORE_API Phase : public Core::BaseObject {
	DECLARE_SC_CLASS(Phase)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Phase();

		//! Copy constructor
		Phase(const Phase& other);

		//! Custom constructor
		Phase(const std::string& code);

		//! Destructor
		~Phase() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator std::string&();
		operator const std::string&() const;

		//! Copies the metadata of other to this
		Phase& operator=(const Phase& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Phase& other) const;
		bool operator!=(const Phase& other) const;

		//! Wrapper that calls operator==
		bool equal(const Phase& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Phase code as given in the IASPEI Standard Seismic Phase
		//! List
		//! (Storchak et al. 2003). String with a maximum length of 32
		//! characters.
		void setCode(const std::string& code);
		const std::string& code() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _code;
};


}
}


#endif
