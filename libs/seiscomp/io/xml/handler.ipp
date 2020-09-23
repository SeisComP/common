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

template <typename T>
void TypedClassHandler<T>::addProperty(const char *t, const char *ns, Type opt, Location l, const Core::MetaProperty *prop) {
	addMember(t, ns, opt, l, new PropertyHandler(prop));
}


template <typename T>
void TypedClassHandler<T>::addProperty(const char *t, const char *ns, Type opt, Location l, const char *property) {
	const Core::MetaObject *obj = T::Meta();
	if ( obj == nullptr )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaobject");

	const Core::MetaProperty *prop = nullptr;

	while ( obj && prop == nullptr ) {
		prop = obj->property(property);
		obj = obj->base();
	}

	if ( prop == nullptr )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaproperty " + property);

	addProperty(t, ns, opt, l, prop);
}


template <typename T>
void TypedClassHandler<T>::addChildProperty(const char *t, const char *ns, const char *property) {
	const Core::MetaObject *obj = T::Meta();
	if ( obj == nullptr )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaobject");

	const Core::MetaProperty *prop = nullptr;

	while ( obj && prop == nullptr ) {
		prop = obj->property(property);
		obj = obj->base();
	}

	if ( prop == nullptr )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaproperty " + property);

	if ( !prop->isArray() )
		throw Core::TypeException(std::string(T::ClassName()) + ": " + property + " property is not an array");

	addChild(t, ns, new ChildPropertyHandler(prop));
}


template <class C, typename R, typename T>
void ClassHandler::addMember(const char *t, const char *ns, Type opt, Location l, R (C::*s)(T)) {
	switch ( l ) {
		case Attribute:
			attributes.push_back(MemberNodeHandler(t, ns, (bool)opt, s));
			break;
		case Element:
			elements.push_back(MemberNodeHandler(t, ns, (bool)opt, s));
			orderedMembers.push_back(&elements.back());
			break;
		case CDATA:
			cdata = MemberNodeHandler(t, ns, (bool)opt, s);
			cdataUsed = true;
			break;
		default:
			break;
	}
}

template <class C, typename T1, typename T2, typename R>
void ClassHandler::addMember(const char *t, const char *ns, Type opt, R (C::*s)(T1), T2 (C::*g)()) {
	elements.push_back(MemberNodeHandler(t, ns, (bool)opt, g, s));
	orderedMembers.push_back(&childs.back());
}


template <typename T>
void TypeMap::registerMapping(const char *tag, const char *ns, NodeHandler *handler) {
	TypeHandler *h = new TypeStaticHandler<T>(handler);
	tags[Tag(tag, ns)] = h->className();

	std::pair<RawTagMap::iterator,bool> itp;
	itp = tagsWithoutNs.insert(RawTagMap::value_type(tag, h->className()));

	// Tag exists already -> set invalid classname
	if ( !itp.second ) itp.first->second.clear();

	classes[h->className()] = Tag(tag, ns);
	handlers[h->className()] = h;
}
