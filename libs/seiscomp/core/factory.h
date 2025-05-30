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


#ifndef SEISCOMP_CORE_FACTORY_H
#define SEISCOMP_CORE_FACTORY_H


#include <map>
#include <vector>
#include <string>

#include "metaobject.h"
#include "rtti.h"


namespace Seiscomp {
namespace Core {
namespace Generic {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Template based class factory interface

	Class factories are build upon polymorphic types.
	In order to provide an generalized factory interface, a template
	class is used to implement factories for different class hierarchies.
	Objects are created by a classname.

	The current implementation sits on top of a custom RTTI implementation.
	A classname in the factory is the same as the one defined by the RTTI
	object. One could think of an implementation where different names could
	be useful. Therefore the static method getClassName(const T*) has been
	implemented. It does not call (T*)->className() but uses its own
	dictionaries to fetch the classname.

	To create an object, use the following code
	\code
	ClassFactoryInterface<MyBaseClassType>::create("MyClassName");
	\endcode
 */
template <typename ROOT_TYPE>
class ClassFactoryInterface {
	public:
		//! The type that represents the root class of the hierarchie.
		using RootType = ROOT_TYPE;
		using ClassPool = std::map<std::string, ClassFactoryInterface<ROOT_TYPE>*>;
		using ClassNames = std::map<const RTTI*, std::string>;
		using NameList = std::list<std::string>;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	protected:
		//! C'tor
		ClassFactoryInterface(const RTTI *typeInfo,
		                      const MetaObject *meta,
		                      bool reregister = false);

	public:
		//! D'tor
		virtual ~ClassFactoryInterface();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Creates an instance of the class with the passed in name
		static ROOT_TYPE* Create(const char *className);
		static ROOT_TYPE* Create(const std::string &className);

		//! Returns the registered classname for an object
		static const char* ClassName(const ROOT_TYPE *object);

		//! Returns the registered classname for a type
		static const char* ClassName(const RTTI *rtti);

		//! Looks up a class factory for a given class name
		static ClassFactoryInterface *FindByClassName(const char *className);
		static ClassFactoryInterface *FindByClassName(const std::string &className);

		static bool IsTypeOf(const char *baseName, const char *derivedName);

		//! Returns the number of registered classes. This is equal to
		//! Classes().size().
		static size_t NumberOfRegisteredClasses();

		//! Returns the registered classes.
		static const ClassNames &RegisteredClasses();

		//! Returns the name of the class (as given during construction) which can be created
		//! by this factory
		const char* className() const;

		//! Returns the class id for the objects the factory can create
		const RTTI* typeInfo() const;

		//! Returns the meta object for the objects the factory can create
		const MetaObject *meta() const;

		//! Derived classes override this method to do the actual creation
		virtual ROOT_TYPE* create() const = 0;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		static ClassPool &Classes();
		static ClassNames &Names();

		//! Adds a factory to the classpool
		//! \return whether the factory has been added or not
		static bool RegisterFactory(ClassFactoryInterface *factory, bool reregister = false);

		//! Removes a factory from the classpool
		//! \return whether the factory has been removed or not
		static bool UnregisterFactory(ClassFactoryInterface *factory);


	private:
		const RTTI       *_typeInfo;
		const MetaObject *_meta;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define IMPLEMENT_CLASSFACTORY(BaseClass, APIDef) \
template class APIDef Seiscomp::Core::Generic::ClassFactoryInterface<BaseClass>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**	\brief Template based class factory for an abstract class

    The class factory for an abstract class of a class hierarchie.
    It only registeres the root classname but does not create
    an object.
 */
template <typename ROOT_TYPE, typename TYPE>
class AbstractClassFactory : public ClassFactoryInterface<ROOT_TYPE> {
	public:
		//! The type that represents the actual polymorphic class.
		typedef TYPE Type;

	public:
		AbstractClassFactory(const char *);

	protected:
		//! Always returns nullptr
		ROOT_TYPE *create() const override;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**	\brief Template based class factory

	Each polymorphic type must be registered with the class factory.
	This is done via a simple declaration:
	\code
	Factory<MyBaseClassType,MyClassType> MyFactory("MyClassName");
	\endcode

	It is however recommended that the REGISTER_CLASS macro is used.
	\code
	REGISTER_CLASS(MyClass)
	\endcode
 */
template <typename ROOT_TYPE, typename TYPE>
class ClassFactory : public ClassFactoryInterface<ROOT_TYPE> {
	public:
		//! The type that represents the actual polymorphic class.
		typedef TYPE Type;

	public:
		ClassFactory(const char *, bool reregister = false);

 protected:
		//! The actual creation
		ROOT_TYPE* create() const override;
};


#define REGISTER_ABSTRACT_CLASS_VAR(BaseClass, Class) \
Seiscomp::Core::Generic::AbstractClassFactory<BaseClass, Class> __##Class##Factory__(#Class)

#define REGISTER_ABSTRACT_CLASS(BaseClass, Class) \
static REGISTER_ABSTRACT_CLASS_VAR(BaseClass, Class)

#define REGISTER_CLASS_VAR(BaseClass, Class) \
Seiscomp::Core::Generic::ClassFactory<BaseClass, Class> __##Class##Factory__(#Class, false)

#define REGISTER_CLASS(BaseClass, Class) \
static REGISTER_CLASS_VAR(BaseClass, Class)

#define REREGISTER_CLASS_VAR(BaseClass, Class) \
Seiscomp::Core::Generic::ClassFactory<BaseClass, Class> __##Class##Factory__(#Class, true)

#define REREGISTER_CLASS(BaseClass, Class) \
static REREGISTER_CLASS_VAR(BaseClass, Class)

#define DECLARE_CLASSFACTORY_FRIEND(BaseClass, Class) \
friend class Seiscomp::Core::Generic::ClassFactory<BaseClass, Class>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include "factory.inl"
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}

#endif
