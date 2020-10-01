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


#ifndef SEISCOMP_CORE_BASEOBJECT_H
#define SEISCOMP_CORE_BASEOBJECT_H

namespace Seiscomp {
namespace Core {

class BaseObject;

}
}


#include <seiscomp/core/defs.h>
#include <seiscomp/core/rtti.h>
#include <seiscomp/core/metaobject.h>
#include <seiscomp/core/archive.h>
#include <seiscomp/core/factory.h>
#include <seiscomp/core.h>


#define DECLARE_CASTS(CLASS) \
		public: \
			static CLASS* Cast(Seiscomp::Core::BaseObject* o) { \
				return dynamic_cast<CLASS*>(o); \
			} \
			\
			static const CLASS* ConstCast(const Seiscomp::Core::BaseObject* o) { \
				return dynamic_cast<const CLASS*>(o); \
			} \
			\
			static CLASS* Cast(Seiscomp::Core::BaseObjectPtr o) { \
				return dynamic_cast<CLASS*>(o.get()); \
			} \
			static const CLASS* ConstCast(Seiscomp::Core::BaseObjectCPtr o) { \
				return dynamic_cast<const CLASS*>(o.get()); \
			}

#define DECLARE_SC_CLASS(CLASS) \
		DECLARE_RTTI; \
		DECLARE_CASTS(CLASS)


#define IMPLEMENT_SC_CLASS(CLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, Seiscomp::Core::BaseObject) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REGISTER_CLASS(Seiscomp::Core::BaseObject, CLASS)

#define IMPLEMENT_SC_CLASS_DERIVED(CLASS, BASECLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, BASECLASS) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REGISTER_CLASS(Seiscomp::Core::BaseObject, CLASS)

#define IMPLEMENT_SC_CLASS_DERIVED_OVERWRITE(CLASS, BASECLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, BASECLASS) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REREGISTER_CLASS(Seiscomp::Core::BaseObject, CLASS)


#define IMPLEMENT_SC_ABSTRACT_CLASS(CLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, Seiscomp::Core::BaseObject) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REGISTER_ABSTRACT_CLASS(Seiscomp::Core::BaseObject, CLASS)

#define IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(CLASS, BASECLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, BASECLASS) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REGISTER_ABSTRACT_CLASS(Seiscomp::Core::BaseObject, CLASS)

#define DECLARE_SC_CLASSFACTORY_FRIEND(CLASS) \
        DECLARE_CLASSFACTORY_FRIEND(Seiscomp::Core::BaseObject, CLASS)


namespace Seiscomp {
namespace Core {


DEFINE_SMARTPOINTER(BaseObject);
typedef Generic::ClassFactoryInterface<BaseObject> ClassFactory;


/**
 * \brief BaseObject has to be used for all classes that want to use
 *        the provided serialization mechanism and reference counting.
 *
 * To derive from BaseObject the following basic steps are necessary:
 * 1. Create a class that derives from BaseObject
 * \code
 * class MyClass : public BaseObject
 * \endcode
 *
 * 2. Add the DECLARE_SC_CLASS macro to add the RTTI interface among other things
 * \code
 * class MyClass : public BaseObject {
 *     DECLARE_SC_CLASS(MyClass);
 *     public:
 *         MyClass();
 *     };
 * \endcode
 *
 * Implement the class RTTI data in the .cpp file
 * \code
 * // First parameter is the classname, second parameter is the name inside RTTI
 * IMPLEMENT_SC_CLASS(MyClass, "MyClass");
 * \endcode
 *
 * If the class is abstract (it has some pure virtual methods) another macro
 * must be used:
 * \code
 * // First parameter is the classname, second parameter is the name inside RTTI
 * IMPLEMENT_SC_ABSTRACR_CLASS(MyClass, "MyClass");
 * \endcode
 *
 * 3. If you want your class to be serialized add the appropriate declaration
 * \code
 * class MyClass : public BaseObject {
 * DECLARE_SC_CLASS(MyClass);
 *
 * // Add serialization interface
 * DECLARE_SERIALIZATION;
 *
 * public:
 *     MyClass();
 *
 * private:
 *     int _myMember;
 * };
 * \endcode
 *
 * The serialization method has to be implemented the following way:
 * \code
 * void MyClass::serialize(Archive& ar) {
 *     // the archive will bind the name 'var1' to the member variable
 *     // _myMember
 *     ar & NAMED_OBJECT("var1", _myMember);
 * }
 * \endcode
 */
class SC_SYSTEM_CORE_API BaseObject {
	DECLARE_BASE_RTTI;
	DECLARE_CASTS(BaseObject)
	DECLARE_ROOT_SERIALIZATION(BaseObject)
	DECLARE_METAOBJECT_INTERFACE;


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	protected:
		//! Constructor
		BaseObject();
		BaseObject(const BaseObject&);

	public:
		//! Destructor
		virtual ~BaseObject();


	// ----------------------------------------------------------------------
	//  Public methods
	// ----------------------------------------------------------------------
	public:
		//! Returns a shallow copy of this instance
		virtual BaseObject *clone() const;


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		virtual BaseObject &operator=(const BaseObject&);


	// ----------------------------------------------------------------------
	//  Reference counting
	// ----------------------------------------------------------------------
	public:
		//! Increment the reference counter
		void incrementReferenceCount() const;

		//! Decrement the reference counter and deletes the object
		//! when reaching 0
		void decrementReferenceCount() const;

		/**
		 * Returns the number of references to this object when using smartpointers
		 * @return current reference count
		 */
		unsigned int referenceCount() const;

		/**
		 * Returns the number of created objects of type BaseObject at the time
		 * of calling this function.
		 * @return number of objects created
		 */
		static unsigned int ObjectCount();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		mutable volatile unsigned int _referenceCount;
		static  volatile unsigned int _objectCount;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include <seiscomp/core/baseobject.inl>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}


using Seiscomp::Core::intrusive_ptr_add_ref;
using Seiscomp::Core::intrusive_ptr_release;


#endif
