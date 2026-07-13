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


#define SEISCOMP_COMPONENT FDSNWSConnection

#include <cstdlib>
#include <string>
#include <set>
#include <utility>

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/io/records/mseedrecord.h>

#include "fdsnws.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
using namespace Seiscomp::RecordStream;


REGISTER_RECORDSTREAM(FDSNWSConnection, "fdsnws");
REGISTER_RECORDSTREAM(FDSNWSSSLConnection, "fdsnwss");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSConnectionBase::FDSNWSConnectionBase(const char *protocol, int defaultPort)
: IO::HTTPClient()
, _protocol(protocol)
, _defaultPort(defaultPort)
, _readingData(false)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setSource(const std::string &source) {
	// IO::RecordStream entry point. All URL normalisation (scheme translation, default
	// path) lives in setURL() so the rules are the same whether the URL came from the
	// user or from an internal redirect.
	return setURL(source);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setURL(const std::string &url) {
	// Translate the FDSNWS-specific URL schemes to the plain HTTP equivalents
	// understood by IO::HTTPClient before forwarding the URL. If no scheme is given at
	// all, apply this connection's configured default (set by the subclass).
	std::string rewritten = url;
	const auto schemeSep = rewritten.find("://");
	if ( schemeSep == string::npos ) {
		rewritten = std::string(_protocol) + "://" + rewritten;
	}
	else {
		std::string scheme = rewritten.substr(0, schemeSep);
		if ( scheme == "fdsnws" ) {
			rewritten.replace(0, schemeSep, "http");
		}
		else if ( scheme == "fdsnwss" ) {
			rewritten.replace(0, schemeSep, "https");
		}
		// Any other scheme (http, https) is passed through unchanged so that cross-host
		// redirects with a "real" scheme continue to work via the base class.
	}

	if ( !HTTPClient::setURL(rewritten) ) {
		SEISCOMP_ERROR("Invalid FDSNWS URL: %s", _url.status().toString());
		return false;
	}

	if ( _url.path().empty() ) {
		if ( !_url.username().empty() ) {
			_url.setPath("/fdsnws/dataselect/1/queryauth");
		}
		else {
			_url.setPath("/fdsnws/dataselect/1/query");
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setRecordType(const char* type) {
	return !strcmp(type, "mseed");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::addStream(const string &networkCode,
                                     const string &stationCode,
                                     const string &locationCode,
                                     const string &channelCode) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(networkCode, stationCode,
	                                   locationCode, channelCode));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::addStream(const string &networkCode,
                                     const string &stationCode,
                                     const string &locationCode,
                                     const string &channelCode,
                                     const OPT(Time) &startTime,
                                     const OPT(Time) &endTime) {
	pair<set<StreamIdx>::iterator, bool> result;
	result = _streams.insert(StreamIdx(networkCode, stationCode,
	                                   locationCode, channelCode,
	                                   startTime, endTime));
	return result.second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setStartTime(const OPT(Time) &startTime) {
	_stime = startTime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setEndTime(const OPT(Time) &endTime) {
	_etime = endTime;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::setTimeout(int seconds) {
	HTTPClient::setTimeout(seconds);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::clear() {
	// Snapshot the configuration before destruction so that the placement-new sees
	// well-defined values (rather than accessing members of an object that has just
	// been destroyed).
	const char *protocol = _protocol;
	int defaultPort = _defaultPort;
	std::string url = _url.toString();
	this->~FDSNWSConnectionBase();
	new(this) FDSNWSConnectionBase(protocol, defaultPort);
	setSource(url);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Hopefully safe to be called from another thread
void FDSNWSConnectionBase::close() {
	HTTPClient::close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSConnectionBase::reconnect() {
	HTTPClient::reset();
	_readingData = false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string FDSNWSConnectionBase::createPostData() {
	string request;
	string lineSeparator;

	auto it = _url.queryItems().find("crlf");
	if ( (it != _url.queryItems().end())
	     && (it->second.empty() || it->second == "true") ) {
		lineSeparator = "\r\n";
	}
	else {
		lineSeparator = "\n";
	}

	for ( auto it = _streams.begin(); it != _streams.end(); ++it ) {
		if ( (!it->startTime() && !_stime) || (!it->endTime() && !_etime) ) {
			/* invalid time window ignore stream */
			SEISCOMP_WARNING("Ignoring request with invalid time window: %s",
			                 it->str(_stime, _etime));
			continue;
		}

		request += it->network() + " " + it->station() + " ";
		if ( it->location().empty() ) {
			request += "--";
		}
		else {
			request += it->location();
		}

		request += " ";
		request += it->channel();
		request += " ";

		if ( it->startTime() ) {
			request += it->startTime()->toString("%FT%T.%f");
		}
		else {
			request += (_stime ? *_stime : Time()).toString("%FT%T.%f");
		}

		request += " ";

		if ( it->endTime() ) {
			request += it->endTime()->toString("%FT%T.%f");
		}
		else {
			request += (_etime ? *_etime : Time()).toString("%FT%T.%f");
		}

		request += lineSeparator;
	}

	return request;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *FDSNWSConnectionBase::next() {
	// On entry, _socket may be null (no request issued yet) or may already point at an
	// open or closed socket. The reading-loop invariant 'data has been requested' is
	// captured by _readingData.
	if ( _readingData && (!_socket || !_socket->isOpen()) ) {
		return nullptr;
	}

	if ( _socket ) {
		_socket->startTimer();
	}

	if ( !_readingData ) {
		try {
			// The HTTPClient API takes the URL as a parameter to post(); the socket is
			// created lazily and the connection is opened (or reused) by
			// prepareConnection().
			post(_url.toString(), createPostData());
		}
		catch ( const GeneralException &e ) {
			SEISCOMP_ERROR("fdsnws: %s", e.what());
			if ( _socket ) {
				_socket->close();
			}
			return nullptr;
		}

		_readingData = true;
		if ( !_chunkMode && _remainingBytes <= 0 ) {
			SEISCOMP_DEBUG("Content length is 0, nothing to read");
			_socket->close();
			return nullptr;
		}
	}

	try {
		while ( true ) {
			_socket->startTimer();

			if ( _error.empty() ) {
				// HACK to retrieve the record length
				string data = readBinary(RECSIZE);
				if ( !data.empty() ) {
					auto reclen = IO::MSeedRecord::Detect(data.data(), RECSIZE);
					std::istringstream stream(std::istringstream::in|std::istringstream::binary);
					if ( reclen > RECSIZE ) {
						stream.str(data + readBinary(reclen - RECSIZE));
					}
					else {
						if ( reclen <= 0 ) {
							SEISCOMP_ERROR("Retrieving the record length failed (try 512 Byte)!");
						}
						stream.str(data);
					}

					auto *rec = new IO::MSeedRecord;
					setupRecord(rec);
					try {
						rec->read(stream);
					}
					catch ( ... ) {
						delete rec;
						continue;
					}

					return rec;
				}

				_socket->close();
				break;
			}

			_error += readBinary(_chunkMode?512:_remainingBytes);

			if ( !_socket->isOpen() && !_error.empty() ) {
				throw GeneralException(_error);
			}
		}
	}
	catch ( const GeneralException &e ) {
		SEISCOMP_ERROR("fdsnws: %s", e.what());
		_socket->close();
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSConnection::FDSNWSConnection()
: FDSNWSConnectionBase("http", 80) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSSSLConnection::FDSNWSSSLConnection()
: FDSNWSConnectionBase("https", 443) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
