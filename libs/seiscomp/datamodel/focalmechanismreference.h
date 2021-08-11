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


#ifndef SEISCOMP_DATAMODEL_FOCALMECHANISMREFERENCE_H
#define SEISCOMP_DATAMODEL_FOCALMECHANISMREFERENCE_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(FocalMechanismReference);

class Event;


class SC_SYSTEM_CORE_API FocalMechanismReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FocalMechanismReferenceIndex();
		FocalMechanismReferenceIndex(const std::string& focalMechanismID);

		//! Copy constructor
		FocalMechanismReferenceIndex(const FocalMechanismReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const FocalMechanismReferenceIndex&) const;
		bool operator!=(const FocalMechanismReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string focalMechanismID;
};


class SC_SYSTEM_CORE_API FocalMechanismReference : public Object {
	DECLARE_SC_CLASS(FocalMechanismReference)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FocalMechanismReference();

		//! Copy constructor
		FocalMechanismReference(const FocalMechanismReference& other);

		//! Custom constructor
		FocalMechanismReference(const std::string& focalMechanismID);

		//! Destructor
		~FocalMechanismReference() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		FocalMechanismReference& operator=(const FocalMechanismReference& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const FocalMechanismReference& other) const;
		bool operator!=(const FocalMechanismReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const FocalMechanismReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setFocalMechanismID(const std::string& focalMechanismID);
		const std::string& focalMechanismID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const FocalMechanismReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const FocalMechanismReference* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Event* event() const;

		//! Implement Object interface
		bool assign(Object *other) override;
		bool attachTo(PublicObject *parent) override;
		bool detachFrom(PublicObject *parent) override;
		bool detach() override;

		//! Creates a clone
		Object *clone() const override;

		void accept(Visitor *visitor) override;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		FocalMechanismReferenceIndex _index;
};


}
}


#endif
