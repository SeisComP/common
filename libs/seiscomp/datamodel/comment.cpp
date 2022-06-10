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
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/momenttensor.h>
#include <seiscomp/datamodel/focalmechanism.h>
#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/stationmagnitude.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/parameter.h>
#include <seiscomp/datamodel/parameterset.h>
#include <seiscomp/datamodel/stream.h>
#include <seiscomp/datamodel/sensorlocation.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/datamodel/network.h>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Comment, Object, "Comment");


Comment::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("text", "string", false, false, false, false, false, false, nullptr, &Comment::setText, &Comment::text));
	addProperty(Core::simpleProperty("id", "string", false, false, true, false, false, false, nullptr, &Comment::setId, &Comment::id));
	addProperty(Core::simpleProperty("start", "datetime", false, false, false, false, true, false, nullptr, &Comment::setStart, &Comment::start));
	addProperty(Core::simpleProperty("end", "datetime", false, false, false, false, true, false, nullptr, &Comment::setEnd, &Comment::end));
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &Comment::setCreationInfo, &Comment::creationInfo));
}


IMPLEMENT_METAOBJECT(Comment)


CommentIndex::CommentIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CommentIndex::CommentIndex(const std::string& id_) {
	id = id_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CommentIndex::CommentIndex(const CommentIndex& idx) {
	id = idx.id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CommentIndex::operator==(const CommentIndex& idx) const {
	return id == idx.id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CommentIndex::operator!=(const CommentIndex& idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment::Comment() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment::Comment(const Comment& other)
: Object() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment::~Comment() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Comment::operator==(const Comment& rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _text != rhs._text ) return false;
	if ( _start != rhs._start ) return false;
	if ( _end != rhs._end ) return false;
	if ( _creationInfo != rhs._creationInfo ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Comment::operator!=(const Comment& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Comment::equal(const Comment& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Comment::setText(const std::string& text) {
	_text = text;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Comment::text() const {
	return _text;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Comment::setId(const std::string& id) {
	_index.id = id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Comment::id() const {
	return _index.id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Comment::setStart(const OPT(Seiscomp::Core::Time)& start) {
	_start = start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time Comment::start() const {
	if ( _start )
		return *_start;
	throw Seiscomp::Core::ValueException("Comment.start is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Comment::setEnd(const OPT(Seiscomp::Core::Time)& end) {
	_end = end;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time Comment::end() const {
	if ( _end )
		return *_end;
	throw Seiscomp::Core::ValueException("Comment.end is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Comment::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& Comment::creationInfo() {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Comment.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& Comment::creationInfo() const {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("Comment.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CommentIndex& Comment::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Comment::equalIndex(const Comment* lhs) const {
	if ( lhs == nullptr ) return false;
	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor* Comment::momentTensor() const {
	return MomentTensor::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism* Comment::focalMechanism() const {
	return FocalMechanism::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Amplitude* Comment::amplitude() const {
	return Amplitude::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Magnitude* Comment::magnitude() const {
	return Magnitude::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationMagnitude* Comment::stationMagnitude() const {
	return StationMagnitude::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Pick* Comment::pick() const {
	return Pick::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Event* Comment::event() const {
	return Event::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin* Comment::origin() const {
	return Origin::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Parameter* Comment::parameter() const {
	return Parameter::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParameterSet* Comment::parameterSet() const {
	return ParameterSet::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Stream* Comment::stream() const {
	return Stream::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation* Comment::sensorLocation() const {
	return SensorLocation::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station* Comment::station() const {
	return Station::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network* Comment::network() const {
	return Network::Cast(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment& Comment::operator=(const Comment& other) {
	_index = other._index;
	_text = other._text;
	_start = other._start;
	_end = other._end;
	_creationInfo = other._creationInfo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Comment::assign(Object* other) {
	Comment* otherComment = Comment::Cast(other);
	if ( other == nullptr )
		return false;

	*this = *otherComment;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Comment::attachTo(PublicObject* parent) {
	if ( parent == nullptr ) return false;

	// check all possible parents
	MomentTensor* momentTensor = MomentTensor::Cast(parent);
	if ( momentTensor != nullptr )
		return momentTensor->add(this);
	FocalMechanism* focalMechanism = FocalMechanism::Cast(parent);
	if ( focalMechanism != nullptr )
		return focalMechanism->add(this);
	Amplitude* amplitude = Amplitude::Cast(parent);
	if ( amplitude != nullptr )
		return amplitude->add(this);
	Magnitude* magnitude = Magnitude::Cast(parent);
	if ( magnitude != nullptr )
		return magnitude->add(this);
	StationMagnitude* stationMagnitude = StationMagnitude::Cast(parent);
	if ( stationMagnitude != nullptr )
		return stationMagnitude->add(this);
	Pick* pick = Pick::Cast(parent);
	if ( pick != nullptr )
		return pick->add(this);
	Event* event = Event::Cast(parent);
	if ( event != nullptr )
		return event->add(this);
	Origin* origin = Origin::Cast(parent);
	if ( origin != nullptr )
		return origin->add(this);
	Parameter* parameter = Parameter::Cast(parent);
	if ( parameter != nullptr )
		return parameter->add(this);
	ParameterSet* parameterSet = ParameterSet::Cast(parent);
	if ( parameterSet != nullptr )
		return parameterSet->add(this);
	Stream* stream = Stream::Cast(parent);
	if ( stream != nullptr )
		return stream->add(this);
	SensorLocation* sensorLocation = SensorLocation::Cast(parent);
	if ( sensorLocation != nullptr )
		return sensorLocation->add(this);
	Station* station = Station::Cast(parent);
	if ( station != nullptr )
		return station->add(this);
	Network* network = Network::Cast(parent);
	if ( network != nullptr )
		return network->add(this);

	SEISCOMP_ERROR("Comment::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Comment::detachFrom(PublicObject* object) {
	if ( object == nullptr ) return false;

	// check all possible parents
	MomentTensor* momentTensor = MomentTensor::Cast(object);
	if ( momentTensor != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return momentTensor->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = momentTensor->comment(index());
			if ( child != nullptr )
				return momentTensor->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(MomentTensor): comment has not been found");
				return false;
			}
		}
	}
	FocalMechanism* focalMechanism = FocalMechanism::Cast(object);
	if ( focalMechanism != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return focalMechanism->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = focalMechanism->comment(index());
			if ( child != nullptr )
				return focalMechanism->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(FocalMechanism): comment has not been found");
				return false;
			}
		}
	}
	Amplitude* amplitude = Amplitude::Cast(object);
	if ( amplitude != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return amplitude->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = amplitude->comment(index());
			if ( child != nullptr )
				return amplitude->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Amplitude): comment has not been found");
				return false;
			}
		}
	}
	Magnitude* magnitude = Magnitude::Cast(object);
	if ( magnitude != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return magnitude->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = magnitude->comment(index());
			if ( child != nullptr )
				return magnitude->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Magnitude): comment has not been found");
				return false;
			}
		}
	}
	StationMagnitude* stationMagnitude = StationMagnitude::Cast(object);
	if ( stationMagnitude != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return stationMagnitude->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = stationMagnitude->comment(index());
			if ( child != nullptr )
				return stationMagnitude->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(StationMagnitude): comment has not been found");
				return false;
			}
		}
	}
	Pick* pick = Pick::Cast(object);
	if ( pick != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return pick->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = pick->comment(index());
			if ( child != nullptr )
				return pick->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Pick): comment has not been found");
				return false;
			}
		}
	}
	Event* event = Event::Cast(object);
	if ( event != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return event->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = event->comment(index());
			if ( child != nullptr )
				return event->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Event): comment has not been found");
				return false;
			}
		}
	}
	Origin* origin = Origin::Cast(object);
	if ( origin != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return origin->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = origin->comment(index());
			if ( child != nullptr )
				return origin->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Origin): comment has not been found");
				return false;
			}
		}
	}
	Parameter* parameter = Parameter::Cast(object);
	if ( parameter != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return parameter->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = parameter->comment(index());
			if ( child != nullptr )
				return parameter->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Parameter): comment has not been found");
				return false;
			}
		}
	}
	ParameterSet* parameterSet = ParameterSet::Cast(object);
	if ( parameterSet != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return parameterSet->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = parameterSet->comment(index());
			if ( child != nullptr )
				return parameterSet->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(ParameterSet): comment has not been found");
				return false;
			}
		}
	}
	Stream* stream = Stream::Cast(object);
	if ( stream != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return stream->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = stream->comment(index());
			if ( child != nullptr )
				return stream->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Stream): comment has not been found");
				return false;
			}
		}
	}
	SensorLocation* sensorLocation = SensorLocation::Cast(object);
	if ( sensorLocation != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return sensorLocation->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = sensorLocation->comment(index());
			if ( child != nullptr )
				return sensorLocation->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(SensorLocation): comment has not been found");
				return false;
			}
		}
	}
	Station* station = Station::Cast(object);
	if ( station != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return station->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = station->comment(index());
			if ( child != nullptr )
				return station->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Station): comment has not been found");
				return false;
			}
		}
	}
	Network* network = Network::Cast(object);
	if ( network != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return network->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Comment* child = network->comment(index());
			if ( child != nullptr )
				return network->remove(child);
			else {
				SEISCOMP_DEBUG("Comment::detachFrom(Network): comment has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Comment::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Comment::detach() {
	if ( parent() == nullptr )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Comment::clone() const {
	Comment* clonee = new Comment();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Comment::accept(Visitor* visitor) {
	visitor->visit(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Comment::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<Version::Major,Version::Minor>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Comment skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("text", _text, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("id", _index.id, Archive::XML_ELEMENT | Archive::INDEX_ATTRIBUTE);
	if ( ar.supportsVersion<0,10>() ) {
		ar & NAMED_OBJECT_HINT("start", _start, Archive::XML_ELEMENT | Archive::SPLIT_TIME);
	}
	if ( ar.supportsVersion<0,10>() ) {
		ar & NAMED_OBJECT_HINT("end", _end, Archive::XML_ELEMENT | Archive::SPLIT_TIME);
	}
	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
