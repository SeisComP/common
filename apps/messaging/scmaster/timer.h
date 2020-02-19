/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#ifndef SEISCOMP_SCMASTER_DEVICES_TIMER_H__
#define SEISCOMP_SCMASTER_DEVICES_TIMER_H__


#include <seiscomp/wired/devices/fd.h>
#include <seiscomp/wired/session.h>

#include <functional>


namespace Seiscomp {
namespace Wired {


class Timer : public Wired::FileDescriptor {
	public:
		Timer();


	public:
		/**
		 * @brief setInterval
		 * @param seconds
		 * @param nanoseconds
		 * @return
		 */
		bool setInterval(uint32_t seconds, uint32_t nanoseconds);
};



DEFINE_SMARTPOINTER(TimerSession);

class TimerSession : public Wired::Session {
	public:
		typedef std::function<void ()> TimeoutFunc;

	public:
		TimerSession(TimeoutFunc func);

	public:
		bool setInterval(uint32_t seconds, uint32_t nanoseconds);

	public:
		virtual void update();

	private:
		Timer       _timerDevice;
		TimeoutFunc _timeout;
};


}
}


#endif
