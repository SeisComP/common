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


#ifndef SEISCOMP_CORE_METAPROPERTY_H
#define SEISCOMP_CORE_METAPROPERTY_H

#include <seiscomp/core/metaobject.h>
#include <seiscomp/core/strings.h>


namespace Seiscomp {
namespace Core {

namespace Generic {


class No { };
class Yes { No no[2]; };


template <typename T>
Yes isOptionalTester(Optional<T>*);
No isOptionalTester(void*);


template <typename T>
class is_optional {
	static T* t;

	public:
		enum { value = sizeof(isOptionalTester(t)) == sizeof(Yes) };
};


template <typename T, int>
struct remove_optional_helper {};

template <typename T>
struct remove_optional_helper<T,0> {
	typedef T type;
};

template <typename T>
struct remove_optional_helper<T,1> {
	typedef typename T::value_type type;
};


template <typename T>
struct remove_optional {
	typedef typename remove_optional_helper<T, is_optional<T>::value>::type type;
};


}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
class MetaEnumImpl : public MetaEnum {
	public:
		MetaEnumImpl() : MetaEnum() {}

	public:
		int keyCount() const override { return T::Quantity; }

		//! Returns the key name at a given index
		const char *key(int index) const override;

		const char *valueToKey(int value) const override;
		int keyToValue(const char *key) const override;
};

#define DECLARE_METAENUM(CLASS, var) Seiscomp::Core::MetaEnumImpl<CLASS> var
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline const char *MetaEnumImpl<T>::key(int index) const {
	return T::NameDispatcher::name(index);
}

template <typename T>
inline const char *MetaEnumImpl<T>::valueToKey(int value) const {
	T tmp;
	if ( !tmp.fromInt(value) )
		throw ValueException("value out of bounds");

	return tmp.toString();
}

template <typename T>
inline int MetaEnumImpl<T>::keyToValue(const char *key) const {
	T tmp;
	if ( !tmp.fromString(key) )
		throw ValueException("invalid key");

	return tmp.toInt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
class MetaClassProperty : public MetaProperty {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		MetaClassProperty() : MetaProperty() {}

		MetaClassProperty(const std::string& name,
		                  const std::string& type,
		                  bool  isArray,
		                  bool  isIndex,
		                  bool  isOptional)
		: MetaProperty(name, type, isArray, true, isIndex,
		               isOptional, false, false, nullptr) {}


	public:
		BaseObject *createClass() const override {
			return new T();
		}
};


template <template <typename T_1, typename T_2, typename T_3, typename T_4> class P,
          class C, typename R1, typename T1, typename T2>
MetaPropertyHandle createProperty(R1 (C::*setter)(T1), T2 (C::*getter)()) {
	typedef typename std::remove_const<
		typename std::remove_cv<
			typename std::remove_pointer<
				typename std::remove_reference<T1>::type
			>::type
		>::type
	>::type T;

	return MetaPropertyHandle(new P<C, T, R1 (C::*)(T1), T2 (C::*)()>(setter, getter));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <template <typename T_1, typename T_2, typename T_3, typename T_4> class P,
          class C, typename R1, typename T1, typename T2>
MetaPropertyHandle createProperty(const std::string& name, const std::string& type,
                                  bool isArray,    bool isClass, bool isIndex,
                                  bool isReference, bool isOptional, bool isEnum,
                                  const MetaEnum *enumeration,
                                  R1 (C::*setter)(T1), T2 (C::*getter)() const) {
	typedef typename std::remove_const<
		typename std::remove_cv<
			typename std::remove_pointer<
				typename std::remove_reference<T1>::type
			>::type
		>::type
	>::type T;

	MetaPropertyHandle h = MetaPropertyHandle(new P<C, T, R1 (C::*)(T1), T2 (C::*)() const>(setter, getter));
	h->setInfo(name, type, isArray, isClass, isIndex, isReference, isOptional, isEnum, enumeration);
	return h;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <template <typename T_1, typename T_2, typename T_3, typename T_4> class P,
          class C, typename R1, typename T1, typename T2>
MetaPropertyHandle createProperty(const std::string& name, const std::string& type,
                                  bool isArray,    bool isClass, bool isIndex,
                                  bool isReference, bool isOptional, bool isEnum,
                                  const MetaEnum *enumeration,
                                  R1 (C::*setter)(T1), T2 (C::*getter)()) {
	typedef typename std::remove_const<
		typename std::remove_cv<
			typename std::remove_pointer<
				typename std::remove_reference<T1>::type
			>::type
		>::type
	>::type T;

	MetaPropertyHandle h = MetaPropertyHandle(new P<C, T, R1 (C::*)(T1), T2 (C::*)()>(setter, getter));
	h->setInfo(name, type, isArray, isClass, isIndex, isReference, isOptional, isEnum, enumeration);
	return h;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, typename U, typename F1, typename F2, int>
class SimplePropertyHelper : public MetaProperty {};

template <typename T, typename U, typename F1, typename F2>
class SimplePropertyHelper<T,U,F1,F2,0> : public MetaProperty {
	public:
		SimplePropertyHelper(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(BaseObject *object, MetaValue value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;
			(target->*_setter)(metaValueCast<U>(value));
			return true;
		}

		bool writeString(BaseObject *object, const std::string &value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;

			U tmp;
			if ( !fromString(tmp, value) )
				return false;

			(target->*_setter)(tmp);
			return true;
		}

		MetaValue read(const BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw GeneralException("invalid object");
			return (target->*_getter)();
		}

		std::string readString(const BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw GeneralException("invalid object");
			return toString((target->*_getter)());
		}

	private:
		F1 _setter;
		F2 _getter;
};

template <typename T, typename U, typename F1, typename F2>
class SimplePropertyHelper<T,U,F1,F2,1> : public MetaProperty {
	public:
		SimplePropertyHelper(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(BaseObject *object, MetaValue value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				(target->*_setter)(Core::None);
			else
				(target->*_setter)(metaValueCast<U>(value));
			return true;
		}

		bool writeString(BaseObject *object, const std::string &value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				(target->*_setter)(Core::None);
			else {
				typename Core::Generic::remove_optional<U>::type tmp;
				if ( !fromString(tmp, value) )
					return false;

				(target->*_setter)(tmp);
			}
			return true;
		}

		MetaValue read(const BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw GeneralException("invalid object");
			return (target->*_getter)();
		}

		std::string readString(const BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw GeneralException("invalid object");
			return toString((target->*_getter)());
		}

	private:
		F1 _setter;
		F2 _getter;
};


template <typename T, typename U, typename F1, typename F2>
class SimpleProperty : public SimplePropertyHelper<T, U, F1, F2, Generic::is_optional<U>::value> {
	public:
		SimpleProperty(F1 setter, F2 getter)
		 : SimplePropertyHelper<T, U, F1, F2, Generic::is_optional<U>::value>(setter, getter) {}
};



//! Creates a simple property assuming simple get/set methods
//! \code
//! class MyClass {
//!   public:
//!     void setAttrib(type value);
//!     type attrib() const;
//! };
//! \endcode
template <class C, typename R1, typename T1, typename T2>
MetaPropertyHandle simpleProperty(R1 (C::*setter)(T1), T2 (C::*getter)() const) {
	return createProperty<SimpleProperty>(setter, getter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <class C, typename R1, typename T1, typename T2>
MetaPropertyHandle simpleProperty(const std::string& name, const std::string& type,
                                  bool isArray,    bool isClass, bool isIndex,
                                  bool isReference, bool isOptional, bool isEnum,
                                  const MetaEnum *enumeration,
                                  R1 (C::*setter)(T1), T2 (C::*getter)() const) {
	return createProperty<SimpleProperty>(name, type, isArray, isClass, isIndex,
	                                      isReference, isOptional, isEnum, enumeration,
	                                      setter, getter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}



#endif
