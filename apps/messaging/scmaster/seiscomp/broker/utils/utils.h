/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#ifndef GEMPA_MESSAGESERVER_UTILS_H__
#define GEMPA_MESSAGESERVER_UTILS_H__


#include <seiscomp/core/exceptions.h>
#include <seiscomp/broker/api.h>

#include <vector>
#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <boost/noncopyable.hpp>


namespace Seiscomp {
namespace Utils {


/**
 * @brief The Randomizer class generated random data of arbitrary length.
 *
 * This class utilized /dev/urandom under Unix. Other operating systems are
 * not yet supported. Randomizer is implemented as a singleton. The usage
 * is as simple as:
 *
 * \code
 * if ( !Randomizer::Instance().fillData(data, len) )
 *     cerr << "Failed to generate random data" << endl;
 * \endcode
 *
 * A helper template method Randomizer::fill is provided which takes an
 * argument of arbitrary type and fills it with random data.
 *
 * \code
 * int id;
 * if ( !Randomizer::Instance().fill(id) )
 *     cerr << "Failed to generate id" << endl;
 * \endcode
 */
class SC_BROKER_API Randomizer {
	// ----------------------------------------------------------------------
	//  Destruction
	// ----------------------------------------------------------------------
	public:
		//! D'tor
		~Randomizer();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns the singleton instance.
		 * @return The singleton instance
		 */
		static Randomizer &Instance() { return _instance; }

		/**
		 * @brief Fills a value with random data.
		 * @param target The value to be filled.
		 * @return true on success, false otherwise
		 */
		template <typename T>
		bool fill(T &target);

		/**
		 * @brief Fills a block of data with random data
		 * @param data The pointer to the memory block
		 * @param len The length in bytes of the memory block
		 * @return true on success, false otherwise
		 */
		bool fillData(void *data, size_t len);


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		//! Private constructor
		Randomizer();


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		static Randomizer _instance;
		FILE *_randomFd;
};


template <typename T>
bool Randomizer::fill(T &target) {
	return fillData(&target, sizeof(target));
}


template <typename T>
class BlockingDequeue : private boost::noncopyable {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef std::unique_lock<std::mutex> lock;

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		BlockingDequeue();
		BlockingDequeue(int n);
		~BlockingDequeue();


	// ----------------------------------------------------------------------
	//  Blocking interface
	// ----------------------------------------------------------------------
	public:
		void resize(int n);

		bool canPush() const;
		bool push(T v);

		bool canPop() const;
		T pop();

		bool pop(T &);

		void close();
		void reopen();

		size_t size() const;

		void lockBuffer();
		void unlockBuffer();

		//! Requires lockBuffer to be called
		size_t buffered() const;

		//! Requires lockBuffer to be called
		T &operator[](size_t idx);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		volatile int            _begin, _end;
		volatile size_t         _buffered;
		volatile bool           _closed;
		std::vector<T>          _buffer;
		std::condition_variable _notFull, _notEmpty;
		mutable std::mutex      _monitor;
};



template <typename T, int IsPtr>
struct BlockingDequeueHelper {};

template <typename T>
struct BlockingDequeueHelper<T,0> {
	static void clean(const std::vector<T> &) {}
	static T defaultValue() { return T(); }
};

template <typename T>
struct BlockingDequeueHelper<T,1> {
	static void clean(const std::vector<T> &b) {
		for ( size_t i = 0; i < b.size(); ++i ) {
			if ( b[i] ) delete b[i];
		}
	}

	static T defaultValue() { return NULL; }
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
BlockingDequeue<T>::BlockingDequeue() :
	_begin(0), _end(0),
	_buffered(0), _closed(false), _buffer(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
BlockingDequeue<T>::BlockingDequeue(int n) :
	_begin(0), _end(0),
	_buffered(0), _closed(false), _buffer(n)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
BlockingDequeue<T>::~BlockingDequeue() {
	close();
	BlockingDequeueHelper<T, std::is_pointer<T>::value>::clean(_buffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BlockingDequeue<T>::resize(int n) {
	lock lk(_monitor);
	_buffer.resize(n);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool BlockingDequeue<T>::canPush() const {
	lock lk(_monitor);

	if ( _closed )
		throw Core::GeneralException("Queue has been closed");

	return _buffered < _buffer.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool BlockingDequeue<T>::push(T v) {
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
bool BlockingDequeue<T>::canPop() const {
	lock lk(_monitor);

	if ( _closed )
		throw Core::GeneralException("Queue has been closed");

	return _buffered > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
T BlockingDequeue<T>::pop() {
	lock lk(_monitor);
	while (_buffered == 0 && !_closed) {
		_notEmpty.wait(lk);
	}
	if ( _closed )
		throw Core::GeneralException("Queue has been closed");
	T v = _buffer[_begin];
	_buffer[_begin] = BlockingDequeueHelper<T, std::is_pointer<T>::value>::defaultValue();
	_begin = (_begin+1) % _buffer.size();
	--_buffered;
	_notFull.notify_all();
	return v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
bool BlockingDequeue<T>::pop(T &v) {
	lock lk(_monitor);

	if ( _closed )
		throw Core::GeneralException("Queue has been closed");

	if ( _buffered > 0 ) {
		v = _buffer[_begin];
		_buffer[_begin] = BlockingDequeueHelper<T, std::is_pointer<T>::value>::defaultValue();
		_begin = (_begin+1) % _buffer.size();
		--_buffered;
		_notFull.notify_all();
		return true;
	}
	else
		return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BlockingDequeue<T>::close() {
	lock lk(_monitor);
	if ( _closed ) return;
	_closed = true;
	_notFull.notify_all();
	_notEmpty.notify_all();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BlockingDequeue<T>::reopen() {
	lock lk(_monitor);
	_closed = false;
	if ( !_buffered )
		_notFull.notify_all();
	else
		_notEmpty.notify_all();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
size_t BlockingDequeue<T>::size() const {
	lock lk(_monitor);
	return _buffered;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BlockingDequeue<T>::lockBuffer() {
	_monitor.lock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void BlockingDequeue<T>::unlockBuffer() {
	_monitor.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
size_t BlockingDequeue<T>::buffered() const {
	return _buffered;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
T &BlockingDequeue<T>::operator[](size_t idx) {
	idx += _begin;
	if ( idx >= _buffer.size() )
		idx -= _buffer.size();

	return _buffer[idx];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}


#endif
