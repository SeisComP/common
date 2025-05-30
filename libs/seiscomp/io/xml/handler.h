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


#ifndef SEISCOMP_IO_XMLHANDLER_H
#define SEISCOMP_IO_XMLHANDLER_H


#include <seiscomp/io/importer.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/metaproperty.h>
#include <seiscomp/core.h>

#include <set>


namespace Seiscomp {
namespace IO {
namespace XML {


using ChildList = std::vector<Core::BaseObject*>;
using TagSet = std::set<std::string>;


struct SC_SYSTEM_CORE_API NodeHandler;

class SC_SYSTEM_CORE_API OutputHandler {
	public:
		virtual ~OutputHandler();

		virtual void handle(Core::BaseObject *, const char *tag, const char *ns, NodeHandler * = nullptr) = 0;

		virtual bool openElement(const char *name, const char *ns) = 0;
		virtual void addAttribute(const char *name, const char *ns, const char *value) = 0;
		virtual void closeElement(const char *name, const char *ns) = 0;

		virtual void put(const char *content) = 0;
};


class SC_SYSTEM_CORE_API MemberNodeHandler;

//! A NodeHandler handles nodes describing a class.
struct SC_SYSTEM_CORE_API NodeHandler {
	virtual ~NodeHandler();

	virtual bool init(Core::BaseObject *obj, void *n, TagSet &mandatoryTags) { return false; }
	// When called, object, next and newInstance are preset by the caller
	// object = nullptr
	// next = noneReader
	// newInstance = false
	// memberHandler = nullptr
	virtual std::string value(Core::BaseObject *obj) { return ""; }
	virtual bool put(Core::BaseObject *obj, const char *tag, const char *ns, OutputHandler *output) = 0;

	virtual bool get(Core::BaseObject *obj, void *n) { return false; }
	virtual bool finalize(Core::BaseObject *obj, ChildList *) { return false; }

	std::string content(void *n) const;
	bool equalsTag(void *n, const char *, const char *) const;

	//! Reset all members to default values
	void propagate(Core::BaseObject *o, bool ni, bool opt);

	Core::BaseObject *object;
	NodeHandler *childHandler;
	MemberNodeHandler *memberHandler;
	bool newInstance;
	bool isOptional;
	bool isAnyType;

	static bool strictNsCheck;
};


//! A MemberHandler handles setting of members of a class. Members can
//! be optional, mandatory and childs.
struct SC_SYSTEM_CORE_API MemberHandler {
	virtual ~MemberHandler();

	virtual std::string value(Core::BaseObject *obj) = 0;
	virtual bool put(Core::BaseObject *object, const char *tag, const char *ns,
		             bool opt, OutputHandler *output, NodeHandler *h);

	//! Get a "node" to the member of an object.
	virtual bool get(Core::BaseObject *object, void *node, NodeHandler *h) = 0;

	//! Finalizes the read member object (can be nullptr).
	virtual bool finalize(Core::BaseObject *parent, Core::BaseObject *member) { return true; }
};



class PropertyHandler : public MemberHandler {
	public:
		PropertyHandler(const Core::MetaProperty *prop)
		 : _property(prop) {}

		std::string value(Core::BaseObject *obj) override {
			try {
				return _property->readString(obj);
			}
			catch ( ... ) {
				return "";
			}
		}

		bool put(Core::BaseObject *object, const char *tag, const char *ns,
		         bool opt, OutputHandler *output, NodeHandler *h) override {
			if ( _property->isClass() ) {
				try {
					output->handle(boost::any_cast<Core::BaseObject*>(_property->read(object)), tag, ns);
					return true;
				}
				catch ( ... ) {}
			}
			else
				return MemberHandler::put(object, tag, ns, opt, output, h);

			return false;
		}

		bool get(Core::BaseObject *object, void *, NodeHandler *) override;
		bool finalize(Core::BaseObject *parent, Core::BaseObject *member) override;

	private:
		const Core::MetaProperty *_property;
};


class ChildPropertyHandler : public MemberHandler {
	public:
		ChildPropertyHandler(const Core::MetaProperty *prop)
		 : _property(prop) {}

		std::string value(Core::BaseObject *obj) override { return ""; }

		bool put(Core::BaseObject *object, const char *tag, const char *ns,
		         bool opt, OutputHandler *output, NodeHandler *h) override {
			size_t count = _property->arrayElementCount(object);
			for ( size_t i = 0; i < count; ++i ) {
				output->handle(_property->arrayObject(object, i), tag, ns);
			}
			return true;
		}

		bool get(Core::BaseObject *object, void *, NodeHandler *h) override {
			h->propagate(_property->createClass(), true, true);
			return true;
		}

		bool finalize(Core::BaseObject *parent, Core::BaseObject *child) override {
			if ( child )
				return _property->arrayAddObject(parent, child);
			return false;
		}

	private:
		const Core::MetaProperty *_property;
};



struct SC_SYSTEM_CORE_API TypeHandler {
	TypeHandler(NodeHandler *nh) : nodeHandler(nh) {}
	virtual ~TypeHandler();

	virtual Core::BaseObject *createClass() = 0;
	virtual const char *className() = 0;

	NodeHandler *nodeHandler;
};


struct SC_SYSTEM_CORE_API TypeNameHandler : public TypeHandler {
	TypeNameHandler(NodeHandler *nh, const char *cn)
	 : TypeHandler(nh), classname(cn) {}

	Core::BaseObject *createClass() override {
		return Core::ClassFactory::Create(classname.c_str());
	}

	const char *className() override {
		return classname.c_str();
	}

	std::string classname;
};


template <typename T>
struct TypeStaticHandler : public TypeHandler {
	TypeStaticHandler(NodeHandler *nh) : TypeHandler(nh) {}

	Core::BaseObject *createClass() override {
		return new T();
	}

	const char *className() override {
		return T::ClassName();
	}
};


struct SC_SYSTEM_CORE_API TypeMap {
	struct Tag {
		Tag();
		Tag(const std::string &, const std::string &);

		bool operator<(const Tag &other) const;

		std::string name;
		std::string ns;
	};

	// Maps a tag to a classname
	typedef std::map<Tag, std::string> TagMap;
	// Maps a tag without namespace to a classname
	typedef std::map<std::string, std::string> RawTagMap;
	// Maps a classname to a tag
	typedef std::map<std::string, Tag> ClassMap;
	// Maps a classname to a handler
	typedef std::map<std::string, TypeHandler*> HandlerMap;

	TagMap tags;
	RawTagMap tagsWithoutNs;
	ClassMap classes;
	HandlerMap handlers;

	TypeMap();
	~TypeMap();

	void registerMapping(const char *tag, const char *ns, const char *classname, NodeHandler *handler);

	template <typename T>
	void registerMapping(const char *tag, const char *ns, NodeHandler *handler);

	const char *getClassname(const char *tag, const char *ns, bool strictNsCheck);
	const Tag *getTag(const char *classname);

	NodeHandler *getHandler(const char *classname);
	Core::BaseObject *createClass(const char *classname);
};


struct SC_SYSTEM_CORE_API MemberNodeHandler {
	MemberNodeHandler() {}

	MemberNodeHandler(const char *t, const char *ns, bool opt)
		: tag(t), nameSpace(ns), optional(opt) {}

	MemberNodeHandler(const char *t, const char *ns, bool opt, MemberHandler *s);

	std::string value(Core::BaseObject *obj) { return setter->value(obj); }
	bool put(Core::BaseObject *obj, OutputHandler *output, NodeHandler *h);

	bool get(Core::BaseObject *obj, void *, NodeHandler *h);
	bool finalize(Core::BaseObject *parent, Core::BaseObject *child);

	std::string tag;
	std::string nameSpace;
	bool optional;
	std::shared_ptr<MemberHandler> setter;
};


struct SC_SYSTEM_CORE_API NoneHandler : public NodeHandler {
	bool put(Core::BaseObject *obj, const char *tag, const char *ns, OutputHandler *output) override;
	bool get(Core::BaseObject *obj, void *n) override;
};


struct SC_SYSTEM_CORE_API GenericHandler : public NodeHandler {
	bool put(Core::BaseObject *obj, const char *tag, const char *ns, OutputHandler *output) override;
	bool get(Core::BaseObject *, void *n) override;

	TypeMap *mapper;
};


struct SC_SYSTEM_CORE_API ClassHandler : public NodeHandler {
	typedef std::list<MemberNodeHandler> MemberList;
	typedef std::list<MemberNodeHandler*> MemberRefList;

	enum Location {
		Attribute,
		Element,
		CDATA
	};

	enum Type {
		Mandatory = 0,
		Optional =  1
	};

	ClassHandler() { cdataUsed = false; }

	template <class C, typename R, typename T>
	void addMember(const char *t, const char *ns, Type opt, Location l, R (C::*s)(T));

	template <class C, typename T1, typename T2, typename R>
	void addMember(const char *t, const char *ns, Type opt, R (C::*s)(T1), T2 (C::*g)());

	void addMember(const char *t, const char *ns, Type opt, Location l, MemberHandler *s);
	void addChild(const char *t, const char *ns, MemberHandler *s);

	bool init(Core::BaseObject *obj, void *n, TagSet &mandatoryTags) override;
	bool get(Core::BaseObject *obj, void *n) override;
	bool put(Core::BaseObject *obj, const char *tag, const char *ns, OutputHandler *output) override;

	MemberRefList orderedMembers;
	MemberList attributes;
	MemberList elements;
	MemberList childs;
	MemberNodeHandler cdata;
	bool cdataUsed;
};


template <typename T>
struct TypedClassHandler : public ClassHandler {
	void addProperty(const char *t, const char *ns, Type opt, Location l, const Core::MetaProperty *prop);
	void addProperty(const char *t, const char *ns, Type opt, Location l, const char *property);

	void addChildProperty(const char *t, const char *ns, const char *property);
};


#include <seiscomp/io/xml/handler.ipp>


}
}
}


#endif
