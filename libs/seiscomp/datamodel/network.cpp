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
#include <seiscomp/datamodel/network.h>
#include <seiscomp/datamodel/inventory.h>
#include <algorithm>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Network, PublicObject, "Network");


Network::MetaObject::MetaObject(const Core::RTTI *rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("code", "string", false, false, true, false, false, false, nullptr, &Network::setCode, &Network::code));
	addProperty(Core::simpleProperty("start", "datetime", false, false, true, false, false, false, nullptr, &Network::setStart, &Network::start));
	addProperty(Core::simpleProperty("end", "datetime", false, false, false, false, true, false, nullptr, &Network::setEnd, &Network::end));
	addProperty(Core::simpleProperty("description", "string", false, false, false, false, false, false, nullptr, &Network::setDescription, &Network::description));
	addProperty(Core::simpleProperty("institutions", "string", false, false, false, false, false, false, nullptr, &Network::setInstitutions, &Network::institutions));
	addProperty(Core::simpleProperty("region", "string", false, false, false, false, false, false, nullptr, &Network::setRegion, &Network::region));
	addProperty(Core::simpleProperty("type", "string", false, false, false, false, false, false, nullptr, &Network::setType, &Network::type));
	addProperty(Core::simpleProperty("netClass", "string", false, false, false, false, false, false, nullptr, &Network::setNetClass, &Network::netClass));
	addProperty(Core::simpleProperty("archive", "string", false, false, false, false, false, false, nullptr, &Network::setArchive, &Network::archive));
	addProperty(Core::simpleProperty("restricted", "boolean", false, false, false, false, true, false, nullptr, &Network::setRestricted, &Network::restricted));
	addProperty(Core::simpleProperty("shared", "boolean", false, false, false, false, true, false, nullptr, &Network::setShared, &Network::shared));
	addProperty(objectProperty<Blob>("remark", "Blob", false, false, true, &Network::setRemark, &Network::remark));
	addProperty(arrayClassProperty<Comment>("comment", "Comment", &Network::commentCount, &Network::comment, static_cast<bool (Network::*)(Comment*)>(&Network::add), &Network::removeComment, static_cast<bool (Network::*)(Comment*)>(&Network::remove)));
	addProperty(arrayObjectProperty("station", "Station", &Network::stationCount, &Network::station, static_cast<bool (Network::*)(Station*)>(&Network::add), &Network::removeStation, static_cast<bool (Network::*)(Station*)>(&Network::remove)));
}


IMPLEMENT_METAOBJECT(Network)


NetworkIndex::NetworkIndex() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkIndex::NetworkIndex(const std::string& code_,
                           Seiscomp::Core::Time start_) {
	code = code_;
	start = start_;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkIndex::NetworkIndex(const NetworkIndex &idx) {
	code = idx.code;
	start = idx.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NetworkIndex::operator==(const NetworkIndex &idx) const {
	return code == idx.code &&
	       start == idx.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NetworkIndex::operator!=(const NetworkIndex &idx) const {
	return !operator==(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network::Network() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network::Network(const Network &other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network::Network(const std::string& publicID)
: PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network::~Network() {
	for ( auto &comment : _comments ) {
		comment->setParent(nullptr);
	}
	for ( auto &station : _stations ) {
		station->setParent(nullptr);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network *Network::Create() {
	Network *object = new Network();
	return static_cast<Network*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network *Network::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != nullptr ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return nullptr;
	}

	return new Network(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network *Network::Find(const std::string& publicID) {
	return Network::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::operator==(const Network &rhs) const {
	if ( _index != rhs._index ) return false;
	if ( _end != rhs._end ) return false;
	if ( _description != rhs._description ) return false;
	if ( _institutions != rhs._institutions ) return false;
	if ( _region != rhs._region ) return false;
	if ( _type != rhs._type ) return false;
	if ( _netClass != rhs._netClass ) return false;
	if ( _archive != rhs._archive ) return false;
	if ( _restricted != rhs._restricted ) return false;
	if ( _shared != rhs._shared ) return false;
	if ( _remark != rhs._remark ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::operator!=(const Network &rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::equal(const Network &other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setCode(const std::string& code) {
	_index.code = code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Network::code() const {
	return _index.code;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setStart(Seiscomp::Core::Time start) {
	_index.start = start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time Network::start() const {
	return _index.start;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setEnd(const OPT(Seiscomp::Core::Time)& end) {
	_end = end;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time Network::end() const {
	if ( _end )
		return *_end;
	throw Seiscomp::Core::ValueException("Network.end is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setDescription(const std::string& description) {
	_description = description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Network::description() const {
	return _description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setInstitutions(const std::string& institutions) {
	_institutions = institutions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Network::institutions() const {
	return _institutions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setRegion(const std::string& region) {
	_region = region;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Network::region() const {
	return _region;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setType(const std::string& type) {
	_type = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Network::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setNetClass(const std::string& netClass) {
	_netClass = netClass;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Network::netClass() const {
	return _netClass;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setArchive(const std::string& archive) {
	_archive = archive;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Network::archive() const {
	return _archive;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setRestricted(const OPT(bool)& restricted) {
	_restricted = restricted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::restricted() const {
	if ( _restricted )
		return *_restricted;
	throw Seiscomp::Core::ValueException("Network.restricted is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setShared(const OPT(bool)& shared) {
	_shared = shared;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::shared() const {
	if ( _shared )
		return *_shared;
	throw Seiscomp::Core::ValueException("Network.shared is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::setRemark(const OPT(Blob)& remark) {
	_remark = remark;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Blob& Network::remark() {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("Network.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Blob& Network::remark() const {
	if ( _remark )
		return *_remark;
	throw Seiscomp::Core::ValueException("Network.remark is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const NetworkIndex &Network::index() const {
	return _index;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::equalIndex(const Network *lhs) const {
	if ( !lhs ) {
		return false;
	}

	return lhs->index() == index();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory *Network::inventory() const {
	return static_cast<Inventory*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Network &Network::operator=(const Network &other) {
	PublicObject::operator=(other);
	_index = other._index;
	_end = other._end;
	_description = other._description;
	_institutions = other._institutions;
	_region = other._region;
	_type = other._type;
	_netClass = other._netClass;
	_archive = other._archive;
	_restricted = other._restricted;
	_shared = other._shared;
	_remark = other._remark;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::assign(Object *other) {
	Network *otherNetwork = Network::Cast(other);
	if ( !other ) {
		return false;
	}

	*this = *otherNetwork;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::attachTo(PublicObject *parent) {
	if ( !parent ) {
		return false;
	}

	// check all possible parents
	Inventory *inventory = Inventory::Cast(parent);
	if ( inventory != nullptr )
		return inventory->add(this);

	SEISCOMP_ERROR("Network::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::detachFrom(PublicObject *object) {
	if ( !object ) {
		return false;
	}

	// check all possible parents
	Inventory *inventory = Inventory::Cast(object);
	if ( inventory != nullptr ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return inventory->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Network *child = inventory->findNetwork(publicID());
			if ( child != nullptr )
				return inventory->remove(child);
			else {
				SEISCOMP_DEBUG("Network::detachFrom(Inventory): network has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Network::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::detach() {
	if ( !parent() ) {
		return false;
	}

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object *Network::clone() const {
	Network *clonee = new Network();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::updateChild(Object *child) {
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

	Station *stationChild = Station::Cast(child);
	if ( stationChild != nullptr ) {
		Station *stationElement
			= Station::Cast(PublicObject::Find(stationChild->publicID()));
		if ( stationElement && stationElement->parent() == this ) {
			*stationElement = *stationChild;
			stationElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::accept(Visitor *visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( auto &&elem : _comments )
		elem->accept(visitor);
	for ( auto &&elem : _stations )
		elem->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Network::commentCount() const {
	return _comments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment *Network::comment(size_t i) const {
	return _comments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment *Network::comment(const CommentIndex &i) const {
	for ( const auto &elem : _comments ) {
		if ( i == elem->index() ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::add(Comment *comment) {
	if ( !comment ) {
		return false;
	}

	// Element has already a parent
	if ( comment->parent() != nullptr ) {
		SEISCOMP_ERROR("Network::add(Comment*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( auto &&elem : _comments ) {
		if ( elem->index() == comment->index() ) {
			SEISCOMP_ERROR("Network::add(Comment*) -> an element with the same index has been added already");
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
bool Network::remove(Comment *comment) {
	if ( !comment ) {
		return false;
	}

	if ( comment->parent() != this ) {
		SEISCOMP_ERROR("Network::remove(Comment*) -> element has another parent");
		return false;
	}

	auto it = std::find(_comments.begin(), _comments.end(), comment);
	// Element has not been found
	if ( it == _comments.end() ) {
		SEISCOMP_ERROR("Network::remove(Comment*) -> child object has not been found although the parent pointer matches???");
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
bool Network::removeComment(size_t i) {
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
bool Network::removeComment(const CommentIndex &i) {
	Comment *object = comment(i);
	if ( !object ) {
		return false;
	}

	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Network::stationCount() const {
	return _stations.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station *Network::station(size_t i) const {
	return _stations[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station *Network::station(const StationIndex &i) const {
	for ( const auto &elem : _stations ) {
		if ( i == elem->index() ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Station *Network::findStation(const std::string& publicID) const {
	for ( const auto &elem : _stations ) {
		if ( elem->publicID() == publicID ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::add(Station *station) {
	if ( !station ) {
		return false;
	}

	// Element has already a parent
	if ( station->parent() != nullptr ) {
		SEISCOMP_ERROR("Network::add(Station*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Station *stationCached = Station::Find(station->publicID());
		if ( stationCached ) {
			if ( stationCached->parent() ) {
				if ( stationCached->parent() == this ) {
					SEISCOMP_ERROR("Network::add(Station*) -> element with same publicID has been added already");
				}
				else {
					SEISCOMP_ERROR("Network::add(Station*) -> element with same publicID has been added already to another object");
				}
				return false;
			}
			else {
				station = stationCached;
			}
		}
	}

	// Add the element
	_stations.push_back(station);
	station->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		station->accept(&nc);
	}

	// Notify registered observers
	childAdded(station);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::remove(Station *station) {
	if ( !station ) {
		return false;
	}

	if ( station->parent() != this ) {
		SEISCOMP_ERROR("Network::remove(Station*) -> element has another parent");
		return false;
	}

	auto it = std::find(_stations.begin(), _stations.end(), station);
	// Element has not been found
	if ( it == _stations.end() ) {
		SEISCOMP_ERROR("Network::remove(Station*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, it->get());
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_stations.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::removeStation(size_t i) {
	// index out of bounds
	if ( i >= _stations.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, _stations[i].get());
	}

	_stations[i]->setParent(nullptr);
	childRemoved(_stations[i].get());

	_stations.erase(_stations.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Network::removeStation(const StationIndex &i) {
	Station *object = station(i);
	if ( !object ) {
		return false;
	}

	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Network::serialize(Archive &ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<Version::Major,Version::Minor>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Network skipped",
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
	ar & NAMED_OBJECT_HINT("description", _description, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("institutions", _institutions, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("region", _region, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("type", _type, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("netClass", _netClass, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("archive", _archive, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("restricted", _restricted, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("shared", _shared, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("remark", _remark, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
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
	ar & NAMED_OBJECT_HINT("station",
		Seiscomp::Core::Generic::containerMember(
			_stations,
			[this](const StationPtr &station) {
				return add(station.get());
			}
		),
		Archive::STATIC_TYPE
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
