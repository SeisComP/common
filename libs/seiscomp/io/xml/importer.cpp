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


#define SEISCOMP_COMPONENT XMLImport
#include <seiscomp/logging/log.h>
#include <seiscomp/io/xml/importer.h>

#include <libxml/xmlreader.h>

#if LIBXML_VERSION < 20900
#  define XML_PARSE_BIG_LINES 4194304
#endif


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace IO {
namespace XML {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


int streamBufReadCallback(void* context, char* buffer, int len) {
	std::streambuf* buf = static_cast<std::streambuf*>(context);
	if ( buf == nullptr ) return -1;

	int count = 0;
	int ch = buf->sgetc();
	while ( ch != EOF && len-- && ch != '\0' ) {
		*buffer++ = (char)buf->sbumpc();
		ch = buf->sgetc();
		++count;
  	}

	return count;
}


int streamBufCloseCallback(void* context) {
	return 0;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NoneHandler Importer::_none;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Importer::Importer() {
	_typemap = nullptr;
	_strictNamespaceCheck = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TypeMap* Importer::typeMap() {
	return _typemap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Importer::setTypeMap(TypeMap *map) {
	_typemap = map;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Importer::setStrictNamespaceCheck(bool e) {
	_strictNamespaceCheck = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Importer::setRootName(std::string h) {
	_headerNode = h;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::BaseObject *Importer::get(std::streambuf* buf) {
	if ( _typemap == nullptr ) return nullptr;
	if ( buf == nullptr ) return nullptr;

	_result = nullptr;

	xmlDocPtr doc;
	doc = xmlReadIO(streamBufReadCallback,
	                streamBufCloseCallback,
	                buf, nullptr, nullptr, XML_PARSE_BIG_LINES);

	if ( doc == nullptr )
		return nullptr;

	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if ( cur == nullptr ) {
		xmlFreeDoc(doc);
		return nullptr;
	}

	_any.mapper = _typemap;

	bool saveStrictNsCheck = NodeHandler::strictNsCheck;

	if ( !_headerNode.empty() ) {
		// Check the root tag matching "seiscomp"
		if ( xmlStrcmp(cur->name, (const xmlChar*)_headerNode.c_str()) ) {
			SEISCOMP_WARNING("Invalid root tag: %s, expected: %s",
			                 reinterpret_cast<const char*>(cur->name),
			                 _headerNode.c_str());
			xmlFreeDoc(doc);
			return nullptr;
		}

		NodeHandler::strictNsCheck = _strictNamespaceCheck;
		_hasErrors = traverse(&_any, cur, cur->children, nullptr) == false;
	}
	else {
		NodeHandler::strictNsCheck = _strictNamespaceCheck;
		_hasErrors = traverse(&_any, nullptr, cur, nullptr) == false;
	}

	NodeHandler::strictNsCheck = saveStrictNsCheck;

	xmlFreeDoc(doc);

	return _result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Importer::traverse(NodeHandler *handler, void *n, void *c, Core::BaseObject *target) {
	xmlNodePtr node = reinterpret_cast<xmlNodePtr>(n);
	xmlNodePtr childs = reinterpret_cast<xmlNodePtr>(c);
	ChildList remaining;
	TagSet mandatory;

	handler->init(target, n, mandatory);

	bool result = true;

	for ( xmlNodePtr child = childs; child != nullptr; child = child->next ) {
		if ( child->type != XML_ELEMENT_NODE ) {
			continue;
		}

		handler->propagate(nullptr, false, true);

		try {
			handler->get(target, child);
		}
		catch ( std::exception &e ) {
			if ( handler->isOptional ) {
				SEISCOMP_WARNING("L%ld: (optional) %s.%s: %s",
				                 xmlGetLineNo(node),
				                 reinterpret_cast<const char*>(node->name),
				                 reinterpret_cast<const char*>(child->name),
				                 e.what());
			}
			else {
				throw e;
			}
		}

		if ( !handler->isOptional )
			mandatory.erase((const char*)child->name);

		if ( handler->object == nullptr && handler->isAnyType ) {
			if ( _any.get(target, child) ) {
				handler->object = _any.object;
				handler->childHandler = _any.childHandler;
				handler->newInstance = _any.newInstance;
			}
		}

		Core::BaseObject *newTarget = handler->object;
		MemberNodeHandler *memberHandler = handler->memberHandler;
		NodeHandler *childHandler = handler->childHandler;
		bool newInstance = handler->newInstance;
		bool optional = handler->isOptional;

		if ( newTarget ) {
			if ( childHandler == nullptr ) {
				childHandler = _typemap->getHandler(newTarget->className());
				if ( childHandler == nullptr ) {
					SEISCOMP_WARNING("No class handler for %s", newTarget->className());
					if ( newInstance )
						delete newTarget;
					handler->object = nullptr;
					newTarget = nullptr;
					childHandler = &_none;
				}
			}
		}
		else
			childHandler = &_none;

		try {
			if ( traverse(childHandler, child, child->children, handler->object) ) {
				if ( newTarget && newInstance && !memberHandler )
					remaining.push_back(newTarget);

			}
			else {
				if ( newTarget && newInstance )
					delete newTarget;
				newTarget = nullptr;
				if ( optional )
					SEISCOMP_INFO("L%ld: Invalid %s element: ignoring",
					              xmlGetLineNo(child),
					              reinterpret_cast<const char*>(child->name));
				else {
					SEISCOMP_WARNING("L%ld: %s is not optional within %s",
					                 xmlGetLineNo(child),
					                 reinterpret_cast<const char*>(child->name),
					                 reinterpret_cast<const char*>(node->name));
					result = false;
				}
			}
		}
		catch ( std::exception &e ) {
			SEISCOMP_WARNING("L%ld: %s: %s", xmlGetLineNo(child),
			                 reinterpret_cast<const char*>(child->name), e.what());
			if ( newTarget ) {
				if ( newInstance )
					delete newTarget;

				if ( !optional ) {
					SEISCOMP_WARNING("L%ld: %s is not optional within %s",
					                 xmlGetLineNo(child),
					                 reinterpret_cast<const char*>(child->name),
					                 reinterpret_cast<const char*>(node->name));
					result = false;
				}
				else
					SEISCOMP_WARNING("L%ld: %s: ignoring optional member %s: invalid",
					                 xmlGetLineNo(child),
					                 reinterpret_cast<const char*>(node->name),
					                 reinterpret_cast<const char*>(child->name));

				newTarget = nullptr;
			}
		}

		if ( memberHandler ) {
			if ( !memberHandler->finalize(target, newTarget) ) {
				if ( newTarget && newInstance )
					remaining.push_back(newTarget);
			}
		}
	}

	handler->finalize(target, &remaining);

	if ( target ) {
		for ( auto *child : remaining )
			if ( child ) {
				delete child;
			}
	}
	else {
		for ( auto *child : remaining ) {
			if ( child ) {
				if ( !_result ) {
					_result = child;
				}
				else {
					delete child;
				}
			}
		}
	}

	if ( !mandatory.empty() ) {
		std::string attribs;
		for ( TagSet::iterator it = mandatory.begin(); it != mandatory.end(); ++it ) {
			if ( it != mandatory.begin() ) {
				attribs += ", ";
			}
			attribs += *it;
		}
		SEISCOMP_WARNING("L%ld: %s: missing mandatory attribute%s: %s",
		                 xmlGetLineNo(node), reinterpret_cast<const char*>(node->name),
		                 mandatory.size() == 1?"":"s", attribs.c_str());
		return false;
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
