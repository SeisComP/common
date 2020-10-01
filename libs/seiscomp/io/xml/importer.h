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


#ifndef SEISCOMP_IO_XMLIMPORTER_H
#define SEISCOMP_IO_XMLIMPORTER_H


#include <seiscomp/io/xml/handler.h>
#include <seiscomp/io/importer.h>


namespace Seiscomp {
namespace IO {
namespace XML {


class SC_SYSTEM_CORE_API Importer : public IO::Importer {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Importer();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		TypeMap* typeMap();
		void setTypeMap(TypeMap *map);

		//! Enables/disables strict namespace checking.
		//! If disabled, tags will be accepted even if the
		//! registered namespace doesn't match and the tag
		//! is only registered with one namespace. Tags with
		//! multiple namespaces will still fail.
		void setStrictNamespaceCheck(bool);

	// ----------------------------------------------------------------------
	// Protected Inteface
	// ----------------------------------------------------------------------
	protected:

		//! Sets the required name of the root tag.
		//! If empty no header is used.
		void setRootName(std::string);

		//! Interface method that must be implemented by real importers.
		virtual Core::BaseObject *get(std::streambuf* buf);


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		bool traverse(NodeHandler *handler,
		              void *node, void *childs,
		              Core::BaseObject *target);


	private:
		static NoneHandler _none;
		GenericHandler _any;
		bool _strictNamespaceCheck;

		Core::BaseObject *_result;
		std::string _headerNode;
		TypeMap *_typemap;
};


}
}
}

#endif
