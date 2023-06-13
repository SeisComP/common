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


#define SEISCOMP_COMPONENT WSSQL
#include <seiscomp/logging/log.h>
#include <seiscomp/client/application.h>
#include <seiscomp/core/endianess.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/system.h>
#include <seiscomp/utils/base64.h>
#include <seiscomp/wired/protocols/websocket.h>

#include <boost/algorithm/string.hpp>

#include <sstream>

#include "websocket_private.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::IO;
using namespace Seiscomp::Wired;


namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_DB_INTERFACE(WebsocketProxy, "ws");
REGISTER_DB_INTERFACE(WebsocketProxySecure, "wss");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct HttpResponse {
	typedef std::pair<std::string, std::string> Header;
	typedef std::vector<Header> Headers;

	int         statusCode;
	int         contentLength;

	std::string statusMessage;
	std::string statusLine;
	Headers     headers;
	std::string body;
	std::string currentHeaderLine;

	bool        statusLineComplete;
	bool        headersComplete;
	bool        isFinished;

	HttpResponse() {
		reset();
	}

	void reset() {
		statusCode = -1;
		headers = Headers();
		body = std::string();
		statusLineComplete = false;
		headersComplete = false;
		isFinished = false;
		contentLength = -1;
		currentHeaderLine = std::string();
	}

	ssize_t feed(const char *data, int len) {
		if ( isFinished ) {
			return 0;
		}

		const char *data_start = data;

		while ( len ) {
			if ( statusLineComplete ) {
				if ( headersComplete ) {
					// Read body
					if ( contentLength > (int)body.size() ) {
						int n = std::min((int)(contentLength-body.size()), len);
						body.append(data, n);
						data += n;
						len -= n;

						if ( contentLength == (int)body.size() ) {
							isFinished = true;
							return data-data_start;
						}
					}
					else if ( contentLength <= 0 ) {
						isFinished = true;
						return data-data_start;
					}
				}
				else {
					// Read headers
					while ( len ) {
						char ch = *data;
						++data; --len;

						if ( ch == '\r' )
							continue;
						else if ( ch == '\n' ) {
							if ( currentHeaderLine.empty() ) {
								headersComplete = true;
								if ( contentLength <= 0 )
									isFinished = true;
								break;
							}

							// Parse header
							const char *lineStart = currentHeaderLine.data();
							size_t lineLen = currentHeaderLine.size();
							size_t key_len;
							const char *key = Core::tokenize(lineStart, ":", lineLen, key_len);
							if ( !key ) {
								SEISCOMP_ERROR("Invalid header");
								return -1;
							}

							size_t val_len = 0;
							const char *val = Core::tokenize(lineStart, ":", lineLen, val_len);
							if ( !val ) {
								SEISCOMP_ERROR("Invalid header");
								return -1;
							}

							Core::trim(key, key_len);
							Core::trim(val, val_len);

							headers.push_back(Header());
							headers.back().first.assign(key, key_len);
							headers.back().second.assign(val, val_len);

							if ( strcasecmp(headers.back().first.c_str(), "Content-Length") == 0 ) {
								if ( !Core::fromString(contentLength, headers.back().second) ) {
									SEISCOMP_ERROR("Invalid content length: %s", headers.back().second.c_str());
									return -1;
								}
							}

							currentHeaderLine.clear();
						}
						else
							currentHeaderLine += ch;
					}
				}
			}
			else {
				// Read status line
				while ( len ) {
					char ch = *data;
					++data; --len;

					if ( ch == '\r' ) {
						continue;
					}
					else if ( ch == '\n' ) {
						statusLineComplete = true;
						size_t p = statusLine.find(' ');
						if ( p == std::string::npos ) {
							return -1;
						}

						if ( p < 5 ) {
							return -1;
						}

						if ( statusLine.compare(0, 5, "HTTP/") ) {
							return -1;
						}

						++p;
						size_t p2 = statusLine.find(' ', p);
						if ( p2 - p == 0 ) {
							return -1;
						}

						int code = 0;
						while ( p < p2 ) {
							char ch = statusLine[p];
							if ( (ch < '0') || (ch > '9') ) {
								return -1;
							}
							code *= 10;
							code += (ch-'0');
							++p;
						}

						statusCode = code;

						p = statusLine.find_first_not_of(' ', p2+1);
						statusMessage = statusLine.substr(p);

						break;
					}
					else
						statusLine += ch;
				}
			}
		}

		return data-data_start;
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WebsocketProxy::WebsocketProxy() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::connect(const char* connection) {
	setDefaults();
	return DatabaseInterface::connect(connection);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxy::disconnect() {
	teardownConnection();
	_disconnected = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::isConnected() const {
	if ( _disconnected ) {
		return false;
	}

	while ( !validConnection() && !_disconnected ) {
		SEISCOMP_INFO("Re-establish database connection");
		if ( !establishConnection() ) {
			Core::msleep(500);
		}
	}

	return validConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxy::start() {
	if ( _debug ) {
		SEISCOMP_DEBUG("start");
	}
	sendAndReceiveStatus(WS_START);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxy::commit() {
	if ( _debug ) {
		SEISCOMP_DEBUG("commit");
	}
	sendAndReceiveStatus(WS_COMMIT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxy::rollback() {
	if ( _debug ) {
		SEISCOMP_DEBUG("rollback");
	}
	sendAndReceiveStatus(WS_ROLLBACK);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::execute(const char* command) {
	if ( _debug ) {
		SEISCOMP_DEBUG("[execute] %s", command);
	}

	return sendAndReceiveStatus(WS_EXECUTE, command, strlen(command));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::beginQuery(const char* query) {
	if ( _debug ) {
		SEISCOMP_DEBUG("[query] %s", query);
	}

	if ( !send(WS_QUERY, query, strlen(query)) ) {
		return false;
	}

	Websocket::Frame responseFrame;
	if ( !readFrame(responseFrame) ) {
		teardownConnection();
		return false;
	}

	if ( responseFrame.type != Websocket::Frame::BinaryFrame ) {
		SEISCOMP_ERROR("[query] protocol error, expected binary frame");
		teardownConnection();
		return false;
	}

	if ( *reinterpret_cast<uint8_t*>(&responseFrame.data[1]) != 0 ) {
		SEISCOMP_ERROR("[query] received error code %d", int(responseFrame.data[1]));
		return false;
	}

	if ( responseFrame.data.size() < 6 ) {
		SEISCOMP_ERROR("[query] invalid frame, code = 1");
		return false;
	}

	int fieldCount = *reinterpret_cast<int*>(&responseFrame.data[2]);
	Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,4>::Take(&fieldCount, 1);

	if ( fieldCount < 0 ) {
		SEISCOMP_ERROR("[query] invalid frame, code = 2");
		return false;
	}

	_fields.resize(fieldCount);

	if ( _debug ) {
		SEISCOMP_DEBUG("Fields: %d", fieldCount);
	}

	const char *fields = responseFrame.data.data() + 6; // Skip code and field count
	for ( int i = 0; i < fieldCount; ++i ) {
		// Convert field size to little endian
		int l = *reinterpret_cast<const int*>(fields);
		Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,4>::Take(&l, 1);
		fields += 4;
		_fields[i].name = string(fields, l);
		if ( _debug ) {
			SEISCOMP_DEBUG("  field #%d: %s", i, _fields[i].name.c_str());
		}
		fields += l;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxy::endQuery() {
	if ( _debug ) {
		SEISCOMP_DEBUG("query end");
	}
	_fields.clear();
	sendAndReceiveStatus(WS_QUERY_END);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseInterface::OID WebsocketProxy::lastInsertId(const char* table) {
	send(WS_LAST_ID, table, strlen(table));

	Websocket::Frame responseFrame;
	if ( !readFrame(responseFrame) ) {
		teardownConnection();
		return INVALID_OID;
	}

	if ( responseFrame.type != Websocket::Frame::BinaryFrame ) {
		SEISCOMP_ERROR("[query] protocol error, expected binary frame");
		teardownConnection();
		return INVALID_OID;
	}

	if ( responseFrame.data.size() != 10 ) {
		SEISCOMP_ERROR("[query] protocol error, expected response with 9 byte payload");
		teardownConnection();
		return INVALID_OID;
	}

	if ( *reinterpret_cast<uint8_t*>(&responseFrame.data[1]) != 0 ) {
		SEISCOMP_ERROR("[query] received error code %d", int(responseFrame.data[1]));
		return INVALID_OID;
	}

	uint64_t oid = *reinterpret_cast<uint64_t*>(&responseFrame.data[2]);
	Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,8>::Take(&oid, 1);
	return oid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
uint64_t WebsocketProxy::numberOfAffectedRows() {
	send(WS_AFFECTED_ROWS);

	Websocket::Frame responseFrame;
	if ( !readFrame(responseFrame) ) {
		teardownConnection();
		return INVALID_OID;
	}

	if ( responseFrame.type != Websocket::Frame::BinaryFrame ) {
		SEISCOMP_ERROR("[query] protocol error, expected binary frame");
		teardownConnection();
		return INVALID_OID;
	}

	if ( responseFrame.data.size() != 10 ) {
		SEISCOMP_ERROR("[query] protocol error, expected response with 9 byte payload");
		teardownConnection();
		return INVALID_OID;
	}

	if ( *reinterpret_cast<uint8_t*>(&responseFrame.data[1]) != 0 ) {
		SEISCOMP_ERROR("[query] received error code %d", int(responseFrame.data[1]));
		return INVALID_OID;
	}

	uint64_t nAffectedRows = *reinterpret_cast<uint64_t*>(&responseFrame.data[2]);
	Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,8>::Take(&nAffectedRows, 1);
	return nAffectedRows;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::fetchRow() {
	if ( !send(WS_FETCH) ) {
		return false;
	}

	Websocket::Frame responseFrame;
	if ( !readFrame(responseFrame) ) {
		teardownConnection();
		return false;
	}

	if ( responseFrame.type != Websocket::Frame::BinaryFrame ) {
		SEISCOMP_ERROR("[fetch] protocol error, expected binary frame");
		teardownConnection();
		return false;
	}

	if ( *reinterpret_cast<uint8_t*>(&responseFrame.data[1]) != 0 ) {
		// SEISCOMP_ERROR("[fetch] received error code %d", int(responseFrame.data[0]));
		// This signals the end of iteration
		return false;
	}

	if ( responseFrame.data.size() < 6 ) {
		SEISCOMP_ERROR("[fetch] invalid frame, code = 1");
		return false;
	}

	// Move data into local row storage and just let the field contents point
	// to them.
	_row = std::move(responseFrame.data);

	int fieldCount = *reinterpret_cast<int*>(&_row[2]);
	Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,4>::Take(&fieldCount, 1);

	if ( fieldCount < 0 ) {
		SEISCOMP_ERROR("[fetch] invalid frame, code = 2");
		return false;
	}

	if ( fieldCount != static_cast<int>(_fields.size()) ) {
		SEISCOMP_ERROR("[fetch] inconsistent field count, %d != %d",
		               fieldCount, static_cast<int>(_fields.size()));
		return false;
	}

	char *fields = &_row[0] + 6; // Skip code and field count
	int i;

	for ( i = 0; i < fieldCount; ++i ) {
		// Convert field size to little endian
		int l = *reinterpret_cast<const int*>(fields);
		Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,4>::Take(&l, 1);
		fields += 4;
		if ( l == -1 ) {
			_fields[i].content = nullptr;
			_fields[i].length = 0;
		}
		else {
			_fields[i].content = fields;
			_fields[i].length = l;
			fields += l;
		}

		if ( i && _fields[i-1].content ) {
			// Terminate previous string with null byte. It is safe to do
			// so as the length indicator is 4 byte preceeding a string so
			// this will just overwrite the first byte of the already processed
			// and copied length buffer.
			_fields[i-1].content[_fields[i-1].length] = '\0';
		}
	}

	if ( i && _fields[i-1].content ) {
		// Terminate previous string with null byte.
		_fields[i-1].content[_fields[i-1].length] = '\0';
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int WebsocketProxy::findColumn(const char* name) {
	for ( size_t i = 0; i < _fields.size(); ++i ) {
		if ( strcasecmp(_fields[i].name.c_str(), name) == 0 ) {
			return static_cast<int>(i);
		}
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int WebsocketProxy::getRowFieldCount() const {
	return static_cast<int>(_fields.size());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *WebsocketProxy::getRowFieldName(int index) {
	return _fields[index].name.data();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const void *WebsocketProxy::getRowField(int index) {
	return _fields[index].content;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t WebsocketProxy::getRowFieldSize(int index) {
	return _fields[index].length;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::handleURIParameter(const std::string &name,
                                        const std::string &value) {
	if ( !DatabaseInterface::handleURIParameter(name, value) ) {
		return false;
	}

	if ( name == "debug" ) {
		if ( value != "0" && value != "false" ) {
			_debug = true;
		}
	}
	else if ( name == "db-url" ) {
		_dbURL = value;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::open() {
	if ( validConnection() ) {
		SEISCOMP_ERROR("Still connected. Disconnect first.");
		return false;
	}

	return establishConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxy::handleInterrupt(int) {
	_disconnected = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::establishConnection() const {
	_socket = createSocket();
	_socket->setNoDelay(true);
	auto r = _socket->connect(_host, _port);
	if ( r != Socket::Success ) {
		_socket = nullptr;
		return false;
	}

	string path;

	if ( !_dbURL.empty() ) {
		path = "api/db";
	}
	else if ( _database.empty() ) {
		path = "production";
	}
	else {
		path = _database;
	}

	string authorization;

	if ( !_user.empty() ) {
		std::string auth;
		Util::encodeBase64(auth, _user + ':' + _password);
		authorization = "Authorization: Basic " + auth + "\r\n";
	}

	SEISCOMP_DEBUG("Attempt to connect to %s:%d/%s", _host.c_str(), _port, path.c_str());

	std::stringstream os;
	os << (_dbURL.empty() ? "GET" : "POST") << " /" << path << " HTTP/1.1\r\n"
	      "Host: " << _host << "\r\n"
	      "Upgrade: websocket\r\n"
	      "Connection: Upgrade\r\n"
	      "Sec-WebSocket-Protocol: scsql\r\n"
	      "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
	      "Sec-WebSocket-Version: 13\r\n";
	if ( !_dbURL.empty() ) {
		os << "Content-Length: " << _dbURL.size() << "\r\n";
	}
	os << authorization
	   << "\r\n";

	if ( !_dbURL.empty() ) {
		os << _dbURL;
	}

	if ( _socket->send(os.str().c_str()) <= 0 ) {
		_socket = nullptr;
		SEISCOMP_ERROR("Connection error: failed to send initial HTTP request");
		return false;
	}

	char buffer[512];
	ssize_t cnt;

	HttpResponse resp;
	while ( true ) {
		cnt = _socket->read(buffer, sizeof(buffer));
		if ( cnt <= 0 ) {
			_socket = nullptr;
			return false;
		}

		cnt = resp.feed(buffer, cnt);
		if ( cnt < 0 ) {
			_socket = nullptr;
			return false;
		}

		if ( resp.isFinished ) {
			break;
		}
	}

	const_cast<string&>(_columnPrefix) = "";
	for ( const auto &header : resp.headers ) {
		if ( header.first == "X-DB-Prefix" ) {
			const_cast<string&>(_columnPrefix) = header.second;
			SEISCOMP_DEBUG("Set column prefix to '%s'", _columnPrefix.c_str());
			break;
		}
	}

	if ( resp.statusCode != 101 ) {
		SEISCOMP_ERROR("Connection error %d: %s", resp.statusCode, resp.body.c_str());
		_socket = nullptr;
		return false;
	}

	_disconnected = false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxy::teardownConnection() {
	if ( _socket ) {
		_socket->close();
		_socket = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket *WebsocketProxy::createSocket() const {
	return new Socket;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxy::setDefaults() {
	_dbURL = string();
	_port = 18180;
	_database = "production";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::send(Command cmd, const char *body, int length) {
	uint64_t cnt, expectedCnt;

	while ( isConnected() ) {
		// Generate websocket header
		uint8_t control = 0x80 | Websocket::Frame::BinaryFrame;
		uint8_t plc;
		uint64_t pl = 1 + length;

		cnt = 0;
		expectedCnt = pl;

		if ( pl > 125 && pl <= 65535 ) {
			// Two byte size
			plc = 126;
			Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,2>::Take(&pl, 1);

			// Write websocket header bytes
			cnt += _socket->write(reinterpret_cast<const char*>(&control), 1);
			cnt += _socket->write(reinterpret_cast<const char*>(&plc), 1);
			cnt += _socket->write(reinterpret_cast<const char*>(&pl), 2);

			expectedCnt += 4;
		}
		else if ( pl > 65535 ) {
			// Eight byte size
			plc = 127;
			Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,8>::Take(&pl, 1);

			// Write websocket header bytes
			cnt += _socket->write(reinterpret_cast<const char*>(&control), 1);
			cnt += _socket->write(reinterpret_cast<const char*>(&plc), 1);
			cnt += _socket->write(reinterpret_cast<const char*>(&pl), 8);

			expectedCnt += 10;
		}
		else {
			// One byte size
			plc = uint8_t(pl);

			// Write websocket header bytes
			cnt += _socket->write(reinterpret_cast<const char*>(&control), 1);
			cnt += _socket->write(reinterpret_cast<const char*>(&plc), 1);

			expectedCnt += 2;
		}

		cnt += _socket->write(reinterpret_cast<const char*>(&cmd), 1);
		if ( length ) {
			cnt += _socket->write(body, length);
		}

		if ( cnt == expectedCnt ) {
			return true;
		}

		// Close socket and try again
		SEISCOMP_WARNING("Unable to talk to remote database proxy ... reconnecting");
		_socket->close();
	}

	// SEISCOMP_ERROR("[query] not connected");
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::sendAndReceiveStatus(Command cmd, const char *body, int length) {
	if ( !send(cmd, body, length) ) {
		teardownConnection();
		return false;
	}

	Websocket::Frame responseFrame;
	if ( !readFrame(responseFrame) ) {
		teardownConnection();
		return false;
	}

	// Command: *reinterpret_cast<uint8_t*>(&responseFrame.data[0]) == cmd
	return *reinterpret_cast<uint8_t*>(&responseFrame.data[1]) == 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketProxy::readFrame(Websocket::Frame &frame) {
	if ( frame.isFinished() ) {
		frame.reset();
	}

	char buffer[512];

	while ( !frame.isFinished() ) {
		auto cnt = _socket->read(buffer, sizeof(buffer));
		if ( cnt <= 0 ) {
			return false;
		}

		if ( frame.feed(buffer, cnt) < 0 ) {
			return false;
		}
	}

	switch ( frame.type ) {
		case Websocket::Frame::ConnectionClose:
			SEISCOMP_INFO("Received close frame: %s", frame.data.c_str());
			return false;
		case Websocket::Frame::ContinuationFrame:
			// Not yet expected
			SEISCOMP_ERROR("Received unexpected continuation frame");
			return false;
		default:
			break;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketProxySecure::setDefaults() {
	WebsocketProxy::setDefaults();
	_port = 18181;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket *WebsocketProxySecure::createSocket() const {
	SSL_CTX *ctx = nullptr;
	if ( !SCCoreApp ) {
		return new SSLSocket();
	}

	const auto &cert = SCCoreApp->messagingCertificate();
	if ( !cert.empty() ) {
		const string DataTag = "data:";
		if ( !boost::istarts_with(cert, DataTag) ) {
			ctx = Wired::SSLSocket::createClientContextFromFile(cert);
		}
		else {
			string base64Data = cert;
			base64Data.replace(0, DataTag.size(), "");
			Seiscomp::Core::trim(base64Data);

			string data;
			Seiscomp::Util::decodeBase64(data, base64Data.c_str(), base64Data.size());
			ctx = Wired::SSLSocket::createClientContext(data);
		}
	}

	return new SSLSocket(ctx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
