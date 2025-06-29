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


#ifndef SEISCOMP_DATAMODEL_EVENT_H
#define SEISCOMP_DATAMODEL_EVENT_H


#include <string>
#include <seiscomp/datamodel/types.h>
#include <seiscomp/datamodel/creationinfo.h>
#include <vector>
#include <seiscomp/datamodel/eventdescription.h>
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/originreference.h>
#include <seiscomp/datamodel/focalmechanismreference.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Event);
DEFINE_SMARTPOINTER(EventDescription);
DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(OriginReference);
DEFINE_SMARTPOINTER(FocalMechanismReference);

class Catalog;
class EventParameters;


/**
 * \brief The class Event describes a seismic event which does not
 * \brief necessarily need
 * \brief to be a tectonic earthquake. An event is usually associated
 * \brief with one or
 * \brief more origins, which contain information about focal time
 * \brief and geographical
 * \brief location of the event. Multiple origins can cover automatic
 * \brief and manual
 * \brief locations, a set of location from different agencies,
 * \brief locations generated
 * \brief with different location programs and earth models, etc.
 * \brief Furthermore, an event
 * \brief is usually associated with one or more magnitudes, and with
 * \brief one or more focal
 * \brief mechanism determinations.
 */
class SC_SYSTEM_CORE_API Event : public PublicObject {
	DECLARE_SC_CLASS(Event)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Event();

	public:
		//! Copy constructor
		Event(const Event &other);

		//! Constructor with publicID
		Event(const std::string& publicID);

		//! Destructor
		~Event() override;


	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Event *Create();
		static Event *Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Event *Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Event &operator=(const Event &other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Event &other) const;
		bool operator!=(const Event &other) const;

		//! Wrapper that calls operator==
		bool equal(const Event &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Refers to the publicID of the preferred Origin object.
		void setPreferredOriginID(const std::string& preferredOriginID);
		const std::string& preferredOriginID() const;

		//! Refers to the publicID of the preferred Magnitude object.
		void setPreferredMagnitudeID(const std::string& preferredMagnitudeID);
		const std::string& preferredMagnitudeID() const;

		//! Refers to the publicID of the preferred FocalMechanism
		//! object.
		void setPreferredFocalMechanismID(const std::string& preferredFocalMechanismID);
		const std::string& preferredFocalMechanismID() const;

		//! Describes the type of an event (Storchak et al. 2012).
		void setType(const OPT(EventType)& type);
		EventType type() const;

		//! Denotes how certain the information on event type is
		//! (Storchak et al. 2012).
		void setTypeCertainty(const OPT(EventTypeCertainty)& typeCertainty);
		EventTypeCertainty typeCertainty() const;

		//! CreationInfo for the Event object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(EventDescription *obj);
		bool add(Comment *obj);
		bool add(OriginReference *obj);
		bool add(FocalMechanismReference *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(EventDescription *obj);
		bool remove(Comment *obj);
		bool remove(OriginReference *obj);
		bool remove(FocalMechanismReference *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeEventDescription(size_t i);
		bool removeEventDescription(const EventDescriptionIndex &i);
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex &i);
		bool removeOriginReference(size_t i);
		bool removeOriginReference(const OriginReferenceIndex &i);
		bool removeFocalMechanismReference(size_t i);
		bool removeFocalMechanismReference(const FocalMechanismReferenceIndex &i);

		//! Retrieve the number of objects of a particular class
		size_t eventDescriptionCount() const;
		size_t commentCount() const;
		size_t originReferenceCount() const;
		size_t focalMechanismReferenceCount() const;

		//! Index access
		//! @return The object at index i
		EventDescription *eventDescription(size_t i) const;
		EventDescription *eventDescription(const EventDescriptionIndex &i) const;

		Comment *comment(size_t i) const;
		Comment *comment(const CommentIndex &i) const;

		OriginReference *originReference(size_t i) const;
		OriginReference *originReference(const OriginReferenceIndex &i) const;

		FocalMechanismReference *focalMechanismReference(size_t i) const;
		FocalMechanismReference *focalMechanismReference(const FocalMechanismReferenceIndex &i) const;

		//! Find an object by its unique attribute(s)

		/**
		 * The following methods return the parent object by type.
		 * Because different parent types are possible, just one
		 * of these methods will return a valid pointer at a time.
		 */
		Catalog *catalog() const;
		EventParameters *eventParameters() const;

		//! Implement Object interface
		bool assign(Object *other) override;
		bool attachTo(PublicObject *parent) override;
		bool detachFrom(PublicObject *parent) override;
		bool detach() override;

		//! Creates a clone
		Object *clone() const override;

		//! Implement PublicObject interface
		bool updateChild(Object *child) override;

		void accept(Visitor *visitor) override;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _preferredOriginID;
		std::string _preferredMagnitudeID;
		std::string _preferredFocalMechanismID;
		OPT(EventType) _type;
		OPT(EventTypeCertainty) _typeCertainty;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<EventDescriptionPtr> _eventDescriptions;
		std::vector<CommentPtr> _comments;
		std::vector<OriginReferencePtr> _originReferences;
		std::vector<FocalMechanismReferencePtr> _focalMechanismReferences;

	DECLARE_SC_CLASSFACTORY_FRIEND(Event);
};


}
}


#endif
