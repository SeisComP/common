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


#ifndef SEISCOMP_IO_RECORDSTREAM_SLINK_H
#define SEISCOMP_IO_RECORDSTREAM_SLINK_H

#include <string>
#include <set>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/core.h>
#include <seiscomp/io/socket.h>


namespace Seiscomp {
namespace RecordStream {


class SC_SYSTEM_CORE_API SeedlinkException: public Seiscomp::IO::RecordStreamException {
	public:
		SeedlinkException(): RecordStreamException("Seedlink exception") {}
		SeedlinkException(const std::string& what): RecordStreamException(what) {}
};

class SC_SYSTEM_CORE_API SeedlinkCommandException: public SeedlinkException {
	public:
		SeedlinkCommandException(): SeedlinkException("command not accepted") {}
		SeedlinkCommandException(const std::string& what): SeedlinkException(what) {}
};


class SC_SYSTEM_CORE_API SLStreamIdx {
	public:
		SLStreamIdx();

		SLStreamIdx(const std::string &net, const std::string &sta,
		            const std::string &loc, const std::string &cha);

		SLStreamIdx(const std::string &net, const std::string &sta,
		            const std::string &loc, const std::string &cha,
		            const Seiscomp::Core::Time &stime,
		            const Seiscomp::Core::Time &etime);

		SLStreamIdx& operator=(const SLStreamIdx &other);

		bool operator<(const SLStreamIdx &other) const;
		bool operator==(const SLStreamIdx &other) const;

		//! Returns the network code
		const std::string &network() const;

		//! Returns the station code
		const std::string &station() const;

		//! Returns the channel code
		const std::string &channel() const;

		//! Returns the location code
		const std::string &location() const;

		//! Returns the selector in <location><channel>.D notation
		//! * wildcards are substituted by a corresponding number of ?
		std::string selector() const;

		//! Returns the start time
		Core::Time startTime() const;

		//! Returns the end time
		Core::Time endTime() const;

		//! Returns the most recent record end time
		Seiscomp::Core::Time timestamp() const;

		//! Sets the time stamp
		void setTimestamp(Seiscomp::Core::Time &rectime) const;

	private:
		const std::string  _net;
		const std::string  _sta;
		const std::string  _loc;
		const std::string  _cha;
		const Core::Time   _stime;
		const Core::Time   _etime;
		mutable Core::Time _timestamp;
};


DEFINE_SMARTPOINTER(SLConnection);

class SC_SYSTEM_CORE_API SLConnection : public Seiscomp::IO::RecordStream {
	DECLARE_SC_CLASS(SLConnection);

	public:
		//! C'tor
		SLConnection();

		//! Initializing Constructor
		SLConnection(std::string serverloc);

		//! Destructor
		virtual ~SLConnection();

	public:
		//! The recordtype cannot be selected when using a seedlink
		//! connection. It will always create MiniSeed records
		virtual bool setRecordType(const char*);

		//! Initialize the seedlink connection.
		virtual bool setSource(const std::string &source);

		//! Adds the given stream to the server connection description
		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode);

		//! Adds a seismic stream request to the record stream (not implemented)
		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const Seiscomp::Core::Time &startTime,
		                       const Seiscomp::Core::Time &endTime);

		//! Adds the given start time to the server connection description
		virtual bool setStartTime(const Seiscomp::Core::Time &startTime);

		//! Adds the given end time to the server connection description
		virtual bool setEndTime(const Seiscomp::Core::Time &endTime);

		//! Sets timeout
		virtual bool setTimeout(int seconds);

		//! Disconnects and terminates (!) the seedlink connection.
		virtual void close();

		//! Reads the data stream
		virtual Record *next();

		//! Removes all stream list, time window, etc. -entries from the connection description object.
		bool clear();

		//! Reconnects a terminated seedlink connection.
		bool reconnect();


	private:
		void handshake();


	private:
		class StreamBuffer : public std::streambuf {
			public:
				StreamBuffer();
				std::streambuf *setbuf(char *s, std::streamsize n);
		};

		StreamBuffer          _streambuf;
		std::string           _serverloc;
		std::string           _slrecord;
		IO::Socket            _sock;
		std::set<SLStreamIdx> _streams;
		Core::Time            _stime;
		Core::Time            _etime;
		bool                  _readingData;
		bool                  _useBatch;
		int                   _maxRetries;
		int                   _retriesLeft;
};


}
}

#endif 
