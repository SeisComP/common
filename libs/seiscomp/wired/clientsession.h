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


#ifndef SEISCOMP_WIRED_CLIENTSESSION_H
#define SEISCOMP_WIRED_CLIENTSESSION_H


#include <seiscomp/wired/session.h>
#include <seiscomp/wired/buffer.h>

#include <vector>


namespace Seiscomp {
namespace Wired {


class SC_SYSTEM_CORE_API ClientSession : public Session {
	public:
		enum Error {
			NoError = 0,
			TooManyCharactersPerLine,
			ErrorQuantity
		};

	public:
		ClientSession(Device *dev, size_t maxCharactersPerLine = 200);

	public:
		//! Flush data still in the buffer.
		virtual void flush();

		//! Update the session state including flushing and
		//! reading available data.
		void update() override;

		//! Queue data in the outbox
		void send(const char *data, size_t len);
		void send(const char *data);

		//! Queues a buffer and returns whether the queue was empty
		bool send(Buffer *);

		//! Sets keepReading to false
		void close() override;

		bool valid() const;
		bool erroneous() const;


	protected:
		virtual void outboxFlushed();

		virtual void buffersFlushed();
		virtual void bufferSent(Buffer*);

		//! Sets the post data size that is read by handleReceived and
		//! passed to handlePostData
		void setPostDataSize(size_t len);
		size_t postDataSize() const;

		void setMIMEUnfoldingEnabled(bool);

		//! Returns the available bytes to send.
		virtual size_t inAvail() const;

		void setError(const char* msg);

		//! Sets the error state
		void invalidate();

		//! Break the reading loop to give control to other sessions. This
		//! calls only makes sense if the reactor is level-triggered.
		void finishReading();

		//! Handles a socket read into _buffer. The default implementation
		//! extracts lines and calls handleInbox/handleData.
		virtual void handleReceive(const char *data, size_t len);

		//! Handles a line
		virtual void handleInbox(const char *data, size_t len);

		//! Handles posted data that has been requested if setPOSTDataSize
		//! is called with len > 0.
		virtual void handlePostData(const char *data, size_t len);

		virtual void handleInboxError(Error error);


	private:
		void flushOutbox();


	protected:
		enum Flags {
			NoFlags       = 0x0000,
			MIMEUnfolding = 0x0001,
			KeepReading   = 0x0002,
			//Future1     = 0x0004,
			//Future2     = 0x0008,
			//Future3     = 0x0010,
			//Future4     = 0x0020,
			//Future5     = 0x0040,
			//Future6     = 0x0080,
			Erroneous     = 0x0100
		};

		std::vector<char> _inbox;
		size_t            _inboxPos;
		std::vector<char> _outbox;
		uint16_t          _flags;
		size_t            _postDataSize;
		Device::count_t   _bytesSent;


	private:
		BufferPtr         _currentBuffer;
		size_t            _currentBufferHeaderOffset;
		size_t            _currentBufferDataOffset;
		BufferList        _bufferQueue;
		size_t            _bufferBytesPending;
};


inline bool ClientSession::valid() const {
	return !erroneous();
}

inline void ClientSession::invalidate() {
	_flags |= Erroneous;
}

inline bool ClientSession::erroneous() const {
	return _flags & Erroneous;
}


}
}


#endif
