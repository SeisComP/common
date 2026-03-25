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
#include <seiscomp/core/optional.h>

#include <ostream>


namespace Seiscomp {
namespace Core {


class SC_SYSTEM_CORE_API TimeWindow {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		TimeWindow() = default;
		TimeWindow(const Time &startTime, double length);
		TimeWindow(const Time &startTime, const TimeSpan length);
		TimeWindow(const Time &startTime, const Time &endTime);
		TimeWindow(const TimeWindow &other);


	// ----------------------------------------------------------------------
	//  Assignment operators
	// ----------------------------------------------------------------------
	public:
		TimeWindow &operator=(const TimeWindow&);


	// ----------------------------------------------------------------------
	//  Comparison operators
	// ----------------------------------------------------------------------
	public:
		bool operator==(const TimeWindow&) const;
		bool operator!=(const TimeWindow&) const;


	// ----------------------------------------------------------------------
	//  Arithmetic operators
	// ----------------------------------------------------------------------
	public:
		//! Returns the minimal timewindow including this and other
		TimeWindow  operator|(const TimeWindow &other) const;
		//! Sets the minimal timewindow including this and other
		TimeWindow &operator|=(const TimeWindow &other);

		//! Returns the intersection of this and other
		TimeWindow  operator&(const TimeWindow &other) const;
		//! Sets the intersection of this and other
		TimeWindow &operator&=(const TimeWindow &other);


	// ----------------------------------------------------------------------
	//  Cast operators
	// ----------------------------------------------------------------------
	public:
		//! Returns if the time window has length larger than 0.
		operator bool() const;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
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
		bool equals(const TimeWindow &tw, TimeSpan tolerance = TimeSpan(0, 0)) const;

		//! does it overlap with time window tw?
		bool overlaps(const TimeWindow &tw) const;

		//! test if this+other would form a contiguous time window
		bool contiguous(const TimeWindow &, TimeSpan tolerance = TimeSpan(0, 0)) const;

		//! Sets the intersection time window with this and other
		TimeWindow &overlap(const TimeWindow &other);

		//! Computes the intersection with time window other
		TimeWindow overlapped(const TimeWindow &other) const;

		//! Merges other into this to the minimal timewindow overlapping both.
		TimeWindow &merge(const TimeWindow &other);

		//! Returns the minimal timewindow including this and other
		TimeWindow merged(const TimeWindow &other) const;


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		Time _startTime;
		Time _endTime;
};



class SC_SYSTEM_CORE_API OpenTimeWindow {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		OpenTimeWindow() = default;
		OpenTimeWindow(const OPT(Time) &startTime, const OPT(Time) &endTime);
		OpenTimeWindow(const OpenTimeWindow &other);


	// ----------------------------------------------------------------------
	//  Assignment operators
	// ----------------------------------------------------------------------
	public:
		OpenTimeWindow &operator=(const OpenTimeWindow&);


	// ----------------------------------------------------------------------
	//  Comparison operators
	// ----------------------------------------------------------------------
	public:
		bool operator==(const OpenTimeWindow&) const;
		bool operator!=(const OpenTimeWindow&) const;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		OPT(Time) startTime() const;
		OPT(Time) endTime() const;
		OPT(TimeSpan) length() const;

		void set(const OPT(Time) &t1, const OPT(Time) &t2);
		void setStartTime(const OPT(Time) &t);
		void setEndTime(const OPT(Time) &t);

		//! does it contain time t?
		bool contains(const Time &t) const;

		//! does it contain time window tw completely?
		bool contains(const OpenTimeWindow &tw) const;

		//! does it overlap with time window tw?
		bool overlaps(const OpenTimeWindow &tw) const;


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		OPT(Time) _startTime;
		OPT(Time) _endTime;
};


std::ostream &operator<<(std::ostream &os, const TimeWindow &timeWindow);
std::ostream &operator<<(std::ostream &os, const OpenTimeWindow &timeWindow);


#include "timewindow.ipp"


}
}


#endif
