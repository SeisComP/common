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


#ifndef SEISCOMP_DATAMODEL_AMPLITUDEREFERENCE_H
#define SEISCOMP_DATAMODEL_AMPLITUDEREFERENCE_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(AmplitudeReference);

class Reading;


class SC_SYSTEM_CORE_API AmplitudeReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AmplitudeReferenceIndex();
		AmplitudeReferenceIndex(const std::string& amplitudeID);

		//! Copy constructor
		AmplitudeReferenceIndex(const AmplitudeReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const AmplitudeReferenceIndex&) const;
		bool operator!=(const AmplitudeReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string amplitudeID;
};


class SC_SYSTEM_CORE_API AmplitudeReference : public Object {
	DECLARE_SC_CLASS(AmplitudeReference)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AmplitudeReference();

		//! Copy constructor
		AmplitudeReference(const AmplitudeReference& other);

		//! Custom constructor
		AmplitudeReference(const std::string& amplitudeID);

		//! Destructor
		~AmplitudeReference() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		AmplitudeReference& operator=(const AmplitudeReference& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const AmplitudeReference& other) const;
		bool operator!=(const AmplitudeReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const AmplitudeReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setAmplitudeID(const std::string& amplitudeID);
		const std::string& amplitudeID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const AmplitudeReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const AmplitudeReference* lhs) const;

	
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
		AmplitudeReferenceIndex _index;
};


}
}


#endif
