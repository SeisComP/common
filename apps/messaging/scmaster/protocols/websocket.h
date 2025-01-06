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


#ifndef SEISCOMP_BROKER_PROTOCOL_WEBSOCKET_H
#define SEISCOMP_BROKER_PROTOCOL_WEBSOCKET_H


#include <seiscomp/broker/client.h>

#include "../server.h"
#include "http.h"


namespace Seiscomp {
namespace Messaging {
namespace Protocols {


class WebsocketSession;

DEFINE_SMARTPOINTER(WebsocketHandler);
class WebsocketHandler : public Seiscomp::Core::BaseObject {
	public:
		WebsocketHandler(WebsocketSession *session) : _session(session) {}

	public:
		//! When the HTTP session upgrades the connection to a websocket
		//! connection then this method is called and can be used to inject
		//! additional HTTP headers into the upgrade response.
		virtual void addUpgradeHeader() {}

		//! Called at the beginning of a new update cycle when the reactor
		//! gives connection control to the underlying session.
		virtual void start() = 0;

		//! Main function to handle a websocket frame.
		virtual void handleFrame(Seiscomp::Wired::Websocket::Frame &frame) = 0;

		//! Callback when all buffers in the underlying session have been
		//! flushed.
		virtual void buffersFlushed() = 0;

		//! Called when a websocket close frame has been received.
		virtual void close() = 0;

		virtual Broker::Queue *queue() const { return nullptr; }

	protected:
		WebsocketSession *_session;
};


/**
 * @brief The WsSession class implements a WebSocket session that communicates
 *        via the WebSocket protocol with the clients.
 */
class WebsocketSession : public HttpSession {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		WebsocketSession(Seiscomp::Wired::Socket *sock, Broker::Server *server);


	// ----------------------------------------------------------------------
	//  Public HTTPSession interface
	// ----------------------------------------------------------------------
	public:
		void update() override;

		void handleHeader(const char *name, size_t nlen,
		                  const char *value, size_t vlen) override;

		void close() override;
		void buffersFlushed() override;

		void requestFinished() override;

		void handleWebsocketFrame(Seiscomp::Wired::Websocket::Frame &frame) override;
		void upgradeToWebsocket(const char *protocol,
		                        WebsocketHandler *handler,
		                        uint64_t maxPayloadSize);

		bool handleBrokerUpgrade(const std::string &requestQueue) override;
		bool handleDatabaseUpgrade(const std::string &dbURL) override;

		Seiscomp::Wired::HttpRequest &request() { return _request; }
		Seiscomp::Wired::Websocket::Frame *frame() { return _websocketFrame.get(); }
		size_t bufferdOutgoingBytes() { return inAvail(); }

		// Expose protected method to public
		using HttpSession::invalidate;
		using HttpSession::finishReading;

		Broker::Queue *queue() const { return _handler ? _handler->queue() : nullptr; }


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		WebsocketHandlerPtr _handler;
};


}
}
}


#endif
