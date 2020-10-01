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


#ifndef SEISCOMP_IO_XMLEXPORTER_H
#define SEISCOMP_IO_XMLEXPORTER_H


#include <seiscomp/io/xml/handler.h>
#include <seiscomp/io/exporter.h>

#include <ostream>
#include <map>


namespace Seiscomp {
namespace IO {
namespace XML {


class SC_SYSTEM_CORE_API Exporter : public IO::Exporter, public OutputHandler {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Exporter();


	// ----------------------------------------------------------------------
	// Public Interface
	// ----------------------------------------------------------------------
	public:
		TypeMap* typeMap();
		void setTypeMap(TypeMap *map);

	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		//! Sets the required name of the root tag.
		//! If empty no header is used.
		void setRootName(std::string);

		virtual void collectNamespaces(Core::BaseObject *);

		//! Interface method that must be implemented by real exporters.
		virtual bool put(std::streambuf* buf, Core::BaseObject *);
		virtual bool put(std::streambuf* buf, const ExportObjectList &);


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		void handle(Core::BaseObject *, const char *tag, const char *ns, NodeHandler *);
		bool openElement(const char *name, const char *ns);
		void addAttribute(const char *name, const char *ns, const char *value);
		void closeElement(const char *name, const char *ns);

		void put(const char *content);

		void writeString(const char *str);


	protected:
		// Maps a namespace to its prefix
		typedef std::map<std::string, std::string> NamespaceMap;

		NamespaceMap _defaultNsMap;
		NamespaceMap _namespaces;


	private:
		std::string  _headerNode;
		std::ostream _ostr;
		TypeMap     *_typemap;
		int          _lastTagState;
		int          _indent;
		bool         _tagOpen;
		bool         _firstElement;
};


}
}
}

#endif
