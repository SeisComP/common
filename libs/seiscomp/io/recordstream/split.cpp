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


#define SEISCOMP_COMPONENT SplitConnection

#include <cstdio>
#include <string>
#include <regex>
#include <iostream>
#include <functional>

#include <seiscomp/logging/log.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/recordinput.h>

#include "split.h"

#include <seiscomp/client/queue.ipp>

namespace Seiscomp {
namespace RecordStream {
namespace Split {
namespace _private {

using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;

const size_t QueueSize = 1024;


namespace {


size_t findClosingParenthesis(const string &s, size_t p) {
	int cnt = 1;
	for ( size_t i = p; i < s.size(); ++i ) {
		if ( s[i] == '(' ) ++cnt;
		else if ( s[i] == ')' ) --cnt;
		if ( !cnt ) return i;
	}

	return string::npos;
}

}


IMPLEMENT_SC_CLASS_DERIVED(SplitConnection, Seiscomp::IO::RecordStream,
                           "SplitConnection");

REGISTER_RECORDSTREAM(SplitConnection, "split");

SplitConnection::SplitConnection(): _started(false), _nthreads(0), _queue(QueueSize),
	_stream(istringstream::in|istringstream::binary) {
}

SplitConnection::SplitConnection(string serverloc): _started(false), _nthreads(0), _queue(QueueSize),
	_stream(istringstream::in|istringstream::binary) {
	setSource(serverloc);
}

SplitConnection::~SplitConnection() {
	close();
}

bool SplitConnection::setSource(const string &source) {
	if ( _started )
		return false;

	_rsarray.clear();

	/*
	 * Format of source is:
	 *  type1/source1??match=pattern;type2/source2??match=pattern;...;typeN/sourceN??match=pattern
	 * where
	 *  sourceN is either source or (source)
	 *  pattern is NET.STA.LOC.CHA and the special charactes ? * | ( ) are allowed
	 */
	string input = source;

	do {

		// Extract type1: anything until '/' (not greedy)
		static const regex re1("^(.+?)/",regex::optimize);
		smatch m1;
		if ( ! regex_search(input, m1, re1) ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': cannot find service type in '%s'",
			                source.c_str(), input.c_str());
			throw RecordStreamException("Invalid RecordStream URL"); 
		}
		const string type1 = m1[1].str();

		input = input.substr( m1.length() );

		// Extract source1: anything until ?? (not greedy)
		string source1;
		if ( input[0] != '(' ) {
			static const regex re2( R"(^(.+?)\?\?)",regex::optimize);
			smatch m2;
			if ( ! regex_search(input, m2, re2) ) {
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': cannot find source in '%s'",
				                source.c_str(), input.c_str());
				throw RecordStreamException("Invalid RecordStream URL"); 
			}
			source1 = m2[1].str();
			input = input.substr( m2.length() );

		} else { // Source surrounded by parentheses
			// Find closing parenthesis
			size_t p2 = findClosingParenthesis(input, 1);
			if ( p2 == string::npos ) {
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected closing parenthesis in '%s'",
				               source.c_str(), input.c_str());
				throw RecordStreamException("Invalid RecordStream URL");
			}
			source1 = input.substr(1, p2-1);
			if ( input.substr(p2, 3) != ")??" ){
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected ?? after closing parenthesis in '%s'",
				               source.c_str(), input.c_str());
				throw RecordStreamException("Invalid RecordStream URL");
			} 
			input = input.substr(p2+3);
		}

		// Extract matching rule
		static const regex re3( R"(^match=([A-Z|a-z|0-9|\?|\*|\||\(|\)]+\.)" 
		                                R"([A-Z|a-z|0-9|\?|\*|\||\(|\)]+\.)"
		                                R"([A-Z|a-z|0-9|\?|\*|\||\(|\)]+\.)"
		                                R"([A-Z|a-z|0-9|\?|\*|\||\(|\)]+)"
		                                 ");?", regex::optimize);
		smatch m3;
		if ( ! regex_search(input, m3, re3) ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': cannot find match option in '%s'",
			               source.c_str(), input.c_str());
			throw RecordStreamException("Invalid RecordStream URL"); 
		}
		const string match1 = m3[1].str();

		input = input.substr( m3.length() );

		// convert user special characters (* ? .) to regex equivalent
		const string reMatch = regex_replace( regex_replace( regex_replace(match1,
		                       regex("\\."), string("\\.")),
		                       regex("\\?"), string(".")),
		                       regex("\\*"), string(".*") );

		SEISCOMP_DEBUG("Type   : %s", type1.c_str());
		SEISCOMP_DEBUG("Source : %s", source1.c_str());
		SEISCOMP_DEBUG("Match  : %s (regex %s)", match1.c_str(), reMatch.c_str());

		RecordStreamPtr rs = RecordStream::Create(type1.c_str());

		if ( rs == nullptr ) {
			SEISCOMP_ERROR("Invalid RecordStream type: %s", type1.c_str());
			return false;
		}

		if ( !rs->setSource(source1) ) {
			SEISCOMP_ERROR("Invalid RecordStream source: %s", source1.c_str());
			return false;
		}

		try {
			regex re = regex(reMatch, regex::optimize);
			_rsarray.push_back(make_pair(rs, StreamConf{false, type1, source1, match1, re}));
		} catch (const regex_error &e) {
			SEISCOMP_ERROR("Invalid match syntax in RecordStream URL: %s", source.c_str());
			throw RecordStreamException("Invalid RecordStream URL"); 
		}

	} while ( !input.empty() );

	return true;
}

int SplitConnection::streamMatch(const string &net, const string &sta,
                                   const string &loc, const string &cha) {

	const string stream = net + "." + sta + "." + loc +  "." + cha;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {

		if ( regex_match(stream, _rsarray[i].second.match) ) {

			SEISCOMP_DEBUG("stream %s -> recordstream %lu (%s %s %s)", stream.c_str(), i,
			               _rsarray[i].second.type.c_str(), _rsarray[i].second.source.c_str(), 
			               _rsarray[i].second.matchOpt.c_str());
			return i;
		}

	}

	SEISCOMP_DEBUG("stream %s doesn't match any RecordStream", stream.c_str());
	return -1;
}

bool SplitConnection::addStream(const string &net, const string &sta,
                                   const string &loc, const string &cha) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = streamMatch(net, sta, loc, cha);

	if (i < 0) return false;

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha) )
		return false;

	_rsarray[i].second.active = true;

	return true;
}

bool SplitConnection::addStream(const string &net, const string &sta,
                                   const string &loc, const string &cha,
                                   const Seiscomp::Core::Time &stime,
                                   const Seiscomp::Core::Time &etime) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = streamMatch(net, sta, loc, cha);

	if (i < 0) return false;

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha, stime, etime) )
		return false;

	_rsarray[i].second.active = true;

	return true;
}

bool SplitConnection::setStartTime(const Seiscomp::Core::Time &stime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setStartTime(stime) )
			return false;
	}

	return true;
}

bool SplitConnection::setEndTime(const Seiscomp::Core::Time &etime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setEndTime(etime) )
			return false;
	}

	return true;
}

bool SplitConnection::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeWindow(w) )
			return false;
	}

	return true;
}

bool SplitConnection::setRecordType(const char* type) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setRecordType(type) )
			return false;
	}

	return true;
}

bool SplitConnection::setTimeout(int seconds) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeout(seconds) )
			return false;
	}

	return true;
}

void SplitConnection::close() {
	lock_guard<mutex> lock(_mtx);

	if ( _rsarray.empty() )
		return;

	for ( size_t i = 0; i < _rsarray.size(); ++i)
		_rsarray[i].first->close();

	_queue.close();

	for ( auto &&thread : _threads )
		thread->join();

	_threads.clear();
	_started = false;
}

void SplitConnection::acquiThread(RecordStreamPtr rs) {
	SEISCOMP_DEBUG("Starting acquisition thread");

	Record *rec;
	try {
		while ( (rec = rs->next()) )
			_queue.push(rec);
	}
	catch ( OperationInterrupted &e ) {
		SEISCOMP_DEBUG("Interrupted acquisition thread, msg: '%s'", e.what());
	}
	catch ( exception& e ) {
		SEISCOMP_ERROR("Exception in acquisition thread: '%s'", e.what());
	}

	SEISCOMP_DEBUG("Finished acquisition thread");

	_queue.push(nullptr);
}

Record *SplitConnection::next() {
	if ( !_started ) {
		_started = true;

		for ( size_t i = 0; i < _rsarray.size(); ++i) {
			if ( _rsarray[i].second.active ) {
				_rsarray[i].first->setDataType(_dataType);
				_rsarray[i].first->setDataHint(_hint);
				_threads.push_back(new thread(std::bind(&SplitConnection::acquiThread, this, _rsarray[i].first)));
				++_nthreads;
			}
		}
	}

	while (_nthreads > 0) {
		Record *rec = _queue.pop();

		if ( rec == nullptr ) {
			--_nthreads;
			continue;
		}

		return rec;
	}

	SEISCOMP_DEBUG("All acquisition threads finished -> finish iteration");

	return nullptr;
}


} // namespace _private
} // namesapce Split
} // namespace RecordStream
} // namespace Seiscomp

