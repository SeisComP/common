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


#ifndef SEISCOMP_IO_IMPORTER_H
#define SEISCOMP_IO_IMPORTER_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/core.h>

#include <streambuf>
#include <string>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(Importer);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An abstract importer interface and factory

	This class provides an interface to import foreign (meta) data
	formats such as QuakeML.
	\endcode
 */
class SC_SYSTEM_CORE_API Importer : public Core::BaseObject {
	DECLARE_SC_CLASS(Importer);
 
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Importer();


	public:
		//! Destructor
		virtual ~Importer();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		static Importer *Create(const char *type);

		Core::BaseObject *read(std::streambuf* buf);
		Core::BaseObject *read(std::string filename);

		bool withoutErrors() const;

	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		//! Interface method that must be implemented by real importers.
		virtual Core::BaseObject *get(std::streambuf* buf) = 0;


	protected:
		bool _hasErrors;
};


DEFINE_INTERFACE_FACTORY(Importer);

#define REGISTER_IMPORTER_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::Importer, Class> __##Class##InterfaceFactory__(Service)

}
}


#endif
