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


#define SEISCOMP_COMPONENT SL4Connection


#include <cstring>
#include <seiscomp/logging/log.h>
#include <seiscomp/core/system.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/io/records/mseed/decoder/format.h>

#include "sl4connection.h"

struct SL4FixedHeader {
	char signature[2];
	char format;
	char subformat;
	uint32_t payloadLength;
	uint64_t seq;
	uint8_t stationIdLength;
} __attribute__ ((packed));

/* Seedlink4 packets consist of a fixed header ... */
#define HEADSIZE sizeof(SL4FixedHeader)
/* ... followed by variable length station ID and payload */
#define MAXPAYLOAD 16384
/* ... server terminates a requested time window with the token END */
#define TERMTOKEN "END"
/* ... or in case of problems with ERROR */
#define ERRTOKEN "ERROR"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace RecordStream {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *DefaultSL4Host = "localhost";

template<typename T>
const char *DefaultSL4Port = nullptr;

template<>
const char *DefaultSL4Port<IO::Socket> = "18000";

template<>
const char *DefaultSL4Port<IO::SSLSocket> = "18500";

typedef SL4Connection<IO::Socket> PlainSL4Connection;
typedef SL4Connection<IO::SSLSocket> SSLSL4Connection;

/* IMPLEMENT_SC_CLASS_DERIVED(PlainSL4Connection,
                           Seiscomp::IO::RecordStream,
                           "PlainSL4Connection"); */
REGISTER_RECORDSTREAM(PlainSL4Connection, "slink4");

/* IMPLEMENT_SC_CLASS_DERIVED(SSLSL4Connection,
                           Seiscomp::IO::RecordStream,
                           "SSLSL4Connection"); */
REGISTER_RECORDSTREAM(SSLSL4Connection, "slink4s");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SL4StreamIdx::SL4StreamIdx() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SL4StreamIdx::SL4StreamIdx(const string &net, const string &sta, const string &loc,
                           const string &cha)
: _net(net), _sta(sta), _loc(loc), _cha(cha) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SL4StreamIdx::SL4StreamIdx(const string &net, const string &sta, const string &loc,
                           const string &cha, const OPT(Time) &stime, const OPT(Time) &etime)
: _net(net), _sta(sta), _loc(loc), _cha(cha)
, _stime(stime), _etime(etime) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SL4StreamIdx& SL4StreamIdx::operator=(const SL4StreamIdx &other) {
	if ( this != &other ) {
		this->~SL4StreamIdx();
		new(this) SL4StreamIdx(other);
	}

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SL4StreamIdx::operator<(const SL4StreamIdx &other) const {
	if ( _net < other._net ) {
		return true;
	}
	else if ( _net > other._net ) {
		return false;
	}

	if ( _sta < other._sta ) {
		return true;
	}
	else if ( _sta > other._sta ) {
		return false;
	}

	// Wildcards precede concrete location codes
	bool isWildcard = _loc.find_first_of("*?") != string::npos;
	bool isOtherWildcard = other._loc.find_first_of("*?") != string::npos;

	if ( isWildcard != isOtherWildcard ) {
		return isWildcard;
	}

	if ( _loc < other._loc ) {
		return true;
	}
	else if ( _loc > other._loc ) {
		return false;
	}

	isWildcard = _cha.find_first_of("*?") != string::npos;
	isOtherWildcard = other._cha.find_first_of("*?") != string::npos;

	if ( isWildcard != isOtherWildcard ) {
		return isWildcard;
	}

	return _cha < other._cha;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SL4StreamIdx::operator==(const SL4StreamIdx &other) const {
	return (_net == other._net && _sta == other._sta &&
	        _loc == other._loc && _cha == other._cha);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &SL4StreamIdx::network() const {
	return _net;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &SL4StreamIdx::station() const {
	return _sta;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &SL4StreamIdx::channel() const {
	return _cha;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &SL4StreamIdx::location() const {
	return _loc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string SL4StreamIdx::selector(int format) const {
	string cha = _cha;

	if ( cha.length() == 3 && cha.find('_') == string::npos )
		cha = cha.substr(0, 1) + "_" + cha.substr(1, 1) + "_" + cha.substr(2, 1);

	string selector = _loc + "_" + cha;

	if ( format == 2 )
		selector += ".2D";
	else if ( format == 3 )
		selector += ".3D";
	else
		selector += ".?D";

	return selector;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const OPT(Time) &SL4StreamIdx::startTime() const {
	return _stime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const OPT(Time) &SL4StreamIdx::endTime() const {
	return _etime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const OPT(Time) &SL4StreamIdx::timestamp() const {
	return _timestamp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SL4StreamIdx::setTimestamp(const OPT(Time) &rectime) const {
	if ( _timestamp < rectime ) {
		_timestamp = rectime;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
SL4Connection<SocketType>::StreamBuffer::StreamBuffer() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
streambuf *SL4Connection<SocketType>::StreamBuffer::setbuf(char *s, streamsize n) {
	setp(nullptr, nullptr);
	setg(s, s, s + n);
	return this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
SL4Connection<SocketType>::SL4Connection()
: RecordStream() {
	_readingData = false;
	_sock.setTimeout(300); // default
	_maxRetries = -1; // default
	_retriesLeft = -1;
	_format = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
SL4Connection<SocketType>::SL4Connection(string serverloc)
: RecordStream() {
	_readingData = false;
	_sock.setTimeout(300); // default
	_maxRetries = -1; // default
	_retriesLeft = -1;
	_format = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
SL4Connection<SocketType>::~SL4Connection() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::setSource(const string &source) {
	_readingData = false;
	_useBatch = true;
	_maxRetries = -1;
	_sock.setTimeout(300); // default

	size_t pos = source.find('?');
	if ( pos != std::string::npos ) {
		_serverloc = source.substr(0, pos);
		std::string params = source.substr(pos+1);
		std::vector<std::string> toks;
		split(toks, params.c_str(), "&");
		if ( !toks.empty() ) {
			for ( std::vector<std::string>::iterator it = toks.begin();
			      it != toks.end(); ++it ) {
				std::string name, value;

				pos = it->find('=');
				if ( pos != std::string::npos ) {
					name = it->substr(0, pos);
					value = it->substr(pos+1);
				}
				else {
					name = *it;
					value = "";
				}

				if ( name == "timeout" ) {
					unsigned int seconds;
					if ( Core::fromString(seconds, value) )
						_sock.setTimeout(seconds);
					else
						return false;
				}
				else if ( name == "retries" ) {
					if ( !Core::fromString(_maxRetries, value) )
						return false;
				}
				else if ( name == "format" ) {
					if ( !Core::fromString(_format, value) ||
							_format < 2 || _format > 3 )
						return false;
				}
			}
		}
	}
	else
		_serverloc = source;

	// set address defaults if necessary
	if ( _serverloc.empty() || _serverloc == ":" ) {
		_serverloc = DefaultSL4Host;
		_serverloc += ":";
		_serverloc += DefaultSL4Port<SocketType>;
	}
	else {
		pos = _serverloc.find(':');
		if ( pos == string::npos ) {
			_serverloc += ":";
			_serverloc += DefaultSL4Port<SocketType>;
		}
		else if ( pos == _serverloc.length()-1 ) {
			_serverloc += DefaultSL4Port<SocketType>;
		}
		else if ( pos == 0 ) {
			_serverloc.insert(0, DefaultSL4Host);
		}
	}

	_retriesLeft = -1;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::clear() {
	this->~SL4Connection<SocketType>();
	new(this) SL4Connection<SocketType>(_serverloc);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void SL4Connection<SocketType>::close() {
	_sock.interrupt();
	_retriesLeft = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::reconnect() {
	_sock.close();
	_readingData = false;
	--_retriesLeft;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::setRecordType(const char* type) {
	return !strcmp(type, "mseed");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::addStream(const string &net, const string &sta,
                                          const string &loc, const string &cha) {
	pair<set<SL4StreamIdx>::iterator, bool> result;
	result = _streams.insert(SL4StreamIdx(net, sta, loc, cha));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::addStream(const string &net, const string &sta,
                                          const string &loc, const string &cha,
                                          const OPT(Core::Time) &stime,
                                          const OPT(Core::Time) &etime) {
	pair<set<SL4StreamIdx>::iterator, bool> result;
	result = _streams.insert(SL4StreamIdx(net, sta, loc, cha, stime, etime));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::setStartTime(const OPT(Core::Time) &stime) {
	_stime = stime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::setEndTime(const OPT(Core::Time) &etime) {
	_etime = etime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
bool SL4Connection<SocketType>::setTimeout(int seconds) {
	_sock.setTimeout(seconds);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void SL4Connection<SocketType>::handshake() {
	Util::StopWatch aStopWatch;
	list<string> request;

	_sock.sendRequest("SLPROTO 4.0", true);
	_sock.sendRequest("USERAGENT SeisComP/" + Core::CurrentVersion.version().toString(), true);

	for ( const auto &idx : _streams ) {
		auto stime = idx.startTime() ? idx.startTime() : _stime;
		auto etime = idx.endTime() ? idx.endTime() : _etime;

		if ( idx.timestamp() ) {
			stime = *idx.timestamp() + TimeSpan(1, 0);
		}
		else if ( !stime ) {
			if ( etime > Time::UTC() ) {
				stime = Time::UTC();
			}
		}

		// Empty time windows are not requested
		if ( stime && etime && *stime >= *etime ) {
			SEISCOMP_DEBUG("Seedlink: ignoring empty request for %s.%s %s %s %s",
			               idx.network(), idx.station(), idx.selector(_format),
			               stime->iso(),
			               etime->iso());
			continue;
		}

		string timestr;

		if ( stime ) {
			timestr = stime->iso();
			if ( etime ) {
				timestr += etime->iso();
			}
		}

		_sock.startTimer();

		request.push_back("STATION " + idx.network() + "_" + idx.station());
		request.push_back("SELECT " + idx.selector(_format));
		request.push_back("DATA ALL " + timestr);
	}

	list<string>::iterator ri = request.begin();

	for ( const auto &req : request ) {
		_sock.sendRequest(req, false);

		while ( _sock.poll() ) {
			string resp = _sock.readline();

			if ( ri == request.end() ) {
				SEISCOMP_ERROR("Unexpected response: %s", resp.c_str());
				continue;
			}

			SEISCOMP_DEBUG("Seedlink command: %s", ri->c_str());
			SEISCOMP_DEBUG("Response: %s", resp.c_str());
			++ri;
		}
	}

	while ( ri != request.end() ) {
		string resp = _sock.readline();
		SEISCOMP_DEBUG("Seedlink command: %s", ri->c_str());
		SEISCOMP_DEBUG("Response: %s", resp.c_str());
		++ri;
	}

	_sock.sendRequest("END", false);

	SEISCOMP_DEBUG("handshake done in %f seconds", (double)aStopWatch.elapsed());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void updateStreams(std::set<SL4StreamIdx> &streams, const MSeedRecord &prec) {
	SL4StreamIdx idx(prec.networkCode(), prec.stationCode(), prec.locationCode(), prec.channelCode());
	if ( auto it = streams.find(idx); it != streams.end() ) {
		it->setTimestamp(prec.endTime());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
Record *SL4Connection<SocketType>::next() {
	if (_readingData && !_sock.isOpen()) {
		SEISCOMP_DEBUG("Socket is closed -> set stream's eofbit");
		return nullptr;
	}

	bool inReconnect = false;

	while ( !_sock.isInterrupted() ) {
		try {
			if ( !_readingData ) {
				if ( _streams.empty() ) {
					break;
				}

				if ( _retriesLeft < 0 ) {
					_retriesLeft = _maxRetries;
				}

				_sock.open(_serverloc);
				_sock.startTimer();
				SEISCOMP_DEBUG("Handshaking SeedLink server at %s", _serverloc.c_str());

				handshake();

				if ( inReconnect) {
					SEISCOMP_INFO("Connection to %s re-established", _serverloc.c_str());
				}

				_readingData = true;
				_retriesLeft = -1;

				inReconnect = false;
			}

			_sock.startTimer();
			_slrecord = _sock.read(strlen(TERMTOKEN));
			if ( !_slrecord.compare(TERMTOKEN) ) {
				_sock.close();
				break;
			}

			_slrecord += _sock.read(strlen(ERRTOKEN) - strlen(TERMTOKEN));
			if ( !_slrecord.compare(ERRTOKEN) ) {
				_sock.close();
				break;
			}

			_slrecord += _sock.read(HEADSIZE - strlen(ERRTOKEN));
			const SL4FixedHeader *head = reinterpret_cast<const SL4FixedHeader *>(_slrecord.data());

			if ( string(head->signature, 2) != "SE" ) {
				SEISCOMP_ERROR("Invalid SeedLink4 signature: %.2s", head->signature);
				_sock.close();
				break;
			}

			if ( _format != 0 && head->format != '0' + _format ) {
				SEISCOMP_ERROR("Unexpected SeedLink4 payload format: %c", head->format);
				_sock.close();
				break;
			}

			if ( head->subformat != 'D' ) {
				SEISCOMP_ERROR("Unexpected SeedLink4 payload subformat: %c", head->subformat);
				_sock.close();
				break;
			}

			if ( head->payloadLength > MAXPAYLOAD ) {
				SEISCOMP_ERROR("Unexpected SeedLink4 payload length: %u", head->payloadLength);
				_sock.close();
				break;
			}

			while ( _slrecord.length() < HEADSIZE + head->stationIdLength + head->payloadLength) {

				_slrecord += _sock.read(HEADSIZE + head->stationIdLength + head->payloadLength - _slrecord.length());
				head = reinterpret_cast<const SL4FixedHeader *>(_slrecord.data());
			}

			char *payload = _slrecord.data() + HEADSIZE + head->stationIdLength;

			if ( head->format == '2' && !MSEED::V2::isValidHeader(payload) ) {
				SEISCOMP_WARNING("Invalid MSEED2 record received (header check failed)");
				continue;
			}

			if ( head->format == '3' && !MSEED::V3::isValidHeader(payload) ) {
				SEISCOMP_WARNING("Invalid MSEED3 record received (header check failed)");
				continue;
			}

			istream stream(&_streambuf);
			stream.clear();
			stream.rdbuf()->pubsetbuf(payload, head->payloadLength);

			auto *rec = new IO::MSeedRecord();
			setupRecord(rec);
			try {
				rec->read(stream);
				if ( (rec->samplingFrequency() == 0.0) && !rec->sampleCount() ) {
					delete rec;
					continue;
				}

				return rec;
			}
			catch ( ... ) {
				SEISCOMP_WARNING("Could not parse the incoming MiniSEED record. Ignore it.");
				delete rec;
				continue;
			}
		}
		catch ( SocketException &ex ) {
			SEISCOMP_ERROR("SocketException: %s",ex.what());

			if ( _sock.isInterrupted() ) {
				_sock.close();
				break;
			}

			if ( _retriesLeft <= 0 && _maxRetries >= 0 ) {
				break;
			}

			if ( !inReconnect ) {
				SEISCOMP_ERROR("Connection or handshake with %s failed. "
				               "Trying to reconnect every 0.5 seconds",
				               _serverloc.c_str());
			}

			inReconnect = true;

			/* sleep before reconnect */
			Core::msleep(500);

			if ( _sock.isInterrupted() ) {
				_sock.close();
				break;
			}

			reconnect();

			continue;
		}
		catch ( GeneralException & ) {
			_sock.close();
			break;
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
