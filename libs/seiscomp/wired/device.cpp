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


#define SEISCOMP_COMPONENT Wired
#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/wired/device.h>

#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

#if defined(SC_HAS_EVENTFD)
#include <sys/eventfd.h>
#endif

#if defined(WIN32)
#include <io.h>
#endif

#if defined(SEISCOMP_WIRED_EPOLL)
#include <sys/timerfd.h>
#include <linux/version.h>
#endif

#include <iostream>
#include <algorithm>
#include <cerrno>
#include <cstring>


using namespace std;

// If EPOLLRDHUP is not defined, don't use it
#if defined(SEISCOMP_WIRED_EPOLL) || defined(SEISCOMP_WIRED_KQUEUE)
#include <sys/time.h>

#ifdef SEISCOMP_WIRED_EPOLL

#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0
#endif

#endif

#endif
namespace {

/*
bool lessThan(const struct timeval *tv1, const struct timeval *tv2) {
	if ( tv1->tv_sec < tv2->tv_sec ) return true;
	if ( tv1->tv_sec > tv2->tv_sec ) return false;
	return tv1->tv_usec < tv2->tv_usec;
}
*/

// Returns milliseconds
int tv_sub_ms(const struct timeval *tv1, const struct timeval *tv2) {
	int64_t msdiff = static_cast<int64_t>((tv1->tv_sec - tv2->tv_sec) * 1000);
	int64_t udiff = static_cast<int64_t>((tv1->tv_usec - tv2->tv_usec) / 1000);
	if ( udiff < 0 ) msdiff -= udiff;
	return static_cast<int>(msdiff);
}

/*
int tv_to_ms(const struct timeval *tv) {
	return tv->tv_sec*1000 + tv->tv_usec/1000;
}
*/



}


namespace Seiscomp {
namespace Wired {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Device::_deviceCount = 0;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Device::Device() : _fd(-1) {
	++_deviceCount;
	_selectMode = Idle;
#ifdef SEISCOMP_WIRED_KQUEUE
	_activeMode = Idle;
#endif
	_group = nullptr;
	_qPrev = _qNext = nullptr;
	_timeout = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Device::Device(int fd) : _fd(fd) {
	++_deviceCount;
	_selectMode = Idle;
#if defined(SEISCOMP_WIRED_EPOLL) || defined(SEISCOMP_WIRED_KQUEUE)
	_group = nullptr;
	_qPrev = _qNext = nullptr;
	_timeout = -1;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Device::~Device() {
	--_deviceCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Device::isValid() const {
	return _fd != -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Device::setMode(int m) {
	if ( _selectMode == m ) return;
	_selectMode = m;
	if ( _group ) _group->updateState(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Device::addMode(int m) {
	setMode(_selectMode | m);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Device::removeMode(int m) {
	setMode(_selectMode & ~m);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Device::setTimeout(int milliseconds) {
	if ( _timeout == milliseconds ) return true;
	_timeout = milliseconds;
	if ( _group ) _group->applyTimeout(this);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Device::Status Device::setNonBlocking(bool nb) {
	if ( !isValid() )
		return InvalidDevice;

#ifndef WIN32
		int flags = fcntl(_fd, F_GETFL, 0);

	if ( nb )
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if ( fcntl(_fd, F_SETFL, flags) == -1 )
		return Error;

	return Success;
#else
	return Error;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Device::takeFd() {
	auto fd = _fd;
	_fd = -1;
	return fd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Device::fd() const {
	return _fd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DeviceGroup::DeviceGroup() {
	_interrupt_read_fd = _interrupt_write_fd = -1;
	_timerFd = -1;
	_timerSingleShot = false;
	_queue = nullptr;
	_isInSelect = false;
	_triggerMode = LevelTriggered;
#ifdef SEISCOMP_WIRED_EPOLL
	_epoll_fd = -1;
	_defaultOps = DEFAULT_EPOLL_OP;
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	_kqueue_fd = -1;
	_defaultOps = DEFAULT_KQUEUE_OP;
#endif
#if defined(SEISCOMP_WIRED_EPOLL) || defined(SEISCOMP_WIRED_KQUEUE)
	_selectIndex = 0;
	_selectSize = 0;
	_count = 0;
	_triggerMode = EdgeTriggered;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DeviceGroup::~DeviceGroup() {
#ifdef SEISCOMP_WIRED_EPOLL
	if ( _epoll_fd > 0 ) {
		::close(_epoll_fd);
		_epoll_fd = -1;
	}

	if ( _timerFd > 0 ) {
		::close(_timerFd);
		_timerFd = -1;
	}
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	if ( _kqueue_fd > 0 ) {
		::close(_kqueue_fd);
		_kqueue_fd = -1;
	}

#endif
	if ( _interrupt_write_fd != -1 && _interrupt_write_fd != _interrupt_read_fd )
		::close(_interrupt_write_fd);
	if ( _interrupt_read_fd != -1 )
		::close(_interrupt_read_fd);

	_interrupt_read_fd = _interrupt_write_fd = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DeviceGroup::setTriggerMode(TriggerMode tm) {
#ifdef SEISCOMP_WIRED_EPOLL
	if ( tm == LevelTriggered ) {
		//SEISCOMP_DEBUG("[reactor] remove edge trigger mode");
		_defaultOps &= ~EPOLLET;
	}
	else
		_defaultOps |= EPOLLET;
	_triggerMode = tm;
	return true;
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	if ( tm == LevelTriggered ) {
		//SEISCOMP_DEBUG("[reactor] remove edge trigger mode");
		_defaultOps &= ~EV_CLEAR;
	}
	else
		_defaultOps |= EV_CLEAR;
	_triggerMode = tm;
	return true;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DeviceGroup::TriggerMode DeviceGroup::triggerMode() const {
	return _triggerMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DeviceGroup::isValid() const {
#ifdef SEISCOMP_WIRED_EPOLL
	return _epoll_fd > 0;
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	return _kqueue_fd > 0;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DeviceGroup::setup() {
#ifdef SEISCOMP_WIRED_EPOLL
	if ( _epoll_fd > 0 ) {
		SEISCOMP_ERROR("[reactor] already set up");
		return false;
	}
	else {
	#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 27)
		_epoll_fd = epoll_create(sizeof(_epoll_events)/sizeof(struct epoll_event)); // size hint is ignored since 2.6.8
	#else
		_epoll_fd = epoll_create1(EPOLL_CLOEXEC); // introduced in 2.6.27
	#endif
		if ( _epoll_fd == -1 ) {
			SEISCOMP_ERROR("epoll_create: %d: %s", errno, strerror(errno));
			return false;
		}
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	if ( _kqueue_fd > 0 ) {
		SEISCOMP_ERROR("[reactor] already set up");
		return false;
	}
	else {
		_kqueue_fd = kqueue();
		if ( _kqueue_fd == -1 ) {
			SEISCOMP_ERROR("kqueue create: %d: %s", errno, strerror(errno));
			return false;
		}
#endif
#ifdef SC_HAS_EVENTFD
		_interrupt_read_fd = _interrupt_write_fd = eventfd(0,0);
		if ( _interrupt_read_fd == -1 ) {
			SEISCOMP_ERROR("event_fd: %d: %s", errno, strerror(errno));
			return false;
		}

		//::fcntl(_interrupt_read_fd, F_SETFL, O_NONBLOCK);
#else
		int pipe_fds[2];
		if ( pipe(pipe_fds) == 0 ) {
			_interrupt_read_fd = pipe_fds[0];
			::fcntl(_interrupt_read_fd, F_SETFL, O_NONBLOCK);
			_interrupt_write_fd = pipe_fds[1];
			::fcntl(_interrupt_write_fd, F_SETFL, O_NONBLOCK);
		}
		else {
			SEISCOMP_ERROR("pipe: %d: %s", errno, strerror(errno));
			return false;
		}
#endif
#ifdef SEISCOMP_WIRED_EPOLL

		struct epoll_event ev;
		ev.events = _defaultOps | EPOLLIN;
		ev.data.ptr = nullptr;
		if ( epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _interrupt_read_fd, &ev) == -1 ) {
			SEISCOMP_ERROR("epoll_add1(%d, %d): %d: %s", _epoll_fd, _interrupt_read_fd, errno, strerror(errno));
			return false;
		}
#endif
#ifdef SEISCOMP_WIRED_KQUEUE

		struct kevent ev;
		EV_SET(&ev, _interrupt_read_fd, EVFILT_READ, EV_RECEIPT | EV_ADD | _defaultOps, 0, 0, 0);
		if ( kevent(_kqueue_fd, &ev, 1, nullptr, 0, nullptr) == -1 ) {
			SEISCOMP_ERROR("kqueue add: %d: %s", errno, strerror(errno));
			return false;
		}
#endif
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DeviceGroup::append(Device *dev) {
	if ( dev->_group ) {
		SEISCOMP_WARNING("Device is already part of a group");
		return false;
	}

	if ( !dev->isValid() ) return false;
#ifdef SEISCOMP_WIRED_EPOLL

	if ( _epoll_fd <= 0 && !setup() ) return false;

	if ( dev->_timeout >= 0 ) applyTimeout(dev);

	struct epoll_event ev;
	ev.events = _defaultOps;
	ev.data.ptr = dev;
	//SEISCOMP_DEBUG("[reactor] appending device");

	if ( epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, dev->fd(), &ev) == -1 ) {
		SEISCOMP_ERROR("epoll_add2(%d, %d): %d: %s", _epoll_fd, dev->fd(), errno, strerror(errno));
		return false;
	}

	//SEISCOMP_DEBUG("Adding device to group");
	++_count;
#endif
#ifdef SEISCOMP_WIRED_KQUEUE

	if ( _kqueue_fd <= 0 && !setup() ) return false;

	if ( dev->_timeout >= 0 ) applyTimeout(dev);

	//SEISCOMP_DEBUG("Adding device to group");
	++_count;
#endif

	dev->_group = this;
	updateState(dev);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DeviceGroup::remove(Device *s) {
#ifdef SEISCOMP_WIRED_EPOLL
	if ( s->_group != this ) {
		SEISCOMP_WARNING("Device is not part of that group");
		return false;
	}

	--_count;

	// Remove from event queue
	for ( size_t i = _selectIndex; i < _selectSize; ++i ) {
		// Is this device still queued in the epoll event list?
		if ( _epoll_events[i].data.ptr == s ) {
			// Remove it
			--_selectSize;
			if ( i < _selectSize ) {
				memcpy(
					_epoll_events + i,
					_epoll_events + i + 1,
					sizeof(struct epoll_event) * (_selectSize - i)
				);
			}
		}
	}
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	if ( s->_group != this ) {
		SEISCOMP_WARNING("Device is not part of that group");
		return false;
	}

	--_count;

	// Remove from event queue
	for ( size_t i = _selectIndex; i < _selectSize; ++i ) {
		// Is this device still queued in the epoll event list?
		if ( _kqueue_events[i].udata == s ) {
			// Remove it
			--_selectSize;
			if ( i < _selectSize )
				memcpy(_kqueue_events+i, _kqueue_events+i+1, sizeof(struct kevent)*(_selectSize-i));
		}
	}
#endif

	s->_group = nullptr;

#ifdef SEISCOMP_WIRED_EPOLL
	if ( _epoll_fd > 0 && s->isValid() ) {
		// Create a dummy event pointer for kernel version < 2.6.9
		struct epoll_event ev;
		if ( epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, s->fd(), &ev) == -1 )
			SEISCOMP_WARNING("epoll_del(%d): %d: %s",
			                 s->fd(), errno, strerror(errno));
	}

#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	if ( _kqueue_fd > 0 && s->isValid() ) {
		struct kevent ev[2];
		int event_count = 0;
		// Remove read and write events
		if ( s->_activeMode & Device::Read ) {
			EV_SET(&ev[event_count], s->fd(), EVFILT_READ, EV_DELETE, 0, 0, s);
			++event_count;
			s->_activeMode &= ~Device::Read;
		}

		if ( s->_activeMode & Device::Write ) {
			EV_SET(&ev[event_count], s->fd(), EVFILT_WRITE, EV_DELETE, 0, 0, s);
			++event_count;
			s->_activeMode &= ~Device::Write;
		}

		if ( event_count && (kevent(_kqueue_fd, ev, event_count, nullptr, 0, nullptr) == -1) )
			SEISCOMP_WARNING("kqueue del(%d): %d: %s", s->fd(), errno, strerror(errno));
	}

#endif
	// Remove from queue
	//cout << "Remove device " << s << " completely" << endl;
	if ( _nextQueue == s ) _nextQueue = _nextQueue->_qNext;
	if ( s->_qPrev || s->_qNext || s == _queue )
		removeFromQueue(s);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DeviceGroup::clear() {
#if defined(SEISCOMP_WIRED_EPOLL) || defined(SEISCOMP_WIRED_KQUEUE)
	_count = 0;
	_selectIndex = _selectSize = 0;
	#ifdef SEISCOMP_WIRED_EPOLL
	if ( _epoll_fd > 0 ) {
		::close(_epoll_fd);
		_epoll_fd = -1;
	}
	if ( _timerFd > 0 ) {
		::close(_timerFd);
		_timerFd = -1;
	}
	#endif
	#ifdef SEISCOMP_WIRED_KQUEUE
	if ( _kqueue_fd > 0 ) {
		::close(_kqueue_fd);
		_kqueue_fd = -1;
	}
	#endif
	if ( _interrupt_write_fd != -1 && _interrupt_write_fd != _interrupt_read_fd )
		::close(_interrupt_write_fd);
	if ( _interrupt_read_fd != -1 )
		::close(_interrupt_read_fd);

	_interrupt_read_fd = _interrupt_write_fd = -1;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DeviceGroup::setTimer(uint32_t seconds, uint32_t milliseconds, TimeoutFunc func) {
	if ( !func ) {
		SEISCOMP_ERROR("Failed to create timer: function required");
		return false;
	}

	_fnTimeout = func;
	_timerSeconds = seconds;
	_timerMilliseconds = milliseconds;
	_timerSingleShot = false;

#ifdef SEISCOMP_WIRED_EPOLL
	if ( _epoll_fd <= 0 && !setup() ) return false;

	if ( _timerFd <= 0 ) {
		_timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
		if ( _timerFd <= 0 ) {
			SEISCOMP_ERROR("Failed to create timer: %d: %s", errno, strerror(errno));
			return false;
		}

		// Add timer to epoll
		struct epoll_event ev;
		ev.events = _defaultOps | EPOLLIN;
		ev.data.ptr = &_timerFd;

		if ( epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _timerFd, &ev) == -1 ) {
			SEISCOMP_ERROR("epoll_add_timer(%d, %d): %d: %s",
			               _epoll_fd, _timerFd, errno, strerror(errno));
			::close(_timerFd);
			_timerFd = -1;
			return false;
		}
	}

	if ( _timerFd > 0 ) {
		itimerspec newValue;
		timespec ts;
		ts.tv_sec = _timerSeconds;
		ts.tv_nsec = _timerMilliseconds * 1000000;

		newValue.it_value = ts;
		if ( !_timerSingleShot )
			newValue.it_interval = ts;
		else {
			newValue.it_interval.tv_sec = 0;
			newValue.it_interval.tv_nsec = 0;
		}

		if ( timerfd_settime(_timerFd, 0, &newValue, nullptr) < 0 ) {
			SEISCOMP_ERROR("Failed to setup timer: %d: %s", errno, strerror(errno));
			clearTimer();
			return false;
		}

		return true;
	}
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	if ( _kqueue_fd <= 0 && !setup() ) return false;

	struct kevent ev;
	int flags = EV_CLEAR;

	if ( _timerFd <= 0 ) {
		flags |= EV_ADD;
		_timerFd = 1;
	}

	if ( _timerSingleShot )
		flags |= EV_ONESHOT;

	EV_SET(&ev, _timerFd, EVFILT_TIMER, flags, 0, _timerSeconds * 1000 + _timerMilliseconds, &_timerFd);

	if ( kevent(_kqueue_fd, &ev, 1, nullptr, 0, nullptr) == -1 ) {
		SEISCOMP_ERROR("Failed to setup timer: %d: %s", errno, strerror(errno));
		clearTimer();
		return false;
	}

	return true;
#endif

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DeviceGroup::clearTimer() {
	if ( _timerFd > 0 ) {
		// Remove from event queue
#ifdef SEISCOMP_WIRED_EPOLL
		for ( size_t i = _selectIndex; i < _selectSize; ++i ) {
			// Is this device still queued in the epoll event list?
			if ( _epoll_events[i].data.ptr == &_timerFd ) {
				// Remove it
				--_selectSize;
				if ( i < _selectSize )
					memcpy(_epoll_events+i, _epoll_events+i+1, sizeof(struct epoll_event)*(_selectSize-i));
			}
		}
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
		for ( int i = _selectIndex; i < _selectSize; ++i ) {
			// Is this device still queued in the epoll event list?
			if ( _kqueue_events[i].udata == &_timerFd ) {
				// Remove it
				--_selectSize;
				if ( i < _selectSize )
					memcpy(_kqueue_events+i, _kqueue_events+i+1, sizeof(struct kevent)*(_selectSize-i));
			}
		}
#endif

#ifdef SEISCOMP_WIRED_EPOLL
		struct epoll_event ev;
		if ( epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _timerFd, &ev) == -1 )
			SEISCOMP_WARNING("epoll_del(%d): %d: %s",
			                 _timerFd, errno, strerror(errno));
		::close(_timerFd);
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
		struct kevent ev;
		EV_SET(&ev, _timerFd, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
#endif
		_timerFd = -1;
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DeviceGroup::interrupt() {
	if ( _interrupt_write_fd > 0 ) {
		uint64_t signum(1L);
		ssize_t written = ::write(_interrupt_write_fd, &signum, sizeof(signum));
		if ( static_cast<size_t>(written) < sizeof(signum) ) {
			SEISCOMP_ERROR(
				"[reactor] interrupt failed, wrote %zi/%zi: %d: %s",
				written, size_t(sizeof(signum)),
				errno, strerror(errno)
			);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DeviceGroup::count() const {
#if defined(SEISCOMP_WIRED_EPOLL) || defined(SEISCOMP_WIRED_KQUEUE)
	return _count;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Device *DeviceGroup::wait() {
	_isInterrupted = false;

#if defined(SEISCOMP_WIRED_EPOLL) || defined(SEISCOMP_WIRED_KQUEUE)
	_selectIndex = _selectSize = 0;

#endif
#ifdef SEISCOMP_WIRED_EPOLL
	if ( _epoll_fd <= 0 ) {
		// This error should only occur if no device has been added so far
		//SEISCOMP_ERROR("[reactor] no epoll handle");
		return nullptr;
	}
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	if ( _kqueue_fd <= 0 ) {
		// This error should only occur if no device has been added so far
		//SEISCOMP_ERROR("[reactor] no kqueue handle");
		return nullptr;
	}
#endif

	_isInSelect = true;
	//SEISCOMP_DEBUG("[reactor] epoll_wait");
#ifdef SEISCOMP_WIRED_KQUEUE
	struct timespec ts_timeout;
	struct timespec *ts_timeout_used = nullptr;
#endif
	int timeout = -1;
	struct timeval tv_start;

	// Compute current timeout
	if ( _queue ) {
		gettimeofday(&tv_start, nullptr);
		timeout = _queue->_ticker;
#ifdef SEISCOMP_WIRED_KQUEUE
		// Convert scalar milliseconds to timespec structure
		ts_timeout.tv_sec = timeout / 1000;
		ts_timeout.tv_nsec = (timeout - ts_timeout.tv_sec*1000)*1000000;
		ts_timeout_used = &ts_timeout;
#endif
		//SEISCOMP_DEBUG("[reactor] set wait timeout: %d ms", timeout);
	}

#ifdef SEISCOMP_WIRED_EPOLL
	int nfds = epoll_wait(_epoll_fd, _epoll_events, SEISCOMP_WIRED_EPOLL_EVENT_BUFFER, timeout);
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	int nfds = kevent(_kqueue_fd, nullptr, 0, _kqueue_events, SEISCOMP_WIRED_KQUEUE_EVENT_BUFFER, ts_timeout_used);
#endif
	//SEISCOMP_DEBUG("[reactor] epoll_wait::finished");
	_isInSelect = false;
	if ( (nfds == -1) && (errno != EINTR) ) {
		//SEISCOMP_ERROR("[reactor] wait: %d: %s", errno, strerror(errno));
		return nullptr;
	}
	// timeout?
	else if ( nfds == 0 && timeout >= 0 ) {
		_lastCallDuration = timeout;
		//SEISCOMP_DEBUG("[reactor] timeout expired");
	}
	else if ( _queue ) {
		struct timeval tv_end;
		gettimeofday(&tv_end, nullptr);
		int ms = tv_sub_ms(&tv_end, &tv_start);
		_lastCallDuration = ms;
		//SEISCOMP_DEBUG("[reactor] last call took %d ms", _lastCallDuration);
	}

	_nextQueue = _queue;

#if defined(SEISCOMP_WIRED_EPOLL) || defined(SEISCOMP_WIRED_KQUEUE)
	_selectIndex = 0;
	_selectSize = nfds > 0 ? static_cast<size_t>(nfds) : 0;

#endif

	/*
	{
		Device *it = _queue;
		if ( it ) cout << "device event queue:" << endl;
		while ( it ) {
			cout << " " << it << "  " << it->_ticker << "  " << it->_qPrev << "  " << it->_qNext << endl;
			if ( it == it->_qNext ) {
				cout << " !this == next" << endl;
				break;
			}
			it = it->_qNext;
		}
	}
	*/

	return next();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Device *DeviceGroup::next() {
	// First check the queued devices for timeouts
	while ( _nextQueue ) {
		_nextQueue->_ticker -= _lastCallDuration;
		//SEISCOMP_DEBUG("[reactor] remaining ticker: %d ms", _nextQueue->_ticker);
		if ( _nextQueue->_ticker <= 0 ) {
			SEISCOMP_DEBUG("[reactor] device %p timed out", static_cast<void*>(_nextQueue));
			Device *dev = _nextQueue;
			_nextQueue = _nextQueue->_qNext;

			bool hasTrigger = false;
			removeFromQueue(dev);

			for ( size_t i = 0; i < _selectSize; ++ i ) {
#ifdef SEISCOMP_WIRED_EPOLL
				if ( _epoll_events[_selectIndex].data.ptr == dev ) {
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
				if ( _kqueue_events[_selectIndex].udata == dev ) {
#endif
					hasTrigger = true;
					break;
				}
			}

			// Only return this device if it is not part of the
			// event list. Otherwise the iteration below will return it.
			if ( !hasTrigger ) {
				_readyForRead = false;
				_readyForWrite = false;
				_timedOut = true;
				return dev;
			}
		}
		else
			_nextQueue = _nextQueue->_qNext;
	}

	_timedOut = false;

	while ( _selectIndex < _selectSize ) {
	#ifdef SEISCOMP_WIRED_EPOLL
		Device *device = reinterpret_cast<Device *>(_epoll_events[_selectIndex].data.ptr);
	#endif
	#ifdef SEISCOMP_WIRED_KQUEUE
		Device *device = reinterpret_cast<Device *>(_kqueue_events[_selectIndex].udata);
	#endif
		if ( !device ) {
			//SEISCOMP_DEBUG("[reactor] got trigger from interrupt");
			// Here the interrupt fd has been triggered
			if ( _interrupt_write_fd == _interrupt_read_fd ) {
				for (;;) {
					// Only perform one read. The kernel maintains an atomic counter.
					uint64_t signum(0);
					errno = 0;
					ssize_t bytes_read = ::read(_interrupt_read_fd, &signum, sizeof(signum));
					if ( bytes_read < 0 && errno == EINTR ) continue;
					break;
				}
			}
			else {
				for (;;) {
					// Clear all data from the pipe.
					char data[16];
					ssize_t bytes_read = ::read(_interrupt_read_fd, data, sizeof(data));
					if ( bytes_read < 0 && errno == EINTR ) continue;
					while ( bytes_read == sizeof(data) )
						bytes_read = ::read(_interrupt_read_fd, data, sizeof(data));
					break;
				}
			}

			_isInterrupted = true;
			++_selectIndex;
			continue;
		}
		else {
			if ( reinterpret_cast<void*>(device) == reinterpret_cast<void*>(&_timerFd) ) {
				++_selectIndex;
	#ifdef SEISCOMP_WIRED_EPOLL
				uint64_t value;
				if ( ::read(_timerFd, &value, sizeof(value)) == sizeof(value) )
					_fnTimeout();
	#endif
	#ifdef SEISCOMP_WIRED_KQUEUE
				_fnTimeout();
	#endif
				continue;
			}

			// Reset ticker and sort item into the correct position
			if ( device->_timeout >= 0 ) applyTimeout(device);
		}

	#ifdef SEISCOMP_WIRED_EPOLL
		_readyForRead = _epoll_events[_selectIndex].events & EPOLLIN;
		_readyForWrite = _epoll_events[_selectIndex].events & EPOLLOUT;
	#endif
	#ifdef SEISCOMP_WIRED_KQUEUE
		_readyForRead = _kqueue_events[_selectIndex].filter == EVFILT_READ;
		_readyForWrite = _kqueue_events[_selectIndex].filter == EVFILT_WRITE;
		SEISCOMP_DEBUG("[reactor] device %d triggered: READ(%s), WRITE(%s)",
		               device->_fd, _readyForRead?"true":"false",
		               _readyForWrite?"true":"false");
	#endif

		// Error condition?
		if ( !_readyForRead &&
	#ifdef SEISCOMP_WIRED_EPOLL
		     (_epoll_events[_selectIndex].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) ) {
			SEISCOMP_DEBUG("[reactor] close erroneous device %p (%zu/%zu) with fd %d: events = %d",
			               static_cast<void*>(device),
			               _selectIndex, _selectSize, device->fd(),
			               uint32_t(_epoll_events[_selectIndex].events));
	#endif
	#ifdef SEISCOMP_WIRED_KQUEUE
		     (_kqueue_events[_selectIndex].flags & (EV_ERROR | EV_EOF)) ) {
			SEISCOMP_DEBUG("[reactor] close erroneous device %lX with fd %d: flags = %d",
			               (long int)device, device->fd(),
			               _kqueue_events[_selectIndex].flags);
	#endif
			device->close();
		}

		++_selectIndex;

		return device;
	}

	_selectIndex = _selectSize = 0;

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DeviceGroup::removeFromQueue(Device *d) {
	//cout << "remove device " << d << " (prev:" << d->_qPrev << ", next:" << d->_qNext << ")" << endl;

	if ( d->_qPrev )
		d->_qPrev->_qNext = d->_qNext;
	else
		_queue = d->_qNext;

	if ( d->_qNext )
		d->_qNext->_qPrev = d->_qPrev;

	d->_qPrev = d->_qNext = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DeviceGroup::applyTimeout(Device *d) {
	// Reset ticker
	d->_ticker = d->_timeout;
	//SEISCOMP_DEBUG("[reactor] set timeout to %dms for fd %d", d->_timeout, d->_fd);

	//cout << "Find new position for " << d << "  " << d->_ticker << "  " << d->_qPrev << "  " << d->_qNext << endl;

	// Device already queued?
	if ( _queue && (d->_qNext || d->_qPrev || _queue == d) ) {
		//cout << " - already queued" << endl;

		if ( d->_timeout >= 0 ) {
			// Update timeout and find the new position for d
			// Move d towards the front
			if ( d->_qPrev && (d->_qPrev->_ticker > d->_ticker) ) {
				//cout << " - move to front" << endl;

				Device *pos = d->_qPrev;
				while ( pos->_qPrev && (pos->_qPrev->_ticker > d->_ticker) )
					pos = pos->_qPrev;

				// Insert item before pos
				// First: remove it from its current position
				removeFromQueue(d);

				// Insert it into the new position
				d->_qPrev = pos->_qPrev;
				d->_qNext = pos;
				if ( d->_qPrev )
					d->_qPrev->_qNext = d;
				else
					_queue = d;

				pos->_qPrev = d;
			}
			// Move d towards the back
			else if ( d->_qNext && (d->_ticker > d->_qNext->_ticker) ) {
				//cout << " - move to back" << endl;

				Device *pos = d->_qNext;
				while ( pos->_qNext && (d->_ticker > pos->_qNext->_ticker) )
					pos = pos->_qNext;

				// Insert item after pos
				// First: remove it from its current position
				removeFromQueue(d);

				// Insert it into the new position
				d->_qPrev = pos;
				d->_qNext = pos->_qNext;
				if ( d->_qNext )
					d->_qNext->_qPrev = d;
				pos->_qNext = d;
			}
		}
		else {
			// Remove d from queue
			removeFromQueue(d);
		}
	}
	else if ( d->_timeout >= 0 ) {
		//cout << " - add to queue" << endl;

		// Add it to the queue and find the correct position
		if ( !_queue ) {
			_queue = d;
			d->_qPrev = d->_qNext = nullptr;
		}
		else {
			Device *pos = _queue;
			Device *last = nullptr;

			while ( pos ) {
				if ( pos->_ticker > d->_ticker ) {
					// Found position
					d->_qNext = pos;
					d->_qPrev = pos->_qPrev;
					pos->_qPrev = d;
					if ( d->_qPrev ) d->_qPrev->_qNext = d;
					// Update head
					if ( pos == _queue ) _queue = d;
					// Done
					return;
				}

				last = pos;
				pos = pos->_qNext;
			}

			// Push it to the back of the queue
			if ( last ) {
				d->_qNext = last->_qNext;
				d->_qPrev = last;
				last->_qNext = d;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DeviceGroup::updateState(Device *dev) {
#ifdef SEISCOMP_WIRED_EPOLL
	struct epoll_event ev;
	if ( _epoll_fd <= 0 ) return;
	ev.events = _defaultOps;
	ev.data.ptr = dev;

#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	if ( _kqueue_fd <= 0 ) return;
	struct kevent ev[2];
	int kevent_cnt = 0;

#endif
	if ( !dev->isValid() ) return;

	int sm = dev->selectMode();

	if ( sm & Device::Read ) {
		//SEISCOMP_DEBUG("fd %d: set read", s->_fd);
#ifdef SEISCOMP_WIRED_EPOLL
		ev.events |= EPOLLIN;
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
		// Only if read was not added previously, add it
		if ( !(dev->_activeMode & Device::Read) ) {
			EV_SET(&ev[kevent_cnt], dev->fd(), EVFILT_READ, _defaultOps | EV_ADD, 0, 0, dev);
			++kevent_cnt;
			//SEISCOMP_DEBUG("[reactor] kqueue enable event READ for device %d", dev->_fd);
		}
#endif
	}
	else {
#ifdef SEISCOMP_WIRED_KQUEUE
		// Only if read was added previously, delete it
		if ( dev->_activeMode & Device::Read ) {
			EV_SET(&ev[kevent_cnt], dev->fd(), EVFILT_READ, _defaultOps | EV_DELETE, 0, 0, dev);
			++kevent_cnt;
			//SEISCOMP_DEBUG("[reactor] kqueue disable event READ for device %d", dev->_fd);
		}
#endif
		//SEISCOMP_DEBUG("fd %d: clear read", dev->_fd);
	}

	if ( sm & Device::Write ) {
		//SEISCOMP_DEBUG("fd %d: set write", dev->_fd);
#ifdef SEISCOMP_WIRED_EPOLL
		ev.events |= EPOLLOUT;
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
		// Only if read was not added previously, add it
		if ( !(dev->_activeMode & Device::Write) ) {
			EV_SET(&ev[kevent_cnt], dev->fd(), EVFILT_WRITE, _defaultOps | EV_ADD, 0, 0, dev);
			++kevent_cnt;
			//SEISCOMP_DEBUG("[reactor] kqueue enable event WRITE for device %d", dev->_fd);
		}
#endif
	}
	else {
#ifdef SEISCOMP_WIRED_KQUEUE
		// Only if write was added previously, delete it
		if ( dev->_activeMode & Device::Write ) {
			EV_SET(&ev[kevent_cnt], dev->fd(), EVFILT_WRITE, _defaultOps | EV_DELETE, 0, 0, dev);
			++kevent_cnt;
			//SEISCOMP_DEBUG("[reactor] kqueue disable event WRITE for device %d", dev->_fd);
		}
#endif
		//SEISCOMP_DEBUG("fd %d: clear write", dev->_fd);
	}

	if ( sm & Device::Closed ) {
#ifdef SEISCOMP_WIRED_EPOLL
		if ( epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, dev->fd(), &ev) == -1 )
			SEISCOMP_ERROR("epoll_del(%d): %d: %s", dev->fd(), errno, strerror(errno));
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
		kevent_cnt = 0;
		//SEISCOMP_DEBUG("[reactor] kqueue delete READ/WRITE events for device %d", s->_fd);
		if ( dev->_activeMode & Device::Read ) {
			EV_SET(&ev[kevent_cnt], dev->fd(), EVFILT_READ, _defaultOps | EV_DELETE, 0, 0, dev);
			++kevent_cnt;
			//SEISCOMP_DEBUG("[reactor] kqueue disable event READ for device %d", s->_fd);
		}
		if ( dev->_activeMode & Device::Write ) {
			EV_SET(&ev[kevent_cnt], dev->fd(), EVFILT_WRITE, _defaultOps | EV_DELETE, 0, 0, dev);
			++kevent_cnt;
			//SEISCOMP_DEBUG("[reactor] kqueue disable event READ for device %d", s->_fd);
		}
		if ( kevent_cnt && kevent(_kqueue_fd, ev, kevent_cnt, nullptr, 0, nullptr) == -1 )
			SEISCOMP_ERROR("kqueue del(%d): %d: %s", dev->fd(), errno, strerror(errno));
#endif
		// Remove device from queue
		if ( _nextQueue == dev ) _nextQueue = _nextQueue->_qNext;
		if ( dev->_qPrev || dev->_qNext || dev == _queue )
			removeFromQueue(dev);
	}
#ifdef SEISCOMP_WIRED_EPOLL
	else if ( epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, dev->fd(), &ev) == -1 )
		SEISCOMP_ERROR("epoll_mod(%d): %d: %s", dev->fd(), errno, strerror(errno));
#endif
#ifdef SEISCOMP_WIRED_KQUEUE
	else if ( kevent_cnt && kevent(_kqueue_fd, ev, kevent_cnt, nullptr, 0, nullptr) == -1 )
		SEISCOMP_ERROR("kqueue mod(%d): %d: %s", dev->fd(), errno, strerror(errno));

	// Remember the last mode
	dev->_activeMode = dev->_selectMode;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
