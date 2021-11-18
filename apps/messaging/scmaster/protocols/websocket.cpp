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
#include <seiscomp/utils/base64.h>

#include <seiscomp/broker/protocol.h>
#include <seiscomp/broker/queue.h>
#include <seiscomp/broker/utils/utils.h>

#include "websocket.h"
#include "handler/broker.h"
#include "handler/db.h"

#include "../settings.h"


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::Wired;


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
	if ( _handler ) {
		_handler->start();
	}

	HttpSession::update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::handleHeader(const char * name, size_t nlen,
                                    const char * value, size_t vlen) {
	// Possibly later checking for custom headers such as username and
	// others. This can also be passed via GET parameters. Simply too
	// many options ...
	if ( !strncasecmp("Expect", name, nlen) ) {
		if ( !strncasecmp("100-continue", value, vlen) ) {
			send("HTTP/1.1 100 Continue\r\n\r\n", 25);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::upgradeToWebsocket(const char *protocol,
                                          WebsocketHandler *handler,
                                          uint64_t maxPayloadSize) {
	_request.status = HTTP_101;
	_request.state = HttpRequest::WAITING;
	_request.keepAlive = true;
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
	string key = _request.secWebsocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	char sha1[SHA_DIGEST_LENGTH]; // == 20
	SHA1(reinterpret_cast<const unsigned char*>(key.data()),
	     key.size(), reinterpret_cast<unsigned char*>(sha1));
	key.clear();
	Util::encodeBase64(key, sha1, SHA_DIGEST_LENGTH);
	send(key.data(), key.size());
	send("\r\n");
	handler->addUpgradeHeader();
	send("\r\n");
	flush();

	_upgradedToWebsocket = true;
	if ( !_websocketFrame )
		_websocketFrame = new Websocket::Frame;
	else
		_websocketFrame->reset();
	_websocketFrame->setMaxPayloadSize(maxPayloadSize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketSession::handleBrokerUpgrade(const string &requestQueue) {
	if ( _request.upgradeTo != "websocket" ) {
		sendStatus(HTTP_426);
		return true;
	}

	if ( _request.secWebsocketVersion != 13 ) {
		SEISCOMP_ERROR("Invalid websocket version: %d", _request.secWebsocketVersion);
		sendStatus(HTTP_400, "Invalid websocket version");
		return true;
	}

	_handler = new BrokerHandler(this, requestQueue);
	upgradeToWebsocket(BrokerHandler::PROTOCOL_NAME, _handler.get(),
	                   BrokerHandler::DEFAULT_MAX_PAYLOAD_SIZE);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WebsocketSession::handleDatabaseUpgrade(const string &dbURL) {
	if ( _request.upgradeTo != "websocket" ) {
		sendStatus(HTTP_426);
		return true;
	}

	if ( _request.secWebsocketVersion != 13 ) {
		SEISCOMP_ERROR("Invalid websocket version: %d", _request.secWebsocketVersion);
		sendStatus(HTTP_400, "Invalid websocket version");
		return true;
	}

	Seiscomp::IO::DatabaseInterfacePtr db = Seiscomp::IO::DatabaseInterface::Open(dbURL.c_str());
	if ( !db ) {
		sendStatus(HTTP_503);
		return true;
	}

	_handler = new DBHandler(this, db);
	upgradeToWebsocket(DBHandler::PROTOCOL_NAME, _handler.get(),
	                   DBHandler::DEFAULT_MAX_PAYLOAD_SIZE);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::close() {
	_handler = nullptr;
	HttpSession::close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::buffersFlushed() {
	if ( _handler ) {
		_handler->buffersFlushed();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::outboxFlushed() {
	HttpSession::outboxFlushed();

	if ( _handler ) {
		_handler->outboxFlushed();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::requestFinished() {
	HttpSession::requestFinished();
	_handler = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WebsocketSession::handleWebsocketFrame(Websocket::Frame &frame) {
	if ( frame.type == Websocket::Frame::ConnectionClose ) {
		SEISCOMP_DEBUG("[websocket] Received close request");
		if ( _handler ) {
			_handler->close();
			_handler = nullptr;
		}
		return;
	}

	if ( !frame.finalFragment ) {
		SEISCOMP_WARNING("[websocket] Unhandled intermediate frame %d",
		                 static_cast<int>(frame.type));
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

			if ( _handler ) {
				_handler->handleFrame(frame);
			}
			break;
		}

		default:
			SEISCOMP_WARNING("[websocket] Unhandled frame type %d",
			                 static_cast<int>(frame.type));
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
