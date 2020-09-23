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


#define SEISCOMP_COMPONENT XMLExport
#include <seiscomp/logging/log.h>
#include <seiscomp/io/xml/exporter.h>

#include <iostream>
#include <set>
#include <ostream>
#include <libxml/xmlreader.h>


namespace Seiscomp {
namespace IO {
namespace XML {


namespace {


class NamespaceCollector : public OutputHandler {
	public:
		void handle(Core::BaseObject *obj, const char *defaultTag, const char *ns, NodeHandler *handler = nullptr) {
			TypeMap::Tag defTag(
					defaultTag?defaultTag:"",
					ns?ns:""
			);

			const TypeMap::Tag *tag =
				(defaultTag != nullptr && *defaultTag != '\0')
					?
				&defTag
					:
				typemap->getTag(obj->className());

			if ( handler == nullptr )
				handler = typemap->getHandler(obj->className());

			if ( handler )
				handler->put(obj, tag->name.c_str(), tag->ns.c_str(), this);
		}

		bool openElement(const char *name, const char *ns) {
			if ( ns && *ns != '\0' )
				namespaces.insert(std::string(ns));
			return true;
		}

		void addAttribute(const char *name, const char *ns, const char *value) {
			if ( ns && *ns != '\0' )
				namespaces.insert(std::string(ns));
		}

		void closeElement(const char *name, const char *ns) {}

		void put(const char *content) {}

	public:
		TypeMap *typemap;
		std::set<std::string> namespaces;
};


}


static const char *xmlHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

Exporter::Exporter() : _ostr(std::cout.rdbuf()) {
	_typemap = nullptr;
}

TypeMap* Exporter::typeMap() {
	return _typemap;
}

void Exporter::setTypeMap(TypeMap *map) {
	_typemap = map;
}


void Exporter::setRootName(std::string h) {
	_headerNode = h;
}


void Exporter::collectNamespaces(Core::BaseObject *obj) {
	NamespaceCollector nsc;
	nsc.typemap = _typemap;
	nsc.handle(obj, "", "", nullptr);

	for ( std::set<std::string>::iterator it = nsc.namespaces.begin();
	      it != nsc.namespaces.end(); ++it ) {
		NamespaceMap::iterator nsit = _defaultNsMap.find(*it);
		std::string prefix;
		if ( nsit == _defaultNsMap.end() )
			prefix = std::string("ns") + Core::toString(_namespaces.size()+1);
		else
			prefix = nsit->second;

		_namespaces[*it] = prefix;
	}
}


bool Exporter::put(std::streambuf* buf, Core::BaseObject *obj) {
	if ( buf == nullptr ) return false;
	if ( obj == nullptr ) return false;

	_lastTagState = 0;
	_tagOpen = false;
	_firstElement = true;
	_indent = 0;

	_ostr.rdbuf(buf);

	_ostr << xmlHeader;
	if ( !_headerNode.empty() )
		_ostr << "<" << _headerNode << ">";

	collectNamespaces(obj);

	handle(obj, "", "", nullptr);

	if ( !_headerNode.empty() )
		_ostr << std::endl << "</" << _headerNode << ">";
	_ostr << std::endl;

	return true;
}


bool Exporter::put(std::streambuf* buf, const ExportObjectList &objects) {
	if ( buf == nullptr ) return false;

	_ostr.rdbuf(buf);

	_ostr << xmlHeader;
	if ( !_headerNode.empty() )
		_ostr << "<" << _headerNode << ">";

	for ( ExportObjectList::const_iterator it = objects.begin(); it != objects.end(); ++it )
		collectNamespaces(*it);

	for ( ExportObjectList::const_iterator it = objects.begin(); it != objects.end(); ++it ) {
		_lastTagState = 0;
		_tagOpen = false;
		_firstElement = true;
		_indent = 0;
		handle(*it, "", "", nullptr);
	}

	if ( !_headerNode.empty() )
		_ostr << std::endl << "</" << _headerNode << ">";
	_ostr << std::endl;

	return true;
}


void Exporter::handle(Core::BaseObject *obj, const char *defaultTag, const char *ns, NodeHandler *handler) {
	TypeMap::Tag defTag(
			defaultTag?defaultTag:"",
			ns?ns:""
	);

	const TypeMap::Tag *tag =
		(defaultTag != nullptr && *defaultTag != '\0')
			?
		&defTag
			:
		_typemap->getTag(obj->className());

	if ( handler == nullptr )
		handler = _typemap->getHandler(obj->className());

	if ( handler )
		handler->put(obj, tag->name.c_str(), tag->ns.c_str(), this);
}


bool Exporter::openElement(const char *name, const char *ns) {
	if ( _tagOpen ) {
		_ostr << ">";
		_tagOpen = false;
	}

	if ( _prettyPrint ) {
		_ostr << std::endl;
		for ( int i = 0; i < _indent; ++i ) _ostr << " ";
	}

	_ostr << "<";

	if ( ns != nullptr && *ns != '\0' ) {
		NamespaceMap::iterator it = _namespaces.find(ns);
		if ( it != _namespaces.end() ) {
			// Reuse namespace prefix
			if ( !it->second.empty() )
				_ostr << it->second << ":" << name;
			else
				_ostr << name;
		}
		else {
			// !Error
		}
	}
	else
		_ostr << name;

	if ( _firstElement ) {
		for ( NamespaceMap::iterator it = _namespaces.begin(); it != _namespaces.end(); ++it )
			if ( it->second.empty() )
				_ostr << " xmlns=\"" << it->first << "\"";
			else
				_ostr << " xmlns:" << it->second << "=\"" << it->first << "\"";
		_firstElement = false;
	}

	_lastTagState = 1;
	_indent += _indentation;
	_tagOpen = true;

	return _ostr.good();
}


void Exporter::addAttribute(const char *name, const char *ns, const char *value) {
	_ostr << " ";

	if ( ns != nullptr && *ns != '\0' ) {
		NamespaceMap::iterator it = _namespaces.find(ns);
		if ( it != _namespaces.end() )
			// Reuse namespace prefix
			_ostr << it->second << ":" << name;
		else {
			// Error!
		}
	}
	else
		_ostr << name;

	_ostr << "=" << "\"";
	writeString(value);
	_ostr << "\"";
}


void Exporter::closeElement(const char *name, const char *ns) {
	_indent -= _indentation;
	if ( _lastTagState == 0 && _prettyPrint ) {
		_ostr << std::endl;
		for ( int i = 0; i < _indent; ++i ) _ostr << " ";
	}

	if ( _tagOpen ) {
		_ostr << "/>";
		_tagOpen = false;
	}
	else {
		_ostr << "</";
		if ( ns != nullptr && *ns != '\0' ) {
			NamespaceMap::iterator it = _namespaces.find(ns);
			if ( it != _namespaces.end() ) {
				// Reuse namespace prefix
				if ( !it->second.empty() )
					_ostr << it->second << ":";
			}
			else
				throw Core::StreamException("No namespace prefix found for closing tag: this should never happen");
		}

		_ostr << name << ">";
 	}

	_lastTagState= 0;
}


void Exporter::writeString(const char *content) {
	// &amp; refers to an ampersand (&)
	// &lt; refers to a less-than symbol (<)
	// &gt; refers to a greater-than symbol (>)
	// &quot; refers to a double-quote mark (")
	// &apos; refers to an apostrophe (')
	while ( *content != '\0' ) {
		switch ( *content ) {
			case '&':
				_ostr << "&amp;";
				break;
			case '<':
				_ostr << "&lt;";
				break;
			case '>':
				_ostr << "&gt;";
				break;
			case '\'':
				_ostr << "&apos;";
				break;
			case '\"':
				_ostr << "&quot;";
				break;
			default:
				_ostr << *content;
		}
		++content;
	}
}


void Exporter::put(const char *content) {
	if ( _tagOpen ) {
		_ostr << ">";
		_tagOpen = false;
	}
	writeString(content);
}


}
}
}

