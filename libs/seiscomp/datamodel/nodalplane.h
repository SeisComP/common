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


#ifndef SEISCOMP_DATAMODEL_NODALPLANE_H
#define SEISCOMP_DATAMODEL_NODALPLANE_H


#include <seiscomp/datamodel/realquantity.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(NodalPlane);


/**
 * \brief This class describes a nodal plane using the attributes
 * \brief strike, dip, and
 * \brief rake. For a definition of the angles see Aki and Richards
 * \brief (1980).
 */
class SC_SYSTEM_CORE_API NodalPlane : public Core::BaseObject {
	DECLARE_SC_CLASS(NodalPlane)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		NodalPlane();

		//! Copy constructor
		NodalPlane(const NodalPlane& other);

		//! Destructor
		~NodalPlane() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		NodalPlane& operator=(const NodalPlane& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const NodalPlane& other) const;
		bool operator!=(const NodalPlane& other) const;

		//! Wrapper that calls operator==
		bool equal(const NodalPlane& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Strike angle of nodal plane in degrees.
		void setStrike(const RealQuantity& strike);
		RealQuantity& strike();
		const RealQuantity& strike() const;

		//! Dip angle of nodal plane in degrees.
		void setDip(const RealQuantity& dip);
		RealQuantity& dip();
		const RealQuantity& dip() const;

		//! Rake angle of nodal plane in degrees.
		void setRake(const RealQuantity& rake);
		RealQuantity& rake();
		const RealQuantity& rake() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealQuantity _strike;
		RealQuantity _dip;
		RealQuantity _rake;
};


}
}


#endif
