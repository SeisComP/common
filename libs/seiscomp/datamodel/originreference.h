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


#ifndef SEISCOMP_DATAMODEL_ORIGINREFERENCE_H
#define SEISCOMP_DATAMODEL_ORIGINREFERENCE_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(OriginReference);

class Event;


class SC_SYSTEM_CORE_API OriginReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		OriginReferenceIndex();
		OriginReferenceIndex(const std::string& originID);

		//! Copy constructor
		OriginReferenceIndex(const OriginReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const OriginReferenceIndex&) const;
		bool operator!=(const OriginReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string originID;
};


class SC_SYSTEM_CORE_API OriginReference : public Object {
	DECLARE_SC_CLASS(OriginReference)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		OriginReference();

		//! Copy constructor
		OriginReference(const OriginReference& other);

		//! Custom constructor
		OriginReference(const std::string& originID);

		//! Destructor
		~OriginReference() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		OriginReference& operator=(const OriginReference& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const OriginReference& other) const;
		bool operator!=(const OriginReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const OriginReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setOriginID(const std::string& originID);
		const std::string& originID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const OriginReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const OriginReference* lhs) const;

	
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
		OriginReferenceIndex _index;
};


}
}


#endif
