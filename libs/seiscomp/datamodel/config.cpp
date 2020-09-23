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
#include <seiscomp/datamodel/config.h>
#include <seiscomp/datamodel/parameterset.h>
#include <seiscomp/datamodel/configmodule.h>
#include <algorithm>
#include <seiscomp/datamodel/metadata.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Config, PublicObject, "Config");


Config::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("parameterSet", "ParameterSet", &Config::parameterSetCount, &Config::parameterSet, static_cast<bool (Config::*)(ParameterSet*)>(&Config::add), &Config::removeParameterSet, static_cast<bool (Config::*)(ParameterSet*)>(&Config::remove)));
	addProperty(arrayObjectProperty("module", "ConfigModule", &Config::configModuleCount, &Config::configModule, static_cast<bool (Config::*)(ConfigModule*)>(&Config::add), &Config::removeConfigModule, static_cast<bool (Config::*)(ConfigModule*)>(&Config::remove)));
}


IMPLEMENT_METAOBJECT(Config)


Config::Config(): PublicObject("Config") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Config::Config(const Config& other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Config::~Config() {
	std::for_each(_parameterSets.begin(), _parameterSets.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ParameterSet::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&ParameterSetPtr::get)));
	std::for_each(_configModules.begin(), _configModules.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ConfigModule::setParent),
	                                         (PublicObject*)nullptr),
	                            std::mem_fun_ref(&ConfigModulePtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::operator==(const Config& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::operator!=(const Config& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::equal(const Config& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Config& Config::operator=(const Config& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::assign(Object* other) {
	Config* otherConfig = Config::Cast(other);
	if ( other == nullptr )
		return false;

	*this = *otherConfig;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Config::clone() const {
	Config* clonee = new Config();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::updateChild(Object* child) {
	ParameterSet* parameterSetChild = ParameterSet::Cast(child);
	if ( parameterSetChild != nullptr ) {
		ParameterSet* parameterSetElement
			= ParameterSet::Cast(PublicObject::Find(parameterSetChild->publicID()));
		if ( parameterSetElement && parameterSetElement->parent() == this ) {
			*parameterSetElement = *parameterSetChild;
			parameterSetElement->update();
			return true;
		}
		return false;
	}

	ConfigModule* configModuleChild = ConfigModule::Cast(child);
	if ( configModuleChild != nullptr ) {
		ConfigModule* configModuleElement
			= ConfigModule::Cast(PublicObject::Find(configModuleChild->publicID()));
		if ( configModuleElement && configModuleElement->parent() == this ) {
			*configModuleElement = *configModuleChild;
			configModuleElement->update();
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Config::accept(Visitor* visitor) {
	for ( auto &&elem : _parameterSets )
		elem->accept(visitor);
	for ( auto &&elem : _configModules )
		elem->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Config::parameterSetCount() const {
	return _parameterSets.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParameterSet* Config::parameterSet(size_t i) const {
	return _parameterSets[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParameterSet* Config::findParameterSet(const std::string& publicID) const {
	for ( std::vector<ParameterSetPtr>::const_iterator it = _parameterSets.begin(); it != _parameterSets.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::add(ParameterSet* parameterSet) {
	if ( parameterSet == nullptr )
		return false;

	// Element has already a parent
	if ( parameterSet->parent() != nullptr ) {
		SEISCOMP_ERROR("Config::add(ParameterSet*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ParameterSet* parameterSetCached = ParameterSet::Find(parameterSet->publicID());
		if ( parameterSetCached ) {
			if ( parameterSetCached->parent() ) {
				if ( parameterSetCached->parent() == this )
					SEISCOMP_ERROR("Config::add(ParameterSet*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Config::add(ParameterSet*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				parameterSet = parameterSetCached;
		}
	}

	// Add the element
	_parameterSets.push_back(parameterSet);
	parameterSet->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		parameterSet->accept(&nc);
	}

	// Notify registered observers
	childAdded(parameterSet);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::remove(ParameterSet* parameterSet) {
	if ( parameterSet == nullptr )
		return false;

	if ( parameterSet->parent() != this ) {
		SEISCOMP_ERROR("Config::remove(ParameterSet*) -> element has another parent");
		return false;
	}

	auto it = std::find(_parameterSets.begin(), _parameterSets.end(), parameterSet);
	// Element has not been found
	if ( it == _parameterSets.end() ) {
		SEISCOMP_ERROR("Config::remove(ParameterSet*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_parameterSets.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::removeParameterSet(size_t i) {
	// index out of bounds
	if ( i >= _parameterSets.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_parameterSets[i]->accept(&nc);
	}

	_parameterSets[i]->setParent(nullptr);
	childRemoved(_parameterSets[i].get());

	_parameterSets.erase(_parameterSets.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Config::configModuleCount() const {
	return _configModules.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConfigModule* Config::configModule(size_t i) const {
	return _configModules[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConfigModule* Config::findConfigModule(const std::string& publicID) const {
	for ( std::vector<ConfigModulePtr>::const_iterator it = _configModules.begin(); it != _configModules.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::add(ConfigModule* configModule) {
	if ( configModule == nullptr )
		return false;

	// Element has already a parent
	if ( configModule->parent() != nullptr ) {
		SEISCOMP_ERROR("Config::add(ConfigModule*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ConfigModule* configModuleCached = ConfigModule::Find(configModule->publicID());
		if ( configModuleCached ) {
			if ( configModuleCached->parent() ) {
				if ( configModuleCached->parent() == this )
					SEISCOMP_ERROR("Config::add(ConfigModule*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Config::add(ConfigModule*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				configModule = configModuleCached;
		}
	}

	// Add the element
	_configModules.push_back(configModule);
	configModule->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		configModule->accept(&nc);
	}

	// Notify registered observers
	childAdded(configModule);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::remove(ConfigModule* configModule) {
	if ( configModule == nullptr )
		return false;

	if ( configModule->parent() != this ) {
		SEISCOMP_ERROR("Config::remove(ConfigModule*) -> element has another parent");
		return false;
	}

	auto it = std::find(_configModules.begin(), _configModules.end(), configModule);
	// Element has not been found
	if ( it == _configModules.end() ) {
		SEISCOMP_ERROR("Config::remove(ConfigModule*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(nullptr);
	childRemoved((*it).get());

	_configModules.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::removeConfigModule(size_t i) {
	// index out of bounds
	if ( i >= _configModules.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_configModules[i]->accept(&nc);
	}

	_configModules[i]->setParent(nullptr);
	childRemoved(_configModules[i].get());

	_configModules.erase(_configModules.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Config::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Config skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("parameterSet",
		Seiscomp::Core::Generic::containerMember(
			_parameterSets,
			[this](const ParameterSetPtr &parameterSet) {
				return add(parameterSet.get());
			}
		),
		Archive::STATIC_TYPE
	);
	ar & NAMED_OBJECT_HINT("module",
		Seiscomp::Core::Generic::containerMember(
			_configModules,
			[this](const ConfigModulePtr &configModule) {
				return add(configModule.get());
			}
		),
		Archive::STATIC_TYPE
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
