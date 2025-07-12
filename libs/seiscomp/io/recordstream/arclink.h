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


#ifndef SEISCOMP_IO_RECORDSTREAM_ARCLINK_H
#define SEISCOMP_IO_RECORDSTREAM_ARCLINK_H


#include <string>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/utils/timer.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/core.h>
#include <seiscomp/io/socket.h>
#include <seiscomp/io/recordstream/streamidx.h>


namespace Seiscomp {
namespace RecordStream {
namespace Arclink {


DEFINE_SMARTPOINTER(ArclinkConnection);
class SC_SYSTEM_CORE_API ArclinkConnection : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(ArclinkConnection);

	public:
		//! C'tor
		ArclinkConnection();

		//! Initializing Constructor
		ArclinkConnection(std::string serverloc);

		//! Destructor
		~ArclinkConnection() override;

	public:
		//! The recordtype cannot be selected when using an arclink
		//! connection. It will always create MiniSeed records
		bool setRecordType(const char*) override;

		//! Initialize the arclink connection.
		bool setSource(const std::string &serverloc) override;

		//! Supply user credentials
		bool setUser(std::string name, std::string password);

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
		               const OPT(Seiscomp::Core::Time) &stime,
		               const OPT(Seiscomp::Core::Time) &etime) override;

		//! Adds the given start time to the server connection description
		bool setStartTime(const OPT(Seiscomp::Core::Time) &stime) override;

		//! Adds the given end time to the server connection description
		bool setEndTime(const OPT(Seiscomp::Core::Time) &etime) override;

		//! Sets timeout
		bool setTimeout(int seconds) override;

		//! Terminates the arclink connection.
		void close() override;

		Record *next() override;

		//! Removes all stream list, time window, etc. -entries from the connection description object.
		bool clear();

		//! Reconnects a terminated arclink connection.
		bool reconnect();


	private:
		Seiscomp::IO::Socket      _sock;
		std::string               _serverloc;
		std::string               _user;
		std::string               _passwd;
		std::list<StreamIdx>      _ordered;
		std::set<StreamIdx>       _streams;
		OPT(Seiscomp::Core::Time) _stime;
		OPT(Seiscomp::Core::Time) _etime;
		std::string               _reqID;
		bool                      _readingData;
		bool                      _chunkMode;
		int                       _remainingBytes;
		std::ofstream             _dump;

		void handshake();
		void cleanup();
};


} // namespace Arclink
} // namespace RecordStream
} // namespace Seiscomp


#endif
