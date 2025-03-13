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
#ifdef BSD
#include <netinet/in.h>
#include <netinet/ip.h>
#endif
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#else
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <seiscomp/wired/devices/socket.h>
#include <seiscomp/wired/session.h>

#include <openssl/err.h>
#include <openssl/pkcs12.h>

#include <pthread.h>
#include <cerrno>
#include <fstream>

using namespace std;


namespace Seiscomp {
namespace Wired {


namespace {


struct SSlDeleter {
	void operator()(EVP_PKEY *key)
	{
		EVP_PKEY_free(key);
	}

	void operator()(PKCS12 *p12)
	{
		PKCS12_free(p12);
	}

	void operator()(X509 *x509)
	{
		X509_free(x509);
	}
};

using X509Ptr = std::unique_ptr<X509, SSlDeleter>;
using EVP_PKEYPtr = std::unique_ptr<EVP_PKEY, SSlDeleter>;
using PKCS12Ptr = std::unique_ptr<PKCS12, SSlDeleter>;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
pthread_mutex_t *ssl_mutex_buffer = nullptr;


unsigned long SSL_thread_id_function(void) {
	return reinterpret_cast<unsigned long>(pthread_self());
}


void SSL_locking_function(int mode, int id, const char *, int) {
	if ( mode & CRYPTO_LOCK )
		pthread_mutex_lock(&ssl_mutex_buffer[id]);
	else
		pthread_mutex_unlock(&ssl_mutex_buffer[id]);
}


void SSL_static_init() {
	if ( ssl_mutex_buffer == nullptr ) {
		ssl_mutex_buffer = static_cast<pthread_mutex_t*>(
			malloc(
				static_cast<size_t>(CRYPTO_num_locks()) * sizeof(pthread_mutex_t)
			)
		);
	}

	for ( int i = 0; i < CRYPTO_num_locks(); ++i )
		pthread_mutex_init(&ssl_mutex_buffer[i], nullptr);

	CRYPTO_set_id_callback(SSL_thread_id_function);
	CRYPTO_set_locking_callback(SSL_locking_function);
}


void SSL_static_cleanup() {
	CRYPTO_set_id_callback(nullptr);
	CRYPTO_set_locking_callback(nullptr);

	if ( ssl_mutex_buffer == nullptr )
		return;

	for ( int i = 0; i < CRYPTO_num_locks(); ++i )
		pthread_mutex_destroy(&ssl_mutex_buffer[i]);

	free(ssl_mutex_buffer);
	ssl_mutex_buffer = nullptr;
}


void SSL_CTX_up_ref(SSL_CTX *ctx) {
	CRYPTO_add(&ctx->references, 1, CRYPTO_LOCK_SSL_CTX);
}


#endif
struct SSLInitializer {
	SSLInitializer() {
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		SSL_load_error_strings();
		ERR_load_crypto_strings();
		ERR_load_SSL_strings();
#if OPENSSL_VERSION_NUMBER < 0x10100000L

		SSL_static_init();
	}

	~SSLInitializer() {
		SSL_static_cleanup();
#endif
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
	unsigned long err = ERR_get_error();
	if ( err != 0 )
		return ERR_reason_error_string(err);
	return nullptr;
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
	if ( inet_pton(AF_INET, str, bytes) != 1 ) {
		return false;
	}

	dwords[1] = dwords[2] = dwords[3] = 0;
	dwords[0] = ntohl(dwords[0]);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Socket::IPAddress::fromStringV6(const char *str) {
	if ( inet_pton(AF_INET6, str, bytes) != 1 ) {
		return false;
	}

	for ( int i = 0; i < BYTES / 2; ++i ) {
		std::swap(bytes[i], bytes[BYTES - 1 - i]);
	}

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
		for ( int i = IPAddress::BYTES-3; i < IPAddress::BYTES; ++i ) {
			addr.s6_addr[i] = bytes[IPAddress::BYTES-1-i];
		}

		if ( inet_ntop(AF_INET6, &addr, str, MAX_IP_STRING_LEN) == nullptr ) {
			return -1;
		}

		return static_cast<int>(strlen(str));
	}
	else {
		in6_addr addr;
		memset(&addr, 0, sizeof(addr));

		for ( int i = 0; i < IPAddress::BYTES; ++i ) {
			addr.s6_addr[i] = bytes[IPAddress::BYTES - 1 - i];
		}

		if ( inet_ntop(AF_INET6, &addr, str, MAX_IP_STRING_LEN) == nullptr ) {
			return -1;
		}

		return static_cast<int>(strlen(str));
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
		case NotSupported:
			return "not supported";
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
		socklen_t optlen;

		if ( secs >= 0 ) {
			timeout.tv_sec = secs;
			timeout.tv_usec = usecs;
			opt = &timeout;
			optlen = sizeof(timeout);
		}
		else {
			opt = nullptr;
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
	socklen_t addrlen;

	struct addrinfo *res;
	struct addrinfo hints;

	setMode(Idle);

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
		setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
	}

	setNonBlocking(_flags & NonBlocking ? true : false);

	if ( _timeOutSecs >= 0 ) {
		if ( applySocketTimeout(_timeOutSecs, _timeOutUsecs) != Success ) {
			this->close();
			return Error;
		}
	}

#ifndef WIN32
	if ( ::connect(_fd, static_cast<const struct sockaddr *>(&addr), addrlen) == -1 ) {
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
	socklen_t addrlen;

	struct addrinfo *res;
	struct addrinfo hints;

	setMode(Idle);

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
	if ( ::connect(_fd, static_cast<const struct sockaddr *>(&addr), addrlen) == -1 ) {
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
	if ( ::bind(_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == -1 ) {
		SEISCOMP_DEBUG("Bind: %s", strerror(errno));
		close();
		return BindError;
	}

	socklen_t size = sizeof(addr);
	if ( getsockname(_fd, reinterpret_cast<sockaddr*>(&addr), &size) != 0 ) {
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
	if ( ::bind(_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == -1 ) {
		SEISCOMP_DEBUG("Bind: %s", strerror(errno));
		close();
		return BindError;
	}

	socklen_t size = sizeof(addr);
	if ( getsockname(_fd, reinterpret_cast<sockaddr*>(&addr), &size) != 0 ) {
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
	if ( !isValid() ) return nullptr;

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int client_fd = ::accept(_fd, reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
	// AcceptError
	if( client_fd < 0 ) {
		if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
			SEISCOMP_DEBUG("Accept: %s", strerror(errno));
		}

		return nullptr;
	}

	Socket *sock = new Socket;
	sock->_fd = client_fd;

	char buf[512];
	if ( (_flags & ResolveName) &&
	     getnameinfo(reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr), buf, 512, nullptr, 0, 0) == 0 )
		sock->_hostname = buf;

	sock->_addr.set(ntohl(addr.sin_addr.s_addr));
	sock->_port = ntohs(addr.sin_port);

	if ( _flags & NoDelay ) {
		int flag = 1;
		setsockopt(sock->_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
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
ssize_t Socket::send(const char *data) {
	return write(data, strlen(data));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ssize_t Socket::write(const char *data, size_t len) {
#if !defined(MACOSX) && !defined(WIN32)
	ssize_t sent = ::send(_fd, data, len, MSG_NOSIGNAL);
#else
	ssize_t sent = ::send(_fd, data, len, 0);
#endif
	if ( sent > 0 ) {
		_bytesSent += static_cast<count_t>(sent);
	}
	return sent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ssize_t Socket::read(char *data, size_t len) {
	ssize_t recvd = ::recv(_fd, data, len, 0);
	if ( recvd > 0 ) _bytesReceived += static_cast<count_t>(recvd);
	return recvd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::SSLSocket() : _ssl(nullptr), _ctx(nullptr) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::SSLSocket(SSL_CTX *ctx, bool shared)
: _ssl(nullptr), _ctx(ctx) {
	if ( _ctx && shared ) SSL_CTX_up_ref(_ctx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::~SSLSocket() {
	close();
	cleanUp();
	if ( _ctx ) SSL_CTX_free(_ctx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSL_CTX *SSLSocket::createClientContextFromFile(const std::string &pkcs12File) {
	FILE *fp = fopen(pkcs12File.c_str(), "rb");
	if (!fp ) {
		SEISCOMP_ERROR("Failed to open PKCS12 file '%s'", pkcs12File.c_str());
		return nullptr;
	}

	PKCS12 *p12 = d2i_PKCS12_fp(fp, NULL);
	fclose (fp);

	PKCS12Ptr p12Ptr(p12);
	if ( !p12Ptr ) {
		SEISCOMP_ERROR("Loading client certificate failed");
		ERR_print_errors_fp(stderr);
		return nullptr;
	}

	return createClientContext(p12Ptr.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSL_CTX *SSLSocket::createClientContext(const std::string &cert) {
	PKCS12 *p12 = nullptr;
	const unsigned char *data = reinterpret_cast<const unsigned char *>(cert.c_str());
	d2i_PKCS12(&p12, &data, cert.size());

	PKCS12Ptr p12Ptr(p12);
	return createClientContext(p12Ptr.get());
}

SSL_CTX *SSLSocket::createClientContext(PKCS12 *p12) {
	X509 *cert = nullptr;
	EVP_PKEY *key = nullptr;

	if ( !PKCS12_parse(p12, nullptr, &key, &cert, nullptr) ) {
		PKCS12_free(p12);
		ERR_print_errors_fp(stderr);
		return nullptr;
	}

	X509Ptr certPtr(cert);
	EVP_PKEYPtr keyPtr(key);

	// Set the certificate to be used
	SSL_CTX *ctx(SSL_CTX_new(SSLv23_client_method()));
	SEISCOMP_DEBUG("Loading client certificate from X509 cert");
	if ( !SSL_CTX_use_certificate(ctx, cert) ) {
		SEISCOMP_ERROR("Loading client certificate failed");
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return nullptr;
	}


	// Set the private key to be used
	SEISCOMP_DEBUG("Loading private key");
	if ( SSL_CTX_use_PrivateKey(ctx, key) <= 0 ) {
		SEISCOMP_ERROR("Loading private key failed");
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return nullptr;
	}

	SEISCOMP_DEBUG("Verifying keys...");
	if ( !SSL_CTX_check_private_key(ctx)) {
		SEISCOMP_ERROR("Private key check failed");
		SSL_CTX_free(ctx);
		return nullptr;
	}

	return ctx;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSL_CTX *SSLSocket::createClientContext(const char *pemCert, const char *pemKey) {
	SSL_CTX *ctx(SSL_CTX_new(SSLv23_client_method()));

	// Set the server certificate to be used
	SEISCOMP_DEBUG("Loading client certificate %s", pemCert);
	if ( !SSL_CTX_use_certificate_file(ctx, pemCert, SSL_FILETYPE_PEM) ) {
		SEISCOMP_ERROR("Loading client certificate failed: %s", pemCert);
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return nullptr;
	}

	// Set the private key to be used
	SEISCOMP_DEBUG("Loading private key %s", pemKey);
	if ( SSL_CTX_use_PrivateKey_file(ctx, pemKey, SSL_FILETYPE_PEM) <= 0 ) {
		SEISCOMP_ERROR("Loading private key failed: %s", pemKey);
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return nullptr;
	}

	// Verify that the private key matches the server certificate
	SEISCOMP_DEBUG("Verifying keys...");
	if ( !SSL_CTX_check_private_key(ctx)) {
		SEISCOMP_ERROR("Private key check failed");
		SSL_CTX_free(ctx);
		return nullptr;
	}

	return ctx;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSL_CTX *SSLSocket::createServerContext(const char *pemCert, const char *pemKey) {
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
		return nullptr;
	}

	// Set the private key to be used
	SEISCOMP_DEBUG("Loading private key %s", pemKey);
	if ( SSL_CTX_use_PrivateKey_file(ctx, pemKey, SSL_FILETYPE_PEM) <= 0 ) {
		SEISCOMP_ERROR("Loading private key failed: %s", pemKey);
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return nullptr;
	}

	// Verify that the private key matches the server certificate
	SEISCOMP_DEBUG("Verifying keys...");
	if ( !SSL_CTX_check_private_key(ctx)) {
		SEISCOMP_ERROR("Private key check failed");
		SSL_CTX_free(ctx);
		return nullptr;
	}

	return ctx;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status SSLSocket::bind(IPAddress ip, port_t port) {
	if ( _ctx == nullptr ) {
		SEISCOMP_DEBUG("Bind: no SSL context");
		return BindError;
	}

	return Socket::bind(ip, port);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status SSLSocket::bindV6(IPAddress ip, port_t port) {
	if ( _ctx == nullptr ) {
		SEISCOMP_DEBUG("Bind: no SSL context");
		return BindError;
	}

	return Socket::bindV6(ip, port);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket* SSLSocket::accept() {
	if ( !isValid() ) return nullptr;

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int client_fd = ::accept(_fd, reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
	// AcceptError
	if( client_fd < 0 ) {
		if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
			SEISCOMP_DEBUG("Accept: %s", strerror(errno));
		}

		return nullptr;
	}

	SSL *ssl = SSL_new(_ctx);
	if ( ssl == nullptr ) {
		SEISCOMP_ERROR("Failed to create SSL channel from context");
		::close(client_fd);
		return nullptr;
	}

	SSL_set_fd(ssl, client_fd);
	SSL_set_shutdown(ssl, 0);
	SSL_set_accept_state(ssl);

	SSLSocket *sock = new SSLSocket();
	sock->_fd = client_fd;
	sock->_ssl = ssl;

	if ( _flags & NoDelay ) {
		int flag = 1;
		setsockopt(sock->_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
	}

	char buf[512];
	if ( (_flags & ResolveName) &&
	     getnameinfo(reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr), buf, 512, nullptr, 0, 0) == 0 )
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
ssize_t SSLSocket::write(const char *data, size_t len) {
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
					SEISCOMP_DEBUG("SSL accept failed with error %d with verify result %ld",
					               err, SSL_get_verify_result(_ssl));
					errno = EINVAL;
					return 0;
			}
		}

		_flags &= ~InAccept;

		if ( _session ) {
			_session->accepted();
		}
	}

	int ret = SSL_write(_ssl, data, static_cast<int>(len));
	if ( ret > 0 ) {
		_bytesSent += static_cast<count_t>(ret);
		return static_cast<ssize_t>(ret);
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

	return static_cast<ssize_t>(ret);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ssize_t SSLSocket::read(char *data, size_t len) {
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
					SEISCOMP_DEBUG("SSL accept failed with error %d with verify result %ld",
					               err, SSL_get_verify_result(_ssl));
					errno = EINVAL;
					return 0;
			}
		}

		_flags &= ~InAccept;

		if ( _session ) {
			_session->accepted();
		}
	}

	int ret = SSL_read(_ssl, data, static_cast<int>(len));
	if ( ret > 0 ) {
		_bytesReceived += static_cast<count_t>(ret);
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

	return static_cast<ssize_t>(ret);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status SSLSocket::connect(const std::string &hostname, port_t port) {
	close();
	cleanUp();
	setMode(Idle);

	if ( !_ctx ) _ctx = SSL_CTX_new(SSLv23_client_method());
	if ( !_ctx ) {
		SEISCOMP_DEBUG("Invalid SSL context");
		return ConnectError;
	}

	SSL_CTX_set_mode(_ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	Status s = Socket::connect(hostname, port);
	if ( s != Success )
		return s;

	_ssl = SSL_new(_ctx);
	if ( _ssl == nullptr ) {
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
	close();
	cleanUp();
	setMode(Idle);

	if ( !_ctx ) _ctx = SSL_CTX_new(SSLv23_client_method());
	if ( !_ctx ) {
		SEISCOMP_DEBUG("Invalid SSL context");
		return ConnectError;
	}

	Status s = Socket::connectV6(hostname, port);
	if ( s != Success )
		return s;

	_ssl = SSL_new(_ctx);
	if ( _ssl == nullptr ) {
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
Socket::Status SSLSocket::take(Socket *socket) {
	close();
	cleanUp();

	if ( !_ctx ) _ctx = SSL_CTX_new(SSLv23_client_method());
	if ( !_ctx ) {
		SEISCOMP_DEBUG("Invalid SSL context");
		return ConnectError;
	}

	_ssl = SSL_new(_ctx);
	if ( _ssl == nullptr ) {
		SEISCOMP_DEBUG("Failed to create SSL context");
		return ConnectError;
	}

	_hostname = socket->hostname();
	_addr = socket->address();
	_port = socket->port();
	_flags = socket->_flags;

	_bytesSent = socket->_bytesSent;
	_bytesReceived = socket->_bytesReceived;
	_timeOutSecs = socket->_timeOutSecs;
	_timeOutUsecs = socket->_timeOutUsecs;
	_fd = socket->takeFd();

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
void SSLSocket::cleanUp() {
	if ( _ssl ) {
		SSL_free(_ssl);
		_ssl = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
