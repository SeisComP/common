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
#include <seiscomp/wired/server.h>
#include <seiscomp/wired/clientsession.h>
#include <openssl/err.h>

#include <csignal>
#include <string.h>


using namespace std;


namespace Seiscomp {
namespace Wired {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Server::Server() {
	signal(SIGPIPE, SIG_IGN);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Server::~Server() {
	// Free up allocated memory
	EVP_cleanup();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::setCertificate(const string &cert) {
	_certificate = cert;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::setPrivateKey(const string &key) {
	_privateKey = key;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::init() {
	if ( _endpoints.empty() ) {
		SEISCOMP_ERROR("No listeners defined");
		return false;
	}

	for ( SessionList::iterator it = _endpoints.begin();
	      it != _endpoints.end(); ++it ) {
		Endpoint *a = static_cast<Endpoint*>(*it);
		if ( a->socket()->listen() != Socket::Success ) {
			SEISCOMP_ERROR("Unable to switch to listen state on port %d: errno=%d: %s",
			               a->socket()->port(), errno, strerror(errno));
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::shutdown() {
	Reactor::shutdown();

	lock_guard<mutex> l(_mutex);

	SEISCOMP_DEBUG("[server] shutdown");

	_shouldRun = false;
	for ( SessionList::iterator it = _endpoints.begin();
	      it != _endpoints.end(); ++it ) {
		if ( (*it)->device() ) (*it)->device()->close();
	}
	_devices.interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::clear() {
	Reactor::clear();
	_endpoints.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::addSession(Session *session) {
	lock_guard<mutex> l(_mutex);

	if ( session == nullptr || session->device() == nullptr ) {
		SEISCOMP_WARNING("[server] invalid session for server");
		return false;
	}

	//cout << session->_parent << endl;
	if ( session->_parent != nullptr ) {
		SEISCOMP_WARNING("[server] session is already part of a server");
		return false;
	}

	if ( !_devices.append(session->device()) ) {
		SEISCOMP_ERROR("[server] failed to add socket to group");
		return false;
	}

	_sessions.push_back(session);
	session->_parent = this;
	SEISCOMP_DEBUG("[server] active sessions/sockets: %zu/%zu",
	               _sessions.size(), _devices.count());
	sessionAdded(session);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::addEndpoint(Socket::IPAddress ip, Socket::port_t port,
                         bool useSSL, Endpoint *endpoint) {
	if ( endpoint == nullptr ) return false;
	if ( endpoint->_parent != nullptr ) {
		SEISCOMP_WARNING("Acceptor is already part of a server");
		return false;
	}

	SocketPtr socket;

	if ( useSSL )
		socket = new SSLSocket(SSLSocket::createServerContext(_certificate.c_str(), _privateKey.c_str()));
	else
		socket = new Socket();

	if ( socket->setReuseAddr(true) != Socket::Success ) {
		SEISCOMP_ERROR("Unable to reuse address");
		return false;
	}

	socket->setNonBlocking(true);

	Socket::Status r = socket->bind(ip, port);
	if ( r != Socket::Success ) {
		char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
		ip.toString(buf);
		SEISCOMP_ERROR("Unable to bind to %s:%d: %d",
		               buf, port, static_cast<int>(r));
		return false;
	}

	socket->setMode(Socket::Read);
	endpoint->setDevice(socket.get());

	if ( !_devices.append(endpoint->device()) ) return false;

	endpoint->_parent = this;
	_endpoints.push_back(endpoint);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::addEndpoint(Socket::IPAddress ip, Socket::port_t port, Endpoint *endpoint) {
	if ( endpoint == nullptr ) return false;
	if ( endpoint->_parent != nullptr ) {
		SEISCOMP_WARNING("Acceptor is already part of a server");
		return false;
	}

	Socket *socket = Socket::Cast(endpoint->device());
	if ( socket == nullptr ) {
		SEISCOMP_ERROR("Endpoint does not have a socket attached to it");
		return false;
	}

	socket->setNonBlocking(true);

	Socket::Status r = socket->bind(ip, port);
	if ( r != Socket::Success ) {
		char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
		ip.toString(buf);
		SEISCOMP_ERROR("Unable to bind to %s:%d: %d, %d", buf, port,
		               static_cast<int>(r), errno);
		return false;
	}

	socket->setMode(Socket::Read);

	if ( !_devices.append(socket) ) return false;

	endpoint->_parent = this;
	_endpoints.push_back(endpoint);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::addEndpointV6(Socket::IPAddress ip, Socket::port_t port,
                           bool useSSL, Endpoint *endpoint) {
	if ( endpoint == nullptr ) return false;
	if ( endpoint->_parent != nullptr ) {
		SEISCOMP_WARNING("Acceptor is already part of a server");
		return false;
	}

	SocketPtr socket;

	if ( useSSL )
		socket = new SSLSocket(SSLSocket::createServerContext(_certificate.c_str(), _privateKey.c_str()));
	else
		socket = new Socket();

	if ( socket->setReuseAddr(true) != Socket::Success ) {
		SEISCOMP_ERROR("Unable to reuse address");
		return false;
	}

	socket->setNonBlocking(true);

	Socket::Status r = socket->bindV6(ip, port);
	if ( r != Socket::Success ) {
		char ip_str[Socket::IPAddress::MAX_IP_STRING_LEN];
		ip.toString(ip_str);
		SEISCOMP_ERROR("Unable to bind to [%s]:%d: %d", ip_str, port,
		               static_cast<int>(r));
		return false;
	}

	socket->setMode(Socket::Read);
	endpoint->setDevice(socket.get());

	if ( !_devices.append(endpoint->device()) ) return false;

	endpoint->_parent = this;
	_endpoints.push_back(endpoint);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SessionList &Server::endpoints() const {
	return _endpoints;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::removeSession(Session *session) {
	if ( session->_parent != this ) return false;
	_devices.remove(session->device());

//	SEISCOMP_INFO("[server] removed client %ld", (long int)session);
	sessionRemoved(session);
	session->_parent = nullptr;
	_sessions.erase(session);

	SEISCOMP_DEBUG("[server] active sessions/sockets: %zu/%zu",
	               _sessions.size(), _devices.count());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::removeEndpoint(Endpoint *endpoint) {
	if ( endpoint->_parent != this ) return false;
	_devices.remove(endpoint->device());

	char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
	endpoint->socket()->address().toString(buf);
	SEISCOMP_INFO("[server] removed endpoint on %s:%d (%s)",
	              buf, endpoint->socket()->port(),
	              endpoint->socket()->hostname().c_str());

	endpointRemoved(endpoint);
	endpoint->_parent = nullptr;
	_endpoints.erase(endpoint);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::clearEndpoints() {
	for ( SessionList::iterator it = _endpoints.begin();
	      it != _endpoints.end(); ++it ) {
		Endpoint *endpoint = static_cast<Endpoint*>(*it);
		_devices.remove(endpoint->device());

		char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
		endpoint->socket()->address().toString(buf);

		SEISCOMP_INFO("[server] removed endpoint on %s:%d (%s)",
		              buf, endpoint->socket()->port(), endpoint->socket()->hostname().c_str());

		endpointRemoved(endpoint);
		endpoint->_parent = nullptr;
	}

	_endpoints.clear();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::endpointRemoved(Endpoint *) {
	SEISCOMP_INFO("Removed endpoint");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace TCP
} // namespace Gempa
