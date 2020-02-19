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


#ifndef GEMPA_BROKER_MESSAGEDISPATCHER_H__
#define GEMPA_BROKER_MESSAGEDISPATCHER_H__


#include <seiscomp/broker/api.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


class Queue;


/**
 * @brief The MessageDispatcher class is used to forward processed messages
 *        from another thread.
 *
 * Since it is not safe to call publish on all registered subscribers, the
 * dispatcher class is provide safe handling within a given framework.
 */
class SC_BROKER_API MessageDispatcher {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		MessageDispatcher() {}


	// ----------------------------------------------------------------------
	//  Dispatcher interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Notifies the dispatcher about a new message. If the message
		 *        should be published, dispatch() must be called.
		 * @param queue The queue that got a new message to be dispatched
		 */
		virtual void messageAvailable(Queue *queue) = 0;

		/**
		 * @brief Dispatches a message from the process-ready-queue.
		 *
		 * This call may block if not issued after the messageAvailable()
		 * signal.
		 * @param queue The target queue
		 */
		void flushMessages(Queue *queue) {
			queue->flushProcessedMessages();
		}
};


}
}
}


#endif
