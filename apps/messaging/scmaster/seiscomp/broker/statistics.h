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


#ifndef SEISCOMP_BROKER_STATISTICS_H__
#define SEISCOMP_BROKER_STATISTICS_H__


#include <seiscomp/core/baseobject.h>
#include <seiscomp/broker/api.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


/**
 * @brief Simple structure to store transfer counts.
 * The unit of the counters is not defined. It can be counts or
 * bytes or something different. That depends on the context.
 */
struct SC_BROKER_API Tx : Core::BaseObject {
	Tx() : received(0), sent(0) {}

	double received; //!< Number of items received
	double sent;     //!< Number of items sent

	Tx &operator+=(const Tx &other) {
		received += other.received;
		sent += other.sent;
		return *this;
	}

	DECLARE_SERIALIZATION {
		ar
		& NAMED_OBJECT("recv", received)
		& NAMED_OBJECT("sent", sent)
		;
	}
};


struct GroupStatistics : Core::BaseObject {
	std::string name;
	Tx          messages;
	Tx          bytes;
	Tx          payload;

	DECLARE_SERIALIZATION {
		ar
		& NAMED_OBJECT("name", name)
		& NAMED_OBJECT_HINT("messages", messages, Archive::STATIC_TYPE)
		& NAMED_OBJECT_HINT("bytes", bytes, Archive::STATIC_TYPE)
		& NAMED_OBJECT_HINT("payload", payload, Archive::STATIC_TYPE)
		;
	}
};


DEFINE_SMARTPOINTER(QueueStatistics);
struct SC_BROKER_API QueueStatistics : Core::BaseObject {
	typedef std::vector<GroupStatistics> Groups;
	std::string name;
	Groups      groups;
	Tx          messages;
	Tx          bytes;
	Tx          payload;

	QueueStatistics &operator+=(const QueueStatistics &stats);

	DECLARE_SERIALIZATION {
		ar
		& NAMED_OBJECT("name", name)
		& NAMED_OBJECT_HINT("messages", messages, Archive::STATIC_TYPE)
		& NAMED_OBJECT_HINT("bytes", bytes, Archive::STATIC_TYPE)
		& NAMED_OBJECT_HINT("payload", payload, Archive::STATIC_TYPE)
		& NAMED_OBJECT_HINT("groups", groups, Archive::STATIC_TYPE)
		;
	}
};



}
}
}


#endif
