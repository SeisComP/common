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

#include <seiscomp/wired/endpoint.h>
#include <seiscomp/wired/clientsession.h>
#include <seiscomp/wired/server.h>

#include <fcntl.h>


namespace Seiscomp {
namespace Wired {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Endpoint::Endpoint(Socket *sock) : Session(sock) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Endpoint::update() {
	if ( !_device->isValid() ) {
		SEISCOMP_INFO("Listener socket at port %d has been closed",
		              socket()->port());
		return;
	}

	while ( true ) {
		Socket *incoming = socket()->accept();
		if ( incoming ) {
			incoming->setNonBlocking(true);
			Session *session = accept(incoming);
			if ( !session ) {
				incoming->close();
				delete incoming;
			}
			else {
				char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
				incoming->address().toString(buf);
				SEISCOMP_INFO("Accepted new client from %s:%d (%s)", buf,
				              incoming->port(), incoming->hostname().c_str());
				incoming->setNonBlocking(true);
				if ( !_parent->addSession(session) ) {
					SEISCOMP_ERROR("Adding session failed");
					delete session;
				}

				if ( socket()->listen() != Socket::Success )
					SEISCOMP_ERROR("Listing failed");
			}
		}
		else
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Session *Endpoint::accept(Socket *socket) {
	if ( !checkSocket(socket) ) return NULL;
	return createSession(socket);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Endpoint::checkSocket(Socket *socket) const {
	bool fdValid = fcntl(socket->fd(), F_GETFD) != -1 || errno != EBADF;
	if ( !fdValid )
		SEISCOMP_ERROR("Invalid fd = %d", socket->fd());

	return fdValid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Session *Endpoint::createSession(Socket *socket) {
	return new ClientSession(socket);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AccessControlledEndpoint::AccessControlledEndpoint(Socket *sock,
                                                  const IPACL &allowedIPs,
                                                   const IPACL &deniedIPs)
: Endpoint(sock), _allowedIPs(allowedIPs), _deniedIPs(deniedIPs) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AccessControlledEndpoint::checkSocket(Socket *socket) const {
	if ( !_allowedIPs.check(socket->address()) ||
	     !_deniedIPs.not_check(socket->address()) ) {
		char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
		socket->address().toString(buf);
		SEISCOMP_INFO("Denied incoming client connection from %s", buf);
		return false;
	}

	return Endpoint::checkSocket(socket);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AccessControlledEndpoint::compareIPACL(const IPACL &allowedIPs,
                                            const IPACL &deniedIPs) const {
	return (_allowedIPs == allowedIPs && _deniedIPs == deniedIPs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
