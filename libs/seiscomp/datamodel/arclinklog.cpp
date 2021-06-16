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
#include <seiscomp/datamodel/arclinklog.h>
#include <algorithm>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(ArclinkLog, PublicObject, "ArclinkLog");


ArclinkLog::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("arclinkRequest", "ArclinkRequest", &ArclinkLog::arclinkRequestCount, &ArclinkLog::arclinkRequest, static_cast<bool (ArclinkLog::*)(ArclinkRequest*)>(&ArclinkLog::add), &ArclinkLog::removeArclinkRequest, static_cast<bool (ArclinkLog::*)(ArclinkRequest*)>(&ArclinkLog::remove)));
	addProperty(arrayObjectProperty("arclinkUser", "ArclinkUser", &ArclinkLog::arclinkUserCount, &ArclinkLog::arclinkUser, static_cast<bool (ArclinkLog::*)(ArclinkUser*)>(&ArclinkLog::add), &ArclinkLog::removeArclinkUser, static_cast<bool (ArclinkLog::*)(ArclinkUser*)>(&ArclinkLog::remove)));
}


IMPLEMENT_METAOBJECT(ArclinkLog)


ArclinkLog::ArclinkLog(): PublicObject("ArclinkLog") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkLog::ArclinkLog(const ArclinkLog& other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkLog::~ArclinkLog() {
	std::for_each(_arclinkRequests.begin(), _arclinkRequests.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ArclinkRequest::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&ArclinkRequestPtr::get)));
	std::for_each(_arclinkUsers.begin(), _arclinkUsers.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ArclinkUser::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&ArclinkUserPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::operator==(const ArclinkLog& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::operator!=(const ArclinkLog& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::equal(const ArclinkLog& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkLog& ArclinkLog::operator=(const ArclinkLog& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::assign(Object* other) {
	ArclinkLog* otherArclinkLog = ArclinkLog::Cast(other);
	if ( other == nullptr )
		return false;

	*this = *otherArclinkLog;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* ArclinkLog::clone() const {
	ArclinkLog* clonee = new ArclinkLog();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::updateChild(Object* child) {
	ArclinkRequest* arclinkRequestChild = ArclinkRequest::Cast(child);
	if ( arclinkRequestChild != nullptr ) {
		ArclinkRequest* arclinkRequestElement
			= ArclinkRequest::Cast(PublicObject::Find(arclinkRequestChild->publicID()));
		if ( arclinkRequestElement && arclinkRequestElement->parent() == this ) {
			*arclinkRequestElement = *arclinkRequestChild;
			arclinkRequestElement->update();
			return true;
		}
		return false;
	}

	ArclinkUser* arclinkUserChild = ArclinkUser::Cast(child);
	if ( arclinkUserChild != nullptr ) {
		ArclinkUser* arclinkUserElement
			= ArclinkUser::Cast(PublicObject::Find(arclinkUserChild->publicID()));
		if ( arclinkUserElement && arclinkUserElement->parent() == this ) {
			*arclinkUserElement = *arclinkUserChild;
			arclinkUserElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkLog::accept(Visitor* visitor) {
	for ( auto &&elem : _arclinkRequests )
		elem->accept(visitor);
	for ( auto &&elem : _arclinkUsers )
		elem->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ArclinkLog::arclinkRequestCount() const {
	return _arclinkRequests.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkLog::arclinkRequest(size_t i) const {
	return _arclinkRequests[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkLog::arclinkRequest(const ArclinkRequestIndex& i) const {
	for ( std::vector<ArclinkRequestPtr>::const_iterator it = _arclinkRequests.begin(); it != _arclinkRequests.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkRequest* ArclinkLog::findArclinkRequest(const std::string& publicID) const {
	for ( std::vector<ArclinkRequestPtr>::const_iterator it = _arclinkRequests.begin(); it != _arclinkRequests.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::add(ArclinkRequest* arclinkRequest) {
	if ( arclinkRequest == nullptr )
		return false;

	// Element has already a parent
	if ( arclinkRequest->parent() != nullptr ) {
		SEISCOMP_ERROR("ArclinkLog::add(ArclinkRequest*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ArclinkRequest* arclinkRequestCached = ArclinkRequest::Find(arclinkRequest->publicID());
		if ( arclinkRequestCached ) {
			if ( arclinkRequestCached->parent() ) {
				if ( arclinkRequestCached->parent() == this )
					SEISCOMP_ERROR("ArclinkLog::add(ArclinkRequest*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("ArclinkLog::add(ArclinkRequest*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				arclinkRequest = arclinkRequestCached;
		}
	}

	// Add the element
	_arclinkRequests.push_back(arclinkRequest);
	arclinkRequest->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		arclinkRequest->accept(&nc);
	}

	// Notify registered observers
	childAdded(arclinkRequest);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::remove(ArclinkRequest* arclinkRequest) {
	if ( arclinkRequest == nullptr )
		return false;

	if ( arclinkRequest->parent() != this ) {
		SEISCOMP_ERROR("ArclinkLog::remove(ArclinkRequest*) -> element has another parent");
		return false;
	}

	auto it = std::find(_arclinkRequests.begin(), _arclinkRequests.end(), arclinkRequest);
	// Element has not been found
	if ( it == _arclinkRequests.end() ) {
		SEISCOMP_ERROR("ArclinkLog::remove(ArclinkRequest*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_arclinkRequests.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::removeArclinkRequest(size_t i) {
	// index out of bounds
	if ( i >= _arclinkRequests.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_arclinkRequests[i]->accept(&nc);
	}

	_arclinkRequests[i]->setParent(nullptr);
	childRemoved(_arclinkRequests[i].get());

	_arclinkRequests.erase(_arclinkRequests.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::removeArclinkRequest(const ArclinkRequestIndex& i) {
	ArclinkRequest* object = arclinkRequest(i);
	if ( object == nullptr ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ArclinkLog::arclinkUserCount() const {
	return _arclinkUsers.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkUser* ArclinkLog::arclinkUser(size_t i) const {
	return _arclinkUsers[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkUser* ArclinkLog::arclinkUser(const ArclinkUserIndex& i) const {
	for ( std::vector<ArclinkUserPtr>::const_iterator it = _arclinkUsers.begin(); it != _arclinkUsers.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkUser* ArclinkLog::findArclinkUser(const std::string& publicID) const {
	for ( std::vector<ArclinkUserPtr>::const_iterator it = _arclinkUsers.begin(); it != _arclinkUsers.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::add(ArclinkUser* arclinkUser) {
	if ( arclinkUser == nullptr )
		return false;

	// Element has already a parent
	if ( arclinkUser->parent() != nullptr ) {
		SEISCOMP_ERROR("ArclinkLog::add(ArclinkUser*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ArclinkUser* arclinkUserCached = ArclinkUser::Find(arclinkUser->publicID());
		if ( arclinkUserCached ) {
			if ( arclinkUserCached->parent() ) {
				if ( arclinkUserCached->parent() == this )
					SEISCOMP_ERROR("ArclinkLog::add(ArclinkUser*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("ArclinkLog::add(ArclinkUser*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				arclinkUser = arclinkUserCached;
		}
	}

	// Add the element
	_arclinkUsers.push_back(arclinkUser);
	arclinkUser->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		arclinkUser->accept(&nc);
	}

	// Notify registered observers
	childAdded(arclinkUser);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::remove(ArclinkUser* arclinkUser) {
	if ( arclinkUser == nullptr )
		return false;

	if ( arclinkUser->parent() != this ) {
		SEISCOMP_ERROR("ArclinkLog::remove(ArclinkUser*) -> element has another parent");
		return false;
	}

	auto it = std::find(_arclinkUsers.begin(), _arclinkUsers.end(), arclinkUser);
	// Element has not been found
	if ( it == _arclinkUsers.end() ) {
		SEISCOMP_ERROR("ArclinkLog::remove(ArclinkUser*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_arclinkUsers.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::removeArclinkUser(size_t i) {
	// index out of bounds
	if ( i >= _arclinkUsers.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_arclinkUsers[i]->accept(&nc);
	}

	_arclinkUsers[i]->setParent(nullptr);
	childRemoved(_arclinkUsers[i].get());

	_arclinkUsers.erase(_arclinkUsers.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkLog::removeArclinkUser(const ArclinkUserIndex& i) {
	ArclinkUser* object = arclinkUser(i);
	if ( object == nullptr ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkLog::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,12>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: ArclinkLog skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("arclinkRequest",
		Seiscomp::Core::Generic::containerMember(
			_arclinkRequests,
			[this](const ArclinkRequestPtr &arclinkRequest) {
				return add(arclinkRequest.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("arclinkUser",
		Seiscomp::Core::Generic::containerMember(
			_arclinkUsers,
			[this](const ArclinkUserPtr &arclinkUser) {
				return add(arclinkUser.get());
			}
		),
		Archive::STATIC_TYPE
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
