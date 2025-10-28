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
#include <iostream>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/timewindow.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/io/recordstream/streamidx.h>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(CombinedConnection);
class SC_SYSTEM_CORE_API CombinedConnection : public IO::RecordStream {
	public:
		//! C'tor
		CombinedConnection();

		//! Initializing Constructor
		CombinedConnection(std::string serverloc);

		//! Destructor
		~CombinedConnection() override;

		bool setRecordType(const char*) override;

		//! Initialize the combined connection.
		bool setSource(const std::string &serverloc) override;

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
		               const OPT(Core::Time) &startTime,
		               const OPT(Core::Time) &endTime) override;

		//! Adds the given start time to the server connection description
		bool setStartTime(const OPT(Core::Time) &stime) override;

		//! Adds the given end time to the server connection description
		bool setEndTime(const OPT(Core::Time) &etime) override;

		//! Sets timeout
		bool setTimeout(int seconds) override;

		//! Terminates the combined connection.
		void close() override;

		//! Returns the data stream
		Record *next() override;

	private:
		void init();

	private:
		bool                _started;

		size_t              _nStream;
		size_t              _nArchive;
		size_t              _nRealtime;

		OPT(Core::Time)     _startTime;
		OPT(Core::Time)     _endTime;
		Core::Time          _archiveEndTime;
		Core::TimeSpan      _realtimeAvailability;

		std::set<StreamIdx> _tmpStreams;
		IO::RecordStreamPtr _realtime;
		IO::RecordStreamPtr _archive;
};


} // namespace RecordStream
} // namespace Seiscomp


#endif

