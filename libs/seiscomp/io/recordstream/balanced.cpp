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


#define SEISCOMP_COMPONENT BalancedConnection

#include <cstdio>
#include <string>
#include <iostream>
#include <functional>

#include <seiscomp/logging/log.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/recordinput.h>

#include "balanced.h"

#include <seiscomp/client/queue.ipp>

namespace Seiscomp {
namespace RecordStream {
namespace Balanced {
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


IMPLEMENT_SC_CLASS_DERIVED(BalancedConnection, Seiscomp::IO::RecordStream,
                           "BalancedConnection");

REGISTER_RECORDSTREAM(BalancedConnection, "balanced");

BalancedConnection::BalancedConnection(): _started(false), _nthreads(0), _queue(QueueSize),
	_stream(istringstream::in|istringstream::binary) {
}

BalancedConnection::BalancedConnection(string serverloc): _started(false), _nthreads(0), _queue(QueueSize),
	_stream(istringstream::in|istringstream::binary) {
	setSource(serverloc);
}

BalancedConnection::~BalancedConnection() {
	close();
}

bool BalancedConnection::setSource(const string &source) {
	if ( _started )
		return false;

	_rsarray.clear();

	size_t p1,p2;

	/*
	 * Format of source is:
	 *  type1/source1;type2/source2;...;typeN/sourceN
	 * where
	 *  sourceN is either source or (source)
	 */

	string serverloc = source;

	while (true) {
		// Find first slash
		p1 = serverloc.find('/');
		string type1;

		if ( p1 == string::npos ) {
			type1 = "slink";
			p1 = 0;
		}
		else {
			type1 = serverloc.substr(0, p1);
			// Move behind '/'
			++p1;
		}

		string source1;

		// Extract source1
		if ( p1 >= serverloc.size() ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': missing source",
				       serverloc.c_str());
			throw RecordStreamException("Invalid RecordStream URL");
		}

		// Source surrounded by parentheses
		if ( serverloc[p1] == '(' ) {
			++p1;
			// Find closing parenthesis
			p2 = findClosingParenthesis(serverloc, p1);
			if ( p2 == string::npos ) {
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected closing parenthesis",
					       serverloc.c_str());
				throw RecordStreamException("Invalid RecordStream URL");
			}

			source1 = serverloc.substr(p1, p2-p1);
			++p2;
		}
		else {
			p2 = serverloc.find(';', p1);
			if ( p2 == string::npos ) {
				p2 = serverloc.length();
			}

			source1 = serverloc.substr(p1, p2-p1);
		}

		SEISCOMP_DEBUG("Type   : %s", type1.c_str());
		SEISCOMP_DEBUG("Source : %s", source1.c_str());

		RecordStreamPtr rs = RecordStream::Create(type1.c_str());

		if ( rs == nullptr ) {
			SEISCOMP_ERROR("Invalid RecordStream type: %s", type1.c_str());
			return false;
		}

		if ( !rs->setSource(source1) ) {
			SEISCOMP_ERROR("Invalid RecordStream source: %s", source1.c_str());
			return false;
		}

		_rsarray.push_back(make_pair(rs, false));

		if ( p2 == serverloc.length() )
			break;

		serverloc = serverloc.substr(p2 + 1, string::npos);
	}

	return true;
}

int BalancedConnection::streamHash(const string &sta) {
	size_t i = 0;
	for ( const char* p = sta.c_str(); *p != 0; ++p ) i += *p;

	return i % _rsarray.size();
}

bool BalancedConnection::addStream(const string &net, const string &sta,
                                   const string &loc, const string &cha) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = streamHash(sta);

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha) )
		return false;

	_rsarray[i].second = true;

	return true;
}

bool BalancedConnection::addStream(const string &net, const string &sta,
                                   const string &loc, const string &cha,
                                   const Seiscomp::Core::Time &stime,
                                   const Seiscomp::Core::Time &etime) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = streamHash(sta);

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha, stime, etime) )
		return false;

	_rsarray[i].second = true;

	return true;
}

bool BalancedConnection::setStartTime(const Seiscomp::Core::Time &stime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setStartTime(stime) )
			return false;
	}

	return true;
}

bool BalancedConnection::setEndTime(const Seiscomp::Core::Time &etime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setEndTime(etime) )
			return false;
	}

	return true;
}

bool BalancedConnection::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeWindow(w) )
			return false;
	}

	return true;
}

bool BalancedConnection::setRecordType(const char* type) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setRecordType(type) )
			return false;
	}

	return true;
}

bool BalancedConnection::setTimeout(int seconds) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeout(seconds) )
			return false;
	}

	return true;
}

void BalancedConnection::close() {
	lock_guard<mutex> lock(_mtx);

	if ( _rsarray.empty() )
		return;

	for ( size_t i = 0; i < _rsarray.size(); ++i)
		_rsarray[i].first->close();

	_rsarray.clear();

	_queue.close();

	for ( auto &&thread : _threads )
		thread->join();

	_threads.clear();
	_started = false;
}

void BalancedConnection::acquiThread(RecordStream* rs) {
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

Record *BalancedConnection::next() {
	lock_guard<mutex> lock(_mtx);
	if ( !_started ) {
		_started = true;

		for ( size_t i = 0; i < _rsarray.size(); ++i) {
			if ( _rsarray[i].second ) {
				_rsarray[i].first->setDataType(_dataType);
				_rsarray[i].first->setDataHint(_hint);
				_threads.push_back(new thread(std::bind(&BalancedConnection::acquiThread, this, _rsarray[i].first.get())));
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
} // namesapce Balanced
} // namespace RecordStream
} // namespace Seiscomp

