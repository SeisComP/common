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

/**
 * @brief The RecordFilterInterface class defines an interface to process
 * records.
 *
 * There are actually two methods involved: push and flush. A filter might
 * produce:
 * 1. the same amount of output records as input records
 * 2. less output records than input records
 * 3. more output records than input records
 *
 * Case 1 and 2 is trivial. Whenever a record is pushed, the method returns
 * either a new record or nothing (nullptr). Case 3 requires that after a
 * record was fed, nullptr must be fed as long as a valid records is being
 * returned.
 *
 * A generic way to implement the consumer part is the following code
 * snippet:
 *
 * @code
 * auto out = filter.feed(rec)
 * while ( out ) {
 *     // Do something with out
 *     out = filter.feed(nullptr)
 * }
 * @endcode
 *
 * Once feeding has finished, call flush to retrieve possible pending records:
 *
 * @code
 * while ( (out = filter.flush()) ) {
 *     // Do something with out
 * }
 * @endcode
 */
class SC_SYSTEM_CORE_API RecordFilterInterface : public Seiscomp::Core::BaseObject {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		virtual ~RecordFilterInterface();


	// ------------------------------------------------------------------
	//  Interface
	// ------------------------------------------------------------------
	public:
		//! Can return a copy of the filtered record. Some filters might
		//! collect more data until a record is output so a return of
		//! nullptr is *not* an error. Other filters might produce more
		//! output records than input records. Calling feed(nullptr) will
		//! return pending records as long as nullptr is returned.
		//! Call flush() if no more records are
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
