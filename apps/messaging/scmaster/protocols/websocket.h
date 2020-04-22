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


/**
 * @brief The WsSession class implements a WebSocket session that communicates
 *        via the WebSocket protocol with the clients.
 */
class WebsocketSession : public HttpSession, Broker::Client {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		WebsocketSession(Wired::Socket *sock, Broker::Server *server);


	// ----------------------------------------------------------------------
	//  Public HTTPSession interface
	// ----------------------------------------------------------------------
	public:
		void update() override;

		void handleHeader(const char *name, size_t nlen,
		                  const char *value, size_t vlen) override;

		bool handleGETRequest(Wired::HttpRequest &req) override;

		void close() override;
		void buffersFlushed() override;
		void outboxFlushed() override;

		void replyWithError(const char *msg, int len);
		void replyWithError(const std::string &msg);

		void handleWebsocketFrame(Wired::Websocket::Frame &frame) override;
		void handleFrame(char *data, int len);

		virtual bool handleWSUpgrade(Wired::HttpRequest &req);

		void commandCONNECT(char *frame, int len);
		void commandDISCONNECT(char *frame, int len);
		void commandSUBSCRIBE(char *frame, int len);
		void commandUNSUBSCRIBE(char *frame, int len);
		void commandSEND(char *frame, int len);
		void commandSTATE(char *frame, int len, bool service);

		Broker::Queue *queue() const { return _queue; }


	// ----------------------------------------------------------------------
	//  Subscriber interface
	// ----------------------------------------------------------------------
	protected:
		Wired::Socket::IPAddress IPAddress() const override;

		size_t publish(Broker::Client *sender, Broker::Message *msg) override;
		void enter(const Broker::Group *group, const Broker::Client *newMember,
		           Broker::Message *msg) override;
		void leave(const Broker::Group *, const Broker::Client *newMember,
		           Broker::Message *msg) override;
		void disconnected(const Broker::Client *newMember,
		                  Broker::Message *msg) override;
		void ack() override;
		void dispose() override;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		size_t sendMessage(Broker::Message *msg);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		OPT(Broker::SequenceNumber) _continueWithSeqNo;
		int                         _bytesSent;
		int                         _messageBacklog;
		std::string                 _requestQueue;
};


}
}
}


#endif
