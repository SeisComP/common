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


#ifndef SEISCOMP_SERVICES_RECORDSTREAM_CAPS_H
#define SEISCOMP_SERVICES_RECORDSTREAM_CAPS_H


#include "caps/packet.h"
#include "caps/sessiontable.h"

#include <seiscomp/wired/devices/socket.h>
#include <seiscomp3/io/recordstream.h>

#include <list>
#include <map>


namespace {


template <int N>
class socketbuf : public std::streambuf {
	public:
		socketbuf() {
			setsocket(NULL);
		}

		socketbuf(Seiscomp::Wired::Socket *sock) {
			setsocket(sock);
		}

		void setsocket(Seiscomp::Wired::Socket *sock) {
			_allowed_reads = -1;
			_real_buffer_size = 0;
			_block_write = false;
			setg(_in, _in, _in);
			setp(_out, _out + N);
			_sock = sock;
		}

		void settimeout(const struct timeval &tv) {
			_timeout = tv;
		}

		void set_read_limit(int bytes) {
			_allowed_reads = bytes;

			if ( _allowed_reads >= 0 ) {
				if ( egptr() - gptr() > _allowed_reads )
					setg(eback(), gptr(), gptr() + _allowed_reads);

				// Set the number of read bytes to the
				// remaining bytes in the buffer
				_allowed_reads -= egptr() - gptr();
			}
			else
				setg(eback(), gptr(), eback() + _real_buffer_size);

			//std::cout << "[" << (void*)eback() << ", " << (void*)gptr() << ", " << (void*)egptr() << "]" << " = " << (egptr() - gptr()) << std::endl;
		}

		int read_limit() const {
			if ( _allowed_reads < 0 ) return -1;
			return egptr() - gptr() + _allowed_reads;
		}


	protected:
		virtual int underflow() {
			// No more reads allowed?
			if ( !_allowed_reads )
				return traits_type::eof();

			// Read available data from socket
			int res = _sock->read(_in, N);
			if ( res <= 0 ) {
				set_read_limit(0);
				return traits_type::eof();
			}

			// Set input sequence pointers
			_real_buffer_size = res;
			setg(_in, _in, _in + _real_buffer_size);

			// clip to limit
			set_read_limit(_allowed_reads);

			return traits_type::to_int_type(*gptr());
		}

		virtual int overflow(int c) {
			if ( _block_write ) return traits_type::eof();

			if ( !traits_type::eq_int_type(traits_type::eof(), c)) {
				traits_type::assign(*pptr(), traits_type::to_char_type(c));
				pbump(1);
			}

			return sync() == 0 ? traits_type::not_eof(c) : traits_type::eof();
		}

		virtual int sync() {
			if ( pbase() == pptr() ) return 0;
			int res = _sock->write(pbase(), pptr() - pbase());
			return res == pptr() - pbase() ? 0 : -1;
		}

		// Only forward seeking is supported
		virtual std::streampos
		seekoff(std::streamoff off, std::ios_base::seekdir way,
		        std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
			if ( way != std::ios_base::cur || which != std::ios_base::in || off < 0 )
				return -1;

			while ( off > 0 ) {
				int ch = sbumpc();
				if ( ch == traits_type::eof() )
					return -1;
				--off;
			}

			return 0;
		}

	private:
		Seiscomp::Wired::Socket *_sock;
		timeval                  _timeout;
		char                     _in[N];
		char                     _out[N];
		bool                     _block_write;
		int                      _real_buffer_size;
		int                      _allowed_reads;
};


class RecordStream : public Seiscomp::IO::RecordStream {
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		RecordStream();
		~RecordStream();

	// ----------------------------------------------------------------------
	// public methods
	// ----------------------------------------------------------------------
	public:
		bool setSource(const std::string &source) override;

		bool addStream(const std::string &networkCode,
		               const std::string &stationCode,
		               const std::string &locationCode,
		               const std::string &channelCode) override;

		bool addStream(const std::string &networkCode,
		               const std::string &stationCode,
		               const std::string &locationCode,
		               const std::string &channelCode,
		               const Seiscomp::Core::Time &startTime,
		               const Seiscomp::Core::Time &endTime) override;

		bool setStartTime(const Seiscomp::Core::Time &stime) override;
		bool setEndTime(const Seiscomp::Core::Time &etime) override;
		bool setTimeWindow(const Seiscomp::Core::TimeWindow &tw) override;

		bool setRecordType(const char *type) override;

		bool setTimeout(int seconds) override;

		void close() override;

		Seiscomp::Record *next() override;

	// ----------------------------------------------------------------------
	// protected methods
	// ----------------------------------------------------------------------
	protected:
		void handleInterrupt(int sig) override;
		virtual Seiscomp::Wired::Socket *createSocket() const;


	// ----------------------------------------------------------------------
	// private types
	// ----------------------------------------------------------------------
	private:
		enum ResponseState {
			Unspecific,
			Requests
		};

		typedef Seiscomp::IO::CAPS::SessionTable SessionTable;
		typedef Seiscomp::IO::CAPS::SessionTableItem SessionTableItem;

		struct Request {
			std::string          net;
			std::string          sta;
			std::string          loc;
			std::string          cha;
			Seiscomp::Core::Time start;
			Seiscomp::Core::Time end;
			bool                 receivedData;
		};


		typedef std::map<std::string, Request> RequestList;


	// ----------------------------------------------------------------------
	// private methods
	// ----------------------------------------------------------------------
	private:
		bool addRequest(const std::string &net, const std::string &sta,
		                const std::string &loc, const std::string &cha,
		                const Seiscomp::Core::Time &stime,
		                const Seiscomp::Core::Time &etime,
		                bool receivedData);

		void wait();
		void disconnect();
		bool handshake();
		void onItemAboutToBeRemoved(const SessionTableItem *item);


	// ----------------------------------------------------------------------
	// private data members
	// ----------------------------------------------------------------------
	protected:
		typedef Seiscomp::Wired::SocketPtr SocketPtr;

		ResponseState           _state;
		std::istream            _stream;
		RequestList             _requests;
		SocketPtr               _socket;
		socketbuf<512>          _buf;
		char                    _lineBuf[201];
		std::string             _host;
		int                     _port;
		Seiscomp::Core::Time    _startTime;
		Seiscomp::Core::Time    _endTime;
		Seiscomp::Record       *_nextRecord;
		bool                    _terminated;
		bool                    _realtime;
		bool                    _ooo;
		std::string             _user;
		std::string             _password;
		SessionTable            _sessionTable;
		int                     _currentID;
		SessionTableItem       *_currentItem;
};


class RecordStreamSecure : public RecordStream {
	public:
		RecordStreamSecure();

	protected:
		Seiscomp::Wired::Socket *createSocket() const;
};


}


#endif
