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


#ifndef SEISCOMP_DATAMODEL_METADATA_H__
#define SEISCOMP_DATAMODEL_METADATA_H__


#include <seiscomp/core/metaproperty.h>


namespace Seiscomp {
namespace DataModel {

namespace Generic {


template <typename T, typename U, typename F1, typename F2, int>
class EnumPropertyBase {};


//! Non-optional enum property specialization
template <typename T, typename U, typename F1, typename F2>
class EnumPropertyBase<T, U, F1, F2, 0> : public Core::MetaProperty {
	public:
		EnumPropertyBase(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;
			U tmp;
			if ( !tmp.fromInt(boost::any_cast<int>(value)) )
				return false;

			(target->*_setter)(tmp);
			return true;
		}

		bool writeString(Core::BaseObject *object, const std::string &value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;
			typename Core::Generic::remove_optional<U>::type tmp;
			if ( !tmp.fromString(value.c_str()) )
				return false;

			(target->*_setter)(tmp);
			return true;
		}

		Core::MetaValue read(const Core::BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return (target->*_getter)().toInt();
		}

		std::string readString(const Core::BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return (target->*_getter)().toString();
		}

	private:
		F1 _setter;
		F2 _getter;
};


//! Optional enum property specialization
template <typename T, typename U, typename F1, typename F2>
class EnumPropertyBase<T, U, F1, F2, 1> : public Core::MetaProperty {
	public:
		EnumPropertyBase(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				(target->*_setter)(Core::None);
			else {
				typename U::value_type tmp;
				if ( !tmp.fromInt(boost::any_cast<int>(value)) )
					return false;

				(target->*_setter)(tmp);
			}

			return true;
		}

		bool writeString(Core::BaseObject *object, const std::string &value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				(target->*_setter)(Core::None);
			else {
				typename Core::Generic::remove_optional<U>::type tmp;
				if ( !tmp.fromString(value.c_str()) )
					return false;

				(target->*_setter)(tmp);
			}

			return true;
		}

		Core::MetaValue read(const Core::BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return (target->*_getter)().toInt();
		}

		std::string readString(const Core::BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return (target->*_getter)().toString();
		}

	private:
		F1 _setter;
		F2 _getter;
};



template <typename A, typename T, typename U, typename F1, typename F2, int>
class BaseObjectPropertyBase {};


//! Non-optional baseobject property specialization
template <typename A, typename T, typename U, typename F1, typename F2>
class BaseObjectPropertyBase<A, T, U, F1, F2, 0> : public Core::MetaClassProperty<A> {
	public:
		BaseObjectPropertyBase(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;

			const Core::BaseObject *v;
			try {
				v = boost::any_cast<const Core::BaseObject*>(value);
			}
			catch ( boost::bad_any_cast & ) {
				try {
					v = boost::any_cast<Core::BaseObject*>(value);
				}
				catch ( boost::bad_any_cast & ) {
					try {
						v = boost::any_cast<const U*>(value);
					}
					catch ( boost::bad_any_cast & ) {
						v = boost::any_cast<U*>(value);
					}
				}
			}

			if ( v == nullptr )
				throw Core::GeneralException("value must not be nullptr");

			const U *uv = U::ConstCast(v);
			if ( uv == nullptr )
				throw Core::GeneralException("value has wrong classtype");

			(target->*_setter)(*uv);
			return true;
		}

		Core::MetaValue read(const Core::BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return static_cast<Core::BaseObject*>(&(const_cast<T*>(target)->*_getter)());
		}

	private:
		F1 _setter;
		F2 _getter;
};


//! Optional baseobject property specialization
template <typename A, typename T, typename U, typename F1, typename F2>
class BaseObjectPropertyBase<A, T, U, F1, F2, 1> : public Core::MetaClassProperty<A> {
	public:
		BaseObjectPropertyBase(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const override {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				(target->*_setter)(Core::None);
			else {
				const Core::BaseObject *v;
				try {
					v = boost::any_cast<const Core::BaseObject*>(value);
				}
				catch ( boost::bad_any_cast & ) {
					try {
						v = boost::any_cast<Core::BaseObject*>(value);
					}
					catch ( boost::bad_any_cast & ) {
						try {
							v = boost::any_cast<const typename U::value_type*>(value);
						}
						catch ( boost::bad_any_cast & ) {
							v = boost::any_cast<typename U::value_type*>(value);
						}
					}
				}

				if ( v == nullptr )
					throw Core::GeneralException("value must not be nullptr");

				const typename U::value_type *uv = U::value_type::ConstCast(v);
				if ( uv == nullptr )
					throw Core::GeneralException("value has wrong classtype");

				(target->*_setter)(*uv);
				return true;
			}

			return true;
		}

		Core::MetaValue read(const Core::BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return static_cast<Core::BaseObject*>(&(const_cast<T*>(target)->*_getter)());
		}

	private:
		F1 _setter;
		F2 _getter;
};



template <typename T, typename U, typename FCOUNT, typename FOBJ, typename FADD, typename FERASE1, typename FERASE2>
class ArrayProperty : public Core::MetaProperty {
	public:
		ArrayProperty(FCOUNT countObjects, FOBJ getObj, FADD addObj, FERASE1 eraseObjIndex, FERASE2 eraseObjPointer)
		 : _countObjects(countObjects),
		   _getObj(getObj),
		   _addObj(addObj),
		   _eraseObjIndex(eraseObjIndex),
		   _eraseObjPointer(eraseObjPointer) {}

		size_t arrayElementCount(const Core::BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return static_cast<size_t>((target->*_countObjects)());
		}

		Core::BaseObject *arrayObject(Core::BaseObject *object, int i) const override {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_getObj)(i);
		}

		bool arrayAddObject(Core::BaseObject *object, Core::BaseObject *ch) const override {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_addObj)(child);
		}

		bool arrayRemoveObject(Core::BaseObject *object, int i) const override {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_eraseObjIndex)(i);
		}

		bool arrayRemoveObject(Core::BaseObject *object, Core::BaseObject *ch) const override {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_eraseObjPointer)(child);
		}

		Core::BaseObject *createClass() const override {
			return U::Create();
		}

	private:
		FCOUNT _countObjects;
		FOBJ _getObj;
		FADD _addObj;
		FERASE1 _eraseObjIndex;
		FERASE2 _eraseObjPointer;
};


template <typename A, typename T, typename U, typename FCOUNT, typename FOBJ, typename FADD, typename FERASE1, typename FERASE2>
class ArrayClassProperty : public Core::MetaClassProperty<A> {
	public:
		ArrayClassProperty(FCOUNT countObjects, FOBJ getObj, FADD addObj, FERASE1 eraseObjIndex, FERASE2 eraseObjPointer)
		 : _countObjects(countObjects),
		   _getObj(getObj),
		   _addObj(addObj),
		   _eraseObjIndex(eraseObjIndex),
		   _eraseObjPointer(eraseObjPointer) {}

		size_t arrayElementCount(const Core::BaseObject *object) const override {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return static_cast<size_t>((target->*_countObjects)());
		}

		Core::BaseObject *arrayObject(Core::BaseObject *object, int i) const override {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_getObj)(i);
		}

		bool arrayAddObject(Core::BaseObject *object, Core::BaseObject *ch) const override {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_addObj)(child);
		}

		bool arrayRemoveObject(Core::BaseObject *object, int i) const override {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_eraseObjIndex)(i);
		}

		bool arrayRemoveObject(Core::BaseObject *object, Core::BaseObject *ch) const override {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_eraseObjPointer)(child);
		}

	private:
		FCOUNT _countObjects;
		FOBJ _getObj;
		FADD _addObj;
		FERASE1 _eraseObjIndex;
		FERASE2 _eraseObjPointer;
};


}



template <typename T, typename U, typename F1, typename F2>
class EnumProperty : public Generic::EnumPropertyBase<T, U, F1, F2, Core::Generic::is_optional<U>::value> {
	public:
		EnumProperty(F1 setter, F2 getter)
		 : Generic::EnumPropertyBase<T, U, F1, F2, Core::Generic::is_optional<U>::value>(setter, getter) {}
};


template <class C, typename R1, typename T1, typename T2>
Core::MetaPropertyHandle enumProperty(
	const std::string& name, const std::string& type,
	bool isIndex, bool isOptional,
	const Core::MetaEnum *enumeration,
	R1 (C::*setter)(T1), T2 (C::*getter)() const) {
	return Core::createProperty<EnumProperty>(
			name, type, false, false, isIndex,
			false, isOptional, true, enumeration,
			setter, getter);
}


template <typename A, typename T, typename U, typename F1, typename F2>
class ObjectProperty : public Generic::BaseObjectPropertyBase<A, T, U, F1, F2, Core::Generic::is_optional<U>::value> {
	public:
		ObjectProperty(F1 setter, F2 getter)
		 : Generic::BaseObjectPropertyBase<A, T, U, F1, F2, Core::Generic::is_optional<U>::value>(setter, getter) {}
};


template <typename A, typename C, typename R1, typename T1, typename T2>
Core::MetaPropertyHandle objectProperty(
	const std::string& name, const std::string& type, bool isIndex, bool isReference, bool isOptional,
	R1 (C::*setter)(T1), T2 (C::*getter)()) {

	typedef typename std::remove_const<
		typename std::remove_cv<
			typename std::remove_pointer<
				typename std::remove_reference<T1>::type
			>::type
		>::type
	>::type T;

	Core::MetaPropertyHandle h = Core::MetaPropertyHandle(new ObjectProperty<A, C, T, R1 (C::*)(T1), T2 (C::*)()>(setter, getter));
	h->setInfo(name, type, false, true, isIndex, isReference, isOptional, false, nullptr);
	return h;
}



template <template <typename, typename, typename, typename, typename, typename, typename> class P,
          class C, typename T>
Core::MetaPropertyHandle createArrayProperty(const std::string& name, const std::string& type,
                                             size_t (C::*counter)() const,
                                             T* (C::*getter)(size_t) const,
                                             bool (C::*adder)(T *),
                                             bool (C::*indexRemove)(size_t),
                                             bool (C::*ptrRemove)(T *)) {
	Core::MetaPropertyHandle h = Core::MetaPropertyHandle(
		new P<C, T, size_t (C::*)() const, T* (C::*)(size_t i) const, bool (C::*)(T *), bool (C::*)(size_t i), bool (C::*)(T *)>(counter, getter ,adder, indexRemove, ptrRemove));
	h->setInfo(name, type, true, true, false, false, false, false, nullptr);
	return h;
}


template <typename A, template <typename ,typename, typename, typename, typename, typename, typename, typename> class P,
          class C, typename T>
Core::MetaPropertyHandle createArrayClassProperty(const std::string& name,
                                                  const std::string& type,
                                                  size_t (C::*counter)() const,
                                                  T* (C::*getter)(size_t) const,
                                                  bool (C::*adder)(T *),
                                                  bool (C::*indexRemove)(size_t),
                                                  bool (C::*ptrRemove)(T *)) {
	Core::MetaPropertyHandle h = Core::MetaPropertyHandle(
		new P<A, C, T, size_t (C::*)() const, T* (C::*)(size_t i) const, bool (C::*)(T *), bool (C::*)(size_t i), bool (C::*)(T *)>(counter, getter ,adder, indexRemove, ptrRemove));
	h->setInfo(name, type, true, true, false, false, false, false, nullptr);
	return h;
}


template <typename C, typename T>
Core::MetaPropertyHandle arrayObjectProperty(
	const std::string& name, const std::string& type,
	size_t (C::*counter)() const,
	T* (C::*getter)(size_t) const,
	bool (C::*adder)(T *),
	bool (C::*indexRemove)(size_t),
	bool (C::*ptrRemove)(T *)
	) {
	return createArrayProperty<Generic::ArrayProperty>(name, type, counter, getter, adder, indexRemove, ptrRemove);
}


template <typename A, typename C, typename T>
Core::MetaPropertyHandle arrayClassProperty(
	const std::string& name, const std::string& type,
	size_t (C::*counter)() const,
	T* (C::*getter)(size_t) const,
	bool (C::*adder)(T *),
	bool (C::*indexRemove)(size_t),
	bool (C::*ptrRemove)(T *)
	) {
	return createArrayClassProperty<A, Generic::ArrayClassProperty>(name, type, counter, getter, adder, indexRemove, ptrRemove);
}


}
}


#endif
