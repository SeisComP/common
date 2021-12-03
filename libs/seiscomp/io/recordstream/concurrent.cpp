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


#define SEISCOMP_COMPONENT ConcurrentConnection

#include <cstdio>
#include <string>
#include <iostream>
#include <functional>

#include <seiscomp/logging/log.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/recordinput.h>

#include "concurrent.h"

#include <seiscomp/client/queue.ipp>

namespace Seiscomp {
namespace RecordStream {
namespace Concurrent {
namespace _private {

using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;

const size_t QueueSize = 1024;

IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(ConcurrentConnection, Seiscomp::IO::RecordStream,
                                    "ConcurrentConnection");

ConcurrentConnection::ConcurrentConnection(): _started(false), _nthreads(0), _queue(QueueSize),
	_stream(istringstream::in|istringstream::binary) {
}

ConcurrentConnection::~ConcurrentConnection() {
	close();
}

bool ConcurrentConnection::addStream(const string &net, const string &sta,
                                   const string &loc, const string &cha) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = getRS(net, sta, loc, cha);

	if (i < 0) return false;

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha) )
		return false;

	_rsarray[i].second = true;

	return true;
}

bool ConcurrentConnection::addStream(const string &net, const string &sta,
                                   const string &loc, const string &cha,
                                   const Seiscomp::Core::Time &stime,
                                   const Seiscomp::Core::Time &etime) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = getRS(net, sta, loc, cha);

	if (i < 0) return false;

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha, stime, etime) )
		return false;

	_rsarray[i].second = true;

	return true;
}

bool ConcurrentConnection::setStartTime(const Seiscomp::Core::Time &stime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setStartTime(stime) )
			return false;
	}

	return true;
}

bool ConcurrentConnection::setEndTime(const Seiscomp::Core::Time &etime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setEndTime(etime) )
			return false;
	}

	return true;
}

bool ConcurrentConnection::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeWindow(w) )
			return false;
	}

	return true;
}

bool ConcurrentConnection::setRecordType(const char* type) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setRecordType(type) )
			return false;
	}

	return true;
}

bool ConcurrentConnection::setTimeout(int seconds) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeout(seconds) )
			return false;
	}

	return true;
}

void ConcurrentConnection::close() {
	lock_guard<mutex> lock(_mtx);

	if ( _rsarray.empty() )
		return;

	for ( size_t i = 0; i < _rsarray.size(); ++i)
		_rsarray[i].first->close();

	_rsarray.clear();

	_queue.close();

	for ( auto &&thread : _threads )
		thread.join();

	_threads.clear();
	_started = false;
}

void ConcurrentConnection::acquiThread(RecordStream* rs) {
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

Record *ConcurrentConnection::next() {
	lock_guard<mutex> lock(_mtx);

	if ( !_started ) {
		_started = true;

		for ( size_t i = 0; i < _rsarray.size(); ++i) {
			if ( _rsarray[i].second ) {
				_rsarray[i].first->setDataType(_dataType);
				_rsarray[i].first->setDataHint(_hint);
				_threads.push_back(thread(std::bind(&ConcurrentConnection::acquiThread, this, _rsarray[i].first.get())));
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
} // namesapce Concurrent
} // namespace RecordStream
} // namespace Seiscomp

