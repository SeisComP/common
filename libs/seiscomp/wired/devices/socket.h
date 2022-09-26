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


#ifndef SEISCOMP_WIRE_SOCKET_H
#define SEISCOMP_WIRE_SOCKET_H

#include <seiscomp/wired/device.h>

#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/pkcs12.h>
#include <ostream>


namespace Seiscomp {
namespace Wired {


DEFINE_SMARTPOINTER(Socket);

class SC_SYSTEM_CORE_API Socket : public Device {
	DECLARE_CASTS(Socket)

	public:
		enum Status {
			Success = 0,
			Error,
			AllocationError,
			ReuseAdressError,
			BindError,
			ListenError,
			AcceptError,
			ConnectError,
			AddrInfoError,
			Timeout,
			InvalidSocket,
			InvalidPort,
			InvalidAddressFamily,
			InvalidAddress,
			InvalidHostname,
			NotSupported
		};

		/**
		 * @brief The IP union holds either a IPv4 or IPv6 address.
		 */
		union IPAddress {
			enum {
				DWORDS = 4,
				BYTES = 4*4,
				MAX_IP_STRING_LEN = 46
			};

			IPAddress() { dwords[0] = dwords[1] = dwords[2] = dwords[3] = 0; }
			explicit IPAddress(uint32_t addr) { dwords[0] = addr; dwords[1] = dwords[2] = dwords[3] = 0; }
			explicit IPAddress(uint32_t addr[DWORDS]) { dwords[0] = addr[0]; dwords[1] = addr[1]; dwords[2] = addr[2]; dwords[3] = addr[3]; }
			IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
				set(a,b,c,d);
			}

			bool operator==(const IPAddress &other) const {
				return (dwords[0] == other.dwords[0])
				    && (dwords[1] == other.dwords[1])
				    && (dwords[2] == other.dwords[2])
				    && (dwords[3] == other.dwords[3]);
			}

			bool equalV4(const IPAddress &other) const {
				return dwords[0] == other.dwords[0];
			}

			bool equalV4(uint32_t addr) const {
				return dwords[0] == addr;
			}

			bool equalV6(const IPAddress &other) const {
				return *this == other;
			}

			bool zero() const {
				return !dwords[0] && !dwords[1] && !dwords[2] && !dwords[3];
			}

			bool not_zero() const {
				return dwords[0] || dwords[1] || dwords[2] || dwords[3];
			}

			/**
			 * @brief Returns whether the address is a v4 or v6 address or more
			 *        technically, if the upper 96bits are set.
			 * @return flag indicating a v4 address
			 */
			bool isV4() const {
				return !dwords[1] && !dwords[2] && !dwords[3];
			}

			void set(uint8_t a, uint8_t b,
			         uint8_t c, uint8_t d) {
				v4.A = a;
				v4.B = b;
				v4.C = c;
				v4.D = d;
				dwords[1] = dwords[2] = dwords[3] = 0;
			}

			void set(uint32_t ipv4addr) {
				dwords[0] = ipv4addr;
				dwords[1] = dwords[2] = dwords[3] = 0;
			}

			bool fromString(const char *);
			bool fromStringV4(const char *);
			bool fromStringV6(const char *);

			/**
			 * @brief Converts the IP to a string representation. The input
			 *        string must at least have space for 46 characters.
			 * @return The number of bytes written including the terminating
			 *         null byte.
			 */
			int toString(char *, bool anonymize = false) const;

			uint32_t dwords[DWORDS];
			uint8_t bytes[DWORDS*4];

			struct {
				uint8_t D;
				uint8_t C;
				uint8_t B;
				uint8_t A;
			} v4;

			uint32_t dwordV4;
		};

		typedef uint16_t port_t;


	public:
		Socket();
		Socket(int fd, const std::string &hostname = "localhost", port_t port = 0);
		~Socket() override;


	public:
		static const char *toString(Status);

		void shutdown();
		void close() override;

		const std::string &hostname() const;
		port_t port() const;
		IPAddress address() const;

		ssize_t send(const char *data);

		ssize_t write(const char *data, size_t len) override;
		ssize_t read(char *data, size_t len) override;

		//! Sets the socket timeout. This utilizes setsockopt which does not
		//! work in non blocking sockets.
		Status setSocketTimeout(int secs, int usecs);

		Device::Status setNonBlocking(bool nb) override;
		bool isNonBlocking() const { return _flags & NonBlocking; }

		bool isAccepted() const { return !(_flags & InAccept); }

		Status setReuseAddr(bool ra);
		Status setNoDelay(bool nd);

		//! Switches resolving host names when a new connection is accepted.
		//! This feature is disabled by default and can significantly slow
		//! down the server if enabled.
		Status setResolveHostnames(bool rh);

		virtual Status connect(const std::string &hostname, port_t port);

		/**
		 * @brief Connects to an IPv6 host.
		 * @param hostname The hostname or IP address
		 * @param port The port number
		 * @return The status of the connect operation
		 */
		virtual Status connectV6(const std::string &hostname, port_t port);

		virtual Status bind(IPAddress ip, port_t port);
		virtual Status bindV6(IPAddress ip, port_t port);

		Status listen(int backlog = 10);
		virtual Socket *accept();

		count_t rx() const { return _bytesReceived; }
		count_t tx() const { return _bytesSent; }


	protected:
		Status applySocketTimeout(int secs, int usecs);


	protected:
		enum Flags {
			NoFlags      = 0x0000,
			ReuseAddress = 0x0001,
			NonBlocking  = 0x0002,
			ResolveName  = 0x0004,
			NoDelay      = 0x0008,
			//Future2    = 0x0010,
			//Future3    = 0x0020,
			//Future4    = 0x0040,
			//Future5    = 0x0080,
			InAccept     = 0x0100
		};

		static int  _socketCount;

		std::string _hostname;
		IPAddress   _addr;
		port_t      _port;
		uint16_t    _flags;

		count_t     _bytesSent;
		count_t     _bytesReceived;

		int         _timeOutSecs;
		int         _timeOutUsecs;

	friend class SSLSocket;
};


DEFINE_SMARTPOINTER(SSLSocket);

class SSLSocket : public Socket {
	public:
		SSLSocket();
		/**
		 * @brief Creates an SSL socket with an SSL context. The socket will
		 *        then re-use this context and not create a default one.
		 * @param ctx The SSL context pointer.
		 * @param shared If true then the reference count of the SSL context
		 *               will be increased. That might be important if the
		 *               same context is shared between many SSL sockets.
		 *               The caller must then call SSL_CTX_free on the passed
		 *               context to release the refence count again.
		 */
		SSLSocket(SSL_CTX *ctx, bool shared = false);
		~SSLSocket();

	public:
		Status bind(IPAddress ip, port_t port) override;
		Status bindV6(IPAddress ip, port_t port) override;

		void close() override;

		Socket *accept() override;

		ssize_t write(const char *data, size_t len) override;
		ssize_t read(char *data, size_t len) override;

		Status connect(const std::string &hostname, port_t port) override;
		Status connectV6(const std::string &hostname, port_t port) override;

		/**
		 * @brief Takes the connection from a socket and renders the source
		 *        socket invalid.
		 * @param socket The socket the connection parameters will be taken from.
		 * @return Status flag
		 */
		Status take(Socket *socket);

		SSL_CTX *sslContext() const;
		SSL *ssl() const;

		/**
		 * @brief Returns the peer certificate presented by the other end.
		 * @return An X509 pointer which is managed by this instance. It must
		 *         not be deleted.
		 */
		X509 *peerCertificate() const;

		/**
		 * @brief Creates SSL client context from PKCS12 file
		 * @param pkcs12File Absolute path to pkcs12File
		 * @return The client SSL context
		 */
		static SSL_CTX *createClientContextFromFile(const std::string &pkcs12File);

		/**
		 * @brief Creates SSL client context from Base64 encoded PKCS12 certificate
		 * @param cert The Base64 encoded PKCS12 certificate
		 * @return The client SSL context
		 */
		static SSL_CTX *createClientContext(const std::string &cert);

		/**
		 * @brief Creates SSL client context from OpenSSL PKCS12 structure
		 * @param p12 Pointer to PKCS12 structure
		 * @return The client SSL context
		 */
		static SSL_CTX *createClientContext(PKCS12 *p12);
		static SSL_CTX *createClientContext(const char *pemCert, const char *pemKey);
		static SSL_CTX *createServerContext(const char *pemCert, const char *pemKey);

	private:
		void cleanUp();

	private:
		SSL     *_ssl;
		SSL_CTX *_ctx;
};


inline SSL_CTX *SSLSocket::sslContext() const {
	return _ctx;
}

inline SSL *SSLSocket::ssl() const {
	return _ssl;
}

inline X509 *SSLSocket::peerCertificate() const {
	return SSL_get_peer_certificate(_ssl);
}


/**
 * @brief Returns a human readable description for the last
 *        failed operation.
 * @return The description string
 */
const char *getSystemError(Socket *socket);

/**
 * @brief Returns a human readable description for the last
 *        failed SSL operation.
 * @return The description string
 */
const char *getSSLSystemError();


std::ostream &operator<<(std::ostream &, const Socket::IPAddress &ip);
std::ostream &operator<<(std::ostream &, const Anonymize<Socket::IPAddress> &ip);

std::string toString(const Socket::IPAddress &ip);


}
}

#endif
