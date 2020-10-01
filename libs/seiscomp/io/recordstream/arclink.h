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
namespace _private {

class SC_SYSTEM_CORE_API ArclinkException: public Seiscomp::IO::RecordStreamException {
	public:
		ArclinkException(): RecordStreamException("ArcLink exception") {}
		ArclinkException(const std::string& what): RecordStreamException(what) {}
};

class SC_SYSTEM_CORE_API ArclinkCommandException: public ArclinkException {
	public:
		ArclinkCommandException(): ArclinkException("command not accepted") {}
		ArclinkCommandException(const std::string& what): ArclinkException(what) {}
};


DEFINE_SMARTPOINTER(ArclinkConnection);

class SC_SYSTEM_CORE_API ArclinkConnection : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(ArclinkConnection);

	public:
		//! C'tor
		ArclinkConnection();
		
		//! Initializing Constructor
		ArclinkConnection(std::string serverloc);

		//! Destructor
		virtual ~ArclinkConnection();

	public:
		//! The recordtype cannot be selected when using an arclink
		//! connection. It will always create MiniSeed records
		virtual bool setRecordType(const char*);

		//! Initialize the arclink connection.
		virtual bool setSource(const std::string &serverloc);
		
		//! Supply user credentials
		bool setUser(std::string name, std::string password);
		
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
		                       const Seiscomp::Core::Time &stime,
		                       const Seiscomp::Core::Time &etime);
  
		//! Adds the given start time to the server connection description
		virtual bool setStartTime(const Seiscomp::Core::Time &stime);
		
		//! Adds the given end time to the server connection description
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);

		//! Sets timeout
		virtual bool setTimeout(int seconds);

		//! Terminates the arclink connection.
		virtual void close();

		virtual Record *next();

		//! Removes all stream list, time window, etc. -entries from the connection description object.
		bool clear();

		//! Reconnects a terminated arclink connection.
		bool reconnect();


	private:
		Seiscomp::IO::Socket _sock;
		std::string _serverloc;
		std::string _user;
		std::string _passwd;
		std::list<StreamIdx> _ordered;
		std::set<StreamIdx> _streams;
		Seiscomp::Core::Time _stime;
		Seiscomp::Core::Time _etime;
		std::string _reqID;
		bool _readingData;
		bool _chunkMode;
		int _remainingBytes;
		std::ofstream _dump;

		void handshake();
		void cleanup();
};

} // namespace _private

//using _private::ArclinkException;
//using _private::ArclinkCommandException;
using _private::ArclinkConnection;
using _private::ArclinkConnectionPtr;

} // namespace Arclink
} // namespace RecordStream
} // namespace Seiscomp

#endif

