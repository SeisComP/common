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
		bool setRecordType(const char *type) override;

		//! Initialize the arclink connection.
		bool setSource(const std::string &source) override;

		//! Supply user credentials
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
		bool setStartTime(const OPT(Core::Time) &startTime) override;

		//! Adds the given end time to the server connection description
		bool setEndTime(const OPT(Core::Time) &endTime) override;

		//! Sets timeout
		bool setTimeout(int seconds) override;

		//! Terminates the arclink connection.
		void close() override;

		Record *next() override;


	private:
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
		OPT(Core::Time)      _stime;
		OPT(Core::Time)      _etime;
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

