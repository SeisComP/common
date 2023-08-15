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


#ifndef SEISCOMP_CLIENT_SCMP_WEBSOCKET_CONNECTION_H
#define SEISCOMP_CLIENT_SCMP_WEBSOCKET_CONNECTION_H


#include <seiscomp/messaging/protocols/scmp/socket.h>
#include <seiscomp/wired/protocols/websocket.h>


namespace Seiscomp {
namespace Client {
namespace SCMP {


class WebsocketConnection : public Socket {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		WebsocketConnection();
		~WebsocketConnection();


	// ----------------------------------------------------------------------
	//  Connection interface
	// ----------------------------------------------------------------------
	public:
		virtual Result connect(const char *address, unsigned int timeoutMs,
		                       const char *clientName = nullptr) override;

		virtual Result subscribe(const std::string &group) override;
		virtual Result unsubscribe(const std::string &group) override;

		virtual Result sendData(const std::string &targetGroup,
		                        const char *data, size_t len,
		                        MessageType type,
		                        ContentEncoding contentEncoding,
		                        ContentType contentType) override;

		virtual Result sendMessage(const std::string &targetGroup,
		                           const Core::Message *msg,
		                           MessageType type,
		                           OPT(ContentEncoding) contentEncoding = Core::None,
		                           OPT(ContentType) contentType = Core::None) override;

		virtual Result fetchInbox() override;
		virtual Result syncOutbox() override;

		virtual Result recv(Packet &p) override;
		virtual Packet *recv(Result *result = nullptr) override;

		virtual Result disconnect() override;

		virtual bool isConnected() override;

		virtual Result setTimeout(int milliseconds) override;


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		typedef Wired::Websocket::Frame WSFrame;

		void waitForAck();
		Result flushBacklog();

		Result readFrame(WSFrame &frame, boost::mutex *mutex, bool forceBlock = false);
		bool handleFrame(WSFrame &frame, Packet *p, Result *result = nullptr);

		/**
		 * Send a data buffer and waits for an acknowledgment message.
		 * @pre _writeMutex is locked
		 * @post _writeMutex is locked
		 */
		Result send(Wired::Buffer *msg, WSFrame::Type type, bool isRegular);
		Result sendSocket(const char *data, int len);

		void updateReceiveBuffer();
		void closeSocket(const char *errorMessage = nullptr,
		                 int errorMessageLen = -1);
		void closeSocketWithoutLock(const char *errorMessage = nullptr,
		                            int errorMessageLen = -1);

		// This requires the read mutex to be locked
		Result fetchAndQueuePacket();


	// ----------------------------------------------------------------------
	//  Private types and members
	// ----------------------------------------------------------------------
	protected:
		mutable boost::mutex _waitMutex;
		WSFrame              _recvFrame;
		size_t               _inboxWaterLevel;
};


}
}
}




#endif
