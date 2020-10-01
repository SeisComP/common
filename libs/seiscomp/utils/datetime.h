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


#ifndef SEISCOMP_UTILS_DATETIME_H
#define SEISCOMP_UTILS_DATETIME_H


#include <seiscomp/core/datetime.h>
#include <seiscomp/core.h>

namespace Seiscomp {
namespace Util {


/**
 * Returns the number of seconds passed since the
 * last day of a given time has been started.
 * @param time The given time
 * @return The number of seconds including microseconds
 */
SC_SYSTEM_CORE_API double timeOfDay(const Seiscomp::Core::Time& time);

/**
 * Returns the number of seconds passed since the current
 * day has been started.
 * @return The number of seconds including microseconds
 */
SC_SYSTEM_CORE_API double timeOfDay();


struct TimeVal {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int usec;
	TimeVal() :
		year(0), month(0), day(0), hour(0), minute(0), second(), usec(0) {
	}
};

SC_SYSTEM_CORE_API bool getTime(const Core::Time& time, TimeVal& tv);

SC_SYSTEM_CORE_API void setTime(Core::Time& time, const TimeVal& tv);

}
}

#endif
