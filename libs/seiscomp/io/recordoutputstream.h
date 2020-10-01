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


#ifndef SEISCOMP_IO_RECORDOUTPUTSTREAM_H
#define SEISCOMP_IO_RECORDOUTPUTSTREAM_H

#include <iostream>
#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/record.h>
#include <seiscomp/io/recordstreamexceptions.h>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(RecordOutputStream);

class SC_SYSTEM_CORE_API RecordOutputStream : public Seiscomp::Core::InterruptibleObject {
	DECLARE_SC_CLASS(RecordOutputStream);

	protected:
		RecordOutputStream();

	public:
		virtual ~RecordOutputStream() {}

	public:
		virtual bool setTarget(std::string) = 0;
		virtual void close() = 0;

		virtual std::ostream& stream() = 0;

		//! Returns a record stream for the given service
		//! @return A pointer to the recordstream object
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordOutputStream* Create(const char* service);

		//! Returns a record stream for the given service that creates
		//! records of type recordType
		//! @return A pointer to the recordstream object. If the recordstream
		//!         does not support the requested type, nullptr will be returned
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordOutputStream* Create(const char* service, const char* recordType);

		//! Opens a recordstream at source.
		//! @param url A source URL of format [service://]address[#type],
		//!            e.g. file:///data/record.mseed#mseed. service defaults to
		//!            'file' and the default type is 'mseed'
		//! @return A pointer to the recordstream object. If the recordstream
		//!         does not support the requested type, nullptr will be returned
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static RecordOutputStream* Open(const char* url);
};


DEFINE_INTERFACE_FACTORY(RecordOutputStream);


#define REGISTER_RECORDOUTPUTSTREAM(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::RecordOutputStreamFactory, Class> __##Class##InterfaceFactory__(Service)

}
}

#endif
