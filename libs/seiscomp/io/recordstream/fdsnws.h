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


#ifndef SEISCOMP_IO_RECORDSTREAM_WS_H
#define SEISCOMP_IO_RECORDSTREAM_WS_H

#include <string>
#include <set>
#include <iostream>
#include <sstream>
#include <seiscomp/core.h>
#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/utils/timer.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/io/socket.h>
#include <seiscomp/io/recordstream/streamidx.h>

namespace Seiscomp {
namespace RecordStream {


class SC_SYSTEM_CORE_API FDSNWSConnectionBase : public IO::RecordStream {
	protected:
		//! C'tor
		FDSNWSConnectionBase(const char *protocol, IO::Socket *socket, int defaultPort);


	public:
		//! The recordtype cannot be selected when using an arclink
		//! connection. It will always create MiniSeed records
		virtual bool setRecordType(const char *type);

		//! Initialize the arclink connection.
		virtual bool setSource(const std::string &source);

		//! Supply user credentials
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
		                       const Core::Time &startTime,
		                       const Core::Time &endTime);

		//! Adds the given start time to the server connection description
		virtual bool setStartTime(const Core::Time &startTime);

		//! Adds the given end time to the server connection description
		virtual bool setEndTime(const Core::Time &endTime);

		//! Sets timeout
		virtual bool setTimeout(int seconds);

		//! Terminates the arclink connection.
		virtual void close();

		virtual Record *next();

		//! Reconnects a terminated arclink connection.
		bool reconnect();

		//! Removes all stream list, time window, etc. -entries from the connection description object.
		bool clear();


	private:
		const char *getProxy() const;
		void openConnection(const std::string &);

		//! Blocking read from socket
		std::string readBinary(int size);
		void handshake();


	private:
		const char          *_protocol;
		IO::SocketPtr        _socket;
		std::string          _host;
		std::string          _url;
		int                  _defaultPort;
		std::set<StreamIdx>  _streams;
		Core::Time           _stime;
		Core::Time           _etime;
		std::string          _reqID;
		bool                 _readingData;
		bool                 _chunkMode;
		int                  _remainingBytes;
		std::string          _error;
};


class SC_SYSTEM_CORE_API FDSNWSConnection : public FDSNWSConnectionBase {
	public:
		FDSNWSConnection();
};


class SC_SYSTEM_CORE_API FDSNWSSSLConnection : public FDSNWSConnectionBase {
	public:
		FDSNWSSSLConnection();
};



}
}

#endif

