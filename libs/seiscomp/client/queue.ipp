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

#ifndef SEISCOMP_CLIENT_QUEUE_IPP
#define SEISCOMP_CLIENT_QUEUE_IPP


#include <seiscomp/core/exceptions.h>

#include <algorithm>
#include <type_traits>


namespace Seiscomp {
namespace Client {


namespace {

template <typename T, int IsPtr>
struct QueueHelper {};

template <typename T>
struct QueueHelper<T,0> {
	static void clean(const std::vector<T> &) {}
	static T defaultValue() { return T(); }
};

template <typename T>
struct QueueHelper<T,1> {
	static void clean(const std::vector<T> &b) {
		for ( size_t i = 0; i < b.size(); ++i ) {
			if ( b[i] ) delete b[i];
		}
	}

	static T defaultValue() { return nullptr; }
};

}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
ThreadedQueue<T>::ThreadedQueue() :
	_begin(0), _end(0),
	_buffered(0), _closed(false), _buffer(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
ThreadedQueue<T>::ThreadedQueue(int n) :
	_begin(0), _end(0),
	_buffered(0), _closed(false), _buffer(n)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
ThreadedQueue<T>::~ThreadedQueue() {
	close();
	QueueHelper<T, std::is_pointer<T>::value>::clean(_buffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void ThreadedQueue<T>::resize(int n) {
	lock lk(_monitor);
	_buffer.resize(n);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool ThreadedQueue<T>::canPush() const {
	lock lk(_monitor);

	if ( _closed )
		throw QueueClosedException();

	return _buffered < _buffer.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool ThreadedQueue<T>::push(T v) {
	lock lk(_monitor);
	while (_buffered == _buffer.size() && !_closed)
		_notFull.wait(lk);
	if ( _closed ) {
		_notEmpty.notify_all();
		return false;
	}
	_buffer[_end] = v;
	_end = (_end+1) % _buffer.size();
	++_buffered;
	_notEmpty.notify_all();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool ThreadedQueue<T>::pushUnique(T v) {
	lock lk(_monitor);
	// Find existing item
	auto it = _begin;
	while ( it != _end ) {
		if ( _buffer[it] == v ) {
			return true;
		}
		it = (it + 1) % _buffer.size();
	}

	while (_buffered == _buffer.size() && !_closed)
		_notFull.wait(lk);
	if ( _closed ) {
		_notEmpty.notify_all();
		return false;
	}
	_buffer[_end] = v;
	_end = (_end+1) % _buffer.size();
	++_buffered;
	_notEmpty.notify_all();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool ThreadedQueue<T>::canPop() const {
	lock lk(_monitor);

	if ( _closed )
		throw QueueClosedException();

	return _buffered > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
T ThreadedQueue<T>::pop() {
	lock lk(_monitor);
	while (_buffered == 0 && !_closed) {
		_notEmpty.wait(lk);
	}
	if ( _closed )
		throw QueueClosedException();
	T v = _buffer[_begin];
	_buffer[_begin] = QueueHelper<T, std::is_pointer<T>::value>::defaultValue();
	_begin = (_begin+1) % _buffer.size();
	--_buffered;
	_notFull.notify_all();
	return v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void ThreadedQueue<T>::close() {
	lock lk(_monitor);
	if ( _closed ) return;
	_closed = true;
	_notFull.notify_all();
	_notEmpty.notify_all();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool ThreadedQueue<T>::isClosed() const {
	lock lk(_monitor);
	return _closed;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
size_t ThreadedQueue<T>::size() const {
	lock lk(_monitor);
	return _buffered;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void ThreadedQueue<T>::reset() {
	lock lk(_monitor);
	_closed = false;
	_begin = _end = 0;
	_buffered = 0;
	QueueHelper<T, std::is_pointer<T>::value>::clean(_buffer);
	std::fill(_buffer.begin(), _buffer.end(), QueueHelper<T, std::is_pointer<T>::value>::defaultValue());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}

#endif
