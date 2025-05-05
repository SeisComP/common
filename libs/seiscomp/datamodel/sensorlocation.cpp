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
#include <seiscomp/datamodel/sensorlocation.h>
#include <seiscomp/datamodel/station.h>
#include <algorithm>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(SensorLocation, PublicObject, "SensorLocation");


SensorLocation::MetaObject::MetaObject(const Core::RTTI *rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("code", "string", false, false, true, false, false, false, nullptr, &SensorLocation::setCode, &SensorLocation::code));
	addProperty(Core::simpleProperty("start", "datetime", false, false, true, false, false, false, nullptr, &SensorLocation::setStart, &SensorLocation::start));
	addProperty(Core::simpleProperty("end", "datetime", false, false, false, false, true, false, nullptr, &SensorLocation::setEnd, &SensorLocation::end));
	addProperty(Core::simpleProperty("latitude", "float", false, false, false, false, true, false, nullptr, &SensorLocation::setLatitude, &SensorLocation::latitude));
	addProperty(Core::simpleProperty("longitude", "float", false, false, false, false, true, false, nullptr, &SensorLocation::setLongitude, &SensorLocation::longitude));
	addProperty(Core::simpleProperty("elevation", "float", false, false, false, false, true, false, nullptr, &SensorLocation::setElevation, &SensorLocation::elevation));
	addProperty(arrayClassProperty<Comment>("comment", "Comment", &SensorLocation::commentCount, &SensorLocation::comment, static_cast<bool (SensorLocation::*)(Comment*)>(&SensorLocation::add), &SensorLocation::removeComment, static_cast<bool (SensorLocation::*)(Comment*)>(&SensorLocation::remove)));
	addProperty(arrayClassProperty<AuxStream>("auxStream", "AuxStream", &SensorLocation::auxStreamCount, &SensorLocation::auxStream, static_cast<bool (SensorLocation::*)(AuxStream*)>(&SensorLocation::add), &SensorLocation::removeAuxStream, static_cast<bool (SensorLocation::*)(AuxStream*)>(&SensorLocation::remove)));
	addProperty(arrayObjectProperty("stream", "Stream", &SensorLocation::streamCount, &SensorLocation::stream, static_cast<bool (SensorLocation::*)(Stream*)>(&SensorLocation::add), &SensorLocation::removeStream, static_cast<bool (SensorLocation::*)(Stream*)>(&SensorLocation::remove)));
}


IMPLEMENT_METAOBJECT(SensorLocation)


SensorLocationIndex::SensorLocationIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocationIndex::SensorLocationIndex(const std::string& code_,
                                         Seiscomp::Core::Time start_) {
	code = code_;
	start = start_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocationIndex::SensorLocationIndex(const SensorLocationIndex &idx) {
	code = idx.code;
	start = idx.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocationIndex::operator==(const SensorLocationIndex &idx) const {
	return code == idx.code &&
	       start == idx.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocationIndex::operator!=(const SensorLocationIndex &idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation::SensorLocation() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation::SensorLocation(const SensorLocation &other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation::SensorLocation(const std::string& publicID)
: PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation::~SensorLocation() {
	for ( auto &comment : _comments ) {
		comment->setParent(nullptr);
	}
	for ( auto &auxStream : _auxStreams ) {
		auxStream->setParent(nullptr);
	}
	for ( auto &stream : _streams ) {
		stream->setParent(nullptr);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation *SensorLocation::Create() {
	SensorLocation *object = new SensorLocation();
	return static_cast<SensorLocation*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation *SensorLocation::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != nullptr ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return nullptr;
	}

	return new SensorLocation(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation *SensorLocation::Find(const std::string& publicID) {
	return SensorLocation::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::operator==(const SensorLocation &rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _end != rhs._end ) return false;
	if ( _latitude != rhs._latitude ) return false;
	if ( _longitude != rhs._longitude ) return false;
	if ( _elevation != rhs._elevation ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::operator!=(const SensorLocation &rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::equal(const SensorLocation &other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SensorLocation::setCode(const std::string& code) {
	_index.code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& SensorLocation::code() const {
	return _index.code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SensorLocation::setStart(Seiscomp::Core::Time start) {
	_index.start = start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time SensorLocation::start() const {
	return _index.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SensorLocation::setEnd(const OPT(Seiscomp::Core::Time)& end) {
	_end = end;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time SensorLocation::end() const {
	if ( _end )
		return *_end;
	throw Seiscomp::Core::ValueException("SensorLocation.end is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SensorLocation::setLatitude(const OPT(double)& latitude) {
	_latitude = latitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double SensorLocation::latitude() const {
	if ( _latitude )
		return *_latitude;
	throw Seiscomp::Core::ValueException("SensorLocation.latitude is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SensorLocation::setLongitude(const OPT(double)& longitude) {
	_longitude = longitude;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double SensorLocation::longitude() const {
	if ( _longitude )
		return *_longitude;
	throw Seiscomp::Core::ValueException("SensorLocation.longitude is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SensorLocation::setElevation(const OPT(double)& elevation) {
	_elevation = elevation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double SensorLocation::elevation() const {
	if ( _elevation )
		return *_elevation;
	throw Seiscomp::Core::ValueException("SensorLocation.elevation is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SensorLocationIndex &SensorLocation::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::equalIndex(const SensorLocation *lhs) const {
	if ( !lhs ) {
		return false;
	}

	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station *SensorLocation::station() const {
	return static_cast<Station*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SensorLocation &SensorLocation::operator=(const SensorLocation &other) {
	PublicObject::operator=(other);
	_index = other._index;
	_end = other._end;
	_latitude = other._latitude;
	_longitude = other._longitude;
	_elevation = other._elevation;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::assign(Object *other) {
	SensorLocation *otherSensorLocation = SensorLocation::Cast(other);
	if ( !other ) {
		return false;
	}

	*this = *otherSensorLocation;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::attachTo(PublicObject *parent) {
	if ( !parent ) {
		return false;
	}

	// check all possible parents
	Station *station = Station::Cast(parent);
	if ( station != nullptr )
		return station->add(this);

	SEISCOMP_ERROR("SensorLocation::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::detachFrom(PublicObject *object) {
	if ( !object ) {
		return false;
	}

	// check all possible parents
	Station *station = Station::Cast(object);
	if ( station != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return station->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			SensorLocation *child = station->findSensorLocation(publicID());
			if ( child != nullptr )
				return station->remove(child);
			else {
				SEISCOMP_DEBUG("SensorLocation::detachFrom(Station): sensorLocation has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("SensorLocation::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::detach() {
	if ( !parent() ) {
		return false;
	}

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object *SensorLocation::clone() const {
	SensorLocation *clonee = new SensorLocation();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::updateChild(Object *child) {
	Comment *commentChild = Comment::Cast(child);
	if ( commentChild != nullptr ) {
		Comment *commentElement = comment(commentChild->index());
		if ( commentElement != nullptr ) {
			*commentElement = *commentChild;
			commentElement->update();
			return true;
		}
		return false;
	}

	AuxStream *auxStreamChild = AuxStream::Cast(child);
	if ( auxStreamChild != nullptr ) {
		AuxStream *auxStreamElement = auxStream(auxStreamChild->index());
		if ( auxStreamElement != nullptr ) {
			*auxStreamElement = *auxStreamChild;
			auxStreamElement->update();
			return true;
		}
		return false;
	}

	Stream *streamChild = Stream::Cast(child);
	if ( streamChild != nullptr ) {
		Stream *streamElement
			= Stream::Cast(PublicObject::Find(streamChild->publicID()));
		if ( streamElement && streamElement->parent() == this ) {
			*streamElement = *streamChild;
			streamElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SensorLocation::accept(Visitor *visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( auto &&elem : _comments )
		elem->accept(visitor);
	for ( auto &&elem : _auxStreams )
		elem->accept(visitor);
	for ( auto &&elem : _streams )
		elem->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t SensorLocation::commentCount() const {
	return _comments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment *SensorLocation::comment(size_t i) const {
	return _comments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment *SensorLocation::comment(const CommentIndex &i) const {
	for ( const auto &elem : _comments ) {
		if ( i == elem->index() ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::add(Comment *comment) {
	if ( !comment ) {
		return false;
	}

	// Element has already a parent
	if ( comment->parent() != nullptr ) {
		SEISCOMP_ERROR("SensorLocation::add(Comment*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( auto &&elem : _comments ) {
		if ( elem->index() == comment->index() ) {
			SEISCOMP_ERROR("SensorLocation::add(Comment*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_comments.push_back(comment);
	comment->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		comment->accept(&nc);
	}

	// Notify registered observers
	childAdded(comment);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::remove(Comment *comment) {
	if ( !comment ) {
		return false;
	}

	if ( comment->parent() != this ) {
		SEISCOMP_ERROR("SensorLocation::remove(Comment*) -> element has another parent");
		return false;
	}

	auto it = std::find(_comments.begin(), _comments.end(), comment);
	// Element has not been found
	if ( it == _comments.end() ) {
		SEISCOMP_ERROR("SensorLocation::remove(Comment*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, it->get());
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_comments.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::removeComment(size_t i) {
	// index out of bounds
	if ( i >= _comments.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, _comments[i].get());
	}

	_comments[i]->setParent(nullptr);
	childRemoved(_comments[i].get());

	_comments.erase(_comments.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::removeComment(const CommentIndex &i) {
	Comment *object = comment(i);
	if ( !object ) {
		return false;
	}

	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t SensorLocation::auxStreamCount() const {
	return _auxStreams.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxStream *SensorLocation::auxStream(size_t i) const {
	return _auxStreams[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AuxStream *SensorLocation::auxStream(const AuxStreamIndex &i) const {
	for ( const auto &elem : _auxStreams ) {
		if ( i == elem->index() ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::add(AuxStream *auxStream) {
	if ( !auxStream ) {
		return false;
	}

	// Element has already a parent
	if ( auxStream->parent() != nullptr ) {
		SEISCOMP_ERROR("SensorLocation::add(AuxStream*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( auto &&elem : _auxStreams ) {
		if ( elem->index() == auxStream->index() ) {
			SEISCOMP_ERROR("SensorLocation::add(AuxStream*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_auxStreams.push_back(auxStream);
	auxStream->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		auxStream->accept(&nc);
	}

	// Notify registered observers
	childAdded(auxStream);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::remove(AuxStream *auxStream) {
	if ( !auxStream ) {
		return false;
	}

	if ( auxStream->parent() != this ) {
		SEISCOMP_ERROR("SensorLocation::remove(AuxStream*) -> element has another parent");
		return false;
	}

	auto it = std::find(_auxStreams.begin(), _auxStreams.end(), auxStream);
	// Element has not been found
	if ( it == _auxStreams.end() ) {
		SEISCOMP_ERROR("SensorLocation::remove(AuxStream*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, it->get());
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_auxStreams.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::removeAuxStream(size_t i) {
	// index out of bounds
	if ( i >= _auxStreams.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, _auxStreams[i].get());
	}

	_auxStreams[i]->setParent(nullptr);
	childRemoved(_auxStreams[i].get());

	_auxStreams.erase(_auxStreams.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::removeAuxStream(const AuxStreamIndex &i) {
	AuxStream *object = auxStream(i);
	if ( !object ) {
		return false;
	}

	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t SensorLocation::streamCount() const {
	return _streams.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Stream *SensorLocation::stream(size_t i) const {
	return _streams[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Stream *SensorLocation::stream(const StreamIndex &i) const {
	for ( const auto &elem : _streams ) {
		if ( i == elem->index() ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Stream *SensorLocation::findStream(const std::string& publicID) const {
	for ( const auto &elem : _streams ) {
		if ( elem->publicID() == publicID ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::add(Stream *stream) {
	if ( !stream ) {
		return false;
	}

	// Element has already a parent
	if ( stream->parent() != nullptr ) {
		SEISCOMP_ERROR("SensorLocation::add(Stream*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Stream *streamCached = Stream::Find(stream->publicID());
		if ( streamCached ) {
			if ( streamCached->parent() ) {
				if ( streamCached->parent() == this ) {
					SEISCOMP_ERROR("SensorLocation::add(Stream*) -> element with same publicID has been added already");
				}
				else {
					SEISCOMP_ERROR("SensorLocation::add(Stream*) -> element with same publicID has been added already to another object");
				}
				return false;
			}
			else {
				stream = streamCached;
			}
		}
	}

	// Add the element
	_streams.push_back(stream);
	stream->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		stream->accept(&nc);
	}

	// Notify registered observers
	childAdded(stream);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::remove(Stream *stream) {
	if ( !stream ) {
		return false;
	}

	if ( stream->parent() != this ) {
		SEISCOMP_ERROR("SensorLocation::remove(Stream*) -> element has another parent");
		return false;
	}

	auto it = std::find(_streams.begin(), _streams.end(), stream);
	// Element has not been found
	if ( it == _streams.end() ) {
		SEISCOMP_ERROR("SensorLocation::remove(Stream*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, it->get());
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_streams.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::removeStream(size_t i) {
	// index out of bounds
	if ( i >= _streams.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, _streams[i].get());
	}

	_streams[i]->setParent(nullptr);
	childRemoved(_streams[i].get());

	_streams.erase(_streams.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SensorLocation::removeStream(const StreamIndex &i) {
	Stream *object = stream(i);
	if ( !object ) {
		return false;
	}

	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SensorLocation::serialize(Archive &ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<Version::Major,Version::Minor>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: SensorLocation skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("code", _index.code, Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	if ( ar.supportsVersion<0,10>() )
		ar & NAMED_OBJECT_HINT("start", _index.start, Archive::XML_ELEMENT | Archive::SPLIT_TIME | Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	else
		ar & NAMED_OBJECT_HINT("start", _index.start, Archive::XML_ELEMENT | Archive::XML_MANDATORY | Archive::INDEX_ATTRIBUTE);
	if ( ar.supportsVersion<0,10>() )
		ar & NAMED_OBJECT_HINT("end", _end, Archive::XML_ELEMENT | Archive::SPLIT_TIME);
	else
		ar & NAMED_OBJECT_HINT("end", _end, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("latitude", _latitude, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("longitude", _longitude, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("elevation", _elevation, Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	if ( ar.supportsVersion<0,10>() )
		ar & NAMED_OBJECT_HINT("comment",
			Seiscomp::Core::Generic::containerMember(
				_comments,
				[this](const CommentPtr &comment) {
					return add(comment.get());
				}
			),
			Archive::STATIC_TYPE
		);
	ar & NAMED_OBJECT_HINT("auxStream",
		Seiscomp::Core::Generic::containerMember(
			_auxStreams,
			[this](const AuxStreamPtr &auxStream) {
				return add(auxStream.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("stream",
		Seiscomp::Core::Generic::containerMember(
			_streams,
			[this](const StreamPtr &stream) {
				return add(stream.get());
			}
		),
		Archive::STATIC_TYPE
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
