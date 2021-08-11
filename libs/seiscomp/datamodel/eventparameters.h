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


#ifndef SEISCOMP_DATAMODEL_EVENTPARAMETERS_H
#define SEISCOMP_DATAMODEL_EVENTPARAMETERS_H


#include <vector>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(EventParameters);
DEFINE_SMARTPOINTER(Pick);
DEFINE_SMARTPOINTER(Amplitude);
DEFINE_SMARTPOINTER(Reading);
DEFINE_SMARTPOINTER(Origin);
DEFINE_SMARTPOINTER(FocalMechanism);
DEFINE_SMARTPOINTER(Event);


/**
 * \brief This type can hold objects of type Event, Origin,
 * \brief Magnitude, StationMagnitude,
 * \brief FocalMechanism, Reading, Amplitude, and Pick.
 */
class SC_SYSTEM_CORE_API EventParameters : public PublicObject {
	DECLARE_SC_CLASS(EventParameters)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		EventParameters();

		//! Copy constructor
		EventParameters(const EventParameters& other);

		//! Destructor
		~EventParameters() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		EventParameters& operator=(const EventParameters& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const EventParameters& other) const;
		bool operator!=(const EventParameters& other) const;

		//! Wrapper that calls operator==
		bool equal(const EventParameters& other) const;

	
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
		bool add(Pick* obj);
		bool add(Amplitude* obj);
		bool add(Reading* obj);
		bool add(Origin* obj);
		bool add(FocalMechanism* obj);
		bool add(Event* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Pick* obj);
		bool remove(Amplitude* obj);
		bool remove(Reading* obj);
		bool remove(Origin* obj);
		bool remove(FocalMechanism* obj);
		bool remove(Event* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removePick(size_t i);
		bool removeAmplitude(size_t i);
		bool removeReading(size_t i);
		bool removeOrigin(size_t i);
		bool removeFocalMechanism(size_t i);
		bool removeEvent(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t pickCount() const;
		size_t amplitudeCount() const;
		size_t readingCount() const;
		size_t originCount() const;
		size_t focalMechanismCount() const;
		size_t eventCount() const;

		//! Index access
		//! @return The object at index i
		Pick* pick(size_t i) const;
		Amplitude* amplitude(size_t i) const;
		Reading* reading(size_t i) const;
		Origin* origin(size_t i) const;
		FocalMechanism* focalMechanism(size_t i) const;
		Event* event(size_t i) const;

		//! Find an object by its unique attribute(s)
		Pick* findPick(const std::string& publicID) const;
		Amplitude* findAmplitude(const std::string& publicID) const;
		Reading* findReading(const std::string& publicID) const;
		Origin* findOrigin(const std::string& publicID) const;
		FocalMechanism* findFocalMechanism(const std::string& publicID) const;
		Event* findEvent(const std::string& publicID) const;

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
		// Aggregations
		std::vector<PickPtr> _picks;
		std::vector<AmplitudePtr> _amplitudes;
		std::vector<ReadingPtr> _readings;
		std::vector<OriginPtr> _origins;
		std::vector<FocalMechanismPtr> _focalMechanisms;
		std::vector<EventPtr> _events;

	DECLARE_SC_CLASSFACTORY_FRIEND(EventParameters);
};


}
}


#endif
