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

#include <openssl/opensslv.h>
#if OPENSSL_VERSION_NUMBER < 0x30000000
#include <openssl/md5.h>
#else
#include <openssl/evp.h>
#endif

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


namespace {


string md5(const string &content) {
#if OPENSSL_VERSION_NUMBER < 0x30000000
	unsigned char arMD[MD5_DIGEST_LENGTH];
	unsigned int nMD = MD5_DIGEST_LENGTH;

	MD5_CTX md5;
	MD5_Init(&md5);
	MD5_Update(&md5, content.c_str(), content.length());
	MD5_Final(arMD, &md5);
#else
	EVP_MD_CTX *ctx = EVP_MD_CTX_new();
	const EVP_MD *md = EVP_md5();
	unsigned char arMD[EVP_MAX_MD_SIZE];
	unsigned int nMD;

	EVP_DigestInit_ex2(ctx, md, NULL);
	EVP_DigestUpdate(ctx, content.c_str(), content.length());
	EVP_DigestFinal_ex(ctx, arMD, &nMD);
	EVP_MD_CTX_free(ctx);
#endif

	string res;
	res.resize(nMD * 2);
	for ( unsigned int i = 0 ; i < nMD ; ++i ) {
		sprintf(&res[i * 2], "%02x", arMD[i]);
	}

	return res;
}


}


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
	_url.setUrl(source);
	if ( !_url.isValid() ) {
		SEISCOMP_ERROR("Invalid FDSNWS URL: %s", _url.status().toString());
		return false;
	}

	if ( _url.path().empty() ) {
		if ( !_url.username().empty() ) {
			_url.setPath("/fdsnws/dataselect/1/queryauth");
		}
		else {
			_url.setPath("/fdsnws/dataselect/1/query");
		}
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
bool FDSNWSConnectionBase::addStream(const string &networkCode,
                                     const string &stationCode,
                                     const string &locationCode,
                                     const string &channelCode) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(networkCode, stationCode,
	                                   locationCode, channelCode));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::addStream(const string &networkCode,
                                     const string &stationCode,
                                     const string &locationCode,
                                     const string &channelCode,
                                     const OPT(Time) &startTime,
                                     const OPT(Time) &endTime) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(networkCode, stationCode,
	                                   locationCode, channelCode,
	                                   startTime, endTime));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setStartTime(const OPT(Time) &startTime) {
	_stime = startTime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setEndTime(const OPT(Time) &endTime) {
	_etime = endTime;
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
	setSource(_url.toString());
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
	if ( _socket->isOpen() ) {
		_socket->close();
	}

	_readingData = false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *FDSNWSConnectionBase::getProxy() const {
	const char *proxy = nullptr;

	proxy = getenv("NO_PROXY");
	if ( !proxy ) {
		proxy = getenv("no_proxy");
	}

	if ( proxy ) {
		if ( !strcmp(proxy, "*") ) {
			return nullptr;
		}

		string remoteHostname = _url.host();
		size_t pos = remoteHostname.rfind(':');
		if ( pos != string::npos ) {
			remoteHostname.erase(remoteHostname.begin() + pos, remoteHostname.end());
		}

		// Check no proxy configuration
		size_t lproxy = strlen(proxy);
		size_t ltok;
		const char *tok;
		while ( (tok = Core::tokenize(proxy, ",", lproxy, ltok)) ) {
			Core::trimFront(tok, ltok);
			Core::trimBack(tok, ltok);
			if ( !ltok ) {
				continue;
			}

			// Match from right
			if ( ltok > remoteHostname.size() ) {
				continue;
			}

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
				               remoteHostname);
				return nullptr;
			}
		}

		proxy = nullptr;
	}

	if ( !strcmp(_protocol, "http") ) {
		proxy = getenv("http_proxy");
		if ( !proxy ) {
			proxy = getenv("HTTP_PROXY");
		}
	}
	else if ( !strcmp(_protocol, "https") ) {
		proxy = getenv("HTTPS_PROXY");
		if ( !proxy ) {
			proxy = getenv("https_proxy");
		}
	}

	return proxy;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FDSNWSConnectionBase::openConnection() {
	const char *proxyHost = getProxy();

	if ( !proxyHost ) {
		if ( !_url.port() ) {
			_socket->open(_url.host() + ":" + Core::toString(_defaultPort));
		}
		else {
			_socket->open(_url.host() + ":" + Core::toString(*_url.port()));
		}

		return;
	}

	Util::Url url(proxyHost);
	string proxyScheme = url.scheme();
	if ( proxyScheme.empty() ) {
		proxyScheme = "http";
	}

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

	if ( url.username().empty() || url.password().empty() ) {
		SEISCOMP_DEBUG("Connect to web proxy at %s://%s:%zu",
		               proxyScheme, url.host(), port);
	}
	else {
		SEISCOMP_DEBUG("Connect to web proxy at %s://%s:****@%s:%zu",
		               proxyScheme, url.username(), url.host(), port);
	}

	_socket->open(url.host() + ":" + toString(port));

	if ( !strcmp(_protocol, "https") ) {
		// Issue connect
		_socket->sendRequest("CONNECT " + _url.host() + " HTTP/1.1", false);
		if ( !url.username().empty() && !url.password().empty() ) {
			string authHeader = "Proxy-Authorization: Basic ";
			Util::encodeBase64(authHeader, url.username() + ":" + url.password());
			_socket->sendRequest(authHeader, false);
		}
		_socket->sendRequest("Host: " + _url.host(), false);
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
				if ( c == '\r' ) {
					continue;
				}
				if ( c == '\n' ) {
					break;
				}

				line += c;
			}

			while ( ::read(fd, &c, 1) == 1 ) {
				if ( c == '\n' ) {
					break;
				}
			}

			_socket = new IO::SSLSocket;
			static_cast<IO::SSLSocket*>(_socket.get())->setFd(fd);
		}
		else {
			line = _socket->readline();
		}

		SEISCOMP_DEBUG_S(line);
		if ( line.compare(0, 7, "HTTP/1.") != 0 ) {
			throw GeneralException("server sent invalid response: " + line);
		}

		size_t pos;
		pos = line.find(' ');
		if ( pos == string::npos ) {
			throw GeneralException("server sent invalid response: " + line);
		}

		line.erase(0, pos+1);

		pos = line.find(' ');
		if ( pos == string::npos ) {
			throw GeneralException("server sent invalid response: " + line);
		}

		int code;
		if ( !fromString(code, line.substr(0, pos)) ) {
			throw GeneralException("server sent invalid status code: " + line.substr(0, pos));
		}

		if ( code != 200 ) {
			throw GeneralException("proxy returned code: " + line.substr(0, pos));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string FDSNWSConnectionBase::createPostData() {
	string request;
	string lineSeparator;

	auto it = _url.queryItems().find("clrf");
	if ( (it != _url.queryItems().end()) && (it->second.empty() || it->second == "true") ) {
		lineSeparator = "\r\n";
	}
	else {
		lineSeparator = "\n";
	}

	for ( auto it = _streams.begin(); it != _streams.end(); ++it ) {
		if ( (!it->startTime() && !_stime) || (!it->endTime() && !_etime) ) {
			/* invalid time window ignore stream */
			SEISCOMP_WARNING("Ignoring request with invalid time window: %s",
			                 it->str(_stime, _etime));
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
		request += lineSeparator;
	}

	return request;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FDSNWSConnectionBase::handshake(const string &postData, size_t redirectCount, string authHeader) {
	SEISCOMP_DEBUG("POST %s://%s%s\n%s", _protocol, _url.host(), _url.path(), postData);

	const char *proxyServer = getProxy();
	if ( proxyServer && !strcmp(_protocol, "http") ) {
		Util::Url url(proxyServer);
		_socket->sendRequest(string("POST ") + _protocol + "://" + _url.host() + _url.path() + " HTTP/1.1", false);
		// Only http protocol need authorization here. Otherwise it has been
		// handled already in openConnection()
		if ( !url.username().empty() && !url.password().empty() ) {
			string proxyAuthHeader = "Proxy-Authorization: Basic ";
			Util::encodeBase64(proxyAuthHeader, url.username() + ":" + url.password());
			_socket->sendRequest(proxyAuthHeader, false);
		}
	}
	else {
		_socket->sendRequest("POST " + _url.path() + " HTTP/1.1", false);
	}
	_socket->sendRequest("Host: " + _url.host(), false);
	_socket->sendRequest("User-Agent: Mosaic/1.0", false);
	_socket->sendRequest("Content-Type: text/plain", false);
	_socket->sendRequest("Content-Length: " + toString(postData.size()), false);
	_socket->sendRequest("Connection: keep-alive", false);
	if ( !authHeader.empty() ) {
		_socket->sendRequest(authHeader, false);
	}
	else if ( !_url.username().empty() && !strcmp(_protocol, "https") ) {
		string basicAuthHeader = "Authorization: Basic ";
		Util::encodeBase64(basicAuthHeader, _url.username() + ":" + _url.password());
		_socket->sendRequest(basicAuthHeader, false);
	}
	_socket->sendRequest("", false);
	_socket->write(postData);

	string line = _socket->readline();
	SEISCOMP_DEBUG("[00] %s", line);

	if ( line.compare(0, 7, "HTTP/1.") != 0 ) {
		throw GeneralException("server sent invalid response: " + line);
	}

	bool keepAlive = false;

	if ( line.compare(0, 9, "HTTP/1.1 ") == 0 ) {
		keepAlive = true;
	}

	size_t pos;
	pos = line.find(' ');
	if ( pos == string::npos ) {
		throw GeneralException("server sent invalid response: " + line);
	}

	line.erase(0, pos + 1);

	pos = line.find(' ');
	if ( pos == string::npos ) {
		throw GeneralException("server sent invalid response: " + line);
	}

	int code;
	if ( !fromString(code, line.substr(0, pos)) ) {
		throw GeneralException("server sent invalid status code: " + line.substr(0, pos));
	}

	bool authRequired = false;

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
		if ( redirectCount > 5 ) {
			throw GeneralException("redirect limit of 5 exceeded");
		}
	}
	else if ( code == 401 ) {
		// Authentication requested
		authRequired = true;
	}
	else if ( code == 403 ) {
		throw GeneralException("access forbidden: " + line);
	}
	else {
		throw GeneralException("server request error: " + line);
	}

	_remainingBytes = -1;

	int lc = 0;
	string redirectLocation;

	while ( !_socket->isInterrupted() ) {
		++lc;
		line = _socket->readline();
		if ( line.empty() ) {
			break;
		}

		SEISCOMP_DEBUG("[%02d] %s", lc, line);

		// Remove whitespaces
		trim(line);

		string_view headerName, headerValue;

		pos = line.find(':');
		if ( pos != string::npos ) {
			// Transform header values to upper case
			transform(line.begin(), line.begin() + pos, line.begin(), ::toupper);
			headerName = string_view(line).substr(0, pos);
			headerName = trim(headerName);
			headerValue = string_view(line).substr(pos + 1);
			headerValue = trim(headerValue);
		}
		else {
			throw GeneralException("invalid response header '" + string(line) + "'");
		}

		if ( authRequired && (headerName == "WWW-AUTHENTICATE") ) {
			string_view token = tokenize(headerValue, " ");
			if ( compareNoCase(token, "digest") ) {
				if ( !compareNoCase(token, "basic") ) {
					throw GeneralException("basic authentication is not sent via " + string(_protocol) + " because it is insecure");
				}
				else {
					throw GeneralException("unknown authentication schema '" + string(token) + "'");
				}
			}

			string_view nonce, qop, alg, realm, opaque;
			string cnonce;

			while ( (token = tokenize(headerValue, ",")).data() ) {
				token = trim(token);
				pos = token.find('=');
				if ( pos != string::npos ) {
					auto key = trim(token.substr(0, pos));
					auto value = trim(token.substr(pos + 1));
					if ( value.compare(0, 1, "\"") == 0 ) {
						value = value.substr(1);
					}
					if ( value.compare(value.length() - 1, 1, "\"") == 0 ) {
						value = value.substr(0, value.length() - 1);
					}

					if ( key == "nonce" ) {
						nonce = value;
					}
					else if ( key == "opaque" ) {
						opaque = value;
					}
					else if ( key == "qop" ) {
						qop = value;
					}
					else if ( key == "realm" ) {
						realm = value;
					}
					else if ( key == "algorithm" ) {
						alg = value;
					}
				}
			}

			if ( !alg.empty() && (alg != "MD5") && (alg != "MD5-sess") ) {
				throw GeneralException("unknown hash algorithm '" + string(alg) + "'");
			}

			if ( !qop.empty() && (qop != "auth") ) {
				throw GeneralException("unknown qop algorithm '" + string(alg) + "'");
			}

			string HA1, HA2;

			// Compute token
			if ( alg.empty() || (alg == "MD5") ) {
				HA1 = md5(_url.username() + ":" + string(realm) + ":" + _url.password());
			}
			else if ( alg == "MD5-sess" ) {
				HA1 = md5(md5(_url.username() + ":" + string(realm) + ":" + _url.password()) + ":" + string(nonce) + ":" + string(cnonce));
			}

			if ( qop.empty() || (qop == "auth") ) {
				HA2 = md5("POST:" + _url.path());
			}

			string authToken;

			if ( qop == "auth" ) {
				bin2Hex(cnonce, rand(), false);
				authToken = md5(HA1 + ":" + string(nonce) + ":1:" + cnonce + ":" + string(qop) + ":" + HA2);
			}
			else if ( qop.empty() ) {
				authToken = md5(HA1 + ":" + string(nonce) + ":" + HA2);
			}

			authHeader = "Authorization: digest username=\"" + _url.username() + "\"" +
			             ", realm=\"" + string(realm) + "\""
			             ", nonce=\"" + string(nonce) + "\""
			             ", response=\"" + authToken + "\""
			             ", uri=\"" + _url.path() + "\"";
			if ( !opaque.empty() ) {
				authHeader+= ", opaque=\"" + string(opaque) + "\"";
			}
			if ( qop == "auth" ) {
				authHeader+= ", qop=\"auth\""
				             ", nc=1"
				             ", cnonce=\"" + cnonce + "\"";
			}
		}
		else if ( headerName == "TRANSFER-ENCODING" ) {
			if ( headerValue == "chunked" ) {
				_chunkMode = true;
				SEISCOMP_DEBUG(" -> enabled 'chunked' transfer");
			}
		}
		else if ( headerName == "CONTENT-LENGTH" ) {
			if ( !fromString(_remainingBytes, headerValue) ) {
				throw GeneralException("invalid Content-Length response");
			}
			if ( _remainingBytes < 0 ) {
				throw GeneralException("Content-Length must be positive");
			}
		}
		else if ( headerName == "LOCATION" ) {
			redirectLocation = headerValue;
		}
		else if ( headerName == "CONNECTION" ) {
			if ( headerValue == "keep-alive" ) {
				keepAlive = true;
				SEISCOMP_DEBUG(" -> enabled 'keep-alive'");
			}
		}
	}

	if ( _chunkMode ) {
		if ( _remainingBytes >= 0 ) {
			throw GeneralException("protocol error: transfer encoding is chunked and "
			                       "content length given");
		}
		_remainingBytes = 0;
	}

	if ( authRequired ) {
		if ( !authHeader.empty() ) {
			if ( !keepAlive ) {
				_socket->close();
				openConnection();
			}
			else if ( _remainingBytes > 0 ) {
				auto response = readBinary(_remainingBytes, false);
				SEISCOMP_DEBUG("%s", response);
			}
			handshake(postData, redirectCount, authHeader);
			return;
		}
		else {
			throw GeneralException("Missing authentication header");
		}
	}

	// Handle redirect
	if ( code / 100 == 3 ) {
		if ( redirectLocation.empty() ) {
			throw GeneralException("Invalid redirect location protocol: "
			                       "location header empty");
		}

		SEISCOMP_INFO("FDSNWS request was redirected (%i) to %s",
		              code, redirectLocation);
		pos = redirectLocation.find("://");

		// Location on same host and protocol
		if ( pos == string::npos ) {
			if ( redirectLocation[0] != '/' ) {
				throw GeneralException("Invalid redirect location protocol: " +
				                       redirectLocation);
			}

			_url.setPath(redirectLocation);

			if ( !keepAlive ) {
				_socket->close();
				openConnection();
			}

			handshake(postData, ++redirectCount);
			return;
		}

		// Location on different host or protocol
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
			throw GeneralException("Invalid redirect location protocol: " +
			                       redirectLocation);
		}

		redirectLocation.erase(0, pos+3);

		auto username = _url.username();
		auto pwd = _url.password();

		setSource(redirectLocation);

		_url.setUsername(username);
		_url.setPassword(pwd);

		openConnection();
		handshake(postData, ++redirectCount);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *FDSNWSConnectionBase::next() {
	if ( _readingData && !_socket->isOpen() ) {
		return nullptr;
	}

	_socket->startTimer();

	if ( !_readingData ) {
		try {
			openConnection();
			handshake(createPostData());
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

					auto *rec = new IO::MSeedRecord;
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

				_socket->close();
				break;
			}

			_error += readBinary(_chunkMode?512:_remainingBytes);

			if ( !_socket->isOpen() && !_error.empty() ) {
				throw GeneralException(_error);
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
std::string FDSNWSConnectionBase::readBinary(int size, bool close) {
	if ( size <= 0 ) {
		return "";
	}

	string data;
	int bytesLeft = size;

	while ( bytesLeft > 0 ) {
		if ( _chunkMode && _remainingBytes <= 0 ) {
			string r = _socket->readline();
			size_t pos = r.find(' ');
			unsigned int remainingBytes;

			if ( sscanf(r.substr(0, pos).c_str(), "%X", &remainingBytes) !=  1 ) {
				throw GeneralException("invalid chunk header: " + r);
			}

			_remainingBytes = remainingBytes;

			if ( _remainingBytes <= 0 ) {
				if ( !_error.empty() ) {
					throw GeneralException(_error);
				}
				if ( close ) {
					_socket->close();
				}
				break;
			}
		}

		int toBeRead = bytesLeft > BUFSIZE ? BUFSIZE : bytesLeft;
		if ( toBeRead > _remainingBytes ) {
			toBeRead = _remainingBytes;
		}

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
			else if ( close ) {
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
