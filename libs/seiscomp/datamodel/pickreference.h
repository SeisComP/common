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


#ifndef SEISCOMP_DATAMODEL_PICKREFERENCE_H
#define SEISCOMP_DATAMODEL_PICKREFERENCE_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(PickReference);

class Reading;


class SC_SYSTEM_CORE_API PickReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PickReferenceIndex();
		PickReferenceIndex(const std::string& pickID);

		//! Copy constructor
		PickReferenceIndex(const PickReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const PickReferenceIndex&) const;
		bool operator!=(const PickReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string pickID;
};


class SC_SYSTEM_CORE_API PickReference : public Object {
	DECLARE_SC_CLASS(PickReference)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PickReference();

		//! Copy constructor
		PickReference(const PickReference& other);

		//! Custom constructor
		PickReference(const std::string& pickID);

		//! Destructor
		~PickReference() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PickReference& operator=(const PickReference& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const PickReference& other) const;
		bool operator!=(const PickReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const PickReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setPickID(const std::string& pickID);
		const std::string& pickID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const PickReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const PickReference* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Reading* reading() const;

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
		PickReferenceIndex _index;
};


}
}


#endif
