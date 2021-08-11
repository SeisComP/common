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


#ifndef SEISCOMP_DATAMODEL_NODALPLANES_H
#define SEISCOMP_DATAMODEL_NODALPLANES_H


#include <seiscomp/datamodel/nodalplane.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(NodalPlanes);


/**
 * \brief This class describes the nodal planes of a double-couple
 * \brief moment-tensor solution. The attribute preferredPlane
 * \brief can be used to define which plane is the preferred one.
 */
class SC_SYSTEM_CORE_API NodalPlanes : public Core::BaseObject {
	DECLARE_SC_CLASS(NodalPlanes)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		NodalPlanes();

		//! Copy constructor
		NodalPlanes(const NodalPlanes& other);

		//! Destructor
		~NodalPlanes() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		NodalPlanes& operator=(const NodalPlanes& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const NodalPlanes& other) const;
		bool operator!=(const NodalPlanes& other) const;

		//! Wrapper that calls operator==
		bool equal(const NodalPlanes& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! First nodal plane of double-couple moment tensor solution.
		void setNodalPlane1(const OPT(NodalPlane)& nodalPlane1);
		NodalPlane& nodalPlane1();
		const NodalPlane& nodalPlane1() const;

		//! Second nodal plane of double-couple moment tensor solution.
		void setNodalPlane2(const OPT(NodalPlane)& nodalPlane2);
		NodalPlane& nodalPlane2();
		const NodalPlane& nodalPlane2() const;

		//! Indicator for preferred nodal plane of moment tensor
		//! solution. It can take integer values 1 or 2.
		void setPreferredPlane(const OPT(int)& preferredPlane);
		int preferredPlane() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(NodalPlane) _nodalPlane1;
		OPT(NodalPlane) _nodalPlane2;
		OPT(int) _preferredPlane;
};


}
}


#endif
