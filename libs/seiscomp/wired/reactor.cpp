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


#define SEISCOMP_COMPONENT Wire
#include <seiscomp/logging/log.h>
#include <seiscomp/core/baseobject.h>
#include <cstdio>

#include <seiscomp/wired/session.h>
#include <seiscomp/wired/reactor.h>


using namespace std;


namespace Seiscomp {
namespace Wired {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Reactor::Reactor() {
	_buffer.resize(4096);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Reactor::addSession(Session *session) {
	lock_guard<mutex> l(_mutex);

	if ( !session || !session->device() ) {
		SEISCOMP_WARNING("[reactor] invalid session for reactor");
		return false;
	}

	if ( session->_parent ) {
		SEISCOMP_WARNING("[reactor] session is already part of a reactor");
		return false;
	}

	// Make sure that the parent is set if that method is called from
	// another thread.
	session->_parent = this;

	if ( !_devices.append(session->device()) ) {
		SEISCOMP_ERROR("[reactor] failed to add socket to group");
		session->_parent = nullptr;
		return false;
	}

	_sessions.push_back(session);
	SEISCOMP_DEBUG("[reactor] active sessions/sockets: %zu/%zu",
	               _sessions.size(), _devices.count());

	sessionAdded(session);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Reactor::addSessionDeferred(Session *session) {
	lock_guard<mutex> l(_sessionMutex);
	_deferredSession.push_back(session);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Reactor::removeSession(Session *session) {
	lock_guard<mutex> l(_mutex);

	if ( !session ) return false;
	if ( session->_parent != this ) {
		SEISCOMP_WARNING("[reactor] session is not part of this reactor");
		return false;
	}

	_devices.remove(session->device());

	/*
	SEISCOMP_INFO("[reactor] removed client from %d.%d.%d.%d:%d (%s)",
	              session->socket()->ip().octetts.A, session->socket()->ip().octetts.B,
	              session->socket()->ip().octetts.C, session->socket()->ip().octetts.D,
	              session->socket()->port(), session->socket()->hostname().c_str());
	*/
	SEISCOMP_INFO("[reactor] removed session %p", static_cast<void*>(session));
	sessionRemoved(session);
	session->_parent = nullptr;
	_sessions.erase(session);

	SEISCOMP_DEBUG("[reactor] active sessions/sockets: %zu/%zu",
	               _sessions.size(), _devices.count());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::moveTo(Reactor *target, Session *session) {
	// Save the session because we remove it
	SessionPtr tmp(session);

	SEISCOMP_DEBUG("[reactor] move 0x%p to 0x%p",
	               static_cast<void*>(session), static_cast<void*>(target));

	removeSession(session);
	target->addSessionDeferred(session);

	// Protect the SmartPointer which is only modified in the reactor
	// thread if the session is added to or removed from the list
	target->release(tmp);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Reactor::count() const {
	lock_guard<mutex> l(_mutex);
	return _sessions.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Reactor::setTimer(uint32_t seconds, uint32_t milliseconds,
                       DeviceGroup::TimeoutFunc func) {
	return _devices.setTimer(seconds, milliseconds, func);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Reactor::clearTimer() {
	return _devices.clearTimer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Device *Reactor::wait() {
	return _devices.wait();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Reactor::run() {
	_shouldRun = true;
	_isRunning = true;

	SEISCOMP_DEBUG("[reactor] running");

	while ( _shouldRun ) {
		for ( Device *device = wait(); device; device = _devices.next() ) {
			Session *session = device->session();

			// Socket closed by peer or other error -> close the session
			if ( !device->isValid() ) {
				SEISCOMP_DEBUG("[reactor] got invalid socket, remove session");
				session->close();
				removeSession(session);
			}
			else {
				if ( _devices.timedOut() && !session->handleTimeout() ) {
					SEISCOMP_DEBUG("[reactor] socket timed out, remove session");
					session->close();
					removeSession(session);
				}
				else {
					session->update();

					// Session closed in update -> remove it
					if ( !device->isValid() ) {
//						SEISCOMP_DEBUG("[reactor] session closed its socket, remove it");
						removeSession(session);
					}
					else if ( session->isTagged() )
						sessionTagged(session);
				}
			}
		}

		{
			// Protect deferred session list
			lock_guard<mutex> l(_sessionMutex);

			// Add scheduled sessions
			while ( !_deferredSession.empty() ) {
				SessionPtr s = _deferredSession.front();
				_deferredSession.pop_front();
				addSession(s.get());
			}
		}

		idle();
	}

	SEISCOMP_DEBUG("[reactor] stopping");
	SEISCOMP_INFO("[reactor] remaining sessions/sockets: %zu/%zu",
	              _sessions.size(), _devices.count());

	lock_guard<mutex> l(_mutex);
	clear();
	_isRunning = false;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::clear() {
	_devices.clear();
	_sessions.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::idle() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::shutdown() {
	lock_guard<mutex> l(_mutex);

	SEISCOMP_DEBUG("[reactor] shutdown");

	_shouldRun = false;
	for ( SessionList::iterator it = _sessions.begin();
	      it != _sessions.end(); ++it ) {
		if ( (*it)->device() ) (*it)->device()->close();
	}

	_devices.interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::release(SessionPtr &ptr) {
	lock_guard<mutex> l(_mutex);
	ptr = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Reactor::setup() {
	return _devices.setup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::interrupt() {
	_devices.interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::stop() {
	_shouldRun = false;
	interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::resume() {
	_shouldRun = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Reactor::setTriggerMode(Reactor::TriggerMode tm) {
	return _devices.setTriggerMode(tm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Reactor::TriggerMode Reactor::triggerMode() const {
	return _devices.triggerMode();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SessionList &Reactor::sessions() const {
	return _sessions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DeviceGroup *Reactor::devices() const {
	return &_devices;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::sessionAdded(Session *) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::sessionRemoved(Session *) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::sessionTagged(Session *) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Reactor::getBuffer(char *&buf, size_t &len) {
	buf = &_buffer[0];
	len = _buffer.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace TCP
} // namespace Gempa
