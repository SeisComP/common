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
		/**
		 * @brief Resizes the queue to hold a maximum of n items before
		 *        blocking.
		 * @param n The number of items to queue before blocking occurs.
		 */
		void resize(int n);

		/**
		 * @brief Checks whether the queue can take new items without blocking.
		 * @return true if non-blocking push is possible, false otherwise.
		 */
		bool canPush() const;

		/**
		 * @brief Appends a new item to the end of the queue. If the queue is
		 *        full then it will block until a consumer has popped an item.
		 * @param v The new item.
		 * @return true if successful, false if queue is closed.
		 */
		bool push(T v);

		/**
		 * @brief Checks with equality operator if the item is already queued
		 *        and if not, pushes it to the end of the queue.
		 * @param v The new item.
		 * @return true if successful which also covers the case that the item
		 *         is already queued. False if the queue is closed.
		 */
		bool pushUnique(T v);

		/**
		 * @brief Checks whether an item can be popped or not.
		 * Actually it returns whether the queue is empty or not.
		 * @return true if not empty, false if empty.
		 */
		bool canPop() const;

		/**
		 * @brief Pops an items from the queue. If the queue is empty then
		 *        it blocks until a producer pushed an item.
		 * @return The popped item.
		 */
		T pop();

		/**
		 * @brief Close the queue and cause all subsequent calls to push and
		 *        pop to fail.
		 */
		void close();

		/**
		 * @brief Returns whether the queue is closed or not.
		 * @return The closed flag.
		 */
		bool isClosed() const;

		/**
		 * @brief Query the number of queued items.
		 * @return The number of currently queued items.
		 */
		size_t size() const;

		/**
		 * @brief Resets the queue which incorporates resetting the buffer
		 *        insertations and the closed state.
		 */
		void reset();


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
