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


#ifndef SEISCOMP_IO_SOCKET_H
#define SEISCOMP_IO_SOCKET_H

#include <string>
#include <set>
#include <iostream>
#include <sstream>

#include <openssl/ssl.h>

#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/utils/timer.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/core.h>

#define BUFSIZE 4096
#define RECSIZE 512

namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API SocketException: public Seiscomp::IO::RecordStreamException {
	public:
		SocketException(): RecordStreamException("Socket exception") {}
		SocketException(const std::string& what): RecordStreamException(what) {}
};

class SC_SYSTEM_CORE_API SocketCommandException: public SocketException {
	public:
		SocketCommandException(): SocketException("command not accepted") {}
		SocketCommandException(const std::string& what): SocketException(what) {}
};

class SC_SYSTEM_CORE_API SocketTimeout: public SocketException {
	public:
		SocketTimeout(): SocketException("timeout") {}
		SocketTimeout(const std::string& what): SocketException(what) {}
};

class SC_SYSTEM_CORE_API SocketResolveError: public SocketException {
	public:
		SocketResolveError(): SocketException("cannot resolve hostname") {}
		SocketResolveError(const std::string& what): SocketException(what) {}
};


DEFINE_SMARTPOINTER(Socket);
class SC_SYSTEM_CORE_API Socket: public Seiscomp::Core::InterruptibleObject {
	public:
		Socket();
		virtual ~Socket();

	public:
		void setTimeout(int seconds);
		void startTimer();
		void stopTimer();
		virtual void open(const std::string& serverLocation);
		virtual void close();
		bool isOpen();
		bool tryReconnect();
		void write(const std::string& s);
		std::string readline();
		std::string read(int size);
		std::string sendRequest(const std::string& request, bool waitResponse);
		bool isInterrupted();
		void interrupt();
		int poll();

		int takeFd();

	protected:
		void handleInterrupt(int) throw();
		virtual int readImpl(char *buf, int count);
		virtual int writeImpl(const char *buf, int count);

	protected:
		void fillbuf();
		int connectSocket(struct sockaddr *addr, int len);
		int nonblockSocket();
		int addrSocket(char *hostname, char *port, struct sockaddr *addr, size_t *len) ;

	protected:
		int _sockfd;

	private:
		enum { READ = 0, WRITE = 1 };
		int _pipefd[2];
		char _buf[BUFSIZE + 1];
		int _rp;
		int _wp;
		int _timeout;
		Util::StopWatch _timer;
		bool _interrupt;
		bool _reconnect;
		std::string _eol;
};


DEFINE_SMARTPOINTER(SSLSocket);
class SC_SYSTEM_CORE_API SSLSocket: public Socket {
	public:
		SSLSocket();
		~SSLSocket() override;

		void open(const std::string& serverLocation) override;
		void close() override;

		void setFd(int fd);

	protected:
		int readImpl(char *buf, int count) override;
		int writeImpl(const char *buf, int count) override;

	private:
		void cleanUp();

	private:
		BIO      *_bio;
		SSL      *_ssl;
		SSL_CTX  *_ctx;
		char      _errBuf[120];
};


} // namespace IO
} // namespace Seiscomp

#endif

