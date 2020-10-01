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


#ifndef SEISCOMP_CLIENT_QUEUE_H
#define SEISCOMP_CLIENT_QUEUE_H


#include <vector>
#include <condition_variable>
#include <thread>

#include <seiscomp/core/baseobject.h>


namespace Seiscomp {
namespace Client {

class QueueClosedException : public Core::GeneralException {
	public:
		QueueClosedException() : Core::GeneralException("Queue has been closed") {}
		QueueClosedException(const std::string& str ) : Core::GeneralException(str) {}
};

template <typename T>
class ThreadedQueue {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef std::unique_lock<std::mutex> lock;


	// ----------------------------------------------------------------------
	//  Non copyable
	// ----------------------------------------------------------------------
	private:
		ThreadedQueue(const ThreadedQueue&) = delete;
		ThreadedQueue &operator=(const ThreadedQueue&) = delete;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		ThreadedQueue();
		ThreadedQueue(int n);
		~ThreadedQueue();


	// ----------------------------------------------------------------------
	//  Interface
	// ----------------------------------------------------------------------
	public:
		void resize(int n);

		bool canPush() const;
		bool push(T v);

		bool canPop() const;
		T pop();

		void close();
		
		size_t size() const;


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


}
}


#endif
