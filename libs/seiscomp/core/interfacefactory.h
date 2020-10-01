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


#ifndef SEISCOMP_CORE_INTERFACEFACTORY_H
#define SEISCOMP_CORE_INTERFACEFACTORY_H

#include <map>
#include <vector>
#include <string>

namespace Seiscomp {
namespace Core {
namespace Generic {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Template based service factory interface

	To create an object, use the following code
	\code
	InterfaceFactory::Create(servicename);
	\endcode
 */
template <typename T>
class InterfaceFactoryInterface {
	public:
		typedef T Interface;
		typedef std::vector<InterfaceFactoryInterface<T>*> ServicePool;
		typedef std::vector<std::string> ServiceNames;


	protected:
		InterfaceFactoryInterface() {}
		InterfaceFactoryInterface(const char* serviceName);

	public:
		virtual ~InterfaceFactoryInterface();


	public:
		static T* Create(const char* serviceName);
		static T* Create(const std::string &serviceName);

		static unsigned int ServiceCount();

		static ServiceNames* Services();

		static InterfaceFactoryInterface* Find(const char* serviceName);

		const char* serviceName() const;

		virtual Interface* create() const = 0;


	private:
		static bool RegisterFactory(InterfaceFactoryInterface* factory);

		static bool UnregisterFactory(InterfaceFactoryInterface* factory);

		static ServicePool &Pool();

	private:
		std::string _serviceName;
};

#define DEFINE_INTERFACE_FACTORY(Class) \
typedef Seiscomp::Core::Generic::InterfaceFactoryInterface<Class> Class##Factory

#define DEFINE_TEMPLATE_INTERFACE_FACTORY(Class) \
template <typename T> \
struct Class##Factory : Seiscomp::Core::Generic::InterfaceFactoryInterface< Class<T> > {}


#define IMPLEMENT_INTERFACE_FACTORY(Class, APIDef) \
template class APIDef Seiscomp::Core::Generic::InterfaceFactoryInterface<Class>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**	\brief Template based service factory
 */
template <typename T, typename TYPE>
class InterfaceFactory : public InterfaceFactoryInterface<T> {
	public:
		//! The type that represents the actual polymorphic class.
		typedef TYPE Type;

	public:
		InterfaceFactory(const char* serviceName)
		 : InterfaceFactoryInterface<T>(serviceName) {}

	public:
		//! The actual creation
		typename InterfaceFactoryInterface<T>::Interface* create() const { return new TYPE; }
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define DECLARE_INTERFACEFACTORY_FRIEND(Interface, Class) \
friend class Seiscomp::Core::Generic::InterfaceFactory<Interface, Class>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}

#endif
