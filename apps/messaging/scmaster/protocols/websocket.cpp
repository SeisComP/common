/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#define SEISCOMP_COMPONENT BROKER

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/datamodel/version.h>

#include <seiscomp/broker/protocol.h>
#include <seiscomp/broker/queue.h>
#include <seiscomp/broker/utils/utils.h>

#include "websocket.h"
#include "../settings.h"


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::Wired;


#define PROTOCOL_NAME "scmp"
#define DEFAULT_MAX_PAYLOAD_SIZE 1024*1024


namespace {


enum ErrorCodes {
	ERR_WRONG_PROTOCOL,
	ERR_REGISTER_CLIENT,
	ERR_NOT_CONNECTED,
	ERR_INVALID_COMMAND,
	ERR_INVALID_FRAME,
	ERR_INVALID_HEADERS,
	ERR_INVALID_HEADER_TOKEN,
	ERR_INVALID_HEADER_PAIR,
	ERR_INVALID_GROUP_NAME,
	ERR_INVALID_ENCODING,
	ERR_INVALID_MIME_TYPE,
	ERR_INVALID_CLIENT_NAME,
	ERR_LENGTH_MISMATCH,
	ERR_QUEUE_INCONSISTENT,
	ERR_QUEUE_DOES_NOT_EXIST,
	ERR_QUEUE_CONNECT_ERROR,
	ERR_INVALID_GROUP,
	ERR_NOT_YET_IMPLEMENTED,
	ERR_WS_MSG_PROTOCOL,
	ERR_WS_CMD_PROTOCOL,
	ERR_WS_CMD_UNKNOWN,
	ERR_NAME_TOO_LONG,
	ERR_CLIENT_INACTIVITY,
	ERR_QUEUE_ERRORS,
	ERR_QUANTITY
};


string ErrorStrings[ERR_QUANTITY + Seiscomp::Messaging::Broker::Queue::Result::Quantity] = {
	"400 Wrong websocket protocol, expected '" PROTOCOL_NAME "'",
	"401 Failed to register client",
	"402 You must connect first",
	"400 Invalid command",
	"400 Invalid frame received",
	"400 Invalid frame headers, missing newline",
	"400 Invalid header token received, only NAME expected",
	"400 Invalid header received, NAME:VALUE expected",
	"400 The group name is invalid",
	"400 The encoding is invalid",
	"400 The MIME type is invalid",
	"400 The client name is invalid",
	"400 The content length does not match with the frame",
	"400 The queue of the connect command does not match the URL path",
	"404 The requested queue does not exist",
	"500 Connection to the queue failed for some internal reason",
	"406 At least one requested group does not exist",
	"501 Not yet implemented",
	"400 Message format error",
	"400 Invalid command syntax",
	"400 Unknown command",
	"407 Clientname exceeds 128 characters",
	"408 Inactivity",
	/* Start of queue error strings */
	"200 QUEUE_OK", // This should never happen
	"500 Internal queue error",
	"408 Client name not unique",
	"409 Client not accepted",
	"410 Group name not unique",
	"411 Group does not exist",
	"412 Message not accepted",
	"500 Not enough client heap"
};


std::string SchemaVersion = Seiscomp::Core::Version(
	Seiscomp::DataModel::Version::Major,
	Seiscomp::DataModel::Version::Minor).toString();


const std::string &str(int code) {
	return ErrorStrings[code];
}


struct FrameHeaderName {
	FrameHeaderName(const char *s) : name(s) {}
	const char *name;
};


struct FrameHeaderValue {
	FrameHeaderValue(const char *s) : value(s) {}
	const char *value;
};


struct FrameHeaders {
	typedef char CH;

	FrameHeaders(CH *src, size_t l)
	: _source(src), _source_len(l)
	, _numberOfHeaders(0) {}

	bool next() {
		size_t len;
		CH *data = const_cast<CH*>(tokenize2(_source, "\n", _source_len, len));

		if ( data ) {
			trim(data, len);

			name_start = data;

			CH *sep = strnchr(data, len, ':');
			if ( sep ) {
				name_len = static_cast<size_t>(sep - data);
				val_start = sep + 1;
				val_len = len - name_len - 1;
				trimBack(name_start, name_len);
				trimFront(val_start, val_len);
			}
			else {
				name_len = len;
				trimBack(name_start, name_len);
				val_start = nullptr;
				val_len = 0;
			}

			++_numberOfHeaders;
			return true;
		}

		return false;
	}

	bool nameEquals(const char *s) const {
		return !strncmp(s, name_start, name_len);
	}

	bool valueEquals(const char *s) const {
		return !strncmp(s, val_start, val_len);
	}

	bool operator==(const FrameHeaderName &wrapper) const {
		return nameEquals(wrapper.name);
	}

	bool operator==(const FrameHeaderValue &wrapper) const {
		return valueEquals(wrapper.value);
	}

	bool empty() const {
		return _numberOfHeaders == 0;
	}

	CH *getptr() const {
		return _source;
	}

	CH     *_source;
	size_t  _source_len;
	size_t  _numberOfHeaders;

	CH     *name_start;
	size_t  name_len;
	CH     *val_start;
	size_t  val_len;
};


}


typedef Seiscomp::Core::ContainerSink<string> string_sink;
typedef boost::iostreams::stream<string_sink> osstream;


namespace Seiscomp {
namespace Messaging {
namespace Protocols {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WebsocketSession::WebsocketSession(Socket *sock, Broker::Server *server)
: HttpSession(sock, server) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::update() {
	// Reset sent bytes in one turn
	_bytesSent = 0;
	HttpSession::update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::handleHeader(const char * /*name*/, size_t /*nlen*/,
                                    const char * /*value*/, size_t /*vlen*/) {
	// Possibly later checking for custom headers such as username and
	// others. This can also be passed via GET parameters. Simply too
	// many options ...
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketSession::handleGETRequest(HttpRequest &req) {
	// We only support web sockets
	if ( req.upgrade )
		return handleWSUpgrade(req);

	return HttpSession::handleGETRequest(req);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketSession::handleWSUpgrade(HttpRequest &req) {
	if ( req.upgradeTo != "websocket" ) {
		sendStatus(HTTP_426);
		return true;
	}

	if ( req.secWebsocketVersion != 13 ) {
		SEISCOMP_ERROR("Invalid websocket version: %d", req.secWebsocketVersion);
		sendStatus(HTTP_400, "Invalid websocket version");
		return true;
	}

	if ( !req.path.compare(0, global.http.brokerPath.size(), global.http.brokerPath) ) {
		// No support for protocols currently
		if ( req.secWebsocketProtocol != PROTOCOL_NAME ) {
			sendStatus(HTTP_400, str(ERR_WRONG_PROTOCOL));
			return true;
		}

		_requestQueue = req.path.substr(global.http.brokerPath.size());

		upgradeToWebsocket(req, PROTOCOL_NAME, DEFAULT_MAX_PAYLOAD_SIZE);
		return true;
	}

	sendStatus(HTTP_404);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::close() {
	if ( _queue ) {
		SEISCOMP_DEBUG("%s: disconnect on close", name().c_str());
		_queue->disconnect(this);
		_continueWithSeqNo = Core::None;
	}

	HttpSession::close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::buffersFlushed() {
	// If we have sent less than 4kb this turn keep on sending otherwise
	// return control to other sessions to not starve slow clients
	if ( _bytesSent < 4096 )
		outboxFlushed();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::outboxFlushed() {
	Broker::Message *msg;

	if ( !_continueWithSeqNo )
		return;

	// If there aren't any more messages, allow real-time
	msg = _queue->getMessage(*_continueWithSeqNo, this);
	if ( msg ) {
		--_messageBacklog;
		_continueWithSeqNo = msg->sequenceNumber+1;
		sendMessage(msg);
	}
	else
		_continueWithSeqNo = Core::None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::replyWithError(const char *msg, int len) {
	if ( _queue ) {
		SEISCOMP_DEBUG("%s: disconnect on error: %s", name().c_str(),
		               string(msg, msg+len).c_str());
		_queue->disconnect(this);
		_continueWithSeqNo = Core::None;
	}

	device()->removeMode(Device::Read);
	BufferPtr resp = new Buffer;

	{
		osstream os(resp->data);
		os << SCMP_PROTO_REPLY_ERROR "\n"
		      SCMP_PROTO_REPLY_ERROR_HEADER_SEQ_NUMBER ":" << _sequenceNumber << "\n\n";
	}

	if ( msg )
		resp->data.append(msg, msg+len);

	Websocket::Frame::finalizeBuffer(resp.get(), Websocket::Frame::TextFrame);
	send(resp.get());

	resp = new Buffer;
	Websocket::Frame::finalizeBuffer(resp.get(),
	                                 Websocket::Frame::ConnectionClose,
	                                 Websocket::CloseProtocolError);
	send(resp.get());

	invalidate();
	_request.state = HttpRequest::FINISHED;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::replyWithError(const std::string &msg) {
	replyWithError(msg.data(), static_cast<int>(msg.size()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::handleWebsocketFrame(Websocket::Frame &frame) {
	if ( frame.type == Websocket::Frame::ConnectionClose ) {
		SEISCOMP_DEBUG("[websocket] Received close request");
		replyWithError(nullptr, 0);
		return;
	}

	if ( !frame.finalFragment ) {
		SEISCOMP_WARNING("[websocket] Unhandled intermediate frame %d", frame.type);
		return;
	}

	switch ( frame.type ) {
		case Websocket::Frame::Ping:
		{
			// We could create a static buffer with an empty
			// pong frame that can be reused when an empty ping is
			// received. I haven't yet figured out how this ping-pong
			// is used in the wild.
			BufferPtr pongFrame = new Buffer;

			// Copy application data from the ping frame
			pongFrame->data.swap(frame.data);

			Websocket::Frame::finalizeBuffer(pongFrame.get(),
			                                 Websocket::Frame::Pong,
			                                 Websocket::NoStatus);

			send(pongFrame.get());
			break;
		}

		case Websocket::Frame::TextFrame:
		case Websocket::Frame::BinaryFrame:
		{
			// Skip empty frames
			if ( frame.data.empty() ) {
				SEISCOMP_WARNING("Empty frame received");
				return;
			}

			handleFrame(&frame.data[0], static_cast<int>(frame.data.size()));
			break;
		}

		default:
			SEISCOMP_WARNING("[websocket] Unhandled frame type %d", frame.type);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::handleFrame(char *data, int data_len) {
	char *content = data;
	size_t content_len = static_cast<size_t>(data_len);
	trimFront(content, content_len);

	// Tell the session to give control to others to not starve
	// slow readers
	finishReading();

	if ( content_len == 0 ) {
		SEISCOMP_WARNING("Empty frame received");
		return;
	}

	/*********************************************************
	 This handles a client frame. See protocol.md for more
	 information. The command is always passed as first line.
	 Actually the commands are:

	 - CONNECT
	 - DISCONNECT
	 - SUBSCRIBE
	 - UNSUBSCRIBE
	 - SEND

	So checking the first charater being one of [C,D,S,U] is
	sufficient to filter invalid commands.
	 *********************************************************/

	if ( !strchr(SCMP_PROTO_CMD_FIRST_CHARS, content[0]) ) {
		replyWithError(str(ERR_INVALID_COMMAND));
		return;
	}

	char *eol = strnchr(content + 1, content_len - 1, '\n');
	if ( !eol ) {
		replyWithError(str(ERR_INVALID_FRAME));
		return;
	}

	size_t line_length = static_cast<size_t>(eol - content);
	int block_length = static_cast<int>(content_len - line_length - 1);

	if ( !strncmp(content, SCMP_PROTO_CMD_CONNECT, line_length) )
		commandCONNECT(eol + 1, block_length);
	else if ( !strncmp(content, SCMP_PROTO_CMD_DISCONNECT, line_length) )
		commandDISCONNECT(eol + 1, block_length);
	else if ( !strncmp(content, SCMP_PROTO_CMD_SUBSCRIBE, line_length) )
		commandSUBSCRIBE(eol + 1, block_length);
	else if ( !strncmp(content, SCMP_PROTO_CMD_UNSUBSCRIBE, line_length) )
		commandUNSUBSCRIBE(eol + 1, block_length);
	else if ( !strncmp(content, SCMP_PROTO_CMD_SEND, line_length) )
		commandSEND(eol + 1, block_length);
	else if ( !strncmp(content, SCMP_PROTO_CMD_STATE, line_length) )
		commandSTATE(eol + 1, block_length, true);
	else {
		replyWithError(str(ERR_INVALID_COMMAND));
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::commandCONNECT(char *frame, int len) {
	Broker::Queue::KeyValueCStrPair inParams[Broker::Queue::MaxAdditionalParams];
	int inParamCount = 0;
	Broker::Queue *queue = nullptr;
	OPT(Broker::SequenceNumber) seqNo;
	const char *groupList = nullptr;
	size_t groupListLen = 0;
	Broker::SequenceNumber ackWindow = 20;
	Broker::Queue::KeyValues outParams;

	if ( !_requestQueue.empty() ) {
		queue = _server->getQueue(_requestQueue,
		                          this, static_cast<Wired::Socket*>(device())->address());
		if ( !queue ) {
			replyWithError(str(ERR_QUEUE_DOES_NOT_EXIST));
			return;
		}
	}

	FrameHeaders headers(frame, static_cast<size_t>(len));
	while ( headers.next() ) {
		if ( !headers.name_len ) break;
		if ( !headers.val_len ) {
			replyWithError(str(ERR_INVALID_HEADER_PAIR));
			return;
		}

		headers.name_start[headers.name_len] = '\0';
		headers.val_start[headers.val_len] = '\0';

		if ( headers.nameEquals(SCMP_PROTO_CMD_CONNECT_HEADER_QUEUE) ) {
			if ( queue ) {
				if ( !headers.valueEquals(_requestQueue.c_str()) ) {
					replyWithError(str(ERR_INVALID_HEADER_PAIR));
					return;
				}
			}
			else {
				queue = _server->getQueue(string(headers.val_start, headers.val_len),
				                          this, static_cast<Wired::Socket*>(device())->address());
				if ( !queue ) {
					replyWithError(str(ERR_QUEUE_DOES_NOT_EXIST));
					return;
				}
			}
		}
		else if ( headers.nameEquals(SCMP_PROTO_CMD_CONNECT_HEADER_CLIENT_NAME) ) {
			if ( headers.val_len > 128 ) {
				replyWithError(str(ERR_NAME_TOO_LONG));
				return;
			}

			_name.assign(headers.val_start, headers.val_len);
		}
		else if ( headers.nameEquals(SCMP_PROTO_CMD_CONNECT_HEADER_MEMBERSHIP_INFO) ) {
			if ( !strncmp(headers.val_start, "1", headers.val_len) ||
			     !strncmp(headers.val_start, "true", headers.val_len) )
				setMembershipInformationEnabled(true);
		}
		else if ( headers.nameEquals(SCMP_PROTO_CMD_CONNECT_HEADER_SELF_DISCARD) ) {
			if ( !strncmp(headers.val_start, "1", headers.val_len) ||
			     !strncmp(headers.val_start, "true", headers.val_len) )
				setDiscardSelf(true);
			else
				setDiscardSelf(false);
		}
		else if ( headers.nameEquals(SCMP_PROTO_CMD_CONNECT_HEADER_ACK_WINDOW) ) {
			char *end;
			headers.val_start[headers.val_len] = '\0';
			ackWindow = static_cast<Broker::SequenceNumber>(strtol(headers.val_start, &end, 10));
		}
		else if ( headers.nameEquals(SCMP_PROTO_CMD_CONNECT_HEADER_SEQ_NUMBER) ) {
			char *end;
			headers.val_start[headers.val_len] = '\0';
			seqNo = static_cast<Broker::SequenceNumber>(strtol(headers.val_start, &end, 10));
		}
		else if ( headers.nameEquals(SCMP_PROTO_CMD_CONNECT_HEADER_SUBSCRIPTIONS) ) {
			// Handle subscription requests
			groupList = headers.val_start;
			groupListLen = headers.val_len;
		}
		else if ( inParamCount < Broker::Queue::MaxAdditionalParams ) {
			// Populate additional parameters interpreted by plugins
			inParams[inParamCount].first = headers.name_start;
			inParams[inParamCount].second = headers.val_start;
			++inParamCount;
		}
		else {
			SEISCOMP_WARNING("Maximum additional header count exceeded with: %s",
			                 headers.name_start);
		}
	}

	if ( headers.empty() ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		return;
	}

	Broker::Queue::Result r = queue->connect(this, inParams, inParamCount, outParams);
	if ( r ) {
		replyWithError(str(ERR_QUEUE_ERRORS + r));
		return;
	}

	_websocketFrame->setMaxPayloadSize(_queue->maxPayloadSize());
	setAcknowledgeWindow(ackWindow);

	// Send the first welcome message. Actually that should be done
	// by the queue during connect!
	BufferPtr welcomeBuffer = new Buffer;
	const Broker::Queue::StringList &availGroups = _queue->groups();

	{
		osstream os(welcomeBuffer->data);

		os << SCMP_PROTO_REPLY_CONNECT"\n"
		   << SCMP_PROTO_REPLY_CONNECT_HEADER_QUEUE ":" << queue->name() << "\n"
		   << SCMP_PROTO_REPLY_CONNECT_HEADER_VERSION ":1.0\n"
		   << SCMP_PROTO_REPLY_CONNECT_HEADER_SCHEMA_VERSION ":" << SchemaVersion << "\n"
		   << SCMP_PROTO_REPLY_CONNECT_HEADER_CLIENT_NAME ":" << name() << "\n"
		   << SCMP_PROTO_REPLY_CONNECT_HEADER_ACK_WINDOW ":" << ackWindow << "\n"
		   << SCMP_PROTO_REPLY_CONNECT_HEADER_GROUPS ":";

		for ( size_t i = 0; i < availGroups.size(); ++i ) {
			if ( i ) os << ",";
			os << availGroups[i];
		}

		os << "\n";

		size_t i = 0;
		for ( ; i < outParams.size(); ++i )
			os << outParams[i].first << ":" << outParams[i].second << "\n";

		os << "\n";
	}

	Websocket::Frame::finalizeBuffer(welcomeBuffer.get(), Websocket::Frame::TextFrame);
	send(welcomeBuffer.get());

	// Collect processors

	_bytesSent += welcomeBuffer->header.size();
	_bytesSent += welcomeBuffer->data.size();

	if ( _queue ) {
		if ( groupList ) {
			size_t group_len;
			const char *group;
			while ( (group = tokenize2(groupList, ",", groupListLen, group_len)) ) {
				trim(group, group_len);
				if ( group_len == 0 ) continue;
				string groupName(group, group_len);
				Broker::Queue::Result r = _queue->subscribe(this, groupName);
				if ( r) {
					replyWithError(str(ERR_QUEUE_ERRORS + r));
					return;
				}
			}
		}

		if ( seqNo )
			_continueWithSeqNo = *seqNo+1;

		setTag(true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::commandDISCONNECT(char *frame, int len) {
	if ( !_queue ) {
		replyWithError(str(ERR_NOT_CONNECTED));
		return;
	}

	FrameHeaders headers(frame, static_cast<size_t>(len));
	const char *receipt = nullptr;
	size_t receipt_len = 0;

	while ( headers.next() ) {
		if ( !headers.name_len ) break;
		if ( !headers.val_len ) {
			replyWithError(str(ERR_INVALID_HEADER_PAIR));
			return;
		}

		if ( headers.nameEquals(SCMP_PROTO_CMD_DISCONNECT_HEADER_RECEIPT) ) {
			receipt = headers.val_start;
			receipt_len = headers.val_len;
		}
	}

	if ( headers.empty() ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		return;
	}

	SEISCOMP_DEBUG("%s requests disconnect", name().c_str());
	_queue->disconnect(this);
	_continueWithSeqNo = Core::None;

	BufferPtr resp = new Buffer;
	resp->data  = SCMP_PROTO_REPLY_RECEIPT "\n";
	if ( receipt ) {
		resp->data += SCMP_PROTO_REPLY_RECEIPT_HEADER_ID ":";
		resp->data.append(receipt, receipt_len);
		resp->data += '\n';
	}
	resp->data += '\n';

	Websocket::Frame::finalizeBuffer(resp.get(), Websocket::Frame::TextFrame);
	send(resp.get());

	resp = new Buffer;
	Websocket::Frame::finalizeBuffer(resp.get(),
	                                 Websocket::Frame::ConnectionClose,
	                                 Websocket::CloseNormal);
	send(resp.get());

	invalidate();
	_request.state = HttpRequest::FINISHED;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::commandSUBSCRIBE(char *frame, int len) {
	if ( !_queue ) {
		replyWithError(str(ERR_NOT_CONNECTED));
		return;
	}

	FrameHeaders headers(frame, static_cast<size_t>(len));
	const char *groupList = nullptr;
	size_t groupListLen = 0;

	while ( headers.next() ) {
		if ( !headers.name_len ) break;
		if ( !headers.val_len ) {
			replyWithError(str(ERR_INVALID_HEADER_PAIR));
			return;
		}

		if ( headers.nameEquals(SCMP_PROTO_CMD_SUBSCRIBE_HEADER_GROUPS) ) {
			// Handle subscription requests
			groupList = headers.val_start;
			groupListLen = headers.val_len;
		}
	}

	if ( headers.empty() ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		return;
	}

	if ( !groupList ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		return;
	}

	size_t group_len;
	const char *group;
	while ( (group = tokenize2(groupList, ",", groupListLen, group_len)) ) {
		trim(group, group_len);
		if ( group_len == 0 ) continue;
		string groupName(group, group_len);
		Broker::Queue::Result r = _queue->subscribe(this, groupName);
		if ( r ) {
			replyWithError(str(ERR_QUEUE_ERRORS + r));
			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::commandUNSUBSCRIBE(char *frame, int len) {
	if ( !_queue ) {
		replyWithError(str(ERR_NOT_CONNECTED));
		return;
	}

	FrameHeaders headers(frame, static_cast<size_t>(len));
	const char *groupList = nullptr;
	size_t groupListLen = 0;

	while ( headers.next() ) {
		if ( !headers.name_len ) break;
		if ( !headers.val_len ) {
			replyWithError(str(ERR_INVALID_HEADER_PAIR));
			return;
		}

		if ( headers.nameEquals(SCMP_PROTO_CMD_UNSUBSCRIBE_HEADER_GROUPS) ) {
			// Handle subscription requests
			groupList = headers.val_start;
			groupListLen = headers.val_len;
		}
	}

	if ( headers.empty() ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		return;
	}

	if ( !groupList ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		return;
	}

	size_t group_len;
	const char *group;
	while ( (group = tokenize2(groupList, ",", groupListLen, group_len)) ) {
		trim(group, group_len);
		if ( group_len == 0 ) continue;
		string groupName(group, group_len);
		Broker::Queue::Result r = _queue->unsubscribe(this, groupName);
		if ( r ) {
			replyWithError(str(ERR_QUEUE_ERRORS + r));
			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::commandSEND(char *frame, int len) {
	if ( !_queue ) {
		replyWithError(str(ERR_NOT_CONNECTED));
		return;
	}

	Broker::Message *msg = nullptr;
	FrameHeaders headers(frame, static_cast<size_t>(len));
	int contentLength = -1;

	while ( headers.next() ) {
		if ( !headers.name_len ) break;
		if ( !headers.val_len ) {
			replyWithError(str(ERR_INVALID_HEADER_PAIR));
			if ( msg ) delete msg;
			return;
		}

		// Destination
		if ( headers.nameEquals(SCMP_PROTO_CMD_SEND_HEADER_DESTINATION) ) {
			if ( headers.val_len > 128
			  || headers.val_len < 1 ) {
				replyWithError(str(ERR_INVALID_GROUP_NAME));
				if ( msg ) delete msg;
				return;
			}

			if ( !msg ) msg = new Broker::Message;
			msg->target.assign(headers.val_start, headers.val_len);
		}
		// Content length
		else if ( headers.nameEquals(SCMP_PROTO_CMD_SEND_HEADER_CONTENT_LENGTH) ) {
			char *end;
			headers.val_start[headers.val_len] = '\0';
			contentLength = static_cast<int>(strtol(headers.val_start, &end, 10));
		}
		// Content encoding
		else if ( headers.nameEquals(SCMP_PROTO_CMD_SEND_HEADER_ENCODING) ) {
			if ( headers.val_len > 128
			  || headers.val_len < 1 ) {
				replyWithError(str(ERR_INVALID_ENCODING));
				if ( msg ) delete msg;
				return;
			}

			if ( !msg ) msg = new Broker::Message;
			msg->encoding.assign(headers.val_start, headers.val_len);
		}
		// Content type
		else if ( headers.nameEquals(SCMP_PROTO_CMD_SEND_HEADER_MIMETYPE) ) {
			if ( headers.val_len > 128
			  || headers.val_len < 1 ) {
				replyWithError(str(ERR_INVALID_MIME_TYPE));
				if ( msg ) delete msg;
				return;
			}

			if ( !msg ) msg = new Broker::Message;
			msg->mimeType.assign(headers.val_start, headers.val_len);
		}
		// Transient?
		else if ( headers.nameEquals(SCMP_PROTO_CMD_SEND_HEADER_TRANSIENT) ) {
			if ( headers.val_len > 0 ) {
				replyWithError(str(ERR_INVALID_HEADER_TOKEN));
				if ( msg ) delete msg;
				return;
			}

			if ( !msg ) msg = new Broker::Message;
			msg->type = Broker::Message::Type::Transient;
		}
	}

	if ( !msg ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		return;
	}

	if ( headers.empty() ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		delete msg;
		return;
	}

	if ( msg->type == Broker::Message::Type::Unspecified )
		msg->type = Broker::Message::Type::Regular;

	size_t headerLength = static_cast<size_t>(headers.getptr() - frame);
	size_t payloadLength = static_cast<size_t>(len) - headerLength;
	if ( contentLength < 0 )
		contentLength = static_cast<int>(payloadLength);
	else if ( contentLength != static_cast<int>(payloadLength) ) {
		replyWithError(str(ERR_LENGTH_MISMATCH));
		delete msg;
		return;
	}

	msg->payload.assign(headers.getptr(), payloadLength);

	if ( msg->target.empty() ) {
		replyWithError(str(ERR_INVALID_FRAME));
		delete msg;
		return;
	}

	Broker::Queue::Result r = _queue->push(this, msg, len);
	if ( r != Broker::Queue::Success ) {
		delete msg;
		replyWithError(str(ERR_QUEUE_ERRORS + r));
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::commandSTATE(char *frame, int len, bool) {
	if ( !_queue ) {
		replyWithError(str(ERR_NOT_CONNECTED));
		return;
	}

	Broker::Message *msg = nullptr;
	FrameHeaders headers(frame, static_cast<size_t>(len));
	int contentLength = -1;

	while ( headers.next() ) {
		if ( !headers.name_len ) break;
		if ( !headers.val_len ) {
			replyWithError(str(ERR_INVALID_HEADER_PAIR));
			if ( msg ) delete msg;
			return;
		}

		// Destination
		if ( headers.nameEquals(SCMP_PROTO_CMD_STATE_HEADER_DESTINATION) ) {
			if ( headers.val_len > 128
			  || headers.val_len < 1 ) {
				replyWithError(str(ERR_INVALID_GROUP_NAME));
				if ( msg ) delete msg;
				return;
			}

			if ( !msg ) msg = new Broker::Message;
			msg->target.assign(headers.val_start, headers.val_len);
		}
		// Content length
		else if ( headers.nameEquals(SCMP_PROTO_CMD_STATE_HEADER_CONTENT_LENGTH) ) {
			char *end;
			headers.val_start[headers.val_len] = '\0';
			contentLength = static_cast<int>(strtol(headers.val_start, &end, 10));
		}
	}

	if ( !msg ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		return;
	}

	if ( headers.empty() ) {
		replyWithError(str(ERR_INVALID_HEADERS));
		delete msg;
		return;
	}

	msg->type = Broker::Message::Type::Status;

	size_t headerLength = static_cast<size_t>(headers.getptr() - frame);
	size_t payloadLength = static_cast<size_t>(len) - headerLength;
	if ( contentLength < 0 )
		contentLength = static_cast<int>(payloadLength);
	else if ( contentLength != static_cast<int>(payloadLength) ) {
		replyWithError(str(ERR_LENGTH_MISMATCH));
		delete msg;
		return;
	}

	msg->payload.assign(headers.getptr(), payloadLength);

	Broker::Queue::Result r = _queue->push(this, msg, len);
	if ( r != Broker::Queue::Success ) {
		delete msg;
		replyWithError(str(ERR_QUEUE_ERRORS + r));
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::IPAddress WebsocketSession::IPAddress() const {
	static Socket::IPAddress NULL_IP(uint32_t(0));
	Device *dev = device();
	if ( !dev )
		return NULL_IP;

	return static_cast<Socket*>(dev)->address();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t WebsocketSession::publish(Broker::Client *sender, Broker::Message *msg) {
	if (_request.state == HttpRequest::FINISHED )
		return 0;

	if ( discardSelf() && msg->selfDiscard && this == sender )
		return 0;

	if ( msg->sequenceNumber != INVALID_SEQUENCE_NUMBER ) {
		if ( _continueWithSeqNo ) {
			++_messageBacklog;
			return 0;
		}
		else if ( inAvail() ) {
			// Remember that there are new messages queued for us. We don't
			// queue more messages if our output buffer still contains some
			// bytes
			_continueWithSeqNo = msg->sequenceNumber;
			++_messageBacklog;
			return 0;
		}
	}

	return sendMessage(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::enter(const Broker::Group *group, const Client *newMember,
                             Broker::Message *msg) {
	if ( newMember == this ) {
		// Create a copy of the message and add a list of the current
		// members to the message
		Broker::Message tmpMsg;
		tmpMsg.encodingWebSocket = new Buffer;
		tmpMsg.encodingWebSocket->data  = SCMP_PROTO_REPLY_ENTER "\n"
		                                  SCMP_PROTO_REPLY_ENTER_HEADER_GROUP ": ";
		tmpMsg.encodingWebSocket->data += msg->target;
		tmpMsg.encodingWebSocket->data += "\n";
		tmpMsg.encodingWebSocket->data += SCMP_PROTO_REPLY_ENTER_HEADER_MEMBER ": ";
		tmpMsg.encodingWebSocket->data += newMember->name();
		tmpMsg.encodingWebSocket->data += "\n\n";

		if ( wantsMembershipInformation() ) {
			Broker::Group::Members::iterator mit;
			int idx = 0;
			for ( mit = group->members().begin(); mit != group->members().end(); ++mit, ++idx ) {
				if ( idx ) tmpMsg.encodingWebSocket->data += ',';
				tmpMsg.encodingWebSocket->data += (*mit)->name();
			}
		}

		Websocket::Frame::finalizeBuffer(tmpMsg.encodingWebSocket.get(), Websocket::Frame::TextFrame);
		sendMessage(&tmpMsg);
	}
	else if ( !msg->encodingWebSocket ) {
		msg->encodingWebSocket = new Buffer;
		msg->encodingWebSocket->data  = SCMP_PROTO_REPLY_ENTER "\n"
		                                SCMP_PROTO_REPLY_ENTER_HEADER_GROUP ": ";
		msg->encodingWebSocket->data += msg->target;
		msg->encodingWebSocket->data += "\n";
		msg->encodingWebSocket->data += SCMP_PROTO_REPLY_ENTER_HEADER_MEMBER ": ";
		msg->encodingWebSocket->data += newMember->name();
		msg->encodingWebSocket->data += "\n\n";
		Websocket::Frame::finalizeBuffer(msg->encodingWebSocket.get(), Websocket::Frame::TextFrame);
		sendMessage(msg);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::leave(const Broker::Group *, const Client *newMember,
                             Broker::Message *msg) {
	if ( !msg->encodingWebSocket ) {
		msg->encodingWebSocket = new Buffer;
		msg->encodingWebSocket->data  = SCMP_PROTO_REPLY_LEAVE "\n"
		                                SCMP_PROTO_REPLY_LEAVE_HEADER_GROUP ": ";
		msg->encodingWebSocket->data += msg->target;
		msg->encodingWebSocket->data += "\n";
		msg->encodingWebSocket->data += SCMP_PROTO_REPLY_LEAVE_HEADER_MEMBER ": ";
		msg->encodingWebSocket->data += newMember->name();
		msg->encodingWebSocket->data += "\n\n";
		Websocket::Frame::finalizeBuffer(msg->encodingWebSocket.get(), Websocket::Frame::TextFrame);
	}

	sendMessage(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::disconnected(const Client *disconnectedClient,
                                    Broker::Message *msg) {
	if ( !msg->encodingWebSocket ) {
		msg->encodingWebSocket = new Buffer;
		msg->encodingWebSocket->data  = SCMP_PROTO_REPLY_DISCONNECTED "\n"
		                                SCMP_PROTO_REPLY_DISCONNECTED_HEADER_CLIENT ": ";
		msg->encodingWebSocket->data += disconnectedClient->name();
		msg->encodingWebSocket->data += "\n\n";
		Websocket::Frame::finalizeBuffer(msg->encodingWebSocket.get(), Websocket::Frame::TextFrame);
	}

	sendMessage(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::ack() {
	Broker::Message msg;
	msg.encodingWebSocket = new Buffer;

	{
		osstream os(msg.encodingWebSocket->data);
		os << SCMP_PROTO_REPLY_ACK "\n"
		      SCMP_PROTO_REPLY_ACK_HEADER_SEQ_NUMBER ":" << _sequenceNumber << "\n\n";
	}

	Websocket::Frame::finalizeBuffer(msg.encodingWebSocket.get(), Websocket::Frame::TextFrame);
	sendMessage(&msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::dispose() {
	replyWithError(str(ERR_CLIENT_INACTIVITY));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t WebsocketSession::sendMessage(Broker::Message *msg) {
	if ( !_upgradedToWebsocket ) return 0;

	if ( !msg->encodingWebSocket ) {
		// Create new buffer. Other session will reuse it and send
		// it without further encoding
		msg->encodingWebSocket = new Buffer;

		// The default frame type is binary
		Websocket::Frame::Type frameType = Websocket::Frame::BinaryFrame;

		if ( msg->type != Broker::Message::Type::Status ) {
			osstream os(msg->encodingWebSocket->data);
			bool identity = true;
			os << SCMP_PROTO_REPLY_SEND "\n"
			   << SCMP_PROTO_REPLY_SEND_HEADER_SENDER ":" << msg->sender << "\n"
			   << SCMP_PROTO_REPLY_SEND_HEADER_DESTINATION ":" << msg->target << "\n";
			if ( msg->sequenceNumber != INVALID_SEQUENCE_NUMBER )
			   os << SCMP_PROTO_REPLY_SEND_HEADER_SEQ_NUMBER ":" << msg->sequenceNumber << "\n";
			os << SCMP_PROTO_REPLY_SEND_HEADER_CONTENT_LENGTH ":" << msg->payload.size() << "\n";
			if ( !msg->encoding.empty() ) {
				os << SCMP_PROTO_REPLY_SEND_HEADER_ENCODING ":" << msg->encoding << "\n";
				if ( msg->encoding != "identity" )
					identity = false;
			}
			if ( !msg->mimeType.empty() ) {
				os << SCMP_PROTO_REPLY_SEND_HEADER_MIMETYPE ":" << msg->mimeType << "\n";
				if ( identity ) {
					// Get the main content type
					size_t p = msg->mimeType.find('/');
					if ( p == string::npos ) {
						if ( msg->mimeType == "text" )
							frameType = Websocket::Frame::TextFrame;
					}
					else if ( msg->mimeType.compare(0, p, "text") == 0 )
						frameType = Websocket::Frame::TextFrame;
				}
			}
			os << "\n"
			   << msg->payload;
		}
		else {
			frameType = Websocket::Frame::TextFrame;
			osstream os(msg->encodingWebSocket->data);
			os << SCMP_PROTO_REPLY_STATE "\n"
			   << SCMP_PROTO_REPLY_STATE_HEADER_DESTINATION ":" << msg->target << "\n"
			   << SCMP_PROTO_REPLY_STATE_HEADER_CLIENT ":" << msg->sender << "\n"
			   << SCMP_PROTO_REPLY_STATE_HEADER_CONTENT_LENGTH ":" << msg->payload.size() << "\n";
			os << "\n"
			   << msg->payload;
		}

		Websocket::Frame::finalizeBuffer(msg->encodingWebSocket.get(), frameType);
	}

	send(msg->encodingWebSocket.get());

	size_t frameLength = msg->encodingWebSocket->data.size();
	frameLength += msg->encodingWebSocket->header.size();
	_bytesSent += frameLength;

	/*
	if ( inAvail() > 1024 )
		std::cerr << "Buffer watermark exceeded with " << inAvail() << std::endl;
	*/

	// Clear payload and object as it is not required anymore. We do not
	// transcode and do not support multiple protocols.
	msg->payload = std::string();
	msg->object = nullptr;

	/*
	SEISCOMP_DEBUG("- message from %s/%s to %s",
	               _queue->name().c_str(), msg->target.c_str(), name().c_str());
	*/

	return frameLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
