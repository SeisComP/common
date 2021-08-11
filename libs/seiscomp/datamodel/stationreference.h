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


#ifndef SEISCOMP_DATAMODEL_STATIONREFERENCE_H
#define SEISCOMP_DATAMODEL_STATIONREFERENCE_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(StationReference);

class StationGroup;


class SC_SYSTEM_CORE_API StationReferenceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationReferenceIndex();
		StationReferenceIndex(const std::string& stationID);

		//! Copy constructor
		StationReferenceIndex(const StationReferenceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const StationReferenceIndex&) const;
		bool operator!=(const StationReferenceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string stationID;
};


/**
 * \brief This type describes a station reference within a station
 * \brief group
 */
class SC_SYSTEM_CORE_API StationReference : public Object {
	DECLARE_SC_CLASS(StationReference)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationReference();

		//! Copy constructor
		StationReference(const StationReference& other);

		//! Custom constructor
		StationReference(const std::string& stationID);

		//! Destructor
		~StationReference() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		StationReference& operator=(const StationReference& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const StationReference& other) const;
		bool operator!=(const StationReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const StationReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Reference to network/station/@publicID
		void setStationID(const std::string& stationID);
		const std::string& stationID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const StationReferenceIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const StationReference* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		StationGroup* stationGroup() const;

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
		StationReferenceIndex _index;
};


}
}


#endif
