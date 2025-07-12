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


#ifndef SEISCOMP_BROKER_PROTOCOL_HANDLER_BROKER_H
#define SEISCOMP_BROKER_PROTOCOL_HANDLER_BROKER_H


#include "../websocket.h"


namespace Seiscomp {
namespace Messaging {
namespace Protocols {


class BrokerHandler : public WebsocketHandler, Broker::Client {
	public:
		static constexpr const char *PROTOCOL_NAME = "scmp";
		static constexpr const int DEFAULT_MAX_PAYLOAD_SIZE = 1024*1024;

	public:
		BrokerHandler(WebsocketSession *session,
		              const std::string &queueName)
		: WebsocketHandler(session), _requestQueue(queueName) {}
		~BrokerHandler();

	public:
		void start() override;
		void handleFrame(Seiscomp::Wired::Websocket::Frame &frame) override;
		void buffersFlushed() override;
		void close() override;

		Broker::Queue *queue() const override { return _queue; }

	// ----------------------------------------------------------------------
	//  Subscriber interface
	// ----------------------------------------------------------------------
	protected:
		Seiscomp::Wired::Socket::IPAddress IPAddress() const override;

		size_t publish(Broker::Client *sender, Broker::Message *msg) override;
		void enter(const Broker::Group *group, const Broker::Client *newMember,
		           Broker::Message *msg) override;
		void leave(const Broker::Group *, const Broker::Client *newMember,
		           Broker::Message *msg) override;
		void disconnected(const Broker::Client *newMember, Broker::Message *msg) override;
		void ack() override;
		void dispose() override;

	private:
		void commandCONNECT(char *frame, size_t len);
		void commandDISCONNECT(char *frame, size_t len);
		void commandSUBSCRIBE(char *frame, size_t len);
		void commandUNSUBSCRIBE(char *frame, size_t len);
		void commandSEND(char *frame, size_t len);
		void commandSTATE(char *frame, size_t len, bool service);

		size_t sendMessage(Broker::Message *msg);

		void replyWithError(const char *msg, size_t len);
		void replyWithError(const std::string &msg);

	private:
		OPT(Broker::SequenceNumber) _continueWithSeqNo;
		int                         _bytesSent{0};
		int                         _messageBacklog{0};
		std::string                 _requestQueue;
};


}
}
}


#endif
