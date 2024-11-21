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


#define SEISCOMP_COMPONENT CAPS
#include "caps_private.h"

#include "caps/anypacket.h"
#include "caps/rawpacket.h"
#include "caps/endianess.h"
#include "caps/utils.h"

#include <seiscomp/logging/log.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/records/mseedrecord.h>

#include <libmseed.h>
#include <ctype.h>
#include <functional>
#include <fstream>


using namespace std;
using namespace Seiscomp;

namespace sc = Seiscomp::Core;
namespace gc = Seiscomp::IO::CAPS;


namespace {


struct RecordHeaderTime {
	bool get(std::streambuf &input) {
		input.sgetn((char*)&seconds, sizeof(seconds));
		return input.sgetn((char*)&microSeconds, sizeof(microSeconds)) == sizeof(microSeconds);
	}

	int dataSize() { return sizeof(seconds) + sizeof(microSeconds); }

	int64_t seconds;
	int32_t microSeconds;
};


template <typename INPUTTYPE>
class CAPSRecord : public GenericRecord {
	public:
		CAPSRecord(std::string net, std::string sta,
		           std::string loc, std::string cha,
		           Core::Time stime, double fsamp, int nsamp)
		: GenericRecord(net, sta, loc ,cha, stime, fsamp) {
			_nsamp = nsamp;
		}

		void read(std::istream &in) {
			ArrayPtr ar;
			bool res = false;

			switch ( _datatype ) {
				case Array::CHAR:
				{
					CharArrayPtr car = new CharArray(_nsamp);
					ar = car;
					res = readData(in, *car);
				}
					break;
				case Array::INT:
				{
					IntArrayPtr iar = new IntArray(_nsamp);
					ar = iar;
					res = readData(in, *iar);
				}
					break;
				default:
				case Array::FLOAT:
				{
					FloatArrayPtr far = new FloatArray(_nsamp);
					ar = far;
					res = readData(in, *far);
				}
					break;
				case Array::DOUBLE:
				{
					DoubleArrayPtr dar = new DoubleArray(_nsamp);
					ar = dar;
					res = readData(in, *dar);
				}
					break;
			};

			if ( !res )
				throw Core::StreamException("not enough samples in stream");

			if ( !ar )
				throw Core::StreamException("invalid datatype request");

			setData(ar.get());
		}


	private:
		template <typename T>
		bool readData(std::istream &in, TypedArray<T> &target) {
			for ( int i = 0; i < _nsamp; ++i ) {
				INPUTTYPE val;
				in.read((char*)&val, sizeof(val));
				if ( !in.good() )
					return false;

				val = gc::Endianess::Converter::FromLittleEndian(val);
				target[i] = (T)val;
			}
			return true;
		}
};


class MSeedRecord_ : public IO::MSeedRecord {
	public:
		MSeedRecord_() {}

	public:
		void read(std::istream &is) {
			int reclen = -1;
			MSRecord *prec = nullptr;
			const int LEN = 128;
			char header[LEN];

			if ( !is.read(header,LEN) )
				throw Core::StreamException("Incomplete header");

			if ( !MS_ISVALIDHEADER(header) )
				throw IO::LibmseedException("Invalid header");

			reclen = ms_detect(header, LEN);
			if ( reclen <= 0 ) /* scan to the next header to retrieve the record length */
				throw IO::LibmseedException("Retrieving the record length failed");

			if ( reclen < LEN )
				throw Core::EndOfStreamException("Invalid miniSEED record, too small");

			if ( reclen > (1 << 20) )
				throw Core::EndOfStreamException("Invalid miniSEED record, too large (> 1**20 bytes)");

			std::vector<char> rawrec(reclen);
			memmove(&rawrec[0], header, LEN);

			if ( !is.read(&rawrec[LEN], reclen-LEN) )
				throw Core::StreamException("Fatal error occured during reading from stream");

			if ( msr_unpack(&rawrec[0], reclen, &prec, 0, 0) == MS_NOERROR ) {
				*static_cast<IO::MSeedRecord*>(this) = IO::MSeedRecord(prec,this->_datatype,this->_hint);
				msr_free(&prec);
				if ( _fsamp <= 0 )
					throw IO::LibmseedException("Unpacking of Mini SEED record failed.");
			}
			else {
				msr_free(&prec);
				throw IO::LibmseedException("Unpacking of Mini SEED record failed.");
			}
		}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_RECORDSTREAM(RecordStream, "caps");
REGISTER_RECORDSTREAM(RecordStreamSecure, "capss");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStream::RecordStream()
: _stream(&_buf) {
	_port = 18002;
	_host = "localhost";
	_terminated = true;
	_state = Unspecific;
	_nextRecord = nullptr;
	_currentItem = nullptr;
	_currentID = -1;
	_realtime = true;
	_sessionTable.setItemAboutToBeRemovedFunc(
		std::bind(
			&RecordStream::onItemAboutToBeRemoved,
			this,
			std::placeholders::_1
		)
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStream::~RecordStream() {
	// Delete temporary record if still available
	if ( _nextRecord )
		delete _nextRecord;

	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setSource(const string &source) {
	string addr = source;
	string params;
	int timeout = 300;

	_port = 18002;

	{
		size_t pos = addr.find('@');

		if ( pos != string::npos ) {
			string cred = addr.substr(0, pos);
			addr.erase(addr.begin(), addr.begin()+pos+1);

			size_t split_pos = cred.find(':');
			if ( split_pos != string::npos ) {
				_user = cred.substr(0, split_pos);
				_password = cred.substr(split_pos+1);
			}
			else {
				_user = cred;
				_password.clear();
			}
		}
	}

	{
		size_t pos = addr.rfind('?');
		if ( pos != string::npos ) {
			params = addr.substr(pos+1);
			addr.erase(pos);
		}

	}

	{
		size_t pos = addr.rfind(':');

		if ( pos == string::npos )
			_host = addr;
		else {
			_host = addr.substr(0, pos);
			if ( !Core::fromString(_port, addr.substr(pos+1)) ) {
				SEISCOMP_ERROR("invalid source address: %s", addr.c_str());
				return false;
			}
		}
	}

	if ( _host.empty() ) {
		_host = "localhost";
	}

	_realtime = true;
	_ooo = false;
	_minMTime = _maxMTime = sc::Time();

	if ( !params.empty() ) {
		vector<string> toks;
		Core::split(toks, params.c_str(), "&");
		if ( !toks.empty() ) {
			for ( std::vector<std::string>::iterator it = toks.begin();
			      it != toks.end(); ++it ) {
				std::string name, value;

				size_t pos = it->find('=');
				if ( pos != std::string::npos ) {
					name = it->substr(0, pos);
					value = it->substr(pos+1);
				}
				else {
					name = *it;
					value = "";
				}

				if ( name == "arch" )
					_realtime = false;
				else if ( name == "ooo" )
					_ooo = true;
				else if ( name == "timeout" ) {
					if ( !Core::fromString(timeout, value) ) {
						SEISCOMP_ERROR("wrong timeout value: %s", value.c_str());
						return false;
					}
				}
				else if ( name == "user" ) {
					_user = value;
				}
				else if ( name == "pwd" || name == "pass" || name == "password" ) {
					_password = value;
				}
				else if ( name == "min-mtime" ) {
					if ( !sc::fromString(_minMTime, value) ) {
						SEISCOMP_ERROR("invalid min-mtime: %s", value.c_str());
						return false;
					}
				}
				else if ( name == "max-mtime" ) {
					if ( !sc::fromString(_maxMTime, value) ) {
						SEISCOMP_ERROR("invalid max-mtime: %s", value.c_str());
						return false;
					}
				}
				else if ( name == "request-file" ) {
					ifstream ifs(value.c_str());
					if ( !ifs.is_open() ) {
						SEISCOMP_ERROR("could not open request file: %s", value.c_str());
						return false;
					}

					string line;
					int lc = 1;

					vector<string> toks;
					Core::Time startTime, endTime;

					while ( getline(ifs, line) ) {
						Core::trim(line);
						if ( line.compare(0, 11, "STREAM ADD ") == 0 ) {
							// Is a request pending?
							if ( !toks.empty() ) {
								addStream(toks[0], toks[1], toks[2], toks[3], startTime, endTime);
								toks.clear();
								startTime = endTime = Core::Time();
							}

							string id = line.substr(11);
							Core::split(toks, id.c_str(), ".", false);
							if ( toks.size() != 4 ) {
								SEISCOMP_ERROR("invalid stream id at %s:%d: %s", value.c_str(), lc, id.c_str());
								return false;
							}
						}
						else if ( line.compare(0, 5, "TIME ") == 0 ) {
							string stime, etime;
							size_t p;
							stime = line.substr(5);
							Core::trim(stime);

							p = stime.find(':');
							if ( p == string::npos ) {
								SEISCOMP_ERROR("invalid time range at %s:%d: %s", value.c_str(), lc, stime.c_str());
								return false;
							}

							etime = stime.substr(p+1);
							stime.erase(stime.begin()+p, stime.end());

							if ( !stime.empty() && !startTime.fromString(stime.c_str(), "%Y,%m,%d,%H,%M,%S,%f") ) {
								SEISCOMP_ERROR("invalid start time at %s:%d: %s", value.c_str(), lc, stime.c_str());
								return false;
							}

							if ( !etime.empty() && !endTime.fromString(etime.c_str(), "%Y,%m,%d,%H,%M,%S,%f") ) {
								SEISCOMP_ERROR("invalid end time at %s:%d: %s", value.c_str(), lc, etime.c_str());
								return false;
							}
						}

						++lc;
					}

					if ( !toks.empty() )
						addStream(toks[0], toks[1], toks[2], toks[3], startTime, endTime);
				}
			}
		}
	}

	_terminated = false;
	_socket = createSocket();
	if ( timeout > 0 ) {
		SEISCOMP_DEBUG("setting socket timeout to %ds", timeout);
		_socket->setSocketTimeout(timeout,0);
	}
	_buf.setsocket(_socket.get());

	_currentID = -1;
	_currentItem = nullptr;
	_sessionTable.reset();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Wired::Socket *RecordStream::createSocket() const {
	return new Wired::Socket;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStream::disconnect() {
	if ( _socket ) {
		_socket->shutdown();
		_socket->close();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStream::close() {
	disconnect();
	_terminated = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStream::handleInterrupt(int) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setRecordType(const char *) {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setTimeout(int seconds) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::addStream(const string &net, const string &sta,
                             const string &loc, const string &cha) {
	return addRequest(net, sta, loc, cha, _startTime, _endTime, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::addStream(const string &net, const string &sta,
                             const string &loc, const string &cha,
                             const Core::Time &stime,
                             const Core::Time &etime) {
	return addRequest(net, sta, loc, cha, stime, etime, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::addRequest(const string &net, const string &sta,
                              const string &loc, const string &cha,
                              const Core::Time &stime,
                              const Core::Time &etime,
                              bool receivedData) {
	string streamID = net + "." + sta + "." + loc + "." + cha;
	Request &req = _requests[streamID];
	req.net = net;
	req.sta = sta;
	req.loc = loc;
	req.cha = cha;
	req.start = stime;
	req.end = etime;
	req.receivedData = receivedData;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStream::wait() {
	// Wait 5 seconds and keep response latency low
	for ( int i = 0; (i < 10) && !_terminated; ++i )
		usleep(500000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::handshake() {
	_buf.set_read_limit(-1);

	while ( !_socket->isValid() ) {
		// Continue already started data
		for ( SessionTable::const_iterator it = _sessionTable.begin();
		      it != _sessionTable.end(); ++it ) {
			if ( it->second.startTime.valid() ) {
				addRequest(it->second.net, it->second.sta,
				           it->second.loc, it->second.cha,
				           it->second.startTime,
				           it->second.endTime, true);
			}
		}

		_currentItem = nullptr;
		_currentID = -1;
		_sessionTable.reset();

		bool first = true;
		while ( !_terminated ) {
			if ( _socket->connect(_host, _port) != Wired::Socket::Success ) {
				if ( first ) {
					SEISCOMP_WARNING("unable to connect to %s:%d, "
					                 "trying again every 5 seconds",
					                 _host.c_str(), _port);
					first = false;
				}

				wait();
			}
			else
				break;
		}

		if ( _terminated )
			return false;

		// Do handshake
		if ( !_user.empty() && !_password.empty() ) {
			string auth = "AUTH "+ _user + " " + _password + "\n";
			_socket->write(auth.c_str(), auth.length());
		}

		_socket->write("BEGIN REQUEST\n", 14);

		if ( !_realtime )
			_socket->write("REALTIME OFF\n", 13);

		if ( _ooo )
			_socket->write("OUTOFORDER ON\n", 14);

		if ( _minMTime.valid() || _maxMTime.valid() ) {
			_socket->write("MTIME ", 6);
			if ( _minMTime.valid() ) {
				auto s = _minMTime.toString("%Y,%m,%d,%H,%M,%S,%f");
				_socket->write(s.c_str(), s.size());
			}
			_socket->write(":", 1);
			if ( _maxMTime.valid() ) {
				auto s = _maxMTime.toString("%Y,%m,%d,%H,%M,%S,%f");
				_socket->write(s.c_str(), s.size());
			}
			_socket->write("\n", 1);
		}

		// First pass: continue all previous streams
		for ( RequestList::iterator it = _requests.begin();
		      it != _requests.end(); ++it ) {
			if ( it->second.receivedData == false ) continue;

			stringstream req;
			req << "STREAM ADD " << it->first << endl;
			req << "TIME ";

			int year, mon, day, hour, minute, second, microseconds;

			if ( it->second.start.valid() ) {
				it->second.start.get(&year, &mon, &day, &hour, &minute, &second, &microseconds);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second << "," << microseconds;
			}
			else if ( _startTime.valid() ) {
				_startTime.get(&year, &mon, &day, &hour, &minute, &second, &microseconds);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second << "," << microseconds;
			}

			req << ":";

			if ( it->second.end.valid() ) {
				it->second.end.get(&year, &mon, &day, &hour, &minute, &second, &microseconds);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second << "," << microseconds;
			}
			else if ( _endTime.valid() ) {
				_endTime.get(&year, &mon, &day, &hour, &minute, &second, &microseconds);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second << "," << microseconds;
			}

			req << endl;

			string line = req.str();
			SEISCOMP_DEBUG("%s", line.c_str());
			_socket->write(line.c_str(), line.size());
		}

		// Second pass: subscribe to remaining streams
		for ( RequestList::iterator it = _requests.begin();
		      it != _requests.end(); ++it ) {
			if ( it->second.receivedData == true ) continue;

			stringstream req;
			req << "STREAM ADD " << it->first << endl;
			req << "TIME ";

			int year, mon, day, hour, minute, second, microseconds;

			if ( it->second.start.valid() ) {
				it->second.start.get(&year, &mon, &day, &hour, &minute, &second, &microseconds);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second << "," << microseconds;
			}
			else if ( _startTime.valid() ) {
				_startTime.get(&year, &mon, &day, &hour, &minute, &second, &microseconds);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second << "," << microseconds;
			}

			req << ":";

			if ( it->second.end.valid() ) {
				it->second.end.get(&year, &mon, &day, &hour, &minute, &second, &microseconds);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second << "," << microseconds;
			}
			else if ( _endTime.valid() ) {
				_endTime.get(&year, &mon, &day, &hour, &minute, &second, &microseconds);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second << "," << microseconds;
			}

			req << endl;

			string line = req.str();
			SEISCOMP_DEBUG("%s", line.c_str());
			_socket->write(line.c_str(), line.size());
		}

		_socket->write("END\n", 4);

		_buf.set_read_limit(-1);
		gc::ResponseHeader header;
		if ( !header.get(_buf) ) {
			// Case to retry, connection closed by peer
			SEISCOMP_ERROR("unable to finish handshake, closing connection and connect again");
			// Do not close, only disconnect to remember last session streams
			disconnect();
			wait();
			continue;
		}

		_buf.set_read_limit(header.size);
		if ( header.id != 0 ) {
			SEISCOMP_ERROR("invalid handshake response, expected ID 0 but got %d", header.id);
			return false;
		}

		istream is(&_buf);
		if ( is.getline(_lineBuf, 200).fail() ) {
			SEISCOMP_ERROR("invalid response: line exceeds maximum of 200 characters");
			return false;
		}

		// Skip remaining stuff
		if ( _buf.pubseekoff(_buf.read_limit(), ios_base::cur, ios_base::in) < 0 ) {
			SEISCOMP_ERROR("seek: connection closed by peer");
			disconnect();
			wait();
			continue;
		}

		if ( strncasecmp(_lineBuf, "ERROR:", 6) == 0 ) {
			// No case to retry
			SEISCOMP_ERROR("error in handshake, server responds: %s", _lineBuf);
			return false;
		}
		else if ( strncasecmp(_lineBuf, "STATUS OK", 9) == 0 ) {
			SEISCOMP_DEBUG("handshaking complete");
			return true;
		}
		else {
			// No case to retry
			SEISCOMP_ERROR("error in handshake, invalid response: %s", _lineBuf);
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *RecordStream::next() {
	if ( !_socket || _terminated ) {
		_stream.setstate(ios_base::eofbit);
		return nullptr;
	}

	if ( _sessionTable.empty() && _requests.empty() ) {
		SEISCOMP_DEBUG("session finished");
		close();
		_stream.setstate(ios_base::eofbit);
		return nullptr;
	}

	while ( !_terminated ) {
		if ( !handshake() ) {
			close();
			_stream.setstate(ios_base::eofbit);
			return nullptr;
		}

		// Read the data
		while ( !_terminated && _socket->isValid() ) {
			// Skip unread bytes from previous record
			int skippies = _buf.read_limit();
			if ( skippies > 0 ) {
				SEISCOMP_WARNING("no seemless reading, skipping %d bytes", skippies);
				if ( _buf.pubseekoff(skippies, ios_base::cur, ios_base::in) < 0 ) {
					SEISCOMP_ERROR("seek: connection closed");
					break;
				}
			}

			_buf.set_read_limit(-1);

			gc::ResponseHeader header;
			if ( !header.get(_buf) ) {
				SEISCOMP_INFO("connection closed by peer");
				disconnect();
				wait();
				break;
			}

			_buf.set_read_limit(header.size);

			if ( header.id == 0 ) {
				// Server ID
				while ( header.size > 0 ) {
					// Server ID
					while ( header.size > 0 ) {
						istream is(&_buf);
						if ( is.getline(_lineBuf, 200).fail() ) {
							SEISCOMP_ERROR("Invalid response: line exceeds maximum of 200 characters");
							// Unrecoverable error, reconnect makes no sense
							_terminated = true;
							break;
						}

						header.size -= is.gcount();

						SessionTable::Status status =
							_sessionTable.handleResponse(_lineBuf, is.gcount());
						if ( status == SessionTable::Error ) {
							SEISCOMP_ERROR("Fatal error: invalid response: %s", _lineBuf);
							break;
						}
						else if ( status == SessionTable::EOD ) {
							_terminated = true;
							break;
						}
					}

					if ( header.size > 0 ) {
						SEISCOMP_ERROR("Header: connection closed: %d bytes left", header.size);
						// Reconnect and try to handshake again
						disconnect();
						wait();
						break;
					}
				}
			}
			else {
				if ( _nextRecord ) {
					delete _nextRecord;
					_nextRecord = nullptr;
				}

				if ( _currentID != header.id ) {
					SessionTableItem *item = _sessionTable.getItem(header.id);
					if ( item == nullptr ) {
						SEISCOMP_WARNING("Unknown data request ID %d", header.id);
						//cerr << "ignoring DATA[] ... " << header.size << endl;
					}

					_currentItem = item;
					_currentID = header.id;
				}

				if ( _currentItem ) {
					switch ( _currentItem->type ) {
						case gc::RawDataPacket:
						{
							RecordHeaderTime time;
							if ( !time.get(_buf) ) {
								SEISCOMP_ERROR("fetch start time: connection closed");
								disconnect();
								wait();
								break;
							}

							header.size -= time.dataSize();

							time.seconds =
							    gc::Endianess::Converter::FromLittleEndian(time.seconds);
							time.microSeconds =
							    gc::Endianess::Converter::FromLittleEndian(time.microSeconds);

							sc::Time stime = sc::Time::FromEpoch(time.seconds, time.microSeconds);

							//SEISCOMP_DEBUG("received record: startTime = %s", stime.iso().c_str());

							switch ( _currentItem->dataType ) {
								case gc::DT_INT64:
									_nextRecord = new CAPSRecord<int64_t>(_currentItem->net, _currentItem->sta, _currentItem->loc, _currentItem->cha, stime, _currentItem->fSamplingFrequency, header.size / sizeof(int64_t));
									break;
								case gc::DT_INT32:
									_nextRecord = new CAPSRecord<int32_t>(_currentItem->net, _currentItem->sta, _currentItem->loc, _currentItem->cha, stime, _currentItem->fSamplingFrequency, header.size / sizeof(int32_t));
									break;
								case gc::DT_INT16:
									_nextRecord = new CAPSRecord<int16_t>(_currentItem->net, _currentItem->sta, _currentItem->loc, _currentItem->cha, stime, _currentItem->fSamplingFrequency, header.size / sizeof(int16_t));
									break;
								case gc::DT_INT8:
									_nextRecord = new CAPSRecord<int8_t>(_currentItem->net, _currentItem->sta, _currentItem->loc, _currentItem->cha, stime, _currentItem->fSamplingFrequency, header.size / sizeof(int8_t));
									break;
								case gc::DT_FLOAT:
									_nextRecord = new CAPSRecord<float>(_currentItem->net, _currentItem->sta, _currentItem->loc, _currentItem->cha, stime, _currentItem->fSamplingFrequency, header.size / sizeof(float));
									break;
								case gc::DT_DOUBLE:
									_nextRecord = new CAPSRecord<double>(_currentItem->net, _currentItem->sta, _currentItem->loc, _currentItem->cha, stime, _currentItem->fSamplingFrequency, header.size / sizeof(double));
									break;
								default:
									break;
							}
						}
							break;
						case gc::MSEEDPacket:
							_nextRecord = new MSeedRecord_;
							break;
						case gc::FixedRawDataPacket:
						{
							gc::FixedRawDataRecord::Header raw_header;
							raw_header.get(_buf);

							header.size -= raw_header.dataSize();

							_nextRecord =
							    new CAPSRecord<float>(_currentItem->net, _currentItem->sta,
							                       _currentItem->loc, _currentItem->cha,
							                       timestampToTime(raw_header.samplingTime),
							                       _currentItem->fSamplingFrequency,
							                       header.size / sizeof(float));

							break;
						}
						case gc::ANYPacket:
						{
							gc::AnyDataRecord::AnyHeader any_header;
							any_header.get(_buf);

							header.size -= any_header.dataSize();

							_nextRecord =
							    new CAPSRecord<char>(_currentItem->net, _currentItem->sta,
							                       _currentItem->loc, _currentItem->cha,
							                       timestampToTime(any_header.dataHeader.samplingTime),
							                       _currentItem->fSamplingFrequency,
							                       header.size);

							break;
						}
						default:
							/*
							cerr << "ignoring DATA[" << _currentItem->streamID
							     << "] ... " << header.size << endl;
							*/
							break;
					}
				}

				// If a record can be read return the stream to actually read it
				if ( _nextRecord != nullptr ) {
					Record *rec = _nextRecord;
					_nextRecord = nullptr;
					setupRecord(rec);

					try {
						rec->read(_stream);

						if ( _currentItem != nullptr ) {
							// Never throw when an invalid record has parsed. Better let the application
							// decide.
							try {
								gc::Time end = rec->endTime() + sc::TimeSpan(0.5 / rec->samplingFrequency());
								if ( end > _currentItem->startTime )
									_currentItem->startTime = end;
							}
							catch ( ... ) {}
						}

						return rec;
					}
					catch ( std::exception &e ) {
						_stream.clear();
						SEISCOMP_WARNING("parse record error: %s", e.what());
					}
				}
			}
		}
	}

	// Delete temporary record
	if ( _nextRecord ) {
		delete _nextRecord;
		_nextRecord = nullptr;
	}

	close();
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setStartTime(const Core::Time &stime) {
	_startTime = stime;
	SEISCOMP_DEBUG("set global start time to %s", _startTime.toString("%F %T.%f").c_str());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setEndTime(const Core::Time &etime) {
	_endTime = etime;
	SEISCOMP_DEBUG("set global end time to %s", _endTime.toString("%F %T.%f").c_str());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStream::setTimeWindow(const Core::TimeWindow &tw) {
	return setStartTime(tw.startTime()) && setEndTime(tw.endTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStream::onItemAboutToBeRemoved(const SessionTableItem *item) {
	if ( _currentItem == item) _currentID = -1;

	addRequest(item->net, item->sta, item->loc, item->cha,
	           item->startTime, item->endTime, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamSecure::RecordStreamSecure() {
	_port = 18022;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Wired::Socket *RecordStreamSecure::createSocket() const {
	return new Wired::SSLSocket;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
