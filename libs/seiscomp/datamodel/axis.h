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


#ifndef SEISCOMP_DATAMODEL_AXIS_H
#define SEISCOMP_DATAMODEL_AXIS_H


#include <seiscomp/datamodel/realquantity.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Axis);


/**
 * \brief This class describes an eigenvector of a moment tensor
 * \brief expressed in its
 * \brief principal-axes system. It uses the angles azimuth, plunge,
 * \brief and the
 * \brief eigenvalue length.
 */
class SC_SYSTEM_CORE_API Axis : public Core::BaseObject {
	DECLARE_SC_CLASS(Axis)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Axis();

		//! Copy constructor
		Axis(const Axis& other);

		//! Destructor
		~Axis() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Axis& operator=(const Axis& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Axis& other) const;
		bool operator!=(const Axis& other) const;

		//! Wrapper that calls operator==
		bool equal(const Axis& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Azimuth of eigenvector of moment tensor expressed in
		//! principal-axes system. Measured clockwise
		//! from South-North direction at epicenter in degrees.
		void setAzimuth(const RealQuantity& azimuth);
		RealQuantity& azimuth();
		const RealQuantity& azimuth() const;

		//! Plunge of eigenvector of moment tensor expressed in
		//! principal-axes system. Measured against downward
		//! vertical direction at epicenter in degrees.
		void setPlunge(const RealQuantity& plunge);
		RealQuantity& plunge();
		const RealQuantity& plunge() const;

		//! Eigenvalue of moment tensor expressed in principal-axes
		//! system in Nm.
		void setLength(const RealQuantity& length);
		RealQuantity& length();
		const RealQuantity& length() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealQuantity _azimuth;
		RealQuantity _plunge;
		RealQuantity _length;
};


}
}


#endif
