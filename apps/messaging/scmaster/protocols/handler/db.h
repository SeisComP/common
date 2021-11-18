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


#ifndef SEISCOMP_BROKER_PROTOCOL_HANDLER_DB_H
#define SEISCOMP_BROKER_PROTOCOL_HANDLER_DB_H


#include <seiscomp/io/database.h>
#include "../websocket.h"


namespace Seiscomp {
namespace Messaging {
namespace Protocols {


class DBHandler : public WebsocketHandler {
	public:
		static constexpr const char *PROTOCOL_NAME = "scsql";
		static constexpr const int DEFAULT_MAX_PAYLOAD_SIZE = 1024*1024;

	public:
		DBHandler(WebsocketSession *session, Seiscomp::IO::DatabaseInterfacePtr db);
		~DBHandler();

	public:
		void addUpgradeHeader() override;
		void start() override;
		void handleFrame(Seiscomp::Wired::Websocket::Frame &frame) override;
		void buffersFlushed() override;
		void outboxFlushed() override;
		void close() override;

	private:
		void sendClose();
		void sendResult(uint8_t command, uint8_t code, const char *message);

	private:
		Seiscomp::IO::DatabaseInterfacePtr _db;
};


}
}
}


#endif
