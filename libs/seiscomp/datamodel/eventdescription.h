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


#ifndef SEISCOMP_DATAMODEL_EVENTDESCRIPTION_H
#define SEISCOMP_DATAMODEL_EVENTDESCRIPTION_H


#include <string>
#include <seiscomp/datamodel/types.h>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(EventDescription);

class Event;


class SC_SYSTEM_CORE_API EventDescriptionIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		EventDescriptionIndex();
		EventDescriptionIndex(EventDescriptionType type);

		//! Copy constructor
		EventDescriptionIndex(const EventDescriptionIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const EventDescriptionIndex&) const;
		bool operator!=(const EventDescriptionIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		EventDescriptionType type;
};


/**
 * \brief Free-form string with additional event description. This
 * \brief can be a
 * \brief well-known name, like 1906 San Francisco Earthquake. A
 * \brief number of
 * \brief categories can be given in type.
 */
class SC_SYSTEM_CORE_API EventDescription : public Object {
	DECLARE_SC_CLASS(EventDescription)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		EventDescription();

		//! Copy constructor
		EventDescription(const EventDescription& other);

		//! Custom constructor
		EventDescription(const std::string& text);
		EventDescription(const std::string& text,
		                 EventDescriptionType type);

		//! Destructor
		~EventDescription() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		EventDescription& operator=(const EventDescription& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const EventDescription& other) const;
		bool operator!=(const EventDescription& other) const;

		//! Wrapper that calls operator==
		bool equal(const EventDescription& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Free-form text with earthquake description.
		void setText(const std::string& text);
		const std::string& text() const;

		//! Category of earthquake description.
		void setType(EventDescriptionType type);
		EventDescriptionType type() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const EventDescriptionIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const EventDescription* lhs) const;

	
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
		EventDescriptionIndex _index;

		// Attributes
		std::string _text;
};


}
}


#endif
