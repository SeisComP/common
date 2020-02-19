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


#ifndef SEISCOMP_SCMASTER_STATISTICS_H__
#define SEISCOMP_SCMASTER_STATISTICS_H__


#include <seiscomp/core/baseobject.h>
#include <seiscomp/broker/message.h>
#include <seiscomp/broker/statistics.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


DEFINE_SMARTPOINTER(ServerStatistics);
struct SC_BROKER_API ServerStatistics : Core::BaseObject {
	DECLARE_SC_CLASS(ServerStatistics)

	ServerStatistics();

	typedef std::vector<QueueStatistics> Queues;
	// Due to the current archive limitation of reading int64 an int
	// has to be used
	int                    sequenceNumber;
	Core::Time             timestamp;
	Tx                     messages;
	Tx                     bytes;
	Tx                     payload;
	Queues                 queues;

	DECLARE_SERIALIZATION {
		ar
		& NAMED_OBJECT("sequenceNumber", sequenceNumber)
		& NAMED_OBJECT("timestamp", timestamp)
		& NAMED_OBJECT_HINT("messages", messages, Archive::STATIC_TYPE)
		& NAMED_OBJECT_HINT("bytes", bytes, Archive::STATIC_TYPE)
		& NAMED_OBJECT_HINT("payload", payload, Archive::STATIC_TYPE)
		& NAMED_OBJECT_HINT("queues", queues, Archive::STATIC_TYPE)
		;
	}
};


}
}
}


#endif
