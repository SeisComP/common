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


#ifndef SEISCOMP_WIRED_REACTOR_H
#define SEISCOMP_WIRED_REACTOR_H


#include <seiscomp/wired/devices/socket.h>
#include <seiscomp/core/list.h>

#include <mutex>


namespace Seiscomp {
namespace Wired {


DEFINE_SMARTPOINTER(Session);
DEFINE_SMARTPOINTER(Reactor);

typedef Core::Generic::IntrusiveList<SessionPtr> SessionList;

/**
 * @brief The Reactor class is the core event loop.
 *
 * The reactor waits for activity on any of its managed devices and dispatches
 * the events. It basically calls the update() method on a session thats device
 * is ready to read or write. In case of an exception the device is closed and
 * the session is being removed.
 *
 * The underlying implementation uses either epoll on Linux or kqueue on BSD
 * or OSX.
 */
class SC_SYSTEM_CORE_API Reactor : public Seiscomp::Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public enumerations
	// ----------------------------------------------------------------------
	public:
		typedef DeviceGroup::TriggerMode TriggerMode;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Reactor();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Adds a session to the reactor.
		virtual bool addSession(Session *session);

		//! Schedules adding a session to the reactor. The session will be added
		//! in the same thread the reactors run loop is running. Call interrupt
		//! after all sessions have been added.
		//! @note This method is only useful when adding a session from a different
		//!       thread.
		virtual bool addSessionDeferred(Session *session);

		//! Removes a session from the reactor
		virtual bool removeSession(Session *session);

		//! Moves a session from this to another reactor. The session
		//! is going to be removed from this reactor and will be scheduled
		//! in the target reactor. Call interrupt afterwards on the
		//! target reactor.
		//! @note This method is only useful when both reactors are running
		//!       in different threads.
		void moveTo(Reactor *target, Session *session);

		//! Returns the number of sessions in this reactor
		size_t count() const;

		bool setTimer(uint32_t seconds, uint32_t milliseconds, DeviceGroup::TimeoutFunc func);
		bool clearTimer();

		//! Sets up the device polling infrastructure and returns a success
		//! flag.
		bool setup();

		//! Run the reactor blocking
		virtual bool run();
		bool isRunning() const { return _isRunning; }

		virtual Device *wait();

		//! Returns if the reactor has been interrupted or not
		bool interrupted() const { return _devices.interrupted(); }

		//! The method is called between DeviceGroup::wait
		virtual void idle();

		//! Shutdown the reactor causing to terminate
		//! the run loop and closing all handled sessions
		virtual void shutdown();

		//! Clean up all sessions and sockets
		virtual void clear();

		//! Synchronizes a release of a session smart pointer in
		//! a threaded environment and sets the passed ptr to nullptr.
		void release(SessionPtr &ptr);

		//! Interrupts the reactors wait causing to idle method to be called
		void interrupt();

		//! Interrupts the reactor causing the run loop to terminate.
		//! The session sockets are not closed and resume can
		//! be used to continue operation.
		void stop();

		//! Clears the stop state and allows run to be called again.
		void resume();

		bool setTriggerMode(TriggerMode);
		TriggerMode triggerMode() const;

		const SessionList &sessions() const;

		const DeviceGroup *devices() const;

		/**
		 * @brief getBuffer returns a temporary buffer that sessions can use
		 *        to read from device.
		 * @param buf Address that is populated with the buffer address
		 * @param len Length will hold the length of the buffer
		 */
		void getBuffer(char *&buf, size_t &len);


	// ----------------------------------------------------------------------
	//  Protected events
	// ----------------------------------------------------------------------
	protected:
		virtual void sessionAdded(Session *session);
		virtual void sessionRemoved(Session *session);
		virtual void sessionTagged(Session *session);


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		mutable std::mutex _mutex;
		mutable std::mutex _sessionMutex;
		std::vector<char>  _buffer;
		bool               _shouldRun;
		bool               _isRunning;
		SessionList        _sessions;
		SessionList        _deferredSession;
		DeviceGroup        _devices;
};


}
}


#endif
