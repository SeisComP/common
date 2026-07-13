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


#define SEISCOMP_COMPONENT HTTPClient

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <string_view>

#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/recordstreamexceptions.h>
#include <seiscomp/utils/base64.h>

#include <openssl/opensslv.h>
#if OPENSSL_VERSION_NUMBER < 0x30000000
#include <openssl/md5.h>
#else
#include <openssl/evp.h>
#endif

#include "httpclient.h"


using namespace std;
using namespace Seiscomp::Core;


namespace Seiscomp::IO {


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

	EVP_DigestInit_ex2(ctx, md, nullptr);
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HTTPClient::protocol() const {
	return _url.scheme();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int HTTPClient::defaultPort() const {
	auto scheme = _url.scheme();
	if ( scheme == "http" ) {
		return 80;
	}

	if ( scheme == "https" ) {
		return 443;
	}

	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HTTPClient::setURL(const string &url) {
	// Ensure the URL carries a scheme; if none is supplied default to "https" as per
	// the documented behavior. We rewrite the URL string here because Util::Url has no
	// setScheme() method.
	string normalized = url;
	if ( normalized.find("://") == string::npos ) {
		normalized = "https://" + normalized;
	}

	_url.setUrl(normalized);
	if ( !_url.isValid() ) {
		SEISCOMP_ERROR("Invalid URL: %s", _url.status().toString());
		return false;
	}

	// Create the matching socket type for the scheme. Any prior socket (and its
	// connection) is dropped when the SocketPtr is reassigned.
	auto scheme = _url.scheme();
	if ( scheme == "http" ) {
		_socket = new IO::Socket;
	}
	else if ( scheme == "https" ) {
		_socket = new IO::SSLSocket;
	}
	else {
		SEISCOMP_ERROR("Unsupported URL scheme '%s' (expected http or https)",
		               scheme);
		return false;
	}

	// Carry forward the configured timeout to the fresh socket.
	if ( _timeout > 0 ) {
		_socket->setTimeout(_timeout);
	}

	// A new socket is by definition not yet associated with any host.
	_keepAlive = false;
	_connectionHost.clear();
	_connectionPort = 0;
	_connectionScheme.clear();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HTTPClient::setTimeout(int seconds) {
	_timeout = seconds;
	if ( _socket ) {
		_socket->setTimeout(seconds);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Hopefully safe to be called from another thread
void HTTPClient::close() {
	if ( _socket ) {
		_socket->interrupt();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HTTPClient::disconnect() {
	if ( _socket && _socket->isOpen() ) {
		_socket->close();
	}

	_keepAlive = false;
	_connectionHost.clear();
	_connectionPort = 0;
	_connectionScheme.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HTTPClient::reset() {
	if ( _socket && _socket->isOpen() ) {
		_socket->close();
	}

	_keepAlive = false;
	_connectionHost.clear();
	_connectionPort = 0;
	_connectionScheme.clear();

	_chunkMode = false;
	_remainingBytes = 0;
	_error.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *HTTPClient::getProxy() const {
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

	const auto scheme = _url.scheme();
	if ( scheme == "http" ) {
		proxy = getenv("http_proxy");
		if ( !proxy ) {
			proxy = getenv("HTTP_PROXY");
		}
	}
	else if ( scheme == "https" ) {
		proxy = getenv("HTTPS_PROXY");
		if ( !proxy ) {
			proxy = getenv("https_proxy");
		}
	}

	return proxy;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HTTPClient::openConnection() {
	const char *proxyHost = getProxy();

	if ( !proxyHost ) {
		if ( !_url.port() ) {
			_socket->open(_url.host() + ":" + Core::toString(defaultPort()));
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

	const auto urlScheme = _url.scheme();
	if ( proxyScheme != urlScheme ) {
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

	if ( urlScheme == "https" ) {
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
void HTTPClient::get(const string &url, string authHeader, size_t redirectCount) {
	prepareConnection(url);
	request("GET", string(), string(), std::move(authHeader), redirectCount);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HTTPClient::post(const string &url, const string &postData,
                      const string &contentType, string authHeader,
                      size_t redirectCount) {
	prepareConnection(url);
	request("POST", postData, contentType, std::move(authHeader), redirectCount);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HTTPClient::prepareConnection(const string &url) {
	// Parse the incoming URL into a temporary so we can compare its host/port/scheme
	// against the active connection without losing the existing _url in case we end up
	// reusing the socket and the virtual setURL() would unexpectedly mutate other
	// state.
	string normalized = url;
	if ( normalized.find("://") == string::npos ) {
		normalized = "https://" + normalized;
	}
	Util::Url candidate(normalized);
	if ( !candidate.isValid() ) {
		// Defer to setURL() to log the error and clean up state.
		setURL(url);
		return;
	}

	const auto &scheme = candidate.scheme();
	size_t port;
	if ( candidate.port() ) {
		port = *candidate.port();
	}
	else if ( scheme == "http" ) {
		port = 80;
	}
	else if ( scheme == "https" ) {
		port = 443;
	}
	else {
		setURL(url);
		return;
	}

	const bool canReuse = _keepAlive
	                   && _socket && _socket->isOpen()
	                   && scheme == _connectionScheme
	                   && candidate.host() == _connectionHost
	                   && port == _connectionPort;

	if ( canReuse ) {
		// Same target host – keep the socket alive and only update the URL (path /
		// query / credentials may have changed).
		_url = candidate;
		// Clear per-response parsing state so the next request starts from a clean
		// slate. The body of the previous response is assumed to have been drained by
		// the caller via readBinary().
		_chunkMode = false;
		_remainingBytes = 0;
		_error.clear();
		SEISCOMP_DEBUG("Reusing keep-alive connection to %s://%s:%zu",
		               scheme, _connectionHost, _connectionPort);
		return;
	}

	// Either there is no live connection, or the target changed.  Drop whatever we had
	// and open a fresh one. setURL() is virtual so a subclass may rewrite the URL
	// (e.g., fdsnws -> http) before the base class parses it.
	disconnect();
	if ( !setURL(url) ) {
		throw GeneralException("Invalid request URL: " + url);
	}
	openConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HTTPClient::requestTarget() const {
	string target = _url.path();
	if ( target.empty() ) {
		target = "/";
	}

	// Rebuild the query string from the parsed items. A key with an empty value is
	// emitted bare (e.g. "allocate" rather than "allocate="), which is what flag-style
	// options like ?allocate require on the receiving side.
	const auto &items = _url.queryItems();
	bool first = true;
	for ( const auto &item : items ) {
		target += first ? '?' : '&';
		first = false;
		target += item.first;
		if ( !item.second.empty() ) {
			target += '=';
			target += item.second;
		}
	}

	return target;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HTTPClient::request(const char *method, const string &body,
                         const string &contentType, string authHeader,
                         size_t redirectCount) {
	const bool hasBody = !strcmp(method, "POST");
	const auto scheme = _url.scheme();
	const string target = requestTarget();

	if ( hasBody ) {
		SEISCOMP_DEBUG("%s %s://%s%s\n%s", method, scheme, _url.host(),
		               target, body);
	}
	else {
		SEISCOMP_DEBUG("%s %s://%s%s", method, scheme, _url.host(),
		               target);
	}

	const char *proxyServer = getProxy();
	if ( proxyServer && scheme == "http" ) {
		Util::Url url(proxyServer);
		_socket->sendRequest(string(method) + " " + scheme + "://" + _url.host() + target + " HTTP/1.1", false);
		// Only http protocol needs authorization here. Otherwise it has been handled
		// already in openConnection()
		if ( !url.username().empty() && !url.password().empty() ) {
			string proxyAuthHeader = "Proxy-Authorization: Basic ";
			Util::encodeBase64(proxyAuthHeader, url.username() + ":" + url.password());
			_socket->sendRequest(proxyAuthHeader, false);
		}
	}
	else {
		_socket->sendRequest(string(method) + " " + target + " HTTP/1.1", false);
	}

	_socket->sendRequest("Host: " + _url.host(), false);
	_socket->sendRequest("User-Agent: Mosaic/1.0", false);

	if ( hasBody ) {
		// Fall back to text/plain if the caller passed an empty content type (e.g.,
		// when request() is invoked through get()).
		const string &ct = contentType.empty() ? string("text/plain") : contentType;
		_socket->sendRequest("Content-Type: " + ct, false);
		_socket->sendRequest("Content-Length: " + toString(body.size()), false);
	}

	_socket->sendRequest("Connection: keep-alive", false);

	if ( !authHeader.empty() ) {
		_socket->sendRequest(authHeader, false);
	}
	else if ( !_url.username().empty() && scheme == "https" ) {
		string basicAuthHeader = "Authorization: Basic ";
		Util::encodeBase64(basicAuthHeader, _url.username() + ":" + _url.password());
		_socket->sendRequest(basicAuthHeader, false);
	}

	_socket->sendRequest("", false);

	if ( hasBody ) {
		_socket->write(body);
	}

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
		throw GeneralException("server sent invalid status code: " +
		                       line.substr(0, pos));
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
	authHeader.clear();

	while ( !_socket->isInterrupted() ) {
		++lc;
		line = _socket->readline();
		if ( line.empty() ) {
			break;
		}

		SEISCOMP_DEBUG("[%02d] %s", lc, line);

		// Remove whitespaces
		trim(line);

		string_view headerName;
		string_view headerValue;

		pos = line.find(':');
		if ( pos != string::npos ) {
			// Transform header names to upper case
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
					throw GeneralException("basic authentication is not sent via "
					                       + scheme + " because it is insecure");
				}

				throw GeneralException("unknown authentication schema '"
				                       + string(token) + "'");
			}

			string_view nonce;
			string_view qop;
			string_view alg;
			string_view realm;
			string_view opaque;
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

			string HA1;
			string HA2;

			// Compute token
			if ( alg.empty() || (alg == "MD5") ) {
				HA1 = md5(_url.username() + ":" + string(realm) + ":"
				          + _url.password());
			}
			else if ( alg == "MD5-sess" ) {
				HA1 = md5(
				    md5(_url.username() + ":" + string(realm) + ":" + _url.password())
				    + ":" + string(nonce) + ":" + string(cnonce)
				);
			}

			if ( qop.empty() || (qop == "auth") ) {
				HA2 = md5(string(method) + ":" + requestTarget());
			}

			string authToken;

			if ( qop == "auth" ) {
				bin2Hex(cnonce, rand(), false);
				stringstream ss;
				ss << HA1 << ":" << nonce << ":1:" << cnonce << ":" << qop << ":"
				   << HA2;
				authToken = md5(ss.str());
			}
			else if ( qop.empty() ) {
				stringstream ss;
				ss << HA1 << ":" << nonce << ":" << HA2;
				authToken = md5(ss.str());
			}

			authHeader = "Authorization: digest username=\"" + _url.username() + "\"" +
			             ", realm=\"" + string(realm) + "\""
			             ", nonce=\"" + string(nonce) + "\""
			             ", response=\"" + authToken + "\""
			             ", uri=\"" + requestTarget() + "\"";
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
			else if ( headerValue == "close" ) {
				keepAlive = false;
				SEISCOMP_DEBUG(" -> disabled 'keep-alive'");
			}
		}
	}

	// Publish the keep-alive decision to the rest of the client so readBinary() and
	// subsequent prepareConnection() calls can act on it. Also record the connection
	// identity so a follow-up request can decide whether it can reuse the socket.
	_keepAlive = keepAlive;
	if ( _keepAlive ) {
		_connectionHost   = _url.host();
		_connectionPort   = _url.port() ? *_url.port() : (size_t)defaultPort();
		_connectionScheme = scheme;
	}
	else {
		_connectionHost.clear();
		_connectionPort = 0;
		_connectionScheme.clear();
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
				// Drain the challenge response body but keep the socket open
				// (close = false) so the authenticated retry below can reuse
				// it.
				string response(static_cast<size_t>(_remainingBytes), '\0');
				std::streamsize n =
					readBody(response.data(), _remainingBytes, false);
				response.resize(static_cast<size_t>(n));
				SEISCOMP_DEBUG("%s", response);
			}
			request(method, body, contentType, authHeader, redirectCount);
			return;
		}

		throw GeneralException("Missing authentication header");
	}

	// Handle redirect
	if ( code / 100 == 3 ) {
		if ( redirectLocation.empty() ) {
			throw GeneralException("Invalid redirect location protocol: location "
			                       "header empty");
		}

		SEISCOMP_INFO("HTTP request was redirected (%i) to %s",
		              code, redirectLocation);
		pos = redirectLocation.find("://");

		// Preserve credentials across the redirect.
		auto username = _url.username();
		auto pwd = _url.password();

		string nextURL;
		if ( pos == string::npos ) {
			// Location on the same host. Rebuild an absolute URL so prepareConnection()
			// can decide whether to reuse the keep-alive socket.
			if ( redirectLocation.empty() || redirectLocation[0] != '/' ) {
				throw GeneralException("Invalid redirect location protocol: " +
				                       redirectLocation);
			}
			nextURL = scheme + "://" + _url.host();
			if ( _url.port() ) {
				nextURL += ":" + toString(*_url.port());
			}
			nextURL += redirectLocation;
		}
		else {
			// Location on a different host or protocol. Validate the scheme up front so
			// we produce a clear error.
			if ( redirectLocation.compare(0, pos, "http") != 0
			  && redirectLocation.compare(0, pos, "https") != 0 ) {
				throw GeneralException("Invalid redirect location protocol: " +
				                       redirectLocation);
			}
			nextURL = redirectLocation;
		}

		// prepareConnection() handles socket reuse (same host with keep-alive in
		// effect) and dispatches via the virtual setURL() so subclasses like FDSNWS can
		// apply service- specific URL normalization.
		prepareConnection(nextURL);

		_url.setUsername(username);
		_url.setPassword(pwd);

		request(method, body, contentType, string(), ++redirectCount);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::streamsize HTTPClient::readBody(char *out, std::streamsize count, bool close) {
	if ( count <= 0 ) {
		return 0;
	}

	std::streamsize bytesLeft = count;

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
				if ( close && !_keepAlive ) {
					_socket->close();
				}
				break;
			}
		}

		std::streamsize toBeRead = bytesLeft > BUFSIZE ? BUFSIZE : bytesLeft;
		if ( toBeRead > _remainingBytes ) {
			toBeRead = _remainingBytes;
		}

		// toBeRead is bounded by BUFSIZE, so the argument fits into int.
		string chunk = _socket->read(static_cast<int>(toBeRead));
		if ( chunk.empty() ) {
			SEISCOMP_WARNING("socket read returned not data");
			break;
		}

		auto bytesRead = static_cast<std::streamsize>(chunk.size());
		memcpy(out, chunk.data(), chunk.size());
		out += bytesRead;

		_remainingBytes -= bytesRead;
		bytesLeft -= bytesRead;

		if ( _remainingBytes <= 0 ) {
			if ( _chunkMode ) {
				// Read trailing new line
				_socket->readline();
			}
			else if ( close && !_keepAlive ) {
				_socket->close();
			}
		}
	}

	return count - bytesLeft;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::streamsize HTTPClient::read(void *b, std::streamsize count) {
	return readBody(static_cast<char *>(b), count);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HTTPClient::readBinary(std::streamsize size) {
	if ( size <= 0 ) {
		return "";
	}

	string data(static_cast<size_t>(size), '\0');
	std::streamsize bytesRead = readBody(data.data(), size);
	data.resize(static_cast<size_t>(bytesRead));

	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
