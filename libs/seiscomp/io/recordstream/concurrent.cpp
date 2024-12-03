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


#define SEISCOMP_COMPONENT Concurrent


#include "concurrent.h"

#include <seiscomp/logging/log.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/recordinput.h>
#include <seiscomp/client/queue.ipp>

#include <cstdio>
#include <string>
#include <iostream>
#include <functional>


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;


namespace Seiscomp {
namespace RecordStream {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Concurrent::Concurrent() : _queue(1024) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Concurrent::~Concurrent() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Concurrent::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = getRS(net, sta, loc, cha);

	if ( i < 0 )
		return false;

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha) )
		return false;

	_rsarray[i].second = true;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Concurrent::addStream(const string &net, const string &sta,
                           const string &loc, const string &cha,
                           const OPT(Seiscomp::Core::Time) &stime,
                           const OPT(Seiscomp::Core::Time) &etime) {
	SEISCOMP_DEBUG("add stream %s.%s.%s.%s", net.c_str(),
	               sta.c_str(), loc.c_str(), cha.c_str());

	if ( _rsarray.empty() )
		return false;

	int i = getRS(net, sta, loc, cha);

	if ( i < 0 )
		return false;

	if ( !_rsarray[i].first->addStream(net, sta, loc, cha, stime, etime) )
		return false;

	_rsarray[i].second = true;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Concurrent::setStartTime(const OPT(Seiscomp::Core::Time) &stime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setStartTime(stime) )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Concurrent::setEndTime(const OPT(Seiscomp::Core::Time) &etime) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setEndTime(etime) )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Concurrent::setTimeWindow(const Seiscomp::Core::TimeWindow &w) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeWindow(w) )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Concurrent::setRecordType(const char *type) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setRecordType(type) )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Concurrent::setTimeout(int seconds) {
	if ( _rsarray.empty() )
		return false;

	for ( size_t i = 0; i < _rsarray.size(); ++i) {
		if ( !_rsarray[i].first->setTimeout(seconds) )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Concurrent::close() {
	lock_guard<mutex> lock(_mtx);

	if ( !_started ) {
		SEISCOMP_DEBUG("Closing without being started");
		return;
	}

	_queue.close();

	for ( size_t i = 0; i < _rsarray.size(); ++i ) {
		_rsarray[i].first->close();
	}

	for ( auto &thread : _threads ) {
		if ( thread.joinable() ) {
			thread.join();
		}
	}

	SEISCOMP_DEBUG("All acquisition threads finished");

	_threads.clear();
	_rsarray.clear();

	_started = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Concurrent::acquiThread(RecordStream *rs) {
	SEISCOMP_DEBUG("Starting acquisition thread");

	Record *rec;

	try {
		while ( (rec = rs->next()) ) {
			_queue.push(rec);
		}
	}
	catch ( OperationInterrupted &e ) {
		SEISCOMP_DEBUG("Interrupted acquisition thread, msg: '%s'", e.what());
	}
	catch ( exception &e ) {
		SEISCOMP_ERROR("Exception in acquisition thread: '%s'", e.what());
	}

	SEISCOMP_DEBUG("Finished acquisition thread");

	_queue.push(nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record *Concurrent::next() {
	{
		lock_guard<mutex> lock(_mtx);

		if ( !_started ) {
			_started = true;

			for ( size_t i = 0; i < _rsarray.size(); ++i) {
				if ( _rsarray[i].second && !_queue.isClosed() ) {
					++_nthreads;
					_rsarray[i].first->setDataType(_dataType);
					_rsarray[i].first->setDataHint(_hint);
					_threads.push_back(
						thread(
							bind(
								&Concurrent::acquiThread,
								this,
								_rsarray[i].first.get()
							)
						)
					);
				}
			}

			if ( _threads.empty() ) {
				return nullptr;
			}
		}
	}

	try {
		while ( true ) {
			Record * rec =  _queue.pop();
			if ( rec) {
				return rec;
			}
			else {
				// Null record received ... a thread finished
				lock_guard<mutex> lock(_mtx);
				if ( --_nthreads ) {
					// Still threads running ... keep on reading the queue
					continue;
				}

				SEISCOMP_DEBUG("Last acquisition thread terminated");
				break;
			}
		}

		if ( _queue.size() > 0 ) {
			SEISCOMP_ERROR("Finished acquisition, but data queue not empty "
			               "(%zu items still)", _queue.size());
		}

		SEISCOMP_DEBUG("Closing record queue");
		_queue.close();
	}
	catch ( Client::QueueClosedException & ) {
		// Since the queue was closed,it doesn't matter if there are still items
		// in the queue, we drop them
		SEISCOMP_DEBUG("Queue closed, break streaming");
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Concurrent::reset() {
	_queue.reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace RecordStream
} // namespace Seiscomp
