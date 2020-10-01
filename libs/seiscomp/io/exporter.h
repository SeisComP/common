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


#ifndef SEISCOMP_IO_EXPORTER_H
#define SEISCOMP_IO_EXPORTER_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/core.h>

#include <streambuf>


namespace Seiscomp {
namespace IO {


struct SC_SYSTEM_CORE_API ExportSink {
	virtual ~ExportSink() {}
	virtual int write(const char *data, int size) { return 0; }
};


typedef std::vector<Core::BaseObject*> ExportObjectList;


DEFINE_SMARTPOINTER(Exporter);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An abstract exporter interface and factory

	This class provides an interface to export sc3 (meta) data
	formats into any other format such as QuakeML.
	\endcode
 */
class SC_SYSTEM_CORE_API Exporter : public Core::BaseObject {
	DECLARE_SC_CLASS(Exporter);


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Exporter();


	public:
		//! Destructor
		virtual ~Exporter();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		static Exporter *Create(const char *type);

		void setFormattedOutput(bool enable);
		void setIndent(int);

		bool write(std::streambuf* buf, Core::BaseObject *);
		bool write(std::string filename, Core::BaseObject *);

		//! Converts the object using the Sink interface to write
		//! the data.
		bool write(ExportSink *sink, Core::BaseObject *);

		bool write(std::streambuf* buf, const ExportObjectList &objects);
		bool write(std::string filename, const ExportObjectList &objects);

		//! Converts the objects using the Sink interface to write
		//! the data.
		bool write(ExportSink *sink, const ExportObjectList &objects);


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		//! Interface method that must be implemented by real exporters.
		virtual bool put(std::streambuf* buf, Core::BaseObject *) = 0;

		//! Interface method that should be implemented by real exporters. The
		//! default implementation does nothing and returns false. Since that
		//! method has been introduced with API 12 it is not abstract to
		//! maintain compilation of existing exporters.
		virtual bool put(std::streambuf* buf, const ExportObjectList &objects);


	protected:
		bool _prettyPrint;
		int  _indentation;
};


DEFINE_INTERFACE_FACTORY(Exporter);

#define REGISTER_EXPORTER_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::Exporter, Class> __##Class##InterfaceFactory__(Service)

}
}


#endif
