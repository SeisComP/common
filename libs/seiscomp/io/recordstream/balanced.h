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


#ifndef SEISCOMP_SERVICES_RECORDSTREAM_COMBINED_H
#define SEISCOMP_SERVICES_RECORDSTREAM_COMBINED_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>
#include <thread>

#include <seiscomp/core/datetime.h>
#include <seiscomp/core/timewindow.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/core.h>
#include <seiscomp/client/queue.h>

namespace Seiscomp {
namespace RecordStream {
namespace Balanced {
namespace _private {

DEFINE_SMARTPOINTER(BalancedConnection);

class SC_SYSTEM_CORE_API BalancedConnection : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(BalancedConnection)

	public:
		//! C'tor
		BalancedConnection();

		//! Initializing Constructor
		BalancedConnection(std::string serverloc);

		//! Destructor
		virtual ~BalancedConnection();

	public:
		virtual bool setRecordType(const char*);

		//! Initialize the combined connection.
		virtual bool setSource(const std::string &serverloc);

		//! Adds the given stream to the server connection description
		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode);

		//! Adds the given stream to the server connection description
		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const Seiscomp::Core::Time &startTime,
		                       const Seiscomp::Core::Time &endTime);

		//! Adds the given start time to the server connection description
		virtual bool setStartTime(const Core::Time &stime);

		//! Adds the given end time to the server connection description
		virtual bool setEndTime(const Core::Time &etime);

		//! Adds the given end time window to the server connection description
		virtual bool setTimeWindow(const Core::TimeWindow &w);

		//! Sets timeout
		virtual bool setTimeout(int seconds);

		//! Terminates the combined connection.
		virtual void close();

		virtual Record *next();


	private:
		int streamHash(const std::string &sta);
		void putRecord(RecordPtr rec);
		Record* getRecord();
		void acquiThread(IO::RecordStreamPtr rs);

	private:
		bool _started;
		int _nthreads;
		std::vector<std::pair<IO::RecordStreamPtr, bool> > _rsarray;
		std::list<std::thread *> _threads;
		Client::ThreadedQueue<Record*> _queue;
		std::istringstream _stream;
		std::mutex _mtx;
};

} // namesapce Balanced
} // namespace _private
} // namespace RecordStream
} // namespace Seiscomp

#endif

