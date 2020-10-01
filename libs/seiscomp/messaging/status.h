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


#ifndef SEISCOMP_CLIENT_STATUS_H
#define SEISCOMP_CLIENT_STATUS_H


#include <stdint.h>

#include <seiscomp/core/enumeration.h>


namespace Seiscomp {
namespace Client {


struct Status {
	MAKEENUM(
		Tag,
		EVALUES(
			Time,
			Hostname,
			Clientname,
			Programname,
			PID,
			CPUUsage,
			TotalMemory,
			ClientMemoryUsage,
			SentMessages,
			SentBytes,
			ReceivedMessages,
			ReceivedBytes,
			MessageQueueSize,
			ObjectCount,
			Address,
			Uptime,
			ResponseTime
		),
		ENAMES(
			"time",
			"hostname",
			"clientname",
			"programname",
			"pid",
			"cpuusage",
			"totalmemory",
			"clientmemoryusage",
			"sentmessages",
			"sentbytes",
			"receivedmessages",
			"receivedbytes",
			"messagequeuesize",
			"objectcount",
			"address",
			"uptime",
			"responsetime"
		)
	);

	void clear();
	bool parse(const std::string &data);

	std::string values[ETagQuantity];
};


// Primary Template
template <Status::ETag T>
struct StatusT;


// Specializations
#define SPECIALIZE_CONNECTIONINFOT(TAG, TYPE) \
	template <> struct StatusT<TAG> { \
		typedef TYPE Type; \
	};


SPECIALIZE_CONNECTIONINFOT(Status::Time, std::string)
SPECIALIZE_CONNECTIONINFOT(Status::Hostname, std::string)
SPECIALIZE_CONNECTIONINFOT(Status::Clientname, std::string)
SPECIALIZE_CONNECTIONINFOT(Status::Programname, std::string)
SPECIALIZE_CONNECTIONINFOT(Status::PID, int)
SPECIALIZE_CONNECTIONINFOT(Status::CPUUsage, double)
SPECIALIZE_CONNECTIONINFOT(Status::TotalMemory, int)
SPECIALIZE_CONNECTIONINFOT(Status::ClientMemoryUsage, int)
SPECIALIZE_CONNECTIONINFOT(Status::SentMessages, int)
SPECIALIZE_CONNECTIONINFOT(Status::SentBytes, uint64_t)
SPECIALIZE_CONNECTIONINFOT(Status::ReceivedMessages, int)
SPECIALIZE_CONNECTIONINFOT(Status::ReceivedBytes, uint64_t)
SPECIALIZE_CONNECTIONINFOT(Status::MessageQueueSize, int)
SPECIALIZE_CONNECTIONINFOT(Status::ObjectCount, int)
SPECIALIZE_CONNECTIONINFOT(Status::Address, std::string)
SPECIALIZE_CONNECTIONINFOT(Status::Uptime, std::string)
SPECIALIZE_CONNECTIONINFOT(Status::ResponseTime, int)


}
}


#endif
