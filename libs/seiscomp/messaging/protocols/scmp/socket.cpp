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


#define SEISCOMP_COMPONENT SCMP

#include <seiscomp/logging/log.h>
#include <seiscomp/messaging/protocols/scmp/socket.h>
#include <seiscomp/core/strings.h>

#include <iostream>


using namespace std;


namespace Seiscomp {
namespace Client {
namespace SCMP {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FrameHeaders::next() {
	size_t len;
	const char *data = Core::tokenize2(_source, "\n", _source_len, len);

	if ( data ) {
		Core::trim(data,len);

		name_start = data;

		const char *sep = Core::strnchr(data, len, ':');
		if ( sep ) {
			name_len = static_cast<size_t>(sep - data);
			val_start = sep + 1;
			val_len = len - name_len - 1;
			Core::trimBack(name_start, name_len);
			Core::trimFront(val_start, val_len);
		}
		else {
			name_len = len;
			Core::trimBack(name_start, name_len);
			val_start = nullptr;
			val_len = 0;
		}

		++_numberOfHeaders;
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Socket()
: _useSSL(false)
, _getcount(0) {
	_ackWindow = 20;
	_wantMembershipInfo = false;
	_inWait = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::~Socket() {
	clearInbox();
	close();
	_select.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Socket::setSSL(bool enable) {
	_useSSL = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Socket::setAckWindow(uint32_t size) {
	_ackWindow = size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Socket::close() {
	if ( _socket ) _socket->close();
	_groups.clear();
	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Socket::wait(boost::mutex *m, boost::mutex *waitLock) {
	Wired::Device *dev = nullptr;
	while ( dev != _socket ) {
		//std::cerr << "Wait is true" << std::endl;
		if ( _inWait ) SEISCOMP_WARNING("Sync error");
		_inWait = true;
		if ( m ) m->unlock();
		if ( waitLock ) waitLock->lock();
		dev = _select.wait();
		if ( waitLock ) waitLock->unlock();
		if ( m ) m->lock();
		_inWait = false;
		//std::cerr << "Wait is false" << std::endl;
		if ( dev == nullptr ) return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Socket::handleInterrupt(int) {
	_select.interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
