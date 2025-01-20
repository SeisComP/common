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
#include <seiscomp/utils/base64.h>

#include <cstdio>
#include <iostream>
#include <string.h>

#include <seiscomp/wired/protocols/http.h>


using namespace std;
using namespace Seiscomp::Core;


namespace Seiscomp {
namespace Wired {

namespace {


const char *WeekDays[7] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat"
};

const char *Months[12] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};


void padzero2(string &out, int value) {
	out += '0' + ((value / 10) % 10);
	out += '0' + value % 10;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool URLOptions::next() {
	size_t len;
	const char *data = tokenize(_source, "&", _source_len, len);

	if ( data ) {
		trim(data, len);

		name_start = data;

		const char *sep = strnchr(data, len, '=');
		if ( sep ) {
			name_len = static_cast<size_t>(sep - data);
			val_start = sep + 1;
			val_len = len - name_len - 1;
		}
		else {
			name_len = len;
			val_start = nullptr;
			val_len = 0;
		}

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool URLOptions::nameEquals(const char *s) const {
	return !strncmp(s, name_start, name_len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool URLOptions::valueEquals(const char *s) const {
	return !strncmp(s, val_start, val_len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool URLInsituOptions::next() {
	size_t len;
	const char *data = tokenize(_source, "&", _source_len, len);

	if ( data ) {
		trim(data,len);

		name = const_cast<char*>(data);

		const char *sep = strnchr(data, len, '=');
		if ( sep ) {
			name_len = static_cast<size_t>(sep - data);
			name[name_len] = '\0';

			val = const_cast<char*>(sep + 1);
			val_len = len - name_len - 1;
			val[val_len] = '\0';
		}
		else {
			name_len = len;
			name[name_len] = '\0';
			val = nullptr;
			val_len = 0;
		}

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool URLInsituOptions::nameEquals(const char *s) const {
	return !strncmp(s, name, name_len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool URLInsituOptions::valueEquals(const char *s) const {
	return !strncmp(s, val, val_len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string HttpSession::EmptyString;

HttpSession::HttpSession(Device *dev, const char *protocol, const char *server)
: ClientSession(dev, 500), _protocol(protocol), _server(server),
  _requestStarted(false), _upgradedToWebsocket(false), _websocketFrame(nullptr) {
	// Typically a HTTP connection needs to read the request
	if ( dev != nullptr ) dev->setMode(Socket::Read);
	_request.state = HttpRequest::ENABLED;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HttpSession::~HttpSession() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HttpSession::urlencode(const string &s) {
	//RFC 3986 section 2.2 Reserved Characters (January 2005)
	//RFC 3986 section 2.3 Unreserved Characters (January 2005)

	static const string dontEscape =
		/*":/?#[]@!$&'()*+,;="*/
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

	string escaped = "";
	for ( size_t i = 0; i < s.length(); ++i ) {
		if ( dontEscape.find_first_of(s[i]) != string::npos )
			escaped += s[i];
		else {
			escaped += '%';
			char buf[3];
			snprintf(buf, 3, "%.2X", s[i]);
			escaped += buf;
		}
	}

	return escaped;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HttpSession::urlencode(const char *s, int len) {
	//RFC 3986 section 2.2 Reserved Characters (January 2005)
	//RFC 3986 section 2.3 Unreserved Characters (January 2005)

	static const string dontEscape =
		/*":/?#[]@!$&'()*+,;="*/
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

	string escaped = "";
	for ( int i = 0; i < len; ++i ) {
		if ( dontEscape.find_first_of(s[i]) != string::npos )
			escaped += s[i];
		else {
			escaped += '%';
			char buf[3];
			sprintf(buf, "%.2hhX", uint8_t(s[i]));
			escaped += buf;
		}
	}

	return escaped;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HttpSession::urldecode(const string &s) {
	string decoded = "";
	for( size_t i = 0; i < s.length(); ++i ) {
		if ( s[i] == '+' )
			decoded += ' ';
		else if ( s[i] == '%' ) {
			++i;
			char hi, lo;
			if ( i < s.length() ) hi = s[i];
			else break;
			++i;
			if ( i < s.length() ) lo = s[i];
			else break;

			hi = char(hi >= 'A' ? ((hi & 0xDF) - 'A') + 10 : (hi - '0'));
			lo = char(lo >= 'A' ? ((lo & 0xDF) - 'A') + 10 : (lo - '0'));

			decoded += char((hi << 4) + lo);
		}
		else
			decoded += s[i];
	}

	return decoded;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string HttpSession::urldecode(const char *s, int len) {
	string decoded = "";
	for ( int i = 0; i < len; ++i ) {
		if ( s[i] == '+' )
			decoded += ' ';
		else if ( s[i] == '%' ) {
			++i;
			char hi, lo;
			if ( i < len ) hi = s[i];
			else break;
			++i;
			if ( i < len ) lo = s[i];
			else break;

			hi = char(hi >= 'A' ? ((hi & 0xDF) - 'A') + 10 : (hi - '0'));
			lo = char(lo >= 'A' ? ((lo & 0xDF) - 'A') + 10 : (lo - '0'));

			decoded += char((hi << 4) + lo);
		}
		else
			decoded += s[i];
	}

	return decoded;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int HttpSession::urldecode(char *s, int len) {
	int out_idx = 0;

	if ( !len ) return 0;

	for ( int i = 0; i < len; ++i ) {
		if ( s[i] == '+' )
			s[out_idx++] = ' ';
		else if ( s[i] == '%' ) {
			++i;
			char hi, lo;
			if ( i < len ) hi = s[i];
			else break;
			++i;
			if ( i < len ) lo = s[i];
			else break;

			hi = char(hi >= 'A' ? ((hi & 0xDF) - 'A') + 10 : (hi - '0'));
			lo = char(lo >= 'A' ? ((lo & 0xDF) - 'A') + 10 : (lo - '0'));

			s[out_idx++] = char((hi << 4) + lo);
		}
		else
			s[out_idx++] = s[i];
	}

	s[out_idx] = '\0';
	return out_idx;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::sendResponse(HttpStatus status) {
	_request.status = status;

	// Send response
	send("HTTP/1.1 ");
	send(status.toString());
	if ( _server ) {
		send("\r\nServer: ");
		send(_server);
	}

	if ( _request.keepAlive ) {
		send("\r\nKeep-Alive: timeout=15");
		send("\r\nConnection: Keep-Alive");
	}

	if ( !_request.origin.empty() )
		send("\r\nAccess-Control-Allow-Origin: *");

	send("\r\nContent-Length: 0");
	send("\r\n\r\n", 4);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::sendResponse(const char *content, size_t len,
                               HttpStatus status,
                               const char *contentType,
                               const char *cookie) {
	// Save status in request
	_request.status = status;

	send("HTTP/1.1 ", 9);
	send(status.toString());
	send("\r\n", 2);

	if ( _server ) {
		send("Server: ");
		send(_server);
		send("\r\n", 2);
	}

	if ( !_request.origin.empty() )
		send("Access-Control-Allow-Origin: *\r\n");

	if ( cookie ) {
		send("Set-Cookie: ");
		send(cookie);
		send("; Path=/\r\n");
	}

	if ( _request.keepAlive ) {
		send("Keep-Alive: timeout=15\r\n");
		send("Connection: Keep-Alive\r\n");
	}

	if ( contentType ) {
		send("Content-Type: ");
		send(contentType);
		send("\r\n", 2);
	}

	send("Content-Length: ");
	const string length = Seiscomp::Core::toString(len);
	send(length.c_str(), length.size());

	send("\r\n\r\n", 4);

	send(content, len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::sendResponse(const std::string &content,
                               HttpStatus status,
                               const char *contentType,
                               const char *cookie) {
	sendResponse(&content[0], content.size(), status, contentType, cookie);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::sendResponse(Buffer* buf, HttpStatus status,
                               const char *contentType,
                               const char *cookie,
                               const char *additionalHeader) {
	// Check modification times and send 304 is not modified
	if ( _request.ifModifiedSince && (buf->lastModified > 0) ) {
		if ( buf->lastModified <= _request.ifModifiedSince->epochSeconds() ) {
			sendResponse(HTTP_304);
			return;
		}
	}

	// Save status in request
	_request.status = status;

	buf->header ="HTTP/1.1 ";
	buf->header += status.toString();
	buf->header += "\r\n";

	if ( _server ) {
		buf->header += "Server: ";
		buf->header += _server;
		buf->header += "\r\n";
	}

	if ( !_request.origin.empty() )
		buf->header += "Access-Control-Allow-Origin: *\r\n";

	if ( cookie ) {
		buf->header += "Set-Cookie: ";
		buf->header += cookie;
		buf->header += "\r\n";
	}

	if ( _request.keepAlive ) {
		buf->header += "Keep-Alive: timeout=15\r\n";
		buf->header += "Connection: Keep-Alive\r\n";
	}

	if ( contentType ) {
		buf->header += "Content-Type: ";
		buf->header += contentType;
		buf->header += "\r\n";
	}

	if ( buf->lastModified > 0 ) {
		buf->header += "Last-Modified: ";

		struct tm t;
		gmtime_r(&buf->lastModified, &t);

		buf->header += WeekDays[t.tm_wday];
		buf->header += ", ";
		padzero2(buf->header, t.tm_mday);
		buf->header += " ";
		buf->header += Months[t.tm_mon];
		buf->header += " ";
		buf->header += Seiscomp::Core::toString(t.tm_year+1900);
		buf->header += " ";
		padzero2(buf->header, t.tm_hour);
		buf->header += ":";
		padzero2(buf->header, t.tm_min);
		buf->header += ":";
		padzero2(buf->header, t.tm_sec);
		buf->header += " GMT";
		buf->header += "\r\n";
	}

	if ( additionalHeader )
		buf->header += additionalHeader;

	switch ( buf->encoding ) {
		case Buffer::Identity:
			break;
		case Buffer::Compress:
			buf->header += "Content-Encoding: compress\r\n";
			break;
		case Buffer::GZip:
			buf->header += "Content-Encoding: gzip\r\n";
			break;
		case Buffer::BZip2:
			buf->header += "Content-Encoding: bzip2\r\n";
			break;
	}

	if ( buf->length() != string::npos ) {
		buf->header +=
			"Content-Length: ";
		buf->header += Seiscomp::Core::toString(buf->length());
	}
	else
		buf->header += "Transfer-Encoding: chunked";

	buf->header += "\r\n\r\n";

	// Add first chunk size if data is not empty, otherwise let
	// updateBuffer handle the header
	if ( buf->length() == string::npos && !buf->data.empty() ) {
		char tmp[20]; tmp[0] = '\0';
		sprintf(tmp, "%zX\r\n", buf->data.size());
		buf->header += tmp;
	}

	send(buf);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::reset() {
	_requestStarted = false;
	_request = HttpRequest();
	_request.state = HttpRequest::ENABLED;
	_dataStarted = false;
	_dataSize = 0;
	_upgradedToWebsocket = false;

	if ( _websocketFrame ) _websocketFrame->reset();

	if ( _device != nullptr ) {
		_device->setMode(Device::Read);
		_device->setTimeout(-1);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::close() {
	_request.state = HttpRequest::FINISHED;
	ClientSession::close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::handleReceive(const char *data, size_t len) {
	if ( _upgradedToWebsocket ) {
		while ( len > 0 && !_websocketFrame->isFinished() ) {
			ssize_t read = _websocketFrame->feed(data, len);
			if ( read <= 0 ) {
				SEISCOMP_ERROR("[websocket] websocket protocol error, closing connection");
				close();
				return;
			}

			data += read; len -= static_cast<size_t>(read);

			if ( _websocketFrame->isFinished() ) {
				handleWebsocketFrame(*_websocketFrame);
				_websocketFrame->reset();
			}
		}
	}
	else
		ClientSession::handleReceive(data, len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::handleInbox(const char *src_data, size_t src_len) {
	const char *data;
	size_t len;
	size_t data_len = static_cast<size_t>(src_len);

	if ( _request.state == HttpRequest::WAITING ) {
		SEISCOMP_WARNING("[http] session %p still set on WAITING but received new request\n"
		                 "       old request: %s %s\n"
		                 "       new request: %s",
		                 reinterpret_cast<void*>(this),
		                 _request.type.toString(),
		                 _request.path.c_str(), src_data);
		if ( !reset() ) {
			SEISCOMP_ERROR("[http] session does not succeed in resetting connection, closing");
			setError("");
			return;
		}
	}

	if ( !_requestStarted ) {
		_request.state = HttpRequest::READING;
		_request.keepAlive = true;
		// Not necessary in HTTP 1.1
		_request.addKeepAliveHeader = false;
		_request.upgrade = false;
		_dataSize = 0;

		SEISCOMP_DEBUG("[http] %s", src_data);
		data = tokenize(src_data, " ", data_len, len);

		if ( len == 3 && strncmp(data, "GET", len) == 0 )
			_request.type = HttpRequest::GET;
		else if ( len == 4 && strncmp(data, "POST", len) == 0 )
			_request.type = HttpRequest::POST;
		else if ( len == 7 && strncmp(data, "OPTIONS", len) == 0 )
			_request.type = HttpRequest::OPTIONS;
		else if ( len == 4 && strncmp(data, "HEAD", len) == 0 )
			_request.type = HttpRequest::HEAD;
		else if ( len == 3 && strncmp(data, "PUT", len) == 0 )
			_request.type = HttpRequest::PUT;
		else if ( len == 6 && strncmp(data, "DELETE", len) == 0 )
			_request.type = HttpRequest::DELETE;
		else if ( len == 5 && strncmp(data, "TRACE", len) == 0 )
			_request.type = HttpRequest::TRACE;
		else {
			SEISCOMP_ERROR("[http] invalid request type: %s", data);
			sendStatus(HTTP_400);
			return;
		}

		data = tokenize(src_data, " ", data_len, len);

		const char *opt_sep = strnchr(data, len, '?');
		if ( opt_sep ) {
			_request.path.assign(data, static_cast<size_t>(opt_sep - data));
			_request.options.assign(opt_sep + 1, data + len);
		}
		else {
			_request.path.assign(data, len);
			_request.options.clear();
		}

		_request.status = HTTP_404;
		_request.contentType.clear();
		_request.userAgent.clear();
		_request.referer.clear();
		_request.cookie.clear();
		_request.origin.clear();
		_request.upgradeTo.clear();
		_request.data.clear();
		_request.secWebsocketProtocol.clear();
		_request.secWebsocketKey.clear();
		_request.secWebsocketVersion = -1;
		_request.ifModifiedSince = Core::None;
		_request.tx = 0;
		// Reset data sent which is increased by ClientSession::flush
		_bytesSent = 0;

		data = tokenize(src_data, " ", data_len, len);
		if ( len < 4 || strncasecmp(data, "HTTP", 4) != 0 ) {
			sendStatus(HTTP_400);
			return;
		}

		trimBack(data,len);
		_request.version.assign(data, len);

		// Keep-alive is default in 1.1 and later
		if ( _request.version == "1.0" ) {
			_request.keepAlive = false;
			_request.addKeepAliveHeader = true;
		}

		_acceptGzip = false;
		_requestStarted = true;
		// Remove timeout in case keep-alive was requested before
		device()->setTimeout(-1);

		setMIMEUnfoldingEnabled(true);
	}
	else {
		// End token, send response or read payload
		if ( data_len == 0 ) {
			setMIMEUnfoldingEnabled(false);

			switch ( _request.type ) {
				case HttpRequest::GET:
				case HttpRequest::OPTIONS:
				case HttpRequest::HEAD:
				case HttpRequest::DELETE:
				case HttpRequest::TRACE:
					SEISCOMP_DEBUG("[%p] %s %s",
					               static_cast<void*>(this),
					               _request.type.toString(), _request.path.c_str());
					handleRequest(_request);
					_requestStarted = false;
					break;
				case HttpRequest::POST:
				case HttpRequest::PUT:
					SEISCOMP_DEBUG("[%p] %s %s",
					               static_cast<void*>(this),
					               _request.type.toString(), _request.path.c_str());
					SEISCOMP_DEBUG("Reading %s (%zu bytes)", _request.type.toString(), _dataSize);
					if ( _dataSize <= 0 ) {
						SEISCOMP_ERROR("HTTP: %s content is empty", _request.type.toString());
						close();
						return;
					}

					_dataStarted = true;
					_request.data.clear();
					setPostDataSize(_dataSize);
					break;
				default:
					SEISCOMP_WARNING("[%p] Unknown request type %s",
					                 static_cast<void*>(this),
					                 _request.type.toString());
					close();
					break;
			}
		}
		else {
			SEISCOMP_DEBUG("[http] %s", src_data);

			data = tokenize(src_data, ":", data_len, len);
			if ( len == 14 && strncasecmp("Content-Length", data, len) == 0 ) {
				data = tokenize(src_data, ":", data_len, len);
				string tmp;
				tmp.assign(data, len);
				if ( !Seiscomp::Core::fromString(_dataSize, tmp) ) {
					SEISCOMP_ERROR("HTTP: invalid Content-Length value: %s",
					               tmp.c_str());
					close();
					return;
				}

				if ( !validatePostDataSize(_dataSize) ) {
					_requestStarted = true;
					setError("");
					return;
				}
			}
			else if ( len == 12 && strncasecmp("Content-Type", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);

				_request.contentType.assign(data, len);
			}
			else if ( len == 4 && strncasecmp("Host", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);

				_request.host.assign(data, len);
			}
			else if ( len == 6 && strncasecmp("Cookie", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);

				_request.cookie.assign(data, len);
			}
			else if ( len == 10 && strncasecmp("User-Agent", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);

				_request.userAgent.assign(data,len);
			}
			else if ( len == 15 && strncasecmp("Accept-Encoding", data, len) == 0 ) {
				++src_data;
				--data_len;

				while ( (data = tokenize(src_data, ",", data_len, len)) != nullptr) {
					trimFront(data,len);
					trimBack(data,len);
					if ( len == 4 && strncasecmp("gzip", data, len) == 0 ) {
						_acceptGzip = true;
						break;
					}
					else if ( len == 6 && strncasecmp("x-gzip", data, len) == 0 ) {
						_acceptGzip = true;
						break;
					}
				}
			}
			else if ( len == 10 && strncasecmp("Connection", data, len) == 0 ) {
				++src_data;
				--data_len;

				while ( (data = tokenize(src_data, ",", data_len, len)) != nullptr) {
					trimFront(data,len);
					trimBack(data,len);

					if ( strncasecmp("Keep-Alive", data, 10) == 0 ) {
						SEISCOMP_DEBUG("[http] Keep-Alive requested");
						_request.keepAlive = true;
					}
					else if ( strncasecmp("Upgrade", data, 7) == 0 ) {
						SEISCOMP_DEBUG("[http] Connection upgrade requested");
						_request.upgrade = true;
					}
					else if ( strncasecmp("Close", data, 5) == 0 ) {
						SEISCOMP_DEBUG("[http] Connection close requested");
						_request.keepAlive = false;
					}
				}
			}
			else if ( len == 7 && strncasecmp("Referer", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);
				_request.referer.assign(data, len);
			}
			else if ( len == 16 && strncasecmp("X-Requested-With", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);
				_request.isXMLHTTP = strncasecmp("XMLHttpRequest", data, len) == 0;
			}
			else if ( len == 6 && strncasecmp("Origin", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);
				_request.origin.assign(data, len);
			}
			else if ( len == 17 && strncasecmp("If-Modified-Since", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);

				string tmp;
				tmp.assign(data, len);

				Core::Time timestamp;
				// RFC 822
				if ( timestamp.fromString(tmp.c_str(), "%a, %d %b %Y %H:%M:%S GMT")
				  // RFC 850
				  || timestamp.fromString(tmp.c_str(), "%A, %d-%b-%y %H:%M:%S GMT")
				  // ANSI C's asctime()
				  || timestamp.fromString(tmp.c_str(), "%a %d %e %H:%M:%S %Y") ) {
					_request.ifModifiedSince = timestamp;
				}
				else {
					SEISCOMP_WARNING("Unable to parse If-Modified-Since date: %s", tmp.c_str());
				}
			}
			else if ( len == 7 && strncasecmp("Upgrade", data, len) == 0 ) {
				// Save upgrade path
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);
				_request.upgradeTo.assign(data, len);
			}
			else if ( len == 22 && strncasecmp("Sec-WebSocket-Protocol", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);
				_request.secWebsocketProtocol.assign(data, len);
			}
			else if ( len == 17 && strncasecmp("Sec-WebSocket-Key", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				trimFront(data,len);
				_request.secWebsocketKey.assign(data, len);
			}
			else if ( len == 21 && strncasecmp("Sec-WebSocket-Version", data, len) == 0 ) {
				data = src_data+1;
				len = data_len-1;
				string tmp;
				tmp.assign(data, len);
				if ( !Seiscomp::Core::fromString(_request.secWebsocketVersion, tmp) )
					_request.secWebsocketVersion = -1;
			}
			else {
				const char *value = src_data + 1;
				size_t vlen = data_len - 1;
				trimFront(value, vlen);
				handleHeader(data, len, value, vlen);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::validatePostDataSize(size_t) {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::handlePostData(const char *data, size_t len) {
	size_t bytes = std::min(_dataSize, len);
	_request.data.append(data, bytes);
	_dataSize -= bytes;

	if ( _dataSize == 0 ) {
		_requestStarted = false;
		_dataStarted = false;
		handleRequest(_request);
		_request.data.clear();
		_dataSize = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::handleInboxError(Error error) {
	if ( error == TooManyCharactersPerLine ) {
		SEISCOMP_ERROR("too many characters (>= %zu) on request\n%.*s",
		               _inbox.size(), int(_inbox.size()), &_inbox[0]);
		sendStatus(HTTP_413);
	}
	else {
		SEISCOMP_ERROR("inbox error on df %d", device()->fd());
		sendStatus(HTTP_400);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::buffersFlushed() {
	if ( _request.state == HttpRequest::FINISHED ) {
		_request.tx = _bytesSent;
		SEISCOMP_DEBUG("[http] request %s in session %p finished",
		               _request.path.c_str(), static_cast<void*>(this));

		requestFinished();

		if ( _request.keepAlive ) {
			_device->setTimeout(15*1000);
			_device->setMode(Device::Read);
			_requestStarted = false;
			SEISCOMP_DEBUG("[http] keeping session for 15 secs");
		}
		else {
			SEISCOMP_DEBUG("[http] close session");
			close();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::requestFinished() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::handleHeader(const char *, size_t, const char *, size_t) {
	// Do nothing, all standard headers are already handled in handleInbox
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleRequest(HttpRequest &req) {
	// Set state to running
	req.state = HttpRequest::RUNNING;

	bool res;

	switch ( req.type ) {
		case HttpRequest::GET:
			res = handleGETRequest(req);
			break;
		case HttpRequest::POST:
			res = handlePOSTRequest(req);
			break;
		case HttpRequest::OPTIONS:
			res = handleOPTIONSRequest(req);
			break;
		case HttpRequest::HEAD:
			res = handleHEADRequest(req);
			break;
		case HttpRequest::PUT:
			res = handlePUTRequest(req);
			break;
		case HttpRequest::DELETE:
			res = handleDELETERequest(req);
			break;
		case HttpRequest::TRACE:
			res = handleTRACERequest(req);
			break;
		default:
			res = false;
			break;
	}

	if ( !res ) {
		SEISCOMP_ERROR("[http] error in request handler: returning 500");
		sendStatus(HTTP_500);
	}

	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleGETRequest(HttpRequest &) {
	sendStatus(HTTP_405);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handlePOSTRequest(HttpRequest &) {
	sendStatus(HTTP_405);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleOPTIONSRequest(HttpRequest &req) {
	req.status = HTTP_200;
	req.state = HttpRequest::FINISHED;
	req.keepAlive = false;
	send("HTTP/1.1 200 OK\r\n");
	if ( _server ) {
		send("Server: ");
		send(_server);
		send("\r\n");
	}
	send("Access-Control-Allow-Origin: *\r\n"
	     "Access-Control-Allow-Methods: *\r\n"
	     "Access-Control-Allow-Headers: Accept, Content-Type, X-Requested-With, Origin\r\n"
	     "Content-Type: text/plain\r\n"
	     "Content-Length: 0\r\n\r\n");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleHEADRequest(HttpRequest &) {
	sendStatus(HTTP_405);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handlePUTRequest(HttpRequest &) {
	sendStatus(HTTP_405);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleDELETERequest(HttpRequest &) {
	sendStatus(HTTP_405);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleTRACERequest(HttpRequest &) {
	sendStatus(HTTP_405);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::upgradeToWebsocket(HttpRequest &req, const char *protocol,
                                     uint64_t maxPayloadSize) {
	req.status = HTTP_101;
	req.state = HttpRequest::WAITING;
	req.keepAlive = true;
	send("HTTP/1.1 101 Switching Protocols\r\n"
	     "Upgrade: websocket\r\n"
	     "Connection: Upgrade\r\n");

	if ( protocol != nullptr ) {
		send("Sec-WebSocket-Protocol: ");
		send(protocol);
		send("\r\n");
	}

	send("Sec-WebSocket-Accept: ");

	// Concatenate the input key and the "magic string"
	string key = req.secWebsocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	char sha1[SHA_DIGEST_LENGTH]; // == 20
	SHA1(reinterpret_cast<const unsigned char*>(key.data()),
	     key.size(), reinterpret_cast<unsigned char*>(sha1));
	key.clear();
	Seiscomp::Util::encodeBase64(key, sha1, SHA_DIGEST_LENGTH);
	send(key.data(), key.size());
	send("\r\n\r\n");

	_upgradedToWebsocket = true;
	if ( !_websocketFrame ) {
		_websocketFrame = new Websocket::Frame;
	}
	else {
		_websocketFrame->reset();
	}
	_websocketFrame->setMaxPayloadSize(maxPayloadSize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::handleWebsocketFrame(Websocket::Frame &) {
	// Do nothing
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::sendWebsocketResponse(const char *data, int len,
                                        Websocket::Frame::Type type,
                                        Websocket::Status statusCode,
                                        bool close) {
	BufferPtr resp = new Buffer;
	if ( data != nullptr )
		resp->data.assign(data, data+len);
	Websocket::Frame::finalizeBuffer(resp.get(), type, statusCode);
	send(resp.get());
	if ( (statusCode != Websocket::NoStatus) || close )
		invalidate();
}



// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::sendStatus(HttpStatus status, const string &content,
                             const char *contentType) {
	// Save status in request
	_request.status = status;

	// Send response
	send("HTTP/1.1 ");
	send(status.toString());
	if ( _server ) {
		send("\r\nServer: ");
		send(_server);
	}

	if ( !content.empty() ) {
		send("\r\nContent-Length: ");
		const string length = Seiscomp::Core::toString(content.size());
		send(length.c_str(), length.size());
	}
	else
		send("\r\nContent-Length: 0");

	if ( contentType != nullptr ) {
		send("\r\nContent-Type: ");
		send(contentType);
	}

	if ( !_request.origin.empty() )
		send("\r\nAccess-Control-Allow-Origin: *");

	if ( _request.keepAlive && _request.addKeepAliveHeader ) {
		send("\r\nKeep-Alive: timeout=15");
		send("\r\nConnection: Keep-Alive");
	}

	send("\r\n\r\n", 4);

	if ( content.size() ) {
		send(content.data(), content.size());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void HttpSession::redirect(const char *path) {
	_request.status = HTTP_302;
	_request.state = HttpRequest::FINISHED;
	_request.keepAlive = false;

	send("HTTP/1.1 ");
	send(HttpStatus(HTTP_302).toString());
	send("\r\n"
	     "Location: ");
	send(path);
	send("\r\n");
	if ( !_request.cookie.empty() ) {
		send("Set-Cookie: ");
		send(_request.cookie.c_str(), _request.cookie.size());
		send("\r\n");
	}
	send("\r\n\r\n");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
