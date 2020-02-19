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


#define SEISCOMP_COMPONENT SCMP

#include <seiscomp/logging/log.h>
#include <seiscomp/datamodel/version.h>
#include <seiscomp/messaging/protocols/scmp/websocket.h>
#include <seiscomp/messaging/messages/database.h>
#include <seiscomp/broker/protocol.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/utils/url.h>
#include <seiscomp/utils/base64.h>
#include <seiscomp/wired/protocols/http.h>

#include <iostream>
#include <limits.h>
#include <errno.h>


using namespace std;


namespace Seiscomp {
namespace Client {
namespace SCMP {


namespace {


typedef Core::ContainerSink<string> StringSink;
typedef boost::iostreams::stream<StringSink> osstream;


/*
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
*/
struct HttpResponse {
	typedef std::pair<std::string, std::string> Header;
	typedef std::vector<Header> Headers;

	int         statusCode;
	std::string statusMessage;
	std::string statusLine;
	bool        statusLineComplete;
	Headers     headers;
	std::string body;
	int         contentLength;
	bool        headersComplete;
	bool        isFinished;

	std::string currentHeaderLine;

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

	int feed(const char *data, int len) {
		if ( isFinished ) return 0;

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
							int lineLen = (int)currentHeaderLine.size();
							int key_len;
							const char *key = Core::tokenize(lineStart, ":", lineLen, key_len);
							if ( !key ) {
								SEISCOMP_ERROR("Invalid header");
								return -1;
							}

							int val_len = 0;
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

					if ( ch == '\r' )
						continue;
					else if ( ch == '\n' ) {
						statusLineComplete = true;
						size_t p = statusLine.find(' ');
						if ( p == std::string::npos )
							return -1;

						if ( p < 5 )
							return -1;

						if ( statusLine.compare(0, 5, "HTTP/") )
							return -1;

						++p;
						size_t p2 = statusLine.find(' ', p);
						if ( p2-p == 0 )
							return -1;

						int code = 0;
						while ( p < p2 ) {
							char ch = statusLine[p];
							if ( ch < '0' || ch > '9' )
								return -1;
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


/**
 * @brief The SecureWebsocketConnection class is a simple wrapper that
 *        registers as "scmps" protocol and sets SSL as default.
 */
class SecureWebsocketConnection : public WebsocketConnection {
	public:
		SecureWebsocketConnection() {
			setSSL(true);
		}
};


}


REGISTER_CONNECTION_PROTOCOL(WebsocketConnection, "scmp");
REGISTER_CONNECTION_PROTOCOL(SecureWebsocketConnection, "scmps");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WebsocketConnection::WebsocketConnection() {
	_inboxWaterLevel = 0;
	_select.setTriggerMode(DeviceGroup::LevelTriggered);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WebsocketConnection::~WebsocketConnection() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::connect(const char *address,
                                    unsigned int timeoutMs,
                                    const char *clientName) {
	boost::mutex::scoped_lock lread(_readMutex);
	boost::mutex::scoped_lock lwrite(_writeMutex);

	SEISCOMP_INFO("scmp://%s", address);

	_state = State();
	_select.clear();
	_groups.clear();
	_errorMessage = string();

	_sockMutex.lock();

	if ( _useSSL )
		_socket = new Wired::SSLSocket;
	else
		_socket = new Wired::Socket;

	_socket->setNoDelay(true);

	_sockMutex.unlock();

	int port = -1;
	string host = "localhost";
	string path, queue;

	Util::Url url(address);
	if ( !url ) {
		_errorMessage = "Invalid URL";
		return InvalidURL;
	}

	host = url.host();
	port = url.port();
	if ( port <= 0 )
		port = _useSSL ? 18181 : 18180;

	path = url.path();
	if ( !path.empty() && *path.rbegin() != '/' ) {
		size_t p = path.find_last_of('/');
		if ( p != string::npos ) {
			queue = path.substr(p + 1);
			path.erase(path.begin() + p + 1, path.end());
		}
	}

	if ( queue.empty() ) {
		queue = "production";
	}

	string authorization;

	if ( url.username().length() > 0 ) {
		std::string auth;
		Util::encodeBase64(auth, url.username() + ':' + url.password());
		authorization = "Authorization: Basic " + auth + "\r\n";
	}

	for ( Util::Url::QueryItems::const_iterator it = url.queryItems().begin();
	      it != url.queryItems().end(); ++it ) {
		const string &param = it->first;
		const string &value = it->second;

		if ( param == "ack" ) {
			int ackWindow;
			if ( !Core::fromString(ackWindow, value) || ackWindow < 0 ) {
				SEISCOMP_ERROR("Invalid 'ack' value: %s: expected non negative number", value.c_str());
				_errorMessage = "Invalid 'ack' URL parameter";
				return InvalidURLParameters;
			}

			setAckWindow(ackWindow);
			SEISCOMP_DEBUG("Set acknowledge window to %d", ackWindow);
		}
	}

	_socket->setSocketTimeout(timeoutMs / 1000, (timeoutMs % 1000) * 1000);

	int ret = _socket->connect(host, port);
	if ( ret != Wired::Socket::Success ) {
		_errorMessage = "Failed to connect";
		_socket = nullptr;
		return NetworkError;
	}

	path += queue;
	SEISCOMP_DEBUG("Attempt to connect to %s:%d%s", host.c_str(), port, path.c_str());

	std::stringstream os;
	os << "GET " << path << " HTTP/1.1\r\n"
	      "Host: " << url.host() << "\r\n"
	      "Upgrade: websocket\r\n"
	      "Connection: Upgrade\r\n"
	      "Sec-WebSocket-Protocol: scmp\r\n"
	      "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
	      "Sec-WebSocket-Version: 13\r\n"
	   << authorization
	   << "\r\n";

	ret = _socket->send(os.str().c_str());
	if ( ret <= 0 ) {
		_errorMessage = "Failed to connect to websocket";
		_socket = nullptr;
		return NetworkError;
	}

	_getcount = 0;
	_getp = _buffer;

	HttpResponse resp;
	while ( true ) {
		ret = _socket->read(_buffer, sizeof(_buffer));
		++_state.systemReadCalls;
		if ( ret <= 0 ) {
			_errorMessage = "Connection lost";
			_socket = nullptr;
			return NetworkError;
		}

		_getcount = ret;
		_getp = _buffer;

		_state.bytesReceived += ret;

		ret = resp.feed(_getp, ret);
		if ( ret < 0 ) {
			_errorMessage = "Invalid HTTP response";
			_socket = nullptr;
			return NetworkProtocolError;
		}

		if ( resp.isFinished ) {
			_getp += ret;
			_getcount -= ret;
			break;
		}
	}

	if ( resp.statusCode != 101 ) {
		_errorMessage = resp.body;
		_socket = nullptr;
		return NetworkProtocolError;
	}

	if ( !_select.isValid() ) {
		if ( !_select.setup() ) {
			_errorMessage = "Failed to setup polling device";
			_socket = nullptr;
			return SystemError;
		}
	}

	Buffer msg;

	{
		osstream os(msg.data);
		os << SCMP_PROTO_CMD_CONNECT "\n"
		      SCMP_PROTO_CMD_CONNECT_HEADER_ACK_WINDOW ": 1.0\n"
		      "Ack-Window: " << _ackWindow << "\n";

		if ( _state.sequenceNumber )
			os << SCMP_PROTO_CMD_CONNECT_HEADER_SEQ_NUMBER ": " << *_state.sequenceNumber << "\n";

		if ( clientName )
			os << SCMP_PROTO_CMD_CONNECT_HEADER_CLIENT_NAME ":" << clientName << "\n";

		if ( !_subscriptions.empty() ) {
			os << SCMP_PROTO_CMD_CONNECT_HEADER_SUBSCRIPTIONS ": ";
			set<string>::iterator it;
			int idx = 0;
			for ( it = _subscriptions.begin(); it != _subscriptions.end(); ++it, ++idx ) {
				if ( idx )
					os << ',';
				os << *it;
			}

			os << '\n';
		}

		os << SCMP_PROTO_CMD_CONNECT_HEADER_MEMBERSHIP_INFO ": " << (_wantMembershipInfo ? "1":"0") << "\n"
		      SCMP_PROTO_CMD_CONNECT_HEADER_SELF_DISCARD ": 1\n"
		      "\n";
	}

	Result r = send(&msg, WSFrame::TextFrame, false);
	if ( r != OK ) {
		_errorMessage = "Failed to send connect message";
		_socket = nullptr;
		return r;
	}

	Wired::Websocket::Frame connectFrame;
	r = readFrame(connectFrame, &_readMutex);
	if ( r != OK ) {
		_errorMessage = "Failed to read connect response";
		return r;
	}

	FrameHeaders headers(connectFrame.data.data(), connectFrame.data.size());
	if ( !headers.next() ) {
		_errorMessage = "Invalid connect response received";
		_socket = nullptr;
		return NetworkProtocolError;
	}

	if ( headers.val_len ) {
		_errorMessage = "Invalid connect response received";
		_socket = nullptr;
		return NetworkProtocolError;
	}

	if ( !headers.nameEquals(SCMP_PROTO_REPLY_CONNECT) ) {
		Result r = NetworkProtocolError;

		if ( headers.nameEquals(SCMP_PROTO_REPLY_ERROR) ) {
			while ( headers.next() )
				if ( !headers.name_len ) break;

			_errorMessage.assign(headers.getptr(), connectFrame.data.data()+connectFrame.data.size()-headers.getptr());

			size_t p = _errorMessage.find(' ');
			if ( p != string::npos ) {
				int code;
				if ( Core::fromString(code, _errorMessage.substr(0, p)) ) {
					_errorMessage.erase(0, p+1);

					switch ( code ) {
						case 408:
							r = DuplicateUsername;
							break;
						case 411:
							r = GroupDoesNotExist;
							break;
						default:
							break;
					}
				}
			}
		}
		else
			_errorMessage = "Expected " SCMP_PROTO_REPLY_CONNECT " messages as first response";
		_socket = nullptr;
		return r;
	}

	bool validHeader = false;
	while ( headers.next() ) {
		if ( !headers.name_len ) {
			validHeader = true;
			break;
		}

		if ( headers.nameEquals(SCMP_PROTO_REPLY_CONNECT_HEADER_CLIENT_NAME) ) {
			_registeredClientName.assign(headers.val_start, headers.val_len);
			SEISCOMP_DEBUG("Registered clientname: %s", _registeredClientName.c_str());
		}
		else if ( headers.nameEquals(SCMP_PROTO_REPLY_CONNECT_HEADER_GROUPS) ) {
			vector<string> groups;
			Core::split(groups, string(headers.val_start, headers.val_len).c_str(), ",", true);
			for ( auto &&group : groups ) _groups.insert(group);
		}
		// Parse DB extensions
		else if ( headers.nameEquals("Schema-Version") ) {
			string version(headers.val_start, headers.val_len);
			if ( !_schemaVersion.fromString(version) ) {
				SEISCOMP_WARNING("Invalid Schema-Version content: %s", version.c_str());
				continue;
			}
		}
		else if ( headers.nameEquals("DB-Access") ) {
			string readParameters(headers.val_start, headers.val_len);
			Packet *packet = new Packet;

			size_t p = readParameters.find("://");
			if ( p != string::npos ) {
				DatabaseProvideMessage msg(readParameters.substr(0, p).c_str(),
				                           readParameters.substr(p+3).c_str());
				packet->target = _registeredClientName;
				packet->type = Packet::Data;
				packet->headerContentType = Protocol::ContentType(Protocol::Binary).toString();
				packet->sender = "MASTER";
				Protocol::encode(packet->payload, &msg, Protocol::Identity, Protocol::Binary, -1);
				queuePacket(packet);
			}
		}
	}

	if ( !validHeader ) {
		_errorMessage = "Invalid response frame";
		_socket = nullptr;
		return NetworkProtocolError;
	}

	_socket->setSocketTimeout(0, 0);

	_socket->setNonBlocking(true);
	_socket->setMode(Wired::Device::Read);
	_select.append(_socket.get());

	Core::Version maxVersion = Core::Version(DataModel::Version::Major, DataModel::Version::Minor);
	if ( _schemaVersion > maxVersion ) {
		SEISCOMP_INFO("Outgoing messages are encoded to match schema version %d.%d, "
		              "although server supports %d.%d",
		              maxVersion.majorTag(), maxVersion.minorTag(),
		              _schemaVersion.majorTag(), _schemaVersion.minorTag());
		_schemaVersion = maxVersion;
	}
	else
		SEISCOMP_INFO("Outgoing messages are encoded to match schema version %d.%d",
		              _schemaVersion.majorTag(), _schemaVersion.minorTag());

	// Flush the outbox with respect to last messages
	return flushBacklog();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::subscribe(const std::string &group) {
	Result r;

	{
		boost::mutex::scoped_lock l(_readMutex);

		_sockMutex.lock();
		if ( !_socket ) {
			_sockMutex.unlock();
			_subscriptions.insert(group);
			return OK;
		}
		_sockMutex.unlock();

		if ( _subscriptions.find(group) != _subscriptions.end() )
			return AlreadySubscribed;

		if ( _groups.find(group) == _groups.end() )
			return GroupDoesNotExist;
	}

	{
		boost::mutex::scoped_lock l(_writeMutex);

		Buffer msg;
		msg.data = SCMP_PROTO_CMD_SUBSCRIBE "\n"
		           SCMP_PROTO_CMD_SUBSCRIBE_HEADER_GROUPS ":";
		msg.data += group;
		msg.data += "\n\n";

		r = send(&msg, WSFrame::TextFrame, false);
		if ( r != OK )
			return r;
	}

	{
		boost::mutex::scoped_lock l(_readMutex);
		do {
			if ( _subscriptions.find(group) != _subscriptions.end() ) {
				return OK;
			}

			r = fetchAndQueuePacket();
		}
		while ( r == OK );
	}

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::unsubscribe(const std::string &group) {
	Result r;

	{
		boost::mutex::scoped_lock l(_readMutex);

		_sockMutex.lock();
		if ( !_socket ) {
			_sockMutex.unlock();
			set<string>::iterator it;
			it = _subscriptions.find(group);
			if ( it == _subscriptions.end() )
				return NotSubscribed;

			_subscriptions.erase(it);
			return OK;
		}
		_sockMutex.unlock();
	}

	{
		set<string>::iterator it;
		it = _subscriptions.find(group);
		if ( it == _subscriptions.end() )
			return NotSubscribed;

		boost::mutex::scoped_lock l(_writeMutex);

		Buffer msg;
		msg.data = SCMP_PROTO_CMD_UNSUBSCRIBE "\n"
		           SCMP_PROTO_CMD_UNSUBSCRIBE_HEADER_GROUPS ":";
		msg.data += group;
		msg.data += "\n\n";

		r = send(&msg, WSFrame::TextFrame, false);
		if ( r != OK )
			return r;
	}

	{
		boost::mutex::scoped_lock l(_readMutex);
		do {
			if ( _subscriptions.find(group) != _subscriptions.end() )
				return OK;

			r = fetchAndQueuePacket();
		}
		while ( r == OK );
	}

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::sendData(const string &targetGroup,
                                     const char *data, size_t len,
                                     MessageType type,
                                     ContentEncoding contentEncoding,
                                     ContentType contentType) {
	if ( targetGroup.empty() )
		return MissingGroup;

	if ( _groups.find(targetGroup) == _groups.end() )
		return GroupDoesNotExist;

	{
		boost::mutex::scoped_lock l(_writeMutex);

		if ( _state.bytesBuffered > _state.maxBufferedBytes )
			_state.maxBufferedBytes = _state.bytesBuffered;

		if ( _outbox.size() > _state.maxOutboxSize )
			_state.maxOutboxSize = _outbox.size();

		BufferPtr websocketFrame = new Buffer;

		switch ( type ) {
			case Protocol::Regular:
			case Protocol::Transient:
			{
				osstream os(websocketFrame->data);
				os << SCMP_PROTO_CMD_SEND << "\n"
				      SCMP_PROTO_CMD_SEND_HEADER_DESTINATION ":" << targetGroup << "\n"
				      SCMP_PROTO_CMD_SEND_HEADER_CONTENT_LENGTH ":" << len << "\n";
				if ( contentEncoding != Identity )
					os << SCMP_PROTO_CMD_SEND_HEADER_ENCODING ":" << contentEncoding.toString() << "\n";
				os << SCMP_PROTO_CMD_SEND_HEADER_MIMETYPE ":" << contentType.toString() << "\n";
				if ( type == Protocol::Transient )
					os << SCMP_PROTO_CMD_SEND_HEADER_TRANSIENT << "\n";
				os << "\n";
				break;
			}
			case Protocol::Status:
			{
				osstream os(websocketFrame->data);
				os << SCMP_PROTO_CMD_STATE << "\n"
				      SCMP_PROTO_CMD_STATE_HEADER_DESTINATION ":" << targetGroup << "\n"
				      SCMP_PROTO_CMD_STATE_HEADER_CONTENT_LENGTH ":" << len << "\n";
				os << "\n";
				break;
			}
			default:
				return InvalidMessageType;

		}

		websocketFrame->data.append(data, len);

		Result r = send(websocketFrame.get(), WSFrame::BinaryFrame, true);
		if ( r != OK ) return r;
	}

	//updateReceiveBuffer();

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::sendMessage(const std::string &targetGroup,
                                        const Core::Message *msg,
                                        MessageType type,
                                        OPT(ContentEncoding) contentEncoding,
                                        OPT(ContentType) contentType) {
	if ( targetGroup.empty() )
		return MissingGroup;

	if ( _groups.find(targetGroup) == _groups.end() )
		return GroupDoesNotExist;

	if ( !contentType )
		return ContentTypeRequired;

	if ( !contentEncoding )
		return ContentEncodingRequired;

	{
		boost::mutex::scoped_lock l(_writeMutex);

		if ( _state.bytesBuffered > _state.maxBufferedBytes )
			_state.maxBufferedBytes = _state.bytesBuffered;

		if ( _outbox.size() > _state.maxOutboxSize )
			_state.maxOutboxSize = _outbox.size();

		BufferPtr websocketFrame = new Buffer;

		string blob;
		if ( !encode(blob, msg, *contentEncoding, *contentType, -1) || blob.empty() )
			return EncodingError;

		{
			osstream os(websocketFrame->data);
			os << SCMP_PROTO_CMD_SEND << "\n"
			      SCMP_PROTO_CMD_SEND_HEADER_DESTINATION ":" << targetGroup << "\n"
			      SCMP_PROTO_CMD_SEND_HEADER_CONTENT_LENGTH ":" << blob.size() << "\n";
			if ( *contentEncoding != Identity )
				os << SCMP_PROTO_CMD_SEND_HEADER_ENCODING ":" << contentEncoding->toString() << "\n";
			os << SCMP_PROTO_CMD_SEND_HEADER_MIMETYPE ":" << contentType->toString() << "\n";
			if ( type == Protocol::Transient )
				os << SCMP_PROTO_CMD_SEND_HEADER_TRANSIENT << "\n";
			os << "\n";
		}

		websocketFrame->data.append(blob);

		Result r = send(websocketFrame.get(), WSFrame::BinaryFrame, true);
		if ( r != OK ) return r;
	}

	//updateReceiveBuffer();

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::fetchInbox() {
	boost::mutex::scoped_lock l(_readMutex);
	Result r = readFrame(_recvFrame, &_readMutex);
	if ( r != OK ) return r;
	handleFrame(_recvFrame, nullptr, &r);
	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::syncOutbox() {
	Result r = OK;

	if ( _ackWindow == 0 ) return r;

	boost::mutex::scoped_lock lw(_writeMutex);

	while ( !_outbox.empty() ) {
		_writeMutex.unlock();
		_readMutex.lock();

		if ( _inWait ) {
			_writeMutex.lock();
			_readMutex.unlock();

			if ( _outbox.empty() ) {
				break;
			}

			_waitMutex.lock();
			_waitMutex.unlock();

			continue;
		}
		else {
			_writeMutex.lock();
			if ( _outbox.empty() ) {
				_readMutex.unlock();
				break;
			}
			_writeMutex.unlock();
		}

		r = readFrame(_recvFrame, nullptr, true);
		if ( r != OK ) {
			_writeMutex.lock();
			_readMutex.unlock();
			break;
		}

		handleFrame(_recvFrame, nullptr);
		_writeMutex.lock();
		_readMutex.unlock();
	}

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::recv(Packet &p) {
	while ( true ) {
		boost::mutex::scoped_lock l(_readMutex);

		// Fill the inbox as much as possible without blocking
		if ( _inboxWaterLevel ) {
			while ( _inbox.size() < _inboxWaterLevel ) {
				if ( readFrame(_recvFrame, nullptr, false) != OK )
					break;
				handleFrame(_recvFrame, nullptr);
			}
		}

		if ( !_inbox.empty() ) {
			Packet *p0 = _inbox.front();
			_inbox.pop_front();
			p.swap(*p0);
			delete p0;
			return OK;
		}

		Result r = readFrame(_recvFrame, &_readMutex);
		if ( r != OK ) return r;

		if ( handleFrame(_recvFrame, &p, &r) ) {
			// Check the inbox again to make sure that messages are
			// processed in order
			if ( !_inbox.empty() ) {
				Packet *p0 = _inbox.front();
				_inbox.pop_front();

				p.swap(*p0);
				queuePacket(p0);
			}

			return OK;
		}
		else if ( r != OK )
			return r;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Packet *WebsocketConnection::recv(Result *result) {
	Packet *p = nullptr;
	Result r = OK;

	while ( true ) {
		boost::mutex::scoped_lock l(_readMutex);

		// Fill the inbox as much as possible without blocking
		if ( _inboxWaterLevel ) {
			while ( _inbox.size() < _inboxWaterLevel ) {
				if ( readFrame(_recvFrame, nullptr, false) != OK )
					break;
				handleFrame(_recvFrame, nullptr);
			}
		}

		if ( !_inbox.empty() ) {
			Packet *p0 = _inbox.front();
			_inbox.pop_front();
			if ( p ) delete p;
			if ( result ) *result = OK;
			return p0;
		}

		if ( !p ) p = new Packet;

		r = readFrame(_recvFrame, &_readMutex);
		if ( r != OK ) break;

		if ( handleFrame(_recvFrame, p, &r) ) {
			// Check the inbox again to make sure that messages are
			// processed in order
			if ( !_inbox.empty() ) {
				Packet *p0 = _inbox.front();
				_inbox.pop_front();
				queuePacket(p);
				if ( result ) *result = OK;
				return p0;
			}

			if ( result ) *result = OK;
			return p;
		}
		else if ( r != OK )
			break;
	}

	if ( result ) *result = r;
	if ( p ) delete p;
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketConnection::waitForAck() {
	if ( _ackWindow == 0 ) return;

	boost::mutex::scoped_lock lw(_writeMutex);
	if ( _state.localSequenceNumber < _ackWindow ) return;

	while ( _outbox.size() >= _ackWindow ) {
		_writeMutex.unlock();
		_readMutex.lock();

		if ( _inWait ) {
			//cerr << "Another thread is currently waiting and outbox is " << _outbox.size() << std::endl;
			_writeMutex.lock();
			_readMutex.unlock();

			if ( _outbox.size() < _ackWindow ) {
				//cerr << "On second attempt" << std::endl;
				break;
			}

			_waitMutex.lock();
			_waitMutex.unlock();

			continue;
		}
		else {
			_writeMutex.lock();
			if ( _outbox.size() < _ackWindow ) {
				_readMutex.unlock();
				//cerr << "On second no-wait attempt" << std::endl;
				break;
			}
			_writeMutex.unlock();
		}

		if ( readFrame(_recvFrame, nullptr, true) != OK ) {
			_writeMutex.lock();
			_readMutex.unlock();
			break;
		}

		handleFrame(_recvFrame, nullptr);
		_writeMutex.lock();
		_readMutex.unlock();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::flushBacklog() {
	SEISCOMP_INFO("Want to flush %d backlog messages", int(_backlog.size()));
	while ( !_backlog.empty() ) {
		// Backlog messages are always binary frames
		Result r = send(_backlog.front().get(), WSFrame::BinaryFrame, true);
		if ( r != OK ) {
			SEISCOMP_INFO("Want to flush %d backlog messages", int(_backlog.size()));
			return r;
		}
		_backlog.pop_front();
	}

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::disconnect() {
	{
		boost::mutex::scoped_lock lsock(_sockMutex);

		if ( !_socket || !_socket->isValid() )
			return NotConnected;
	}

	// Send the disconnect message
	{
		boost::mutex::scoped_lock lwrite(_writeMutex);

		if ( _registeredClientName.empty() )
			return NotConnected;

		_subscriptions.clear();

		Buffer buf;
		buf.data  = SCMP_PROTO_CMD_DISCONNECT "\n"
		            SCMP_PROTO_CMD_DISCONNECT_HEADER_RECEIPT ":";
		buf.data += _registeredClientName;
		buf.data += "\n\n";

		Result r = send(&buf, WSFrame::TextFrame, false);
		if ( r != OK ) return r;
	}

	boost::mutex::scoped_lock lread(_readMutex);

	if ( _registeredClientName.empty() ) {
		// Already disconnected
		return OK;
	}

	clearInbox();
	_groups.clear();

	// Receive and ignore all frames unless a receipt frame is
	// received
	Packet p;
	while ( readFrame(_recvFrame, nullptr, true) == OK )
		handleFrame(_recvFrame, &p);

	_registeredClientName = string();
	_state.sequenceNumber = Core::None;
	_schemaVersion = 0;
	// Remove all un-ack'ed messages as we have actively disconnected
	// the session
	_backlog.clear();
	_outbox.clear();

	return Socket::close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketConnection::isConnected() {
	boost::mutex::scoped_lock lread(_readMutex);
	return !_registeredClientName.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::setTimeout(int milliseconds) {
	boost::mutex::scoped_lock l(_sockMutex);
	if ( _socket ) {
		_socket->setTimeout(milliseconds > 0 ? milliseconds : -1);
		return OK;
	}
	else
		return NotConnected;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::readFrame(Wired::Websocket::Frame &frame,
                                      boost::mutex *mutex,
                                      bool forceBlock) {
	while ( true ) {
		if ( frame.isFinished() ) frame.reset();
		while ( _getcount ) {
			while ( !frame.isFinished() ) {
				int len = frame.feed(_getp, _getcount);
				if ( len <= 0 ) {
					SEISCOMP_ERROR("WS::readFrame consume error: %d -> %d",
					               _getcount, len);
					return Error;
				}

				_getcount -= len;
				_getp += len;
				if ( !_getcount ) break;
			}

			if ( frame.isFinished() ) {
				switch ( frame.type ) {
					case Wired::Websocket::Frame::ConnectionClose:
						closeSocket("Server closed the connection");
						return ConnectionClosedByPeer;
					case Wired::Websocket::Frame::Ping:
						// TODO: send PONG
						continue;
					case Wired::Websocket::Frame::Pong:
						// Ignore PONG frames
						SEISCOMP_WARNING("Received PONG frame???");
						continue;
					case Wired::Websocket::Frame::ContinuationFrame:
						// Not yet expected
						closeSocket("Received unexpected continuation frame");
						return NetworkProtocolError;
					default:
						size_t p = frame.data.find('\n');
						if ( p == string::npos ) {
							// Failed to read command
							closeSocket("Invalid frame format received, missing newline");
							return NetworkProtocolError;
						}
						break;
				}

				return OK;
			}
		}

		_sockMutex.lock();
		if ( !_socket || !_socket->isValid() ) {
			_sockMutex.unlock();
			_errorMessage = "Not connected";
			return NotConnected;
		}

		int remainingBytes = _socket->read(_buffer, sizeof(_buffer));
		_sockMutex.unlock();
		++_state.systemReadCalls;
		if ( remainingBytes < 0 ) {
			if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
				closeSocket(strerror(errno));
				return SystemError;
			}
			else {
				if ( mutex ) {
					if ( !wait(mutex, &_waitMutex) )
						return SystemError;
					else if ( _select.timedOut() )
						return TimeoutError;
					continue;
				}
				else if ( forceBlock ) {
					if ( !_inWait ) {
						if ( !wait(nullptr, &_waitMutex) )
							return SystemError;
						else if ( _select.timedOut() )
							return TimeoutError;
					}
					else {
						_waitMutex.lock();
						_waitMutex.unlock();
					}
					continue;
				}

				// Do not close the socket as this mode is desired to read
				// non blocking.
				return SystemError;
			}
		}
		else if ( remainingBytes == 0 ) {
			closeSocket("Server closed the connection");
			return ConnectionClosedByPeer;
		}

		_getcount = remainingBytes;
		_getp = _buffer;

		_state.bytesReceived += remainingBytes;
	}

	SEISCOMP_ERROR("WS::readFrame: unspecified");
	return Error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::send(Buffer *msg, WSFrame::Type type, bool isRegular) {
	Result r;

	_sockMutex.lock();
	if ( !_socket || !_socket->isValid() ) {
		_sockMutex.unlock();
		_errorMessage = "Not connected";
		return NotConnected;
	}

	Wired::Websocket::Frame::finalizeBuffer(msg, type);
	_socket->addMode(Wired::Device::Write);
	r = sendSocket(msg->header.data(), msg->header.size());
	if ( r == OK )
		r = sendSocket(msg->data.data(), msg->data.size());
	_socket->removeMode(Wired::Device::Write);
	_sockMutex.unlock();

	if ( isRegular && (r == OK) ) {
		_state.bytesBuffered += msg->data.size();
		++_state.localSequenceNumber;
		++_state.sentMessages;

		if ( _ackWindow ) _outbox.push_back(msg);

		if ( _ackWindow ) {
			_writeMutex.unlock();
			waitForAck();
			_writeMutex.lock();
		}
	}

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::sendSocket(const char *data, int len) {
	int written = 0;
	while ( len > 0 ) {
		written = _socket->write(data, len);
		++_state.systemWriteCalls;
		if ( written < 0 ) {
			if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
				// Close the session
				//SEISCOMP_ERROR("[client] read error: %s", strerror(errno));
				closeSocketWithoutLock(strerror(errno));
				return SystemError;
			}

			_sockMutex.unlock();
			_readMutex.lock();
			if ( !_inWait ) {
				wait(&_readMutex, &_waitMutex);
			}
			else {
				_waitMutex.lock();
				_waitMutex.unlock();
			}
			_readMutex.unlock();
			_sockMutex.lock();

			/*
			if ( _select.readable() )
				updateReceiveBuffer();
			*/
		}
		else if ( written == 0 ) {
			// Closed by peer
			closeSocketWithoutLock("Connection closed by peer");
			return ConnectionClosedByPeer;
		}
		else {
			data += written;
			len -= written;
			_state.bytesSent += written;
		}
	}

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketConnection::updateReceiveBuffer() {
	boost::mutex::scoped_lock l(_readMutex);
	while ( readFrame(_recvFrame, nullptr) == OK ) {
		handleFrame(_recvFrame, nullptr);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result WebsocketConnection::fetchAndQueuePacket() {
	Result r;

	if ( (r = readFrame(_recvFrame, nullptr, true)) == OK )
		handleFrame(_recvFrame, nullptr, &r);

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketConnection::handleFrame(Wired::Websocket::Frame &frame,
                                      Packet *p, Result *r) {
	FrameHeaders headers(_recvFrame.data.data(), _recvFrame.data.size());
	if ( r ) *r = OK;

	if ( !headers.next() ) {
		SEISCOMP_ERROR("decoding frame");
		if ( r ) *r = Error;
		return false;
	}

	if ( headers.nameEquals(SCMP_PROTO_REPLY_SEND) ) {
		Packet *inboxPkt = nullptr;
		if ( !p ) {
			inboxPkt = new Packet;
			p = inboxPkt;
		}

		bool gotSequenceNumber = false;
		int contentLength = -1;

		while ( headers.next() ) {
			// End of header section
			if ( !headers.name_len ) break;
			if ( headers.nameEquals(SCMP_PROTO_REPLY_SEND_HEADER_SEQ_NUMBER) ) {
				char *end;
				((char*)headers.val_start)[headers.val_len] = '\0';
				_state.sequenceNumber = strtol(headers.val_start, &end, 10);
				gotSequenceNumber = true;
			}
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_SEND_HEADER_SENDER) ) {
				p->sender.assign(headers.val_start, headers.val_len);
			}
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_SEND_HEADER_DESTINATION) ) {
				p->target.assign(headers.val_start, headers.val_len);
			}
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_SEND_HEADER_CONTENT_LENGTH) ) {
				char *end;
				((char*)headers.val_start)[headers.val_len] = '\0';
				contentLength = strtol(headers.val_start, &end, 10);
			}
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_SEND_HEADER_ENCODING) ) {
				p->headerContentEncoding.assign(headers.val_start, headers.val_len);
			}
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_SEND_HEADER_MIMETYPE) ) {
				p->headerContentType.assign(headers.val_start, headers.val_len);
			}
		}

		++_state.receivedMessages;

		int headerLength = headers.getptr() - frame.data.data();
		int payloadLength = frame.data.size() - headerLength;
		if ( contentLength < 0 )
			contentLength = payloadLength;
		else if ( contentLength != payloadLength ) {
			SEISCOMP_ERROR("Mismatching payload length with respect to header");
			if ( r ) *r = NetworkProtocolError;
			return false;
		}

		p->type = Packet::Data;
		if ( gotSequenceNumber )
			p->seqNo = *_state.sequenceNumber;
		p->payload.assign(headers.getptr(), payloadLength);

		if ( inboxPkt )
			queuePacket(inboxPkt);

		return true;
	}

	if ( headers.nameEquals(SCMP_PROTO_REPLY_ACK) ) {
		// Handle acknowledgement
		while ( headers.next() ) {
			if ( !headers.name_len ) break;
			if ( headers.nameEquals(SCMP_PROTO_REPLY_ACK_HEADER_SEQ_NUMBER) ) {
				char *end;
				((char*)headers.val_start)[headers.val_len] = '\0';
				int64_t ssn = strtoll(headers.val_start, &end, 10);
				if ( (errno == ERANGE && (ssn == LLONG_MAX || ssn == LLONG_MIN))
				  || (errno != 0 && ssn == 0) || (ssn < 0) ) {
					SEISCOMP_ERROR("acknowledge: not a valid sequence number: %s",
					               headers.val_start);
				}

				uint64_t sn = (uint64_t)ssn;

				_writeMutex.lock();
				_readMutex.unlock();

				if ( sn > _state.localSequenceNumber ) {
					SEISCOMP_ERROR("acknowledge: %ld > %ld", sn, _state.localSequenceNumber);
				}
				else {
					uint64_t remainingBuffers = _state.localSequenceNumber-sn;
					while ( _outbox.size() > remainingBuffers ) {
						_state.bytesBuffered -= _outbox.front()->data.size();
						_outbox.pop_front();
					}
					SEISCOMP_DEBUG("Received ack, %d remaining messages in buffer",
					               int(_outbox.size()));
				}

				_writeMutex.unlock();
				_readMutex.lock();

				break;
			}
		}
	}
	else if ( headers.nameEquals(SCMP_PROTO_REPLY_ENTER) ) {
		Packet *inboxPkt = nullptr;
		if ( !p ) {
			inboxPkt = new Packet;
			p = inboxPkt;
		}

		while ( headers.next() ) {
			if ( !headers.name_len ) break;
			if ( headers.nameEquals(SCMP_PROTO_REPLY_ENTER_HEADER_GROUP) )
				p->target.assign(headers.val_start, headers.val_len);
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_ENTER_HEADER_MEMBER) )
				p->subject.assign(headers.val_start, headers.val_len);
		}

		int headerLength = headers.getptr() - frame.data.data();
		int payloadLength = frame.data.size() - headerLength;

		p->type = Packet::Enter;
		p->payload.assign(headers.getptr(), payloadLength);

		if ( p->subject == _registeredClientName ) {
			// Register the subscribed group
			_subscriptions.insert(p->target);
		}

		if ( inboxPkt )
			queuePacket(inboxPkt);

		return true;
	}
	else if ( headers.nameEquals(SCMP_PROTO_REPLY_LEAVE) ) {
		Packet *inboxPkt = nullptr;
		if ( !p ) {
			inboxPkt = new Packet;
			p = inboxPkt;
		}

		while ( headers.next() ) {
			if ( !headers.name_len ) break;
			if ( headers.nameEquals(SCMP_PROTO_REPLY_LEAVE_HEADER_GROUP) )
				p->target.assign(headers.val_start, headers.val_len);
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_LEAVE_HEADER_MEMBER) )
				p->subject.assign(headers.val_start, headers.val_len);
		}

		p->type = Packet::Leave;

		if ( p->subject == clientName() ) {
			// Deregister the subscribed group
			set<string>::iterator it;
			it = _subscriptions.find(p->target);
			if ( it != _subscriptions.end() )
				_subscriptions.erase(it);
			else {
				SEISCOMP_WARNING("[websocket] Received leave message for group %s which we did not subscribe",
				                 p->target.c_str());
			}
		}

		if ( inboxPkt )
			queuePacket(inboxPkt);

		return true;
	}
	else if ( headers.nameEquals(SCMP_PROTO_REPLY_STATE) ) {
		Packet *inboxPkt = nullptr;
		if ( !p ) {
			inboxPkt = new Packet;
			p = inboxPkt;
		}

		int contentLength = -1;

		while ( headers.next() ) {
			// End of header section
			if ( !headers.name_len ) break;
			if ( headers.nameEquals(SCMP_PROTO_REPLY_STATE_HEADER_DESTINATION) ) {
				p->target.assign(headers.val_start, headers.val_len);
			}
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_STATE_HEADER_CLIENT) ) {
				p->subject.assign(headers.val_start, headers.val_len);
			}
			else if ( headers.nameEquals(SCMP_PROTO_REPLY_STATE_HEADER_CONTENT_LENGTH) ) {
				char *end;
				((char*)headers.val_start)[headers.val_len] = '\0';
				contentLength = strtol(headers.val_start, &end, 10);
			}
		}

		++_state.receivedMessages;

		int headerLength = headers.getptr() - frame.data.data();
		int payloadLength = frame.data.size() - headerLength;
		if ( contentLength < 0 )
			contentLength = payloadLength;
		else if ( contentLength != payloadLength ) {
			SEISCOMP_ERROR("Mismatching payload length with respect to header");
			if ( r ) *r = NetworkProtocolError;
			return false;
		}

		p->type = Packet::Status;
		p->payload.assign(headers.getptr(), payloadLength);

		if ( inboxPkt )
			queuePacket(inboxPkt);

		return true;
	}
	else if ( headers.nameEquals(SCMP_PROTO_REPLY_DISCONNECTED) ) {
		Packet *inboxPkt = nullptr;
		if ( !p ) {
			inboxPkt = new Packet;
			p = inboxPkt;
		}

		while ( headers.next() ) {
			if ( !headers.name_len ) break;
			if ( headers.nameEquals(SCMP_PROTO_REPLY_DISCONNECTED_HEADER_CLIENT) )
				p->subject.assign(headers.val_start, headers.val_len);
		}

		p->type = Packet::Disconnected;

		if ( inboxPkt )
			queuePacket(inboxPkt);

		return true;
	}
	else if ( headers.nameEquals(SCMP_PROTO_REPLY_RECEIPT) ) {
		while ( headers.next() ) {
			if ( headers.nameEquals(SCMP_PROTO_REPLY_RECEIPT_HEADER_ID) ) {
				if ( headers.val_len
				  && !_registeredClientName.compare(0, _registeredClientName.size(),
				                                    headers.val_start, headers.val_len) ) {
					SEISCOMP_DEBUG("Shutdown gracefully");
					_registeredClientName = string();
					closeSocket();
					clearInbox();
					return true;
				}
			}
		}
	}
	else if ( headers.nameEquals(SCMP_PROTO_REPLY_ERROR) ) {
		if ( r ) *r = ConnectionClosedByPeer;

		while ( headers.next() ) {
			if ( !headers.name_len ) break;
			if ( headers.nameEquals(SCMP_PROTO_REPLY_ERROR_HEADER_SEQ_NUMBER) ) {
				char *end;
				((char*)headers.val_start)[headers.val_len] = '\0';
				int64_t ssn = strtoll(headers.val_start, &end, 10);
				if ( (errno == ERANGE && (ssn == LLONG_MAX || ssn == LLONG_MIN))
				  || (errno != 0 && errno != EAGAIN && ssn == 0) || (ssn < 0) ) {
					SEISCOMP_ERROR("error: not a valid sequence number: %s (%d: %s)",
					               headers.val_start, errno, strerror(errno));
				}

				uint64_t sn = (uint64_t)ssn;

				_writeMutex.lock();
				_readMutex.unlock();

				if ( sn > _state.localSequenceNumber ) {
					SEISCOMP_ERROR("acknowledge: %ld > %ld", sn, _state.localSequenceNumber);
				}
				else {
					uint64_t remainingBuffers = _state.localSequenceNumber-sn;
					while ( _outbox.size() > remainingBuffers ) {
						_state.bytesBuffered -= _outbox.front()->data.size();
						_outbox.pop_front();
					}
					SEISCOMP_DEBUG("Received sequence number on error, %d remaining messages in buffer",
					               int(_outbox.size()));
				}

				_writeMutex.unlock();
				_readMutex.lock();

				break;
			}
		}

		string errorMessage;
		errorMessage.assign(headers.getptr(), _recvFrame.data.data()+_recvFrame.data.size()-headers.getptr());

		size_t p = errorMessage.find(' ');
		if ( p != string::npos ) {
			int code;
			if ( Core::fromString(code, errorMessage.substr(0, p)) ) {
				/*
				if ( r ) {
					switch ( code ) {
						case 411:
							*r = GroupDoesNotExist;
							break;
						default:
							break;
					}
				}
				*/

				errorMessage.erase(0, p+1);
				if ( !errorMessage.empty() ) {
					SEISCOMP_ERROR("Received peer error: %s", errorMessage.c_str());
				}
			}
		}

		closeSocket(errorMessage.c_str(), int(errorMessage.size()));
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketConnection::closeSocket(const char *errorMessage,
                                      int errorMessageLen) {
	_sockMutex.lock();
	if ( _socket ) {
		closeSocketWithoutLock(errorMessage, errorMessageLen);
	}
	_sockMutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketConnection::closeSocketWithoutLock(const char *errorMessage,
                                                 int errorMessageLen) {
	if ( !_socket->isValid() ) return;
	if ( !errorMessage )
		_errorMessage = std::string();
	else if ( errorMessageLen < 0 )
		_errorMessage = errorMessage;
	else
		_errorMessage.assign(errorMessage, (size_t)errorMessageLen);
	_socket->close();
	for ( auto &&msg : _outbox ) _backlog.push_back(msg);
	SEISCOMP_INFO("Keep %d messages in the backlog", int(_backlog.size()));
	_outbox.clear();
	_registeredClientName = string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
