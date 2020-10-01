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


#ifndef SEISCOMP_IO_RECORDFILTER_H
#define SEISCOMP_IO_RECORDFILTER_H

#include <seiscomp/core.h>
#include <seiscomp/core/genericrecord.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(RecordFilterInterface);

class SC_SYSTEM_CORE_API RecordFilterInterface : public Seiscomp::Core::BaseObject {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		virtual ~RecordFilterInterface();


	// ------------------------------------------------------------------
	//  Interface
	// ------------------------------------------------------------------
	public:
		//! Can return a copy of the filtered record. Some filters might
		//! collect more data until a record is output so a return of
		//! nullptr is *not* an error. Call flush() if no more records are
		//! expected to be fed.
		//! @return A copy of a filtered record
		virtual Record *feed(const Record *rec) = 0;

		//! Requests to flush pending data. Flush should be called until
		//! nullptr is returned to flush all pending records.
		//! @return A copy of the flushed record
		virtual Record *flush() = 0;

		//! Resets the record filter.
		virtual void reset() = 0;

		//! Clones a filter and must preserve currently configured parameters
		//! but not the states (e.g. last record time). Basically clone must
		//! result in the same as copying the instance and calling reset.
		virtual RecordFilterInterface *clone() const = 0;
};


}
}

#endif
