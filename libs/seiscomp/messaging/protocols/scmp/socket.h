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


#ifndef SEISCOMP_CLIENT_SCMP_CONNECTION_H
#define SEISCOMP_CLIENT_SCMP_CONNECTION_H


#include <seiscomp/messaging/packet.h>
#include <seiscomp/messaging/protocol.h>

#include <seiscomp/wired/buffer.h>
#include <seiscomp/wired/devices/socket.h>

#include <boost/thread/mutex.hpp>

#include <deque>
#include <set>
#include <string>


namespace Seiscomp {
namespace Client {
namespace SCMP {


class Socket : public Protocol {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Socket();
		virtual ~Socket();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Sets whether to use an encrypted socket or not. This call
		 *        has only effect *before* calling connect. An established
		 *        connection won't be changed.
		 * @param enable true to use SSL, false to use an unencrypted socket.
		 */
		void setSSL(bool enable);

		/**
		 * @brief Sets the acknowledgement window. This window is actually
		 *        the number of messages the server will handle without
		 *        sending an acknowledgement message.
		 *        This method has to be called prior to connect to have an
		 *        affect.
		 * @param size The number of allowed unacknowledged messages. If more
		 *             messages should be sent the connection blocks and waits
		 *             for an acknowledgement. The default is 20.
		 */
		void setAckWindow(uint32_t size);

		/**
		 * @brief Closes the underlying socket and resources
		 * @return Success flag
		 */
		virtual Result close() override;

		bool erroneous() const;

		const std::string &errorMessage() const;

		const State &state() const;

		/**
		 * @brief Queries the local message outbox size. If a message is still
		 *        in the outbox does not mean that is hasn't been sent over the
		 *        wire. It just means that the server hasn't acknowledged the
		 *        message yet and that in case of a reconnect the outbox will
		 *        be sent again.
		 * @return The number of unacknowledged messages.
		 */
		virtual size_t outboxSize() const override;


	// ----------------------------------------------------------------------
	//  Private types and members
	// ----------------------------------------------------------------------
	protected:
		/**
		 * @brief Wait until the sockets becomes ready to transfer data, either
		 *        read or write depending on the current mode.
		 * @param m An optional mutex to be unlocked before waiting and locked
		 *          again afterwards.
		 * @param waitLock An optional waitlock which is acquired before the
		 *                 actual wait and released afterwards.
		 */
		bool wait(boost::mutex *m = nullptr, boost::mutex *waitLock = nullptr);

		virtual void handleInterrupt(int) override;


	// ----------------------------------------------------------------------
	//  Private types and members
	// ----------------------------------------------------------------------
	protected:
		typedef Wired::DeviceGroup    DeviceGroup;
		typedef Wired::SocketPtr      SocketPtr;
		typedef Wired::Buffer         Buffer;
		typedef Wired::BufferPtr      BufferPtr;

		typedef std::deque<BufferPtr> BufferQueue;

		SocketPtr              _socket;
		DeviceGroup            _select;
		BufferQueue            _outbox;
		BufferQueue            _backlog; // Messages need to be sent after re-connect
		bool                   _useSSL;
		std::set<std::string>  _subscriptions;
		char                   _buffer[1024];
		char                  *_getp;
		int                    _getcount;
		uint32_t               _ackWindow;

		mutable bool           _inWait;
		mutable boost::mutex   _sockMutex;
		// Mutex to synchronize write access from separate threads.
		mutable boost::mutex   _writeMutex;
};


inline size_t Socket::outboxSize() const {
	boost::mutex::scoped_lock l(_writeMutex);
	return _outbox.size();
}


struct FrameHeaderName {
	FrameHeaderName(const char *s) : name(s) {}
	const char *name;
};


struct FrameHeaderValue {
	FrameHeaderValue(const char *s) : value(s) {}
	const char *value;
};


struct FrameHeaders {
	FrameHeaders(const char *src, size_t l)
	: _source(src), _source_len(l)
	, _numberOfHeaders(0) {}

	bool next();

	bool nameEquals(const char *s) const {
		return !strncmp(s, name_start, name_len);
	}

	bool valueEquals(const char *s) const {
		return !strncmp(s, val_start, val_len);
	}

	bool operator==(const FrameHeaderName &wrapper) const {
		return nameEquals(wrapper.name);
	}

	bool operator==(const FrameHeaderValue &wrapper) const {
		return valueEquals(wrapper.value);
	}

	bool empty() const {
		return _numberOfHeaders == 0;
	}

	const char *getptr() const {
		return _source;
	}

	const char  *_source;
	size_t       _source_len;
	size_t       _numberOfHeaders;

	const char  *name_start;
	size_t       name_len;
	const char  *val_start;
	size_t       val_len;
};


}
}
}


#endif
