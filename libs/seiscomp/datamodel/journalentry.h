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


#ifndef SEISCOMP_DATAMODEL_JOURNALENTRY_H
#define SEISCOMP_DATAMODEL_JOURNALENTRY_H


#include <seiscomp/core/datetime.h>
#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(JournalEntry);

class Journaling;


class SC_SYSTEM_CORE_API JournalEntry : public Object {
	DECLARE_SC_CLASS(JournalEntry)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		JournalEntry();

		//! Copy constructor
		JournalEntry(const JournalEntry& other);

		//! Destructor
		~JournalEntry() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		JournalEntry& operator=(const JournalEntry& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const JournalEntry& other) const;
		bool operator!=(const JournalEntry& other) const;

		//! Wrapper that calls operator==
		bool equal(const JournalEntry& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setCreated(const OPT(Seiscomp::Core::Time)& created);
		Seiscomp::Core::Time created() const;

		void setObjectID(const std::string& objectID);
		const std::string& objectID() const;

		void setSender(const std::string& sender);
		const std::string& sender() const;

		void setAction(const std::string& action);
		const std::string& action() const;

		void setParameters(const std::string& parameters);
		const std::string& parameters() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Journaling* journaling() const;

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
		// Attributes
		OPT(Seiscomp::Core::Time) _created;
		std::string _objectID;
		std::string _sender;
		std::string _action;
		std::string _parameters;
};


}
}


#endif
