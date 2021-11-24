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


#ifndef SEISCOMP_WIRED_DEVICE_H
#define SEISCOMP_WIRED_DEVICE_H

#include <seiscomp/core/platform/platform.h>
#include <seiscomp/core/interruptible.h>

#if defined(SC_HAS_EPOLL)
	#define SEISCOMP_WIRED_EPOLL
#elif defined(SC_HAS_KQUEUE)
	#define SEISCOMP_WIRED_KQUEUE
#else
	#error "Require either epoll or kqueue support"
#endif

#ifdef SEISCOMP_WIRED_SELECT
  #ifndef WIN32
    #include <sys/select.h>
  #endif
#endif
#ifdef SEISCOMP_WIRED_EPOLL
  #ifndef WIN32
    #include <sys/epoll.h>
  #endif
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
  #include <sys/types.h>
  #include <sys/event.h>
#endif

#include <seiscomp/core/baseobject.h>

#include <list>
#include <stdint.h>
#include <functional>


namespace Seiscomp {
namespace Wired {


class DeviceGroup;
class Session;


/**
 * Helper class to wrap any type of object and output it anomymized, e.g.
 * IP addresses, password and so on
 */
template <typename T>
struct Anonymize {
	Anonymize(const T &t) : target(t) {}
	const T &target;
};


template <typename T>
Anonymize<T> anonymize(const T &t) {
	return Anonymize<T>(t);
}


DEFINE_SMARTPOINTER(Device);

class SC_SYSTEM_CORE_API Device : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public enumerations
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief The Mode enum holds the states of a device.
		 */
		enum Mode {
			Idle      = 0x00,
			Read      = 0x01,
			Write     = 0x02,
			ReadWrite = Read | Write,
			Exception = 0x04,
			Closed    = 0x80
		};

		enum Status {
			Success = 0,
			Error,
			InvalidDevice,
			AllocationError
		};

		typedef uint64_t count_t;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Device();

		/**
		 * @brief Device constructor
		 * @param fd The file descriptor of the device
		 */
		Device(int fd);

		//! D'tor
		~Device() override;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void setSession(Session *session) { _session = session; }
		Session *session() const { return _session; }

		//! Returns whether the device is valid (functioning) or
		//! invalid (closed or malfunctioning)
		bool isValid() const;

		void setMode(int m);
		void addMode(int m);
		void removeMode(int m);
		int mode() const { return _selectMode; }

		//! Sets a timeout when this device is active in a device group.
		//! A negative disables the timeout. Timeouts relative values that
		//! will be evaluated before the next events are checked.
		//! setTimeout must not be called from a different thread in which
		//! the reactor is running.
		bool setTimeout(int milliseconds);

		int selectMode() const { return _selectMode; }

		virtual void close() = 0;
		virtual ssize_t write(const char *data, size_t len) = 0;
		virtual ssize_t read(char *data, size_t len) = 0;

		/**
		 * @brief Returns the current file descriptor and sets the internal
		 *        file descriptor to invalid
		 * @return The current file descriptor
		 */
		int takeFd();
		int fd() const;

		virtual Status setNonBlocking(bool nb);


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		Session     *_session;

		int          _fd;
		int          _selectMode;
#ifdef SEISCOMP_WIRED_KQUEUE
		int          _activeMode;
#endif

		static int   _deviceCount;

	private:
#ifdef SEISCOMP_WIRED_SELECT
		typedef Device                 *Item;
		typedef std::list<Item>         List;
		typedef List::iterator          Iterator;

		Iterator     _groupIterator;
#endif
		DeviceGroup   *_group;  //<! The group the device is part of
		Device        *_qPrev;  //<! The prev pointer used by intrusive list
		Device        *_qNext;  //<! The next pointer used by intrusive list
		// Store timeout in milliseconds.
		// A negative value means no timeout.
		int            _timeout;
		int            _ticker;

	friend class DeviceGroup;
};


class SC_SYSTEM_CORE_API DeviceGroup : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public enumerations and types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief The TriggerMode enum defined how the device group unblocks,
		 *        either edge triggered only when the mode changes or level
		 *        triggered as long as the mode is not idle.
		 */
		enum TriggerMode {
			EdgeTriggered,
			LevelTriggered
		};

		typedef std::function<void ()> TimeoutFunc;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		DeviceGroup();

		//! D'tor
		~DeviceGroup() override;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @return Whether the device group is initialized or not
		 */
		bool isValid() const;

		/**
		 * @brief Sets up a device group by allocating its resources.
		 * @return true on success, false otherwise
		 */
		bool setup();

		//! Appends a device to a group. If the timeout of a device is
		//! activated this call is not thread-safe and must not be called
		//! from a different thread. Additional synchin' is required then.
		bool append(Device *);

		//! Removes a device from a group. If the timeout of a device is
		//! activated this call is not thread-safe and must not be called
		//! from a different thread. Additional synchin' is required then.
		bool remove(Device *);

		size_t count() const;

		void clear();

		bool setTimer(uint32_t seconds, uint32_t milliseconds, TimeoutFunc func);
		bool clearTimer();

		/**
		 * @brief Interrupt the blocking wait.
		 */
		void interrupt();

		Device *wait();
		Device *next();

		bool readable() const;
		bool writable() const;
		bool timedOut() const;
		bool interrupted() const { return _isInterrupted; }

		//! Set the trigger mode which is only supported when epoll is
		//! used. The default trigger mode is then EdgeTriggered which
		//! is different to select that is level triggered.
		bool setTriggerMode(TriggerMode);
		TriggerMode triggerMode() const;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		void applyTimeout(Device *d);
		void removeFromQueue(Device *d);

		void updateState(Device *);
		void clearState(Device *);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
#ifdef SEISCOMP_WIRED_SELECT
		Device::List         _devices;

#endif
		TriggerMode          _triggerMode;
		bool                 _readyForRead;
		bool                 _readyForWrite;
		bool                 _timedOut;
		bool                 _isInSelect;
		bool                 _isInterrupted;

		uint32_t             _timerSeconds;
		uint32_t             _timerMilliseconds;
		bool                 _timerSingleShot;
		TimeoutFunc          _fnTimeout;
		int                  _timerFd;

		int                  _interrupt_read_fd;
		int                  _interrupt_write_fd;

#ifdef SEISCOMP_WIRED_SELECT
		Device::Iterator     _selectIterator;
		fd_set               _read_set;
		fd_set               _write_set;

		fd_set               _read_active_set;
		fd_set               _write_active_set;
#endif
#if defined(SEISCOMP_WIRED_EPOLL) || defined(SEISCOMP_WIRED_KQUEUE)
		unsigned int         _defaultOps;
		size_t               _count;
#ifdef SEISCOMP_WIRED_EPOLL
#define SEISCOMP_WIRED_EPOLL_EVENT_BUFFER 10
		int                  _epoll_fd;
		struct epoll_event   _epoll_events[SEISCOMP_WIRED_EPOLL_EVENT_BUFFER];
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
#define SEISCOMP_WIRED_KQUEUE_EVENT_BUFFER 10
		int                  _kqueue_fd;
		struct kevent        _kqueue_events[SEISCOMP_WIRED_KQUEUE_EVENT_BUFFER];
#endif
		size_t               _selectIndex;
		size_t               _selectSize;
#endif
		int                  _lastCallDuration;
		Device              *_queue;
		Device              *_nextQueue;

	friend class Device;
};


inline bool DeviceGroup::readable() const {
	return _readyForRead;
}

inline bool DeviceGroup::writable() const {
	return _readyForWrite;
}

inline bool DeviceGroup::timedOut() const {
	return _timedOut;
}


}
}


#endif
