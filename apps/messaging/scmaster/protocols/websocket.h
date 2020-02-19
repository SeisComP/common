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


#ifndef SEISCOMP_BROKER_PROTOCOL_WEBSOCKET_H__
#define SEISCOMP_BROKER_PROTOCOL_WEBSOCKET_H__


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
		virtual void update();

		virtual void handleHeader(const char *name, int nlen,
		                          const char *value, int vlen);

		virtual bool handleGETRequest(Wired::HttpRequest &req);
		virtual bool handleWSUpgrade(Wired::HttpRequest &req);

		virtual void close();
		virtual void buffersFlushed();
		virtual void outboxFlushed();

		void replyWithError(const char *msg, int len);
		void replyWithError(const std::string &msg);

		void handleWebsocketFrame(Wired::Websocket::Frame &frame);
		void handleFrame(char *data, int len);

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
		virtual Wired::Socket::IPAddress IPAddress() const override;

		virtual void publish(Broker::Client *sender, Broker::Message *msg) override;
		virtual void enter(const Broker::Group *group, const Broker::Client *newMember,
		                   Broker::Message *msg) override;
		virtual void leave(const Broker::Group *, const Broker::Client *newMember,
		                   Broker::Message *msg) override;
		virtual void disconnected(const Broker::Client *newMember,
		                          Broker::Message *msg) override;
		virtual void ack() override;
		virtual void dispose() override;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		void sendMessage(Broker::Message *msg);


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
