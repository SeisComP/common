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


#ifndef SEISCOMP_SERVICES_RECORDSTREAM_CONCURRENT_H
#define SEISCOMP_SERVICES_RECORDSTREAM_CONCURRENT_H


#include <seiscomp/core/datetime.h>
#include <seiscomp/core/timewindow.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/core.h>
#include <seiscomp/client/queue.h>


namespace Seiscomp {
namespace RecordStream {


class SC_SYSTEM_CORE_API Concurrent : public IO::RecordStream {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Concurrent();

		//! Destructor
		~Concurrent() override;


	// ----------------------------------------------------------------------
	//  RecordStream Interface
	// ----------------------------------------------------------------------
	public:
		bool setRecordType(const char*) override;

		//! Adds the given stream to the server connection description
		bool addStream(const std::string &networkCode,
		               const std::string &stationCode,
		               const std::string &locationCode,
		               const std::string &channelCode) override;

		//! Adds the given stream to the server connection description
		bool addStream(const std::string &networkCode,
		               const std::string &stationCode,
		               const std::string &locationCode,
		               const std::string &channelCode,
		               const Seiscomp::Core::Time &startTime,
		               const Seiscomp::Core::Time &endTime) override;

		//! Adds the given start time to the server connection description
		bool setStartTime(const Core::Time &stime) override;

		//! Adds the given end time to the server connection description
		bool setEndTime(const Core::Time &etime) override;

		//! Adds the given end time window to the server connection description
		bool setTimeWindow(const Core::TimeWindow &w) override;

		//! Sets timeout
		bool setTimeout(int seconds) override;

		//! Terminates the combined connection.
		void close() override;

		Record *next() override;


	// ----------------------------------------------------------------------
	//  Concurrent interface
	// ----------------------------------------------------------------------
	protected:
		virtual int getRS(const std::string &networkCode,
		                      const std::string &stationCode,
		                      const std::string &locationCode,
		                      const std::string &channelCode) = 0;

		void reset();


	// ----------------------------------------------------------------------
	//  Private methods and members
	// ----------------------------------------------------------------------
	private:
		void acquiThread(IO::RecordStream *rs);

	protected:
		using RecordStreamItem = std::pair<IO::RecordStreamPtr, bool>;

		bool                           _started{false};
		std::vector<RecordStreamItem>  _rsarray;

	private:
		int                            _nthreads{0};
		std::list<std::thread>         _threads;
		Client::ThreadedQueue<Record*> _queue;
		std::mutex                     _mtx;
};


} // namespace RecordStream
} // namespace Seiscomp


#endif
