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


#define SEISCOMP_COMPONENT FDSNWSConnection

#include <cstdlib>
#include <string>
#include <set>
#include <utility>
#include <cerrno>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/utils/base64.h>
#include <seiscomp/utils/url.h>

#include "fdsnws.h"


#ifdef WIN32
#undef min
#undef max
#define posix_read _read
typedef int ssize_t;
#else
#define posix_read read
#endif


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
using namespace Seiscomp::RecordStream;


REGISTER_RECORDSTREAM(FDSNWSConnection, "fdsnws");
REGISTER_RECORDSTREAM(FDSNWSSSLConnection, "fdsnwss");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSConnectionBase::FDSNWSConnectionBase(const char *protocol, IO::Socket *socket, int defaultPort)
: _protocol(protocol)
, _socket(socket)
, _defaultPort(defaultPort)
, _readingData(false)
, _chunkMode(false)
, _remainingBytes(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setSource(const std::string &source) {
	size_t pos = source.find('/');
	if ( pos != string::npos ) {
		_url = source.substr(pos);
		_host = source.substr(0, pos);
	}
	else {
		_url = "/fdsnws/dataselect/1/query";
		_host = source;
	}

	if ( _host.find(':') == string::npos ) {
		_host += ":";
		_host += Core::toString(_defaultPort);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setRecordType(const char* type) {
	return !strcmp(type, "mseed");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::addStream(const string &net, const string &sta,
                                     const string &loc, const string &cha) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net, sta, loc, cha));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::addStream(const string &net, const string &sta,
                                     const string &loc, const string &cha,
                                     const OPT(Time) &stime,
                                     const OPT(Time) &etime) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(net, sta, loc, cha, stime, etime));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setStartTime(const OPT(Time) &stime) {
	_stime = stime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setEndTime(const OPT(Time) &etime) {
	_etime = etime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setTimeout(int seconds) {
	_socket->setTimeout(seconds);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::clear() {
	this->~FDSNWSConnectionBase();
	new(this) FDSNWSConnectionBase(_protocol, _socket.get(), _defaultPort);
	setSource(_host + _url);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Hopefully safe to be called from another thread
void FDSNWSConnectionBase::close() {
	_socket->interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::reconnect() {
	if ( _socket->isOpen() )
		_socket->close();

	_readingData = false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *FDSNWSConnectionBase::getProxy() const {
	const char *proxy = nullptr;

	proxy = getenv("NO_PROXY");
	if ( !proxy )
		proxy = getenv("no_proxy");

	if ( proxy ) {
		if ( !strcmp(proxy, "*") )
			return nullptr;

		string remoteHostname = _host;
		size_t pos = remoteHostname.rfind(':');
		if ( pos != string::npos ) {
			remoteHostname.erase(remoteHostname.begin() + pos, remoteHostname.end());
		}

		// Check no proxy configuration
		size_t lproxy = strlen(proxy), ltok;
		const char *tok;
		while ( (tok = Core::tokenize(proxy, ",", lproxy, ltok)) ) {
			Core::trimFront(tok, ltok);
			Core::trimBack(tok, ltok);
			if ( !ltok ) continue;

			// Match from right
			if ( ltok > remoteHostname.size() )
				continue;

			bool match = true;
			for ( size_t i = 0; i < ltok; ++i ) {
				if ( remoteHostname[remoteHostname.size()-ltok+i] != tok[i] ) {
					match = false;
					break;
				}
			}

			// Match!
			if ( match ) {
				SEISCOMP_DEBUG("'%s' in no_proxy, ignore proxy configuration",
				               remoteHostname.c_str());
				return nullptr;
			}
		}

		proxy = nullptr;
	}

	if ( !strcmp(_protocol, "http") ) {
		proxy = getenv("http_proxy");
		if ( !proxy )
			proxy = getenv("HTTP_PROXY");
	}
	else if ( !strcmp(_protocol, "https") ) {
		proxy = getenv("HTTPS_PROXY");
		if ( !proxy )
			proxy = getenv("https_proxy");
	}

	return proxy;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FDSNWSConnectionBase::openConnection(const std::string &host) {
	const char *proxyHost = getProxy();

	if ( proxyHost ) {
		Util::Url url(proxyHost);
		string proxyScheme = url.scheme();
		if ( proxyScheme.empty() )
			proxyScheme = "http";

		if ( proxyScheme != _protocol ) {
			if ( proxyScheme == "http" ) {
				_socket = new IO::Socket;
			}
			else if ( proxyScheme == "https" ) {
				_socket = new IO::SSLSocket;
			}
			else {
				throw GeneralException("Request and proxy protocol mismatch");
			}
		}

		size_t port = url.port() ? *url.port() : 3128;

		if ( url.username().empty() || url.password().empty() )
			SEISCOMP_DEBUG("Connect to web proxy at %s://%s:%d",
			               proxyScheme.c_str(),
			               url.host().c_str(), int(port));
		else
			SEISCOMP_DEBUG("Connect to web proxy at %s://%s:****@%s:%d",
			               proxyScheme.c_str(),
			               url.username().c_str(), url.host().c_str(), int(port));

		_socket->open(url.host() + ":" + toString(port));

		if ( !strcmp(_protocol, "https") ) {
			// Issue connect
			_socket->sendRequest("CONNECT " + _host + " HTTP/1.1", false);
			if ( !url.username().empty() && !url.password().empty() ) {
				string authHeader = "Proxy-Authorization: Basic ";
				Util::encodeBase64(authHeader, url.username() + ":" + url.password());
				_socket->sendRequest(authHeader, false);
			}
			_socket->sendRequest("Host: " + _host, false);
			_socket->sendRequest("", false);

			string line;

			if ( proxyScheme == "http" ) {
				// Now read result unbuffered and blocking
				int fd = _socket->takeFd();
				int flags = fcntl(fd, F_GETFL, 0);
				flags &= ~O_NONBLOCK;
				fcntl(fd, F_SETFL, flags);

				char c;
				while ( ::read(fd, &c, 1) == 1 ) {
					if ( c == '\r' ) continue;
					if ( c == '\n' )
						break;

					line += c;
				}

				while ( ::read(fd, &c, 1) == 1 ) {
					if ( c == '\n' )
						break;
				}

				_socket = new IO::SSLSocket;
				static_cast<IO::SSLSocket*>(_socket.get())->setFd(fd);
			}
			else
				line = _socket->readline();

			SEISCOMP_DEBUG("%s", line.c_str());
			if ( line.compare(0, 7, "HTTP/1.") != 0 )
				throw GeneralException("server sent invalid response: " + line);

			size_t pos;
			pos = line.find(' ');
			if ( pos == string::npos )
				throw GeneralException("server sent invalid response: " + line);

			line.erase(0, pos+1);

			pos = line.find(' ');
			if ( pos == string::npos )
				throw GeneralException("server sent invalid response: " + line);

			int code;
			if ( !fromString(code, line.substr(0, pos)) ) {
				throw GeneralException("server sent invalid status code: " + line.substr(0, pos));
			}

			if ( code != 200 ) {
				throw GeneralException("proxy returned code: " + line.substr(0, pos));
			}
		}
	}
	else {
		_socket->open(host);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FDSNWSConnectionBase::handshake() {
	string request;

	for ( auto it = _streams.begin(); it != _streams.end(); ++it ) {
		SEISCOMP_DEBUG("Request: %s", it->str(_stime, _etime).c_str());
		if ( (!it->startTime() && !_stime) || (!it->endTime() && !_etime) ) {
			/* invalid time window ignore stream */
			SEISCOMP_WARNING("... has invalid time window -> ignore this request above");
			continue;
		}

		request += it->network() + " " + it->station() + " ";
		if ( it->location().empty() ) {
			request += "--";
		}
		else {
			request += it->location();
		}

		request += " ";
		request += it->channel();
		request += " ";

		if ( it->startTime() ) {
			request += it->startTime()->toString("%FT%T.%f");
		}
		else {
			request += (_stime ? *_stime : Time()).toString("%FT%T.%f");
		}
		request += " ";
		if ( it->endTime() ) {
			request += it->endTime()->toString("%FT%T.%f");
		}
		else {
			request += (_etime ? *_etime : Time()).toString("%FT%T.%f");
		}
		request += "\r\n";
	}

	SEISCOMP_DEBUG("POST %s://%s%s", _protocol, _host.c_str(), _url.c_str());
	SEISCOMP_DEBUG("Sending request:\n%s", request.c_str());

	const char *proxyServer = getProxy();
	if ( proxyServer && !strcmp(_protocol, "http") ) {
		Util::Url url(proxyServer);
		_socket->sendRequest(string("POST ") + _protocol + "://" + _host + _url + " HTTP/1.1", false);
		// Only http protocol need authorization here. Otherwise it has been
		// handled already in openConnection()
		if ( !url.username().empty() && !url.password().empty() ) {
			string authHeader = "Proxy-Authorization: Basic ";
			Util::encodeBase64(authHeader, url.username() + ":" + url.password());
			_socket->sendRequest(authHeader, false);
		}
	}
	else
		_socket->sendRequest("POST " + _url + " HTTP/1.1", false);
	_socket->sendRequest("Host: " + _host, false);
	_socket->sendRequest("User-Agent: Mosaic/1.0", false);
	_socket->sendRequest("Content-Type: text/plain", false);
	_socket->sendRequest("Content-Length: " + toString(request.size()), false);
	_socket->sendRequest("", false);
	_socket->write(request);

	string line = _socket->readline();
	if ( line.compare(0, 7, "HTTP/1.") != 0 )
		throw GeneralException("server sent invalid response: " + line);

	size_t pos;
	pos = line.find(' ');
	if ( pos == string::npos )
		throw GeneralException("server sent invalid response: " + line);

	line.erase(0, pos+1);

	pos = line.find(' ');
	if ( pos == string::npos )
		throw GeneralException("server sent invalid response: " + line);

	int code;
	if ( !fromString(code, line.substr(0, pos)) ) {
		throw GeneralException("server sent invalid status code: " + line.substr(0, pos));
	}

	if ( code == 200 ) {
		// Keep on reading body
	}
	else if ( code == 204 ) {
		// No data
		_remainingBytes = 0;
		return;
	}
	else if ( code / 100 == 3 ) {
		// Redirect
	}
	else {
		_error = "server request error: " + line;
	}

	_remainingBytes = -1;

	int lc = 0;
	string redirectLocation;

	while ( !_socket->isInterrupted() ) {
		++lc;
		line = _socket->readline();
		if ( line.empty() ) break;

		SEISCOMP_DEBUG("[%02d] %s", lc, line.c_str());

		// Remove whitespaces
		trim(line);

		pos = line.find(':');
		if ( pos != string::npos )
			// Transform header values to upper case
			transform(line.begin(), line.begin()+pos, line.begin(), ::toupper);

		if ( line.compare(0, 18, "TRANSFER-ENCODING:") == 0 ) {
			line.erase(line.begin(), line.begin()+18);
			trim(line);
			if ( line == "chunked" ) {
				_chunkMode = true;
				SEISCOMP_DEBUG(" -> enabled 'chunked' transfer");
			}
		}
		else if ( line.compare(0, 15, "CONTENT-LENGTH:") == 0 ) {
			if ( !fromString(_remainingBytes, trim(line.substr(15))) ) {
				throw GeneralException("invalid Content-Length response");
			}
			if ( _remainingBytes < 0 ) {
				throw GeneralException("Content-Length must be positive");
			}
		}
		else if ( line.compare(0, 9, "LOCATION:") == 0 ) {
			redirectLocation = line.substr(pos+1);
			trim(redirectLocation);
		}
	}

	if ( _chunkMode ) {
		if ( _remainingBytes >= 0 )
			throw GeneralException("protocol error: transfer encoding is chunked and content length given");
		_remainingBytes = 0;
	}

	if ( code / 100 == 3 ) {
		if ( redirectLocation.empty() ) {
			_error = "Invalid redirect response";
			SEISCOMP_ERROR("302 returned but empty Location header");
		}
		else {
			SEISCOMP_DEBUG("FDSNWS request was redirected to %s", redirectLocation.c_str());
			pos = redirectLocation.find("://");
			if ( pos == string::npos ) {
				if ( redirectLocation[0] == '/' ) {
					redirectLocation = _host + redirectLocation;
					_socket->close();
				}
				else {
					_error = "Invalid redirect location protocol";
					SEISCOMP_ERROR("Redirect URL invalid: %s", redirectLocation.c_str());
					return;
				}
			}
			else {
				if ( redirectLocation.compare(0, pos, "http") == 0 ) {
					_socket = new IO::Socket;
					_protocol = "http";
					_defaultPort = 80;
				}
				else if ( redirectLocation.compare(0, pos, "https") == 0 ) {
					_socket = new IO::SSLSocket;
					_protocol = "https";
					_defaultPort = 443;
				}
				else {
					_error = "Invalid redirect location protocol";
					SEISCOMP_ERROR("Redirect URL invalid: %s", redirectLocation.c_str());
					return;
				}

				redirectLocation.erase(0, pos+3);
			}

			setSource(redirectLocation);
			openConnection(_host);
			handshake();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *FDSNWSConnectionBase::next() {
	if ( _readingData && !_socket->isOpen() )
		return nullptr;

	_socket->startTimer();

	if ( !_readingData ) {
		try {
			openConnection(_host);
			handshake();
		}
		catch ( const GeneralException &e ) {
			SEISCOMP_ERROR("fdsnws: %s", e.what());
			_socket->close();
			return nullptr;
		}

		_readingData = true;
		if ( !_chunkMode && _remainingBytes <= 0 ) {
			SEISCOMP_DEBUG("Content length is 0, nothing to read");
			_socket->close();
			return nullptr;
		}
	}

	try {
		while ( true ) {
			_socket->startTimer();

			if ( _error.empty() ) {
				// HACK to retrieve the record length
				string data = readBinary(RECSIZE);
				if ( !data.empty() ) {
					auto reclen = IO::MSeedRecord::Detect(data.data(), RECSIZE);
					std::istringstream stream(std::istringstream::in|std::istringstream::binary);
					if ( reclen > RECSIZE ) {
						stream.str(data + readBinary(reclen - RECSIZE));
					}
					else {
						if ( reclen <= 0 ) {
							SEISCOMP_ERROR("Retrieving the record length failed (try 512 Byte)!");
						}
						stream.str(data);
					}

					IO::MSeedRecord *rec = new IO::MSeedRecord;
					setupRecord(rec);
					try {
						rec->read(stream);
					}
					catch ( ... ) {
						delete rec;
						continue;
					}

					return rec;
				}
				else {
					_socket->close();
					break;
				}
			}
			else
				_error += readBinary(_chunkMode?512:_remainingBytes);

			if ( !_socket->isOpen() ) {
				if ( _error.size() )
					throw GeneralException(_error.c_str());
			}
		}
	}
	catch ( const GeneralException &e ) {
		SEISCOMP_ERROR("fdsnws: %s", e.what());
		_socket->close();
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string FDSNWSConnectionBase::readBinary(int size) {
	if ( size <= 0 ) return "";

	string data;
	int bytesLeft = size;

	while ( bytesLeft > 0 ) {
		if ( _chunkMode && _remainingBytes <= 0 ) {
			string r = _socket->readline();
			size_t pos = r.find(' ');
			unsigned int remainingBytes;

			if ( sscanf(r.substr(0, pos).c_str(), "%X", &remainingBytes) !=  1 )
				throw GeneralException("invalid chunk header: " + r);

			_remainingBytes = remainingBytes;

			if ( _remainingBytes <= 0 ) {
				if ( _error.size() )
					throw GeneralException(_error);
				_socket->close();
				break;
			}
		}

		int toBeRead = bytesLeft > BUFSIZE ? BUFSIZE : bytesLeft;
		if ( toBeRead > _remainingBytes ) toBeRead = _remainingBytes;

		int bytesRead = (int)data.size();
		data += _socket->read(toBeRead);
		bytesRead = (int)data.size() - bytesRead;
		if ( bytesRead <= 0 ) {
			SEISCOMP_WARNING("socket read returned not data");
			break;
		}

		_remainingBytes -= bytesRead;
		bytesLeft -= bytesRead;

		if ( _remainingBytes <= 0 ) {
			if ( _chunkMode ) {
				// Read trailing new line
				_socket->readline();
			}
			else {
				_socket->close();
			}
		}
	}

	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSConnection::FDSNWSConnection()
: FDSNWSConnectionBase("http", new IO::Socket, 80) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSSSLConnection::FDSNWSSSLConnection()
: FDSNWSConnectionBase("https", new IO::SSLSocket, 443) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
