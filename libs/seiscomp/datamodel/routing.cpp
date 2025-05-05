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
#include <seiscomp/datamodel/routing.h>
#include <algorithm>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Routing, PublicObject, "Routing");


Routing::MetaObject::MetaObject(const Core::RTTI *rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("route", "Route", &Routing::routeCount, &Routing::route, static_cast<bool (Routing::*)(Route*)>(&Routing::add), &Routing::removeRoute, static_cast<bool (Routing::*)(Route*)>(&Routing::remove)));
	addProperty(arrayClassProperty<Access>("access", "Access", &Routing::accessCount, &Routing::access, static_cast<bool (Routing::*)(Access*)>(&Routing::add), &Routing::removeAccess, static_cast<bool (Routing::*)(Access*)>(&Routing::remove)));
}


IMPLEMENT_METAOBJECT(Routing)


Routing::Routing(): PublicObject("Routing") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Routing::Routing(const Routing &other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Routing::~Routing() {
	for ( auto &route : _routes ) {
		route->setParent(nullptr);
	}
	for ( auto &access : _accesss ) {
		access->setParent(nullptr);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::operator==(const Routing &rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::operator!=(const Routing &rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::equal(const Routing &other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Routing &Routing::operator=(const Routing &other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::assign(Object *other) {
	Routing *otherRouting = Routing::Cast(other);
	if ( !other ) {
		return false;
	}

	*this = *otherRouting;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::attachTo(PublicObject *parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::detachFrom(PublicObject *object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object *Routing::clone() const {
	Routing *clonee = new Routing();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::updateChild(Object *child) {
	Route *routeChild = Route::Cast(child);
	if ( routeChild != nullptr ) {
		Route *routeElement
			= Route::Cast(PublicObject::Find(routeChild->publicID()));
		if ( routeElement && routeElement->parent() == this ) {
			*routeElement = *routeChild;
			routeElement->update();
			return true;
		}
		return false;
	}

	Access *accessChild = Access::Cast(child);
	if ( accessChild != nullptr ) {
		Access *accessElement = access(accessChild->index());
		if ( accessElement != nullptr ) {
			*accessElement = *accessChild;
			accessElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Routing::accept(Visitor *visitor) {
	for ( auto &&elem : _routes )
		elem->accept(visitor);
	for ( auto &&elem : _accesss )
		elem->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Routing::routeCount() const {
	return _routes.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route *Routing::route(size_t i) const {
	return _routes[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route *Routing::route(const RouteIndex &i) const {
	for ( const auto &elem : _routes ) {
		if ( i == elem->index() ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Route *Routing::findRoute(const std::string& publicID) const {
	for ( const auto &elem : _routes ) {
		if ( elem->publicID() == publicID ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::add(Route *route) {
	if ( !route ) {
		return false;
	}

	// Element has already a parent
	if ( route->parent() != nullptr ) {
		SEISCOMP_ERROR("Routing::add(Route*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		Route *routeCached = Route::Find(route->publicID());
		if ( routeCached ) {
			if ( routeCached->parent() ) {
				if ( routeCached->parent() == this ) {
					SEISCOMP_ERROR("Routing::add(Route*) -> element with same publicID has been added already");
				}
				else {
					SEISCOMP_ERROR("Routing::add(Route*) -> element with same publicID has been added already to another object");
				}
				return false;
			}
			else {
				route = routeCached;
			}
		}
	}

	// Add the element
	_routes.push_back(route);
	route->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		route->accept(&nc);
	}

	// Notify registered observers
	childAdded(route);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::remove(Route *route) {
	if ( !route ) {
		return false;
	}

	if ( route->parent() != this ) {
		SEISCOMP_ERROR("Routing::remove(Route*) -> element has another parent");
		return false;
	}

	auto it = std::find(_routes.begin(), _routes.end(), route);
	// Element has not been found
	if ( it == _routes.end() ) {
		SEISCOMP_ERROR("Routing::remove(Route*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, it->get());
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_routes.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::removeRoute(size_t i) {
	// index out of bounds
	if ( i >= _routes.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, _routes[i].get());
	}

	_routes[i]->setParent(nullptr);
	childRemoved(_routes[i].get());

	_routes.erase(_routes.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::removeRoute(const RouteIndex &i) {
	Route *object = route(i);
	if ( !object ) {
		return false;
	}

	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Routing::accessCount() const {
	return _accesss.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Access *Routing::access(size_t i) const {
	return _accesss[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Access *Routing::access(const AccessIndex &i) const {
	for ( const auto &elem : _accesss ) {
		if ( i == elem->index() ) {
			return elem.get();
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::add(Access *access) {
	if ( !access ) {
		return false;
	}

	// Element has already a parent
	if ( access->parent() != nullptr ) {
		SEISCOMP_ERROR("Routing::add(Access*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( auto &&elem : _accesss ) {
		if ( elem->index() == access->index() ) {
			SEISCOMP_ERROR("Routing::add(Access*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_accesss.push_back(access);
	access->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		access->accept(&nc);
	}

	// Notify registered observers
	childAdded(access);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::remove(Access *access) {
	if ( !access ) {
		return false;
	}

	if ( access->parent() != this ) {
		SEISCOMP_ERROR("Routing::remove(Access*) -> element has another parent");
		return false;
	}

	auto it = std::find(_accesss.begin(), _accesss.end(), access);
	// Element has not been found
	if ( it == _accesss.end() ) {
		SEISCOMP_ERROR("Routing::remove(Access*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, it->get());
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_accesss.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::removeAccess(size_t i) {
	// index out of bounds
	if ( i >= _accesss.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		Notifier::Create(this, OP_REMOVE, _accesss[i].get());
	}

	_accesss[i]->setParent(nullptr);
	childRemoved(_accesss[i].get());

	_accesss.erase(_accesss.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Routing::removeAccess(const AccessIndex &i) {
	Access *object = access(i);
	if ( !object ) {
		return false;
	}

	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Routing::serialize(Archive &ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<Version::Major,Version::Minor>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Routing skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("route",
		Seiscomp::Core::Generic::containerMember(
			_routes,
			[this](const RoutePtr &route) {
				return add(route.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("access",
		Seiscomp::Core::Generic::containerMember(
			_accesss,
			[this](const AccessPtr &access) {
				return add(access.get());
			}
		),
		Archive::STATIC_TYPE
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
