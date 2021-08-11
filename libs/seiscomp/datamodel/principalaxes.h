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


#ifndef SEISCOMP_DATAMODEL_PRINCIPALAXES_H
#define SEISCOMP_DATAMODEL_PRINCIPALAXES_H


#include <seiscomp/datamodel/axis.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(PrincipalAxes);


/**
 * \brief This class describes the principal axes of a double-couple
 * \brief moment tensor solution. tAxis and pAxis are required,
 * \brief while nAxis is optional.
 */
class SC_SYSTEM_CORE_API PrincipalAxes : public Core::BaseObject {
	DECLARE_SC_CLASS(PrincipalAxes)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PrincipalAxes();

		//! Copy constructor
		PrincipalAxes(const PrincipalAxes& other);

		//! Destructor
		~PrincipalAxes() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PrincipalAxes& operator=(const PrincipalAxes& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const PrincipalAxes& other) const;
		bool operator!=(const PrincipalAxes& other) const;

		//! Wrapper that calls operator==
		bool equal(const PrincipalAxes& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! T (tension) axis of a double-couple moment tensor solution.
		void setTAxis(const Axis& tAxis);
		Axis& tAxis();
		const Axis& tAxis() const;

		//! P (pressure) axis of a double-couple moment tensor solution.
		void setPAxis(const Axis& pAxis);
		Axis& pAxis();
		const Axis& pAxis() const;

		//! N (neutral) axis of a double-couple moment tensor solution.
		void setNAxis(const OPT(Axis)& nAxis);
		Axis& nAxis();
		const Axis& nAxis() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		Axis _tAxis;
		Axis _pAxis;
		OPT(Axis) _nAxis;
};


}
}


#endif
