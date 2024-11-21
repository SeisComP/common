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


#ifndef SC_CORE_TIMESPAN_H
#define SC_CORE_TIMESPAN_H

#include <seiscomp/core.h>
#include <seiscomp/core/datetime.h>

#include <ostream>


namespace Seiscomp {
namespace Core {


class SC_SYSTEM_CORE_API TimeWindow {
	//  X'truction
	public:
		TimeWindow();
		TimeWindow(const Time &startTime, double length);
		TimeWindow(const Time &startTime, const TimeSpan length);
		TimeWindow(const Time &startTime, const Time &endTime);
		TimeWindow(const TimeWindow &tw);
		~TimeWindow() {}

	//  Operators
	public:
		TimeWindow &operator=(const TimeWindow&);
		bool operator==(const TimeWindow&) const;
		bool operator!=(const TimeWindow&) const;

		//! Returns the minimal timewindow including this and other
		TimeWindow operator|(const TimeWindow &other) const;

		//! Returns if the time window has length larger than 0.
		operator bool() const;

	//  Interface
	public:
		Time startTime() const;
		Time endTime() const;
		TimeSpan length() const;

		void set(const Time &t1, const Time &t2);
		void setStartTime(const Time &t);
		void setEndTime(const Time &t);
		//! set length in seconds, affects endTime
		void setLength(TimeSpan length);

		//! does it contain time t?
		bool contains(const Time &t) const;

		//! does it contain time window tw completely?
		bool contains(const TimeWindow &tw) const;

		//! is equal to time window?
		//! +/- tolerance in seconds
		bool equals(const TimeWindow &tw, double tolerance=0.0) const;

		//! does it overlap with time window tw?
		bool overlaps(const TimeWindow &tw) const;

		//! compute overlap with time window tw
		TimeWindow overlap(const TimeWindow &tw) const;

		//! test if this+other would form a contiguous time window
		bool contiguous(const TimeWindow&, double tolerance=0) const;

		//! extend time window by appending the other (without check!)
		void extend(const TimeWindow&);

		//! merges this and other to the minimal timewindow overlapping both
		TimeWindow merge(const TimeWindow&) const;

	//  Implementation
	private:
		Time _startTime, _endTime;
};


std::ostream &operator<<(std::ostream &os, const TimeWindow &timeWindow);


#include "timewindow.ipp"


}
}


#endif
