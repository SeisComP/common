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

#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#else
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <seiscomp/wired/devices/socket.h>

#include <openssl/err.h>

#include <pthread.h>
#include <cerrno>


using namespace std;


namespace Seiscomp {
namespace Wired {


namespace {


pthread_mutex_t *ssl_mutex_buffer = NULL;


unsigned long SSL_thread_id_function(void) {
	return ((unsigned long)pthread_self());
}


void SSL_locking_function(int mode, int id, const char *file, int line) {
	if ( mode & CRYPTO_LOCK )
		pthread_mutex_lock(&ssl_mutex_buffer[id]);
	else
		pthread_mutex_unlock(&ssl_mutex_buffer[id]);
}


void SSL_static_init() {
	if ( ssl_mutex_buffer == NULL )
		ssl_mutex_buffer = (pthread_mutex_t*)malloc(CRYPTO_num_locks()*sizeof(pthread_mutex_t));

	for ( int i = 0; i < CRYPTO_num_locks(); ++i )
		pthread_mutex_init(&ssl_mutex_buffer[i], NULL);

	CRYPTO_set_id_callback(SSL_thread_id_function);
	CRYPTO_set_locking_callback(SSL_locking_function);
}


void SSL_static_cleanup() {
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);

	if ( ssl_mutex_buffer == NULL )
		return;

	for ( int i = 0; i < CRYPTO_num_locks(); ++i )
		pthread_mutex_destroy(&ssl_mutex_buffer[i]);

	free(ssl_mutex_buffer);
	ssl_mutex_buffer = NULL;
}


struct SSLInitializer {
	SSLInitializer() {
		SSL_library_init();
		OpenSSL_add_all_algorithms();

		SSL_static_init();
	}

	~SSLInitializer() {
		SSL_static_cleanup();
	}
};


SSLInitializer __sslInitializer;


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const Socket::IPAddress &ip) {
	char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
	ip.toString(buf);
	os << buf;
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const Anonymize<Socket::IPAddress> &ip) {
	char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
	ip.target.toString(buf, true);
	os << buf;
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string toString(const Socket::IPAddress &ip) {
	char buf[Socket::IPAddress::MAX_IP_STRING_LEN];
	ip.toString(buf);
	return buf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::_socketCount = -1;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *getSSLSystemError() {
	int err = ERR_get_error();
	if ( err != 0 )
		return ERR_reason_error_string(err);
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *getSystemError(Socket *socket) {
	if ( dynamic_cast<SSLSocket*>(socket) )
		return getSSLSystemError();

	if ( errno != 0 ) return strerror(errno);

	static const char *errorStr = "Unkown error";
	return errorStr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Socket::IPAddress::fromString(const char *str) {
	return fromStringV4(str) || fromStringV6(str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Socket::IPAddress::fromStringV4(const char *str) {
	if ( inet_pton(AF_INET, str, bytes) != 1 )
		return false;

	dwords[1] = dwords[2] = dwords[3] = 0;
	dwords[0] = ntohl(dwords[0]);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Socket::IPAddress::fromStringV6(const char *str) {
	if ( inet_pton(AF_INET6, str, bytes) != 1 )
		return false;

	for ( int i = 0; i < BYTES/2; ++i )
		std::swap(bytes[i], bytes[BYTES-1-i]);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::IPAddress::toString(char *str, bool anonymize) const {
	if ( !dwords[1] && !dwords[2] && !dwords[3] ) {
		if ( anonymize )
			return snprintf(str, 46, "%d.%d.0.0", v4.A, v4.B) + 1;
		else
			return snprintf(str, 46, "%d.%d.%d.%d", v4.A, v4.B, v4.C, v4.D) + 1;
	}

	if ( anonymize ) {
		in6_addr addr;
		memset(&addr, 0, sizeof(addr));

		// Just keep 24 bit
		for ( int i = IPAddress::BYTES-3; i < IPAddress::BYTES; ++i )
			addr.s6_addr[i] = bytes[IPAddress::BYTES-1-i];

		if ( inet_ntop(AF_INET6, &addr, str, MAX_IP_STRING_LEN) == NULL )
			return -1;

		return strlen(str);
		/*
		return snprintf(str, 46, "%04x:%04x::",
		               (dwords[3] >> 16) & 0xffff, dwords[3] & 0xff00);
		*/
	}
	else {
		in6_addr addr;
		memset(&addr, 0, sizeof(addr));

		for ( int i = 0; i < IPAddress::BYTES; ++i )
			addr.s6_addr[i] = bytes[IPAddress::BYTES-1-i];

		if ( inet_ntop(AF_INET6, &addr, str, MAX_IP_STRING_LEN) == NULL )
			return -1;

		return strlen(str);
		/*
		return snprintf(str, 46, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
		               (dwords[3] >> 16) & 0xffff, dwords[3] & 0xffff,
		               (dwords[2] >> 16) & 0xffff, dwords[2] & 0xffff,
		               (dwords[1] >> 16) & 0xffff, dwords[1] & 0xffff,
		               (dwords[0] >> 16) & 0xffff, dwords[0] & 0xffff);
		*/
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Socket() {
	if ( _socketCount == -1 ) {
#if WIN32
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if ( iResult != NO_ERROR ) {
			// throw Exception
		}

		_socketCount = 0;
#endif
	}

	++_socketCount;

	_timeOutSecs = -1;
	_timeOutUsecs = 0;
	_selectMode = Idle;

	_hostname = "localhost";
	_port = 0;
	_flags = NoFlags;

	_bytesSent = _bytesReceived = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Socket(int fd, const std::string &hostname, uint16_t port)
: Device(fd) {
	if ( _socketCount == -1 ) {
#if WIN32
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if ( iResult != NO_ERROR ) {
			// throw Exception
		}

		_socketCount = 0;
#endif
	}

	++_socketCount;

	_timeOutSecs = -1;
	_timeOutUsecs = 0;
	_selectMode = Idle;

	_hostname = hostname;
	_port = port;
	_flags = NoFlags;

	_bytesSent = _bytesReceived = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::~Socket() {
	--_socketCount;
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *Socket::toString(Status stat) {
	switch ( stat ) {
		case Success:
			return "success";
		default:
		case Error:
			return "error";
		case AllocationError:
			return "allocation error";
		case ReuseAdressError:
			return "reusing address failed";
		case BindError:
			return "bind error";
		case ListenError:
			return "listen error";
		case AcceptError:
			return "accept error";
		case ConnectError:
			return "connect error";
		case AddrInfoError:
			return "address info error";
		case Timeout:
			return "timeout";
		case InvalidSocket:
			return "invalid socket";
		case InvalidPort:
			return "invalid port";
		case InvalidAddressFamily:
			return "invalid address family";
		case InvalidAddress:
			return "invalid address";
		case InvalidHostname:
			return "invalid hostname";
	}

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Socket::shutdown() {
	if ( _fd == -1 ) return;
	//SEISCOMP_DEBUG("Socket::shutdown");
	::shutdown(_fd, SHUT_RDWR);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Socket::close() {
	if ( _fd != -1 ) {
		//SEISCOMP_DEBUG("[socket] close %lX with fd = %d", (long int)this, _fd);
		int fd = _fd;
		setMode(Closed);
		_fd = -1;
		::close(fd);
	}

	_flags &= ~InAccept;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::setSocketTimeout(int secs, int usecs) {
	_timeOutSecs = secs;
	_timeOutUsecs = usecs;

	if ( _fd != -1 )
		return applySocketTimeout(_timeOutSecs, _timeOutUsecs);

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::applySocketTimeout(int secs, int usecs) {
	if ( _fd != -1 ) {
		struct timeval timeout;
		void *opt;
		int optlen;

		if ( secs >= 0 ) {
			timeout.tv_sec = secs;
			timeout.tv_usec = usecs;
			opt = &timeout;
			optlen = sizeof(timeout);
		}
		else {
			opt = NULL;
			optlen = 0;
		}

		//SEISCOMP_DEBUG("set socket timeout to %d.%06ds", secs, usecs);

		if ( setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, opt, optlen) )
			return Error;

		if ( setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, opt, optlen) )
			return Error;
	}
	else
		return InvalidSocket;

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Device::Status Socket::setNonBlocking(bool nb) {
	if ( nb )
		_flags |= NonBlocking;
	else
		_flags &= ~NonBlocking;

	if ( !isValid() )
		return Device::Success;

#ifndef WIN32
	int flags = fcntl(_fd, F_GETFL, 0);

	if ( nb )
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if ( fcntl(_fd, F_SETFL, flags) == -1 ) {
		//SEISCOMP_ERROR("Socket::setNonBlocking(%d, %d): %s", _fd, nb, strerror(errno));
		return Device::Error;
	}
#else
	u_long arg = nb?1:0;
	if ( ioctlsocket(_fd, FIONBIO, &arg) != 0 )
		return Device::Error;
#endif

	return Device::Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::setReuseAddr(bool ra) {
	if ( ra )
		_flags |= ReuseAddress;
	else
		_flags &= ~ReuseAddress;
	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::setNoDelay(bool nd) {
	if ( nd )
		_flags |= NoDelay;
	else
		_flags &= ~NoDelay;
	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::setResolveHostnames(bool rh) {
	if ( rh )
		_flags |= ResolveName;
	else
		_flags &= ~ResolveName;
	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::connect(const std::string &hostname, port_t port) {
	if ( _fd != -1 ) {
		SEISCOMP_WARNING("closing stale socket");
		this->close();
	}

	struct sockaddr addr;
	size_t addrlen;

	struct addrinfo *res;
	struct addrinfo hints;

	memset (&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

	std::string strPort = Seiscomp::Core::toString(port);

	if ( getaddrinfo(hostname.c_str(), strPort.c_str(), &hints, &res) ) {
		SEISCOMP_DEBUG("Socket::connect(%s:%d): %s",
		               hostname.c_str(), port, strerror(errno));
		return AddrInfoError;
	}

	addr = *(res->ai_addr);
	addrlen = res->ai_addrlen;
	freeaddrinfo(res);

	if ( (_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
		SEISCOMP_DEBUG("Socket::connect(%s:%d): %s",
		               hostname.c_str(), port, strerror(errno));
		return AllocationError;
	}

	if ( _flags & NoDelay ) {
		int flag = 1;
		setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
	}

	setNonBlocking(_flags & NonBlocking ? true : false);

	if ( _timeOutSecs >= 0 ) {
		if ( applySocketTimeout(_timeOutSecs, _timeOutUsecs) != Success ) {
			this->close();
			return Error;
		}
	}

#ifndef WIN32
	if ( ::connect(_fd, (struct sockaddr *)&addr, addrlen) == -1 ) {
		if ( errno != EINPROGRESS ) {
			SEISCOMP_DEBUG("Socket::connect(%s:%d): %s",
			               hostname.c_str(), port, strerror(errno));
			this->close();
			return errno == ETIMEDOUT?Timeout:ConnectError;
		}
	}
#else
	if ( ::connect(_fd, (struct sockaddr *)&addr, addrlen) == SOCKET_ERROR ) {
		int err = WSAGetLastError();
		if (err != WSAEINPROGRESS && err != WSAEWOULDBLOCK) {
			SEISCOMP_DEBUG("Socket::connect(%s:%d): %d",
			               hostname.c_str(), port, err);
			close();
			return ConnectError;
		}
	}
#endif

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::connectV6(const std::string &hostname, port_t port) {
	if ( _fd != -1 ) {
		SEISCOMP_WARNING("closing stale socket");
		this->close();
	}

	struct sockaddr addr;
	size_t addrlen;

	struct addrinfo *res;
	struct addrinfo hints;

	memset (&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET6;
	hints.ai_socktype = SOCK_STREAM;

	std::string strPort = Seiscomp::Core::toString(port);

	if ( getaddrinfo(hostname.c_str(), strPort.c_str(), &hints, &res) ) {
		SEISCOMP_DEBUG("Socket::connect(%s:%d): %s",
		               hostname.c_str(), port, strerror(errno));
		return AddrInfoError;
	}

	addr = *(res->ai_addr);
	addrlen = res->ai_addrlen;
	freeaddrinfo(res);

	if ( (_fd = socket(PF_INET6, SOCK_STREAM, 0)) < 0 ) {
		SEISCOMP_DEBUG("Socket::connect(%s:%d): %s",
		               hostname.c_str(), port, strerror(errno));
		return AllocationError;
	}

	if ( _timeOutSecs >= 0 ) {
		if ( applySocketTimeout(_timeOutSecs, _timeOutUsecs) != Success ) {
			this->close();
			return Error;
		}
	}

#ifndef WIN32
	if ( ::connect(_fd, (struct sockaddr *)&addr, addrlen) == -1 ) {
		if ( errno != EINPROGRESS ) {
			SEISCOMP_DEBUG("Socket::connect(%s:%d): %s",
			               hostname.c_str(), port, strerror(errno));
			this->close();
			return errno == ETIMEDOUT?Timeout:ConnectError;
		}
	}
#else
	if ( ::connect(_fd, (struct sockaddr *)&addr, addrlen) == SOCKET_ERROR ) {
		int err = WSAGetLastError();
		if (err != WSAEINPROGRESS && err != WSAEWOULDBLOCK) {
			SEISCOMP_DEBUG("Socket::connect(%s:%d): %d",
			               hostname.c_str(), port, err);
			close();
			return ConnectError;
		}
	}
#endif

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::bind(IPAddress ip, port_t port) {
	struct sockaddr_in addr;

	// InvalidPort
	if ( port < 0 ) return InvalidPort;

	_port = port;
	_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

#ifdef MACOSX
	int set = 1;
	if ( setsockopt(_fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)) < 0 ) {
		close();
		return Error;
	}
#endif

	// AllocationError
	if ( _fd == -1 ) return AllocationError;

	if ( _flags & ReuseAddress ) {
		int arg = 1;
		if ( setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) != 0 )
			return ReuseAdressError;
	}

	setNonBlocking(_flags & NonBlocking ? true : false);

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = htonl(ip.dwords[0]);

	// BindError
	if ( ::bind(_fd, (const sockaddr*)&addr, sizeof(addr)) == -1 ) {
		SEISCOMP_DEBUG("Bind: %s", strerror(errno));
		close();
		return BindError;
	}

	socklen_t size = sizeof(addr);
	if ( getsockname(_fd, (sockaddr*)&addr, &size) != 0 ) {
		SEISCOMP_DEBUG("getsockname: %s", strerror(errno));
		close();
		return BindError;
	}

	_port = ntohs(addr.sin_port);

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::bindV6(IPAddress ip, port_t port) {
	struct sockaddr_in6 addr;

	// InvalidPort
	if ( port < 0 ) return InvalidPort;

	_port = port;
	_fd = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);

#ifdef MACOSX
	int set = 1;
	if ( setsockopt(_fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)) < 0 ) {
		close();
		return Error;
	}
#endif

	// AllocationError
	if ( _fd == -1 ) return AllocationError;

	if ( _flags & ReuseAddress ) {
		int arg = 1;
		if ( setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) != 0 )
			return ReuseAdressError;
	}

	setNonBlocking(_flags & NonBlocking ? true : false);

	memset(&addr, 0, sizeof(addr));

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(_port);

	for ( int i = 0; i < IPAddress::BYTES; ++i )
		addr.sin6_addr.s6_addr[i] = ip.bytes[IPAddress::BYTES-1-i];

	// BindError
	if ( ::bind(_fd, (const sockaddr*)&addr, sizeof(addr)) == -1 ) {
		SEISCOMP_DEBUG("Bind: %s", strerror(errno));
		close();
		return BindError;
	}

	socklen_t size = sizeof(addr);
	if ( getsockname(_fd, (sockaddr*)&addr, &size) != 0 ) {
		SEISCOMP_DEBUG("getsockname: %s", strerror(errno));
		close();
		return BindError;
	}

	_port = ntohs(addr.sin6_port);

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::listen(int backlog) {
	if ( !isValid() ) return InvalidSocket;

	// ListenError
	if ( ::listen(_fd, backlog) == -1 )
		return ListenError;

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket *Socket::accept() {
	if ( !isValid() ) return NULL;

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int client_fd = ::accept(_fd, (struct sockaddr*)&addr, &addr_len);
	// AcceptError
	if( client_fd < 0 ) {
		if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
			SEISCOMP_DEBUG("Accept: %s", strerror(errno));
		}

		return NULL;
	}

	Socket *sock = new Socket;
	sock->_fd = client_fd;

	char buf[512];
	if ( (_flags & ResolveName) &&
	     getnameinfo((struct sockaddr*)&addr, sizeof(addr), buf, 512, NULL, 0, 0) == 0 )
		sock->_hostname = buf;

	sock->_addr.set(ntohl(addr.sin_addr.s_addr));
	sock->_port = ntohs(addr.sin_port);

	if ( _flags & NoDelay ) {
		int flag = 1;
		setsockopt(sock->_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
	}

	return sock;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Socket::hostname() const {
	return _hostname;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::IPAddress Socket::address() const {
	return _addr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::port_t Socket::port() const {
	return _port;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::send(const char *data) {
	return write(data, strlen(data));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::write(const char *data, int len) {
#if !defined(MACOSX) && !defined(WIN32)
	int sent = (int)::send(_fd, data, len, MSG_NOSIGNAL);
#else
	int sent = (int)::send(_fd, data, len, 0);
#endif
	if ( sent > 0 ) {
		_bytesSent += sent;
	}
	return sent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::read(char *data, int len) {
	int recvd = (int)::recv(_fd, data, len, 0);
	if ( recvd > 0 ) _bytesReceived += recvd;
	return recvd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const unsigned char *Socket::sessionID() const {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned int Socket::sessionIDLength() const {
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::SSLSocket() : _ssl(NULL), _ctx(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::SSLSocket(SSL_CTX *ctx) : _ssl(NULL), _ctx(ctx) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::~SSLSocket() {
	close();
	cleanUp();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSL_CTX *SSLSocket::createServerContext(const char *pemCert, const char *pemKey) {
	// Initialize ciphers and digests
	SSL_library_init();
	// Load human readable error reporting
	SSL_load_error_strings();
	ERR_load_crypto_strings();
	ERR_load_SSL_strings();
	// Load available cipher and digest algorithms
	OpenSSL_add_all_algorithms();

	SSL_CTX *ctx(SSL_CTX_new(SSLv23_method()));

#ifdef SSL_OP_NO_COMPRESSION
	const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
#else
	const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
#endif
	SSL_CTX_set_options(ctx, flags);

	SSL_CTX_set_mode(ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	// Set the server certificate to be used
	SEISCOMP_DEBUG("Loading server certificate %s", pemCert);
	if ( !SSL_CTX_use_certificate_file(ctx, pemCert, SSL_FILETYPE_PEM) ) {
		SEISCOMP_ERROR("Loading server certificate failed: %s", pemCert);
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return NULL;
	}

	// Set the private key to be used
	SEISCOMP_DEBUG("Loading private key %s", pemKey);
	if ( SSL_CTX_use_PrivateKey_file(ctx, pemKey, SSL_FILETYPE_PEM) <= 0 ) {
		SEISCOMP_ERROR("Loading private key failed: %s", pemKey);
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return NULL;
	}

	// Verify that the private key matches the server certificate
	SEISCOMP_DEBUG("Verifying keys...");
	if ( !SSL_CTX_check_private_key(ctx)) {
		SEISCOMP_ERROR("Private key check failed");
		SSL_CTX_free(ctx);
		return NULL;
	}

	return ctx;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status SSLSocket::bind(IPAddress ip, port_t port) {
	if ( _ctx == NULL ) {
		SEISCOMP_DEBUG("Bind: no SSL context");
		return BindError;
	}

	return Socket::bind(ip, port);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status SSLSocket::bindV6(IPAddress ip, port_t port) {
	if ( _ctx == NULL ) {
		SEISCOMP_DEBUG("Bind: no SSL context");
		return BindError;
	}

	return Socket::bindV6(ip, port);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket* SSLSocket::accept() {
	if ( !isValid() ) return NULL;

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int client_fd = ::accept(_fd, (struct sockaddr*)&addr, &addr_len);
	// AcceptError
	if( client_fd < 0 ) {
		if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
			SEISCOMP_DEBUG("Accept: %s", strerror(errno));
		}

		return NULL;
	}

	SSL *ssl = SSL_new(_ctx);
	if ( ssl == NULL ) {
		SEISCOMP_ERROR("Failed to create SSL channel from context");
		::close(client_fd);
		return NULL;
	}

	SSL_set_fd(ssl, client_fd);
	SSL_set_shutdown(ssl, 0);
	SSL_set_accept_state(ssl);

	SSLSocket *sock = new SSLSocket();
	sock->_fd = client_fd;
	sock->_ssl = ssl;

	if ( _flags & NoDelay ) {
		int flag = 1;
		setsockopt(sock->_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
	}

	char buf[512];
	if ( (_flags & ResolveName) &&
	     getnameinfo((struct sockaddr*)&addr, sizeof(addr), buf, 512, NULL, 0, 0) == 0 )
		sock->_hostname = buf;

	sock->_addr.set(ntohl(addr.sin_addr.s_addr));
	sock->_port = ntohs(addr.sin_port);
	sock->_flags |= InAccept;

	if ( !(mode() & Read) )
		setMode(Read);

	return sock;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SSLSocket::write(const char *data, int len) {
	if ( _flags & InAccept ) {
		int ret = SSL_accept(_ssl);
		if ( ret <= 0 ) {
			int err = SSL_get_error(_ssl, ret);
			switch ( err ) {
				case SSL_ERROR_WANT_READ:
					setMode(Read);
					errno = EAGAIN;
					return -1;
				case SSL_ERROR_WANT_WRITE:
					errno = EAGAIN;
					return -1;
				default:
					SEISCOMP_DEBUG("SSL accept failed with error %d", err);
					errno = EINVAL;
					return 0;
			}
		}

		_flags &= ~InAccept;
	}

	int ret = SSL_write(_ssl, data, len);
	if ( ret > 0 ) {
		_bytesSent += ret;
		return ret;
	}

	int err = SSL_get_error(_ssl, ret);

	switch ( err ) {
		case SSL_ERROR_WANT_X509_LOOKUP:
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_WANT_READ:
			setMode(Read);
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_WANT_WRITE:
			addMode(Write);
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_ZERO_RETURN:
			errno = EINVAL;
			return 0;
		default:
			break;
	}

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SSLSocket::read(char *data, int len) {
	if ( _flags & InAccept ) {
		int ret = SSL_accept(_ssl);
		if ( ret <= 0 ) {
			int err = SSL_get_error(_ssl, ret);
			switch ( err ) {
				case SSL_ERROR_WANT_READ:
					errno = EAGAIN;
					return -1;
				case SSL_ERROR_WANT_WRITE:
					setMode(Write);
					errno = EAGAIN;
					return -1;
				default:
					SEISCOMP_DEBUG("SSL accept failed with error %d", err);
					errno = EINVAL;
					return 0;
			}
		}

		_flags &= ~InAccept;
	}

	int ret = SSL_read(_ssl, data, len);
	if ( ret > 0 ) {
		_bytesReceived += ret;
		return ret;
	}

	int err = SSL_get_error(_ssl, ret);

	switch ( err ) {
		case SSL_ERROR_WANT_X509_LOOKUP:
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_WANT_READ:
			addMode(Read);
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_WANT_WRITE:
			setMode(Write);
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_ZERO_RETURN:
			errno = EINVAL;
			return 0;
		default:
			break;
	}

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status SSLSocket::connect(const std::string &hostname, port_t port) {
	cleanUp();

	_ctx = SSL_CTX_new(SSLv23_client_method());
	if ( _ctx == NULL ) {
		SEISCOMP_DEBUG("Invalid SSL context");
		return ConnectError;
	}

	SSL_CTX_set_mode(_ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	Status s = Socket::connect(hostname, port);
	if ( s != Success )
		return s;

	_ssl = SSL_new(_ctx);
	if ( _ssl == NULL ) {
		SEISCOMP_DEBUG("Failed to create SSL context");
		return ConnectError;
	}

	SSL_set_fd(_ssl, _fd);
	SSL_set_shutdown(_ssl, 0);
	SSL_set_connect_state(_ssl);
	int err = SSL_connect(_ssl);
	if ( err < 0 ) {
		SEISCOMP_ERROR("Failed to connect with SSL, error %d",
		               SSL_get_error(_ssl, err));
		close();
		return ConnectError;
	}

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status SSLSocket::connectV6(const std::string &hostname, port_t port) {
	cleanUp();

	_ctx = SSL_CTX_new(SSLv23_client_method());
	if ( _ctx == NULL ) {
		SEISCOMP_DEBUG("Invalid SSL context");
		return ConnectError;
	}

	Status s = Socket::connectV6(hostname, port);
	if ( s != Success )
		return s;

	_ssl = SSL_new(_ctx);
	if ( _ssl == NULL ) {
		SEISCOMP_DEBUG("Failed to create SSL context");
		return ConnectError;
	}

	SSL_set_fd(_ssl, _fd);
	SSL_set_shutdown(_ssl, 0);
	SSL_set_connect_state(_ssl);
	int err = SSL_connect(_ssl);
	if ( err < 0 ) {
		SEISCOMP_ERROR("Failed to connect with SSL, error %d",
		               SSL_get_error(_ssl, err));
		close();
		return ConnectError;
	}

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SSLSocket::close() {
	Socket::close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const unsigned char *SSLSocket::sessionID() const {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	return _ssl?_ssl->session->session_id:NULL;
#else
	return _ssl?SSL_SESSION_get0_id_context(SSL_get0_session(_ssl), NULL):NULL;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned int SSLSocket::sessionIDLength() const {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	return _ssl?_ssl->session->session_id_length:0;
#else
	unsigned int len;
	if ( !_ssl ) return 0;
	SSL_SESSION_get0_id_context(SSL_get0_session(_ssl), &len);
	return len;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
X509 *SSLSocket::peerCertificate() {
	if ( _ssl == NULL ) return NULL;
	return SSL_get_peer_certificate(_ssl);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SSLSocket::cleanUp() {
	if ( _ssl ) {
		SSL_free(_ssl);
		_ssl = NULL;
	}

	if ( _ctx ) {
		SSL_CTX_free(_ctx);
		_ctx = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
