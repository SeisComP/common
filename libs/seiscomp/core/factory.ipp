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

#include <seiscomp/core/exceptions.h>

namespace Seiscomp {
namespace Core {
namespace Generic {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ClassFactoryInterface<ROOT_TYPE>::ClassFactoryInterface(const RTTI* typeInfo, bool reregister) {
	_typeInfo = typeInfo;

	// while construction the interface will be added
	// to the classpool
	RegisterFactory(this, reregister);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ClassFactoryInterface<ROOT_TYPE>::~ClassFactoryInterface() {
	// remove the Factory from the classpool
	UnregisterFactory(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ROOT_TYPE* ClassFactoryInterface<ROOT_TYPE>::Create(const char* className) {
	ClassFactoryInterface<ROOT_TYPE>* factoryInterface = FindByClassName(className);
	if ( factoryInterface )
		return factoryInterface->create();

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
bool ClassFactoryInterface<ROOT_TYPE>::IsTypeOf(const char* baseName, const char* derivedName) {
	ClassFactoryInterface<ROOT_TYPE>* derivedFactory = FindByClassName(derivedName);
	if ( !derivedFactory )
		return false;

	ClassFactoryInterface<ROOT_TYPE>* baseFactory = FindByClassName(baseName);
	if ( !baseFactory )
		return false;

	return derivedFactory->typeInfo()->isTypeOf(*baseFactory->typeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ROOT_TYPE* ClassFactoryInterface<ROOT_TYPE>::Create(const std::string& className) {
	return Create(className.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
const char* ClassFactoryInterface<ROOT_TYPE>::ClassName(const ROOT_TYPE* object) {
	ClassNames::iterator it = Names().find(&object->typeInfo());
	return it == Names().end() ? nullptr : (*it).second.c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
const char* ClassFactoryInterface<ROOT_TYPE>::ClassName(const RTTI* info) {
	ClassNames::iterator it = Names().find(info);
	return it == Names().end() ? nullptr : (*it).second.c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
ClassFactoryInterface<ROOT_TYPE>* ClassFactoryInterface<ROOT_TYPE>::FindByClassName(const char* className) {
	if ( !className )
		return nullptr;

	typename ClassPool::iterator it = Classes().find(className);
	if ( it == Classes().end() )
		return nullptr;

	return (*it).second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
unsigned int ClassFactoryInterface<ROOT_TYPE>::NumberOfRegisteredClasses() {
	return static_cast<unsigned int>(Classes().size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
const char* ClassFactoryInterface<ROOT_TYPE>::className() const {
	return _typeInfo->className();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
const RTTI* ClassFactoryInterface<ROOT_TYPE>::typeInfo() const {
	return _typeInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
typename ClassFactoryInterface<ROOT_TYPE>::ClassPool& ClassFactoryInterface<ROOT_TYPE>::Classes() {
    static typename ClassFactoryInterface<ROOT_TYPE>::ClassPool* classes = new typename ClassFactoryInterface<ROOT_TYPE>::ClassPool;
    return *classes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
typename ClassFactoryInterface<ROOT_TYPE>::ClassNames& ClassFactoryInterface<ROOT_TYPE>::Names() {
    static typename ClassFactoryInterface<ROOT_TYPE>::ClassNames* names = new typename ClassFactoryInterface<ROOT_TYPE>::ClassNames;
    return *names;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
bool ClassFactoryInterface<ROOT_TYPE>::RegisterFactory(ClassFactoryInterface<ROOT_TYPE>* factory, bool reregister) {
	if ( !factory )
		return false;

	if ( !reregister ) {
		if ( Classes().find(factory->className()) != Classes().end() ) {
			throw DuplicateClassname(factory->className());
			return false;
		}
	}

	Classes()[factory->className()] = factory;
	Names()[factory->typeInfo()] = factory->className();
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
bool ClassFactoryInterface<ROOT_TYPE>::UnregisterFactory(ClassFactoryInterface<ROOT_TYPE>* factory) {
	if ( !factory )
		return false;

	typename ClassPool::iterator it = Classes().find(factory->className());
	if ( it == Classes().end() )
		// the factory has not been registered already
		return false;

	Classes().erase(it);

	typename ClassNames::iterator it_names = Names().find(factory->typeInfo());
	if ( it_names != Names().end() )
		Names().erase(it_names);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
