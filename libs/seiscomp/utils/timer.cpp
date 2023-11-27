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


#define SEISCOMP_COMPONENT Utils/Timer

#include <seiscomp/logging/log.h>
#include <seiscomp/core/system.h>
#include <seiscomp/utils/timer.h>
#include <assert.h>
#include <iostream>

#if !defined(SC_HAS_TIMER_CREATE)
#include <thread>
#endif

#ifndef WIN32
#include <errno.h>
#include <string.h>

#define TIMER_CLOCKID CLOCK_REALTIME
#endif


namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StopWatch::StopWatch(bool autorun) {
	if ( autorun ) {
		restart();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StopWatch::reset() {
	_start = Core::None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StopWatch::isActive() const {
	return static_cast<bool>(_start);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::TimeSpan StopWatch::elapsed() const {
	if ( !_start ) {
		return Core::TimeSpan();
	}

	auto us = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - *_start).count();
	return Core::TimeSpan(us / 1000000, us % 1000000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if !defined(SC_HAS_TIMER_CREATE)
Timer::TimerList Timer::_timers;
std::thread *Timer::_thread = nullptr;
std::mutex Timer::_mutex;
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Timer::Timer(unsigned int timeout) {
	_singleShot = false;
#if defined(SC_HAS_TIMER_CREATE)
	_timerID = 0;
#else
	_isActive = false;
#endif
	setTimeout(timeout);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Timer::~Timer() {
#if !defined(SC_HAS_TIMER_CREATE)
	if ( _isActive )
		deactivate(true);
#else
	destroy();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Timer::setTimeout(unsigned int timeout) {
	_timeout = timeout;
#if !defined(SC_HAS_TIMER_CREATE)
	if ( !_timeout && _isActive )
#else
	_timeoutNs = 0;
	if ( !_timeout && !_timeoutNs && _timerID )
#endif
		stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::setTimeout2(unsigned int seconds, unsigned int nanoseconds) {
#if !defined(SC_HAS_TIMER_CREATE)
	if ( nanoseconds )
		return false;

	setTimeout(seconds);
	return true;
#else
	_timeout = seconds;
	_timeoutNs = nanoseconds;

	if ( !_timeout && !_timeoutNs && _timerID )
		stop();

	return true;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Timer::setCallback(const Callback &cb) {
	_callback = cb;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Timer::setSingleShot(bool s) {
	_singleShot = s;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::start() {
#if !defined(SC_HAS_TIMER_CREATE)
	if ( !_timeout )
#else
	if ( !_timeout && !_timeoutNs )
#endif
		return false;

#if !defined(SC_HAS_TIMER_CREATE)
	std::lock_guard<std::mutex> lk(_mutex);

	if ( _isActive )
		return false;

	if ( find(_timers.begin(), _timers.end(), this) == _timers.end() )
		_timers.push_back(this);

	_isActive = true;
	_value = _timeout;

	if ( !_thread ) {
		_thread = new std::thread(Timer::Loop);
		std::this_thread::yield();
	}
#else
	if ( _timerID ) return false;

	sigevent sev;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	sched_param parm;

	parm.sched_priority = 255;
	pthread_attr_setschedparam(&attr, &parm);

	sev.sigev_notify_attributes = &attr;
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = handleTimeout;
	sev.sigev_signo = SIGUSR1;
	sev.sigev_value.sival_ptr = this;

	if ( timer_create(TIMER_CLOCKID, &sev, &_timerID) ) {
		SEISCOMP_ERROR("Failed to create timer: %d: %s", errno, strerror(errno));
		_timerID = 0;
		return false;
	}

	itimerspec its;

	/* Single shot */
	its.it_value.tv_sec = _timeout;
	its.it_value.tv_nsec = _timeoutNs;

	/* Periodically */
	its.it_interval.tv_sec = _singleShot ? 0 : _timeout;
	its.it_interval.tv_nsec = _singleShot ? 0 : _timeoutNs;

	if ( timer_settime(_timerID, 0, &its, nullptr) ) {
		SEISCOMP_ERROR("Failed to set timer: %d: %s", errno, strerror(errno));
		timer_delete(_timerID);
		_timerID = 0;
		return false;
	}
#endif

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::stop() {
#if !defined(SC_HAS_TIMER_CREATE)
	return deactivate(false);
#else
	return destroy();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::disable() {
#if !defined(SC_HAS_TIMER_CREATE)
	if ( _isActive )
		return deactivate(true);
	return false;
#else
	return destroy();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if !defined(SC_HAS_TIMER_CREATE)
bool Timer::deactivate(bool remove) {
	assert(_isActive == true);

	_isActive = false;

	std::lock_guard<std::mutex> lk(_mutex);

	if ( remove ) {
		for ( TimerList::iterator it = _timers.begin(); it != _timers.end(); ++it ) {
			if ( *it == this ) {
				_timers.erase(it);
				break;
			}
		}
	}

	if ( _timers.empty() && _thread ) {
		if ( _thread->joinable() ) {
			_thread->join();
		}
		delete _thread;
		_thread = nullptr;
	}

	return true;
}
#else
bool Timer::destroy() {
	std::lock_guard<std::mutex> lock(_callbackMutex);

	if ( !_timerID ) return false;

	if ( timer_delete(_timerID) ) {
		SEISCOMP_ERROR("Failed to delete timer %p: %d: %s", _timerID, errno, strerror(errno));
		_timerID = 0;
		return false;
	}

	_timerID = 0;

	return true;
}
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::isActive() const {
#if !defined(SC_HAS_TIMER_CREATE)
	return _isActive;
#else
	return _timerID != 0;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if !defined(SC_HAS_TIMER_CREATE)
void Timer::Loop() {
	do {
		Core::sleep(1);
	}
	while ( Update() );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Timer::Update() {
	std::lock_guard<std::mutex> lk(_mutex);

	for ( TimerList::iterator it = _timers.begin(); it != _timers.end(); ) {
		Timer *t = *it;
		if ( --t->_value == 0 ) {
			if ( t->_isActive && t->_callback ) {
				//lk.unlock();
				t->_callback();
				//lk.lock();
			}

			if ( t->_singleShot || !t->_isActive ) {
				t->_isActive = false;
				it = _timers.erase(it);
				continue;
			}
			else
				t->_value = t->_timeout;
		}
		else if ( !t->_isActive ) {
			it = _timers.erase(it);
			continue;
		}

		++it;
	}

	return !_timers.empty();
}
#else
void Timer::handleTimeout(sigval_t self) {
	Timer *timer = reinterpret_cast<Timer*>(self.sival_ptr);
	if ( timer->_callback ) {
		if ( timer->_callbackMutex.try_lock() ) {
			timer->_callback();
			timer->_callbackMutex.unlock();
		}
	}
}
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
