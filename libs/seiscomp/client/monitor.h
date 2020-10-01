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


#ifndef SEISCOMP_UTILS_MONITOR_H
#define SEISCOMP_UTILS_MONITOR_H

#include <seiscomp/client.h>
#include <seiscomp/core/datetime.h>
#include <string>
#include <vector>
#include <list>


namespace Seiscomp {
namespace Client {


//! A running average calculator that logs the number of
//! objects/thingies in a certain interval. The accuracy is
//! a second.
class SC_SYSTEM_CLIENT_API RunningAverage {
	public:
		RunningAverage(int timeSpanInSeconds);


	public:
		int timeSpan() const { return _timeSpan; }

		void push(const Core::Time &time, size_t count = 1);

		//! Returns the current count per time span.
		int count(const Core::Time &time) const;

		//! Returns the value (average) per time span.
		double value(const Core::Time &time) const;

		//! Returns the timestamp of the last values pushed
		Core::Time last() const;

		void dumpBins() const;


	private:
		Core::Time          _first;
		Core::Time          _last;
		size_t              _timeSpan;
		double              _scale;
		mutable
		double              _shift;
		mutable
		std::vector<size_t> _bins;
		size_t              _front;
};


class SC_SYSTEM_CLIENT_API ObjectMonitor {
	public:
		typedef RunningAverage Log;

		ObjectMonitor(int timeSpanInSeconds);
		~ObjectMonitor();


	public:
		Log *add(const std::string &name, const std::string &channel = "");
		void update(const Core::Time &time);


	public:
		struct Test {
			std::string  name;
			std::string  channel;
			Core::Time   updateTime;
			size_t       count;
			Log         *test;
		};

		typedef std::list<Test> Tests;
		typedef Tests::const_iterator const_iterator;

		const_iterator begin() const;
		const_iterator end() const;

		size_t size() const;


	private:
		Tests _tests;
		int   _timeSpan;
};


}
}


#endif
