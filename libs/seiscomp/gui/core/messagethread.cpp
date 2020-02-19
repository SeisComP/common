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




#define SEISCOMP_COMPONENT GUI::MessageThread

#include "messagethread.h"
#include <seiscomp/logging/log.h>
#include <seiscomp/messaging/connection.h>


using namespace Seiscomp::Client;


namespace Seiscomp {
namespace Gui {


MessageThread::MessageThread(Seiscomp::Client::Connection* c)
: _reconnectOnError(false)
, _connection(c) {}


void MessageThread::setReconnectOnErrorEnabled(bool e) {
	_reconnectOnError = e;
	SEISCOMP_DEBUG("Setting automatic reconnect to: %d", e);
}


MessageThread::~MessageThread() {
	SEISCOMP_INFO("destroying message thread");
}


void MessageThread::run() {
	Result result;

	SEISCOMP_INFO("starting message thread");
	emit messagesAvailable();

	while ( true ) {
		//SEISCOMP_DEBUG("Automatic reconnect: %d", _reconnectOnError);
		result = _connection->fetchInbox();
		if ( result == OK ) {
			emit messagesAvailable();
		}
		else {
			if ( _connection->isConnected() ) {
				SEISCOMP_WARNING("Connection::wait() returned error %d, but still connected: %s",
				                 result.toInt(), _connection->lastErrorMessage().c_str());
			}

			if ( _reconnectOnError ) {
				emit connectionLost();
				SEISCOMP_INFO("Trying to reconnect to messaging");
				while ( _connection->reconnect() != OK
				     && _reconnectOnError ) {
					SEISCOMP_ERROR("Reconnect failed, wait 2 sec and try again...");
					sleep(2);
				}

				if ( !_connection->isConnected() ) {
					if ( !_reconnectOnError ) {
						emit connectionError(result.toInt());
						break;
					}
				}
				else
					emit connectionEstablished();
			}
			else {
				SEISCOMP_DEBUG("Connection::wait() returned error %d and automatic reconnect is disabled: %s",
				               result.toInt(), result.toString());
				emit connectionError(result.toInt());
				break;
			}
		}
	}

	SEISCOMP_INFO("leaving message thread");

	return;
}


Seiscomp::Client::Connection *MessageThread::connection() const {
	return _connection;
}


}
}
