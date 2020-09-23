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


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/datamodel/reading.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/focalmechanism.h>
#include <seiscomp/datamodel/event.h>
#include <algorithm>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(EventParameters, PublicObject, "EventParameters");


EventParameters::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("pick", "Pick", &EventParameters::pickCount, &EventParameters::pick, static_cast<bool (EventParameters::*)(Pick*)>(&EventParameters::add), &EventParameters::removePick, static_cast<bool (EventParameters::*)(Pick*)>(&EventParameters::remove)));
	addProperty(arrayObjectProperty("amplitude", "Amplitude", &EventParameters::amplitudeCount, &EventParameters::amplitude, static_cast<bool (EventParameters::*)(Amplitude*)>(&EventParameters::add), &EventParameters::removeAmplitude, static_cast<bool (EventParameters::*)(Amplitude*)>(&EventParameters::remove)));
	addProperty(arrayObjectProperty("reading", "Reading", &EventParameters::readingCount, &EventParameters::reading, static_cast<bool (EventParameters::*)(Reading*)>(&EventParameters::add), &EventParameters::removeReading, static_cast<bool (EventParameters::*)(Reading*)>(&EventParameters::remove)));
	addProperty(arrayObjectProperty("origin", "Origin", &EventParameters::originCount, &EventParameters::origin, static_cast<bool (EventParameters::*)(Origin*)>(&EventParameters::add), &EventParameters::removeOrigin, static_cast<bool (EventParameters::*)(Origin*)>(&EventParameters::remove)));
	addProperty(arrayObjectProperty("focalMechanism", "FocalMechanism", &EventParameters::focalMechanismCount, &EventParameters::focalMechanism, static_cast<bool (EventParameters::*)(FocalMechanism*)>(&EventParameters::add), &EventParameters::removeFocalMechanism, static_cast<bool (EventParameters::*)(FocalMechanism*)>(&EventParameters::remove)));
	addProperty(arrayObjectProperty("event", "Event", &EventParameters::eventCount, &EventParameters::event, static_cast<bool (EventParameters::*)(Event*)>(&EventParameters::add), &EventParameters::removeEvent, static_cast<bool (EventParameters::*)(Event*)>(&EventParameters::remove)));
}


IMPLEMENT_METAOBJECT(EventParameters)


EventParameters::EventParameters(): PublicObject("EventParameters") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventParameters::EventParameters(const EventParameters& other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventParameters::~EventParameters() {
	std::for_each(_picks.begin(), _picks.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Pick::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&PickPtr::get)));
	std::for_each(_amplitudes.begin(), _amplitudes.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Amplitude::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&AmplitudePtr::get)));
	std::for_each(_readings.begin(), _readings.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Reading::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&ReadingPtr::get)));
	std::for_each(_origins.begin(), _origins.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Origin::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&OriginPtr::get)));
	std::for_each(_focalMechanisms.begin(), _focalMechanisms.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&FocalMechanism::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&FocalMechanismPtr::get)));
	std::for_each(_events.begin(), _events.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Event::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&EventPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::operator==(const EventParameters& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::operator!=(const EventParameters& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::equal(const EventParameters& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventParameters& EventParameters::operator=(const EventParameters& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::assign(Object* other) {
	EventParameters* otherEventParameters = EventParameters::Cast(other);
	if ( other == nullptr )
		return false;

	*this = *otherEventParameters;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* EventParameters::clone() const {
	EventParameters* clonee = new EventParameters();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::updateChild(Object* child) {
	Pick* pickChild = Pick::Cast(child);
	if ( pickChild != nullptr ) {
		Pick* pickElement
			= Pick::Cast(PublicObject::Find(pickChild->publicID()));
		if ( pickElement && pickElement->parent() == this ) {
			*pickElement = *pickChild;
			pickElement->update();
			return true;
		}
		return false;
	}

	Amplitude* amplitudeChild = Amplitude::Cast(child);
	if ( amplitudeChild != nullptr ) {
		Amplitude* amplitudeElement
			= Amplitude::Cast(PublicObject::Find(amplitudeChild->publicID()));
		if ( amplitudeElement && amplitudeElement->parent() == this ) {
			*amplitudeElement = *amplitudeChild;
			amplitudeElement->update();
			return true;
		}
		return false;
	}

	Reading* readingChild = Reading::Cast(child);
	if ( readingChild != nullptr ) {
		Reading* readingElement
			= Reading::Cast(PublicObject::Find(readingChild->publicID()));
		if ( readingElement && readingElement->parent() == this ) {
			*readingElement = *readingChild;
			readingElement->update();
			return true;
		}
		return false;
	}

	Origin* originChild = Origin::Cast(child);
	if ( originChild != nullptr ) {
		Origin* originElement
			= Origin::Cast(PublicObject::Find(originChild->publicID()));
		if ( originElement && originElement->parent() == this ) {
			*originElement = *originChild;
			originElement->update();
			return true;
		}
		return false;
	}

	FocalMechanism* focalMechanismChild = FocalMechanism::Cast(child);
	if ( focalMechanismChild != nullptr ) {
		FocalMechanism* focalMechanismElement
			= FocalMechanism::Cast(PublicObject::Find(focalMechanismChild->publicID()));
		if ( focalMechanismElement && focalMechanismElement->parent() == this ) {
			*focalMechanismElement = *focalMechanismChild;
			focalMechanismElement->update();
			return true;
		}
		return false;
	}

	Event* eventChild = Event::Cast(child);
	if ( eventChild != nullptr ) {
		Event* eventElement
			= Event::Cast(PublicObject::Find(eventChild->publicID()));
		if ( eventElement && eventElement->parent() == this ) {
			*eventElement = *eventChild;
			eventElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventParameters::accept(Visitor* visitor) {
	for ( auto &&elem : _picks )
		elem->accept(visitor);
	for ( auto &&elem : _amplitudes )
		elem->accept(visitor);
	for ( auto &&elem : _readings )
		elem->accept(visitor);
	for ( auto &&elem : _origins )
		elem->accept(visitor);
	for ( auto &&elem : _focalMechanisms )
		elem->accept(visitor);
	for ( auto &&elem : _events )
		elem->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t EventParameters::pickCount() const {
	return _picks.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Pick* EventParameters::pick(size_t i) const {
	return _picks[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Pick* EventParameters::findPick(const std::string& publicID) const {
	for ( std::vector<PickPtr>::const_iterator it = _picks.begin(); it != _picks.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::add(Pick* pick) {
	if ( pick == nullptr )
		return false;

	// Element has already a parent
	if ( pick->parent() != nullptr ) {
		SEISCOMP_ERROR("EventParameters::add(Pick*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Pick* pickCached = Pick::Find(pick->publicID());
		if ( pickCached ) {
			if ( pickCached->parent() ) {
				if ( pickCached->parent() == this )
					SEISCOMP_ERROR("EventParameters::add(Pick*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("EventParameters::add(Pick*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				pick = pickCached;
		}
	}

	// Add the element
	_picks.push_back(pick);
	pick->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		pick->accept(&nc);
	}

	// Notify registered observers
	childAdded(pick);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::remove(Pick* pick) {
	if ( pick == nullptr )
		return false;

	if ( pick->parent() != this ) {
		SEISCOMP_ERROR("EventParameters::remove(Pick*) -> element has another parent");
		return false;
	}

	auto it = std::find(_picks.begin(), _picks.end(), pick);
	// Element has not been found
	if ( it == _picks.end() ) {
		SEISCOMP_ERROR("EventParameters::remove(Pick*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_picks.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::removePick(size_t i) {
	// index out of bounds
	if ( i >= _picks.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_picks[i]->accept(&nc);
	}

	_picks[i]->setParent(nullptr);
	childRemoved(_picks[i].get());

	_picks.erase(_picks.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t EventParameters::amplitudeCount() const {
	return _amplitudes.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Amplitude* EventParameters::amplitude(size_t i) const {
	return _amplitudes[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Amplitude* EventParameters::findAmplitude(const std::string& publicID) const {
	for ( std::vector<AmplitudePtr>::const_iterator it = _amplitudes.begin(); it != _amplitudes.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::add(Amplitude* amplitude) {
	if ( amplitude == nullptr )
		return false;

	// Element has already a parent
	if ( amplitude->parent() != nullptr ) {
		SEISCOMP_ERROR("EventParameters::add(Amplitude*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Amplitude* amplitudeCached = Amplitude::Find(amplitude->publicID());
		if ( amplitudeCached ) {
			if ( amplitudeCached->parent() ) {
				if ( amplitudeCached->parent() == this )
					SEISCOMP_ERROR("EventParameters::add(Amplitude*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("EventParameters::add(Amplitude*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				amplitude = amplitudeCached;
		}
	}

	// Add the element
	_amplitudes.push_back(amplitude);
	amplitude->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		amplitude->accept(&nc);
	}

	// Notify registered observers
	childAdded(amplitude);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::remove(Amplitude* amplitude) {
	if ( amplitude == nullptr )
		return false;

	if ( amplitude->parent() != this ) {
		SEISCOMP_ERROR("EventParameters::remove(Amplitude*) -> element has another parent");
		return false;
	}

	auto it = std::find(_amplitudes.begin(), _amplitudes.end(), amplitude);
	// Element has not been found
	if ( it == _amplitudes.end() ) {
		SEISCOMP_ERROR("EventParameters::remove(Amplitude*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_amplitudes.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::removeAmplitude(size_t i) {
	// index out of bounds
	if ( i >= _amplitudes.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_amplitudes[i]->accept(&nc);
	}

	_amplitudes[i]->setParent(nullptr);
	childRemoved(_amplitudes[i].get());

	_amplitudes.erase(_amplitudes.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t EventParameters::readingCount() const {
	return _readings.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Reading* EventParameters::reading(size_t i) const {
	return _readings[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Reading* EventParameters::findReading(const std::string& publicID) const {
	for ( std::vector<ReadingPtr>::const_iterator it = _readings.begin(); it != _readings.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::add(Reading* reading) {
	if ( reading == nullptr )
		return false;

	// Element has already a parent
	if ( reading->parent() != nullptr ) {
		SEISCOMP_ERROR("EventParameters::add(Reading*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Reading* readingCached = Reading::Find(reading->publicID());
		if ( readingCached ) {
			if ( readingCached->parent() ) {
				if ( readingCached->parent() == this )
					SEISCOMP_ERROR("EventParameters::add(Reading*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("EventParameters::add(Reading*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				reading = readingCached;
		}
	}

	// Add the element
	_readings.push_back(reading);
	reading->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		reading->accept(&nc);
	}

	// Notify registered observers
	childAdded(reading);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::remove(Reading* reading) {
	if ( reading == nullptr )
		return false;

	if ( reading->parent() != this ) {
		SEISCOMP_ERROR("EventParameters::remove(Reading*) -> element has another parent");
		return false;
	}

	auto it = std::find(_readings.begin(), _readings.end(), reading);
	// Element has not been found
	if ( it == _readings.end() ) {
		SEISCOMP_ERROR("EventParameters::remove(Reading*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_readings.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::removeReading(size_t i) {
	// index out of bounds
	if ( i >= _readings.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_readings[i]->accept(&nc);
	}

	_readings[i]->setParent(nullptr);
	childRemoved(_readings[i].get());

	_readings.erase(_readings.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t EventParameters::originCount() const {
	return _origins.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* EventParameters::origin(size_t i) const {
	return _origins[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* EventParameters::findOrigin(const std::string& publicID) const {
	for ( std::vector<OriginPtr>::const_iterator it = _origins.begin(); it != _origins.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::add(Origin* origin) {
	if ( origin == nullptr )
		return false;

	// Element has already a parent
	if ( origin->parent() != nullptr ) {
		SEISCOMP_ERROR("EventParameters::add(Origin*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Origin* originCached = Origin::Find(origin->publicID());
		if ( originCached ) {
			if ( originCached->parent() ) {
				if ( originCached->parent() == this )
					SEISCOMP_ERROR("EventParameters::add(Origin*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("EventParameters::add(Origin*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				origin = originCached;
		}
	}

	// Add the element
	_origins.push_back(origin);
	origin->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		origin->accept(&nc);
	}

	// Notify registered observers
	childAdded(origin);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::remove(Origin* origin) {
	if ( origin == nullptr )
		return false;

	if ( origin->parent() != this ) {
		SEISCOMP_ERROR("EventParameters::remove(Origin*) -> element has another parent");
		return false;
	}

	auto it = std::find(_origins.begin(), _origins.end(), origin);
	// Element has not been found
	if ( it == _origins.end() ) {
		SEISCOMP_ERROR("EventParameters::remove(Origin*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_origins.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::removeOrigin(size_t i) {
	// index out of bounds
	if ( i >= _origins.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_origins[i]->accept(&nc);
	}

	_origins[i]->setParent(nullptr);
	childRemoved(_origins[i].get());

	_origins.erase(_origins.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t EventParameters::focalMechanismCount() const {
	return _focalMechanisms.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism* EventParameters::focalMechanism(size_t i) const {
	return _focalMechanisms[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism* EventParameters::findFocalMechanism(const std::string& publicID) const {
	for ( std::vector<FocalMechanismPtr>::const_iterator it = _focalMechanisms.begin(); it != _focalMechanisms.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::add(FocalMechanism* focalMechanism) {
	if ( focalMechanism == nullptr )
		return false;

	// Element has already a parent
	if ( focalMechanism->parent() != nullptr ) {
		SEISCOMP_ERROR("EventParameters::add(FocalMechanism*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		FocalMechanism* focalMechanismCached = FocalMechanism::Find(focalMechanism->publicID());
		if ( focalMechanismCached ) {
			if ( focalMechanismCached->parent() ) {
				if ( focalMechanismCached->parent() == this )
					SEISCOMP_ERROR("EventParameters::add(FocalMechanism*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("EventParameters::add(FocalMechanism*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				focalMechanism = focalMechanismCached;
		}
	}

	// Add the element
	_focalMechanisms.push_back(focalMechanism);
	focalMechanism->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		focalMechanism->accept(&nc);
	}

	// Notify registered observers
	childAdded(focalMechanism);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::remove(FocalMechanism* focalMechanism) {
	if ( focalMechanism == nullptr )
		return false;

	if ( focalMechanism->parent() != this ) {
		SEISCOMP_ERROR("EventParameters::remove(FocalMechanism*) -> element has another parent");
		return false;
	}

	auto it = std::find(_focalMechanisms.begin(), _focalMechanisms.end(), focalMechanism);
	// Element has not been found
	if ( it == _focalMechanisms.end() ) {
		SEISCOMP_ERROR("EventParameters::remove(FocalMechanism*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_focalMechanisms.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::removeFocalMechanism(size_t i) {
	// index out of bounds
	if ( i >= _focalMechanisms.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_focalMechanisms[i]->accept(&nc);
	}

	_focalMechanisms[i]->setParent(nullptr);
	childRemoved(_focalMechanisms[i].get());

	_focalMechanisms.erase(_focalMechanisms.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t EventParameters::eventCount() const {
	return _events.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* EventParameters::event(size_t i) const {
	return _events[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* EventParameters::findEvent(const std::string& publicID) const {
	for ( std::vector<EventPtr>::const_iterator it = _events.begin(); it != _events.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::add(Event* event) {
	if ( event == nullptr )
		return false;

	// Element has already a parent
	if ( event->parent() != nullptr ) {
		SEISCOMP_ERROR("EventParameters::add(Event*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Event* eventCached = Event::Find(event->publicID());
		if ( eventCached ) {
			if ( eventCached->parent() ) {
				if ( eventCached->parent() == this )
					SEISCOMP_ERROR("EventParameters::add(Event*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("EventParameters::add(Event*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				event = eventCached;
		}
	}

	// Add the element
	_events.push_back(event);
	event->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		event->accept(&nc);
	}

	// Notify registered observers
	childAdded(event);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::remove(Event* event) {
	if ( event == nullptr )
		return false;

	if ( event->parent() != this ) {
		SEISCOMP_ERROR("EventParameters::remove(Event*) -> element has another parent");
		return false;
	}

	auto it = std::find(_events.begin(), _events.end(), event);
	// Element has not been found
	if ( it == _events.end() ) {
		SEISCOMP_ERROR("EventParameters::remove(Event*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_events.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventParameters::removeEvent(size_t i) {
	// index out of bounds
	if ( i >= _events.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_events[i]->accept(&nc);
	}

	_events[i]->setParent(nullptr);
	childRemoved(_events[i].get());

	_events.erase(_events.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventParameters::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: EventParameters skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("pick",
		Seiscomp::Core::Generic::containerMember(
			_picks,
			[this](const PickPtr &pick) {
				return add(pick.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("amplitude",
		Seiscomp::Core::Generic::containerMember(
			_amplitudes,
			[this](const AmplitudePtr &amplitude) {
				return add(amplitude.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("reading",
		Seiscomp::Core::Generic::containerMember(
			_readings,
			[this](const ReadingPtr &reading) {
				return add(reading.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("origin",
		Seiscomp::Core::Generic::containerMember(
			_origins,
			[this](const OriginPtr &origin) {
				return add(origin.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("focalMechanism",
		Seiscomp::Core::Generic::containerMember(
			_focalMechanisms,
			[this](const FocalMechanismPtr &focalMechanism) {
				return add(focalMechanism.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("event",
		Seiscomp::Core::Generic::containerMember(
			_events,
			[this](const EventPtr &event) {
				return add(event.get());
			}
		),
		Archive::STATIC_TYPE
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
