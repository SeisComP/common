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


#define SEISCOMP_COMPONENT Wired
#include <seiscomp/logging/log.h>
#include <seiscomp/core/baseobject.h>
#include <seiscomp/wired/clientsession.h>
#include <seiscomp/wired/reactor.h>

#include <cerrno>
#include <iostream>


using namespace std;


namespace Seiscomp {
namespace Wired {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ClientSession::ClientSession(Device *dev, size_t maxCharactersPerLine)
: Session(dev), _bytesSent(0) {
	_inbox.resize(maxCharactersPerLine);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::setPostDataSize(size_t len) {
	_postDataSize = len;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ClientSession::postDataSize() const {
	return _postDataSize;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::setMIMEUnfoldingEnabled(bool f) {
	if ( f )
		_flags |= MIMEUnfolding;
	else
		_flags &= ~MIMEUnfolding;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::flush(bool flagFlush) {
	// Try to empty the outbox
	if ( !_currentBuffer ) {
		if ( _flags & PendingFlush ) {
			_flags &= ~PendingFlush;
			buffersFlushed();
			if ( !_currentBuffer ) {
				// If there is nothing to do, return.
				return;
				// Otherwise try to flush as much as possible.
			}
		}
		else {
			return;
		}
	}

	if ( !_writeQuota ) {
		return;
	}

	_flags &= ~PendingFlush;

	while ( _currentBuffer ) {
		size_t remaining = min(_currentBuffer->header.size() - _currentBufferHeaderOffset, _writeQuota);
		ssize_t written;

		if ( remaining > 0 ) {
			written = _device->write(
				&_currentBuffer->header[_currentBufferHeaderOffset],
				static_cast<size_t>(remaining)
			);

			// Error on socket?
			if ( written < 0 ) {
				if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
					// Close the session
					_currentBuffer = nullptr;
					close();
					break;
				}
			}
			// No non-blocking writing possible?
			else if ( written == 0 ) {
			}
			else {
				_writeQuota -= static_cast<size_t>(written);
				_currentBufferHeaderOffset += static_cast<size_t>(written);
				if ( static_cast<size_t>(written) <= _bufferBytesPending ) {
					_bufferBytesPending -= static_cast<size_t>(written);
				}
				else {
					_bufferBytesPending = 0;
				//SEISCOMP_DEBUG("Bytes pending: %d, written: %d", (int)_bytesPending, written);
				}
			}
		}

		// header not yet completely sent
		if ( _currentBufferHeaderOffset < _currentBuffer->header.size() ) {
			return;
		}

		remaining = min(_currentBuffer->data.size() - _currentBufferDataOffset, _writeQuota);
		if ( remaining > 0 ) {
			written = _device->write(&_currentBuffer->data[_currentBufferDataOffset],
			                         remaining);

			// Error on socket?
			if ( written < 0 ) {
				if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
					_currentBuffer = nullptr;
					// Close the session
					close();
					break;
				}
			}
			// No non-blocking writing possible?
			else if ( written == 0 ) {
			}
			else {
				_writeQuota -= static_cast<size_t>(written);
				_currentBufferDataOffset += static_cast<size_t>(written);
				if ( static_cast<size_t>(written) <= _bufferBytesPending ) {
					_bufferBytesPending -= static_cast<size_t>(written);
				}
				else {
					_bufferBytesPending = 0;
				//SEISCOMP_DEBUG("Bytes pending: %d, written: %d", (int)_bytesPending, written);
				}
			}
		}

		// Finished current?
		if ( _currentBufferDataOffset == _currentBuffer->data.size() ) {
			_currentBufferHeaderOffset = 0;
			_currentBufferDataOffset = 0;

			if ( !_currentBuffer->updateBuffer() ) {
				bufferSent(_currentBuffer.get());
				_currentBuffer = nullptr;
				_currentBufferHeaderOffset = 0;

				if ( !_bufferQueue.empty() ) {
					_currentBuffer = _bufferQueue.front();
					_bufferQueue.pop_front();
				}
			}
			else {
				_bufferBytesPending += _currentBuffer->header.size();
				size_t buf_length = _currentBuffer->length();
				if ( buf_length == string::npos )
					_bufferBytesPending += _currentBuffer->data.size();
			}
		}
		else
			// No all data has been written
			break;

		if ( !_currentBuffer ) {
			if ( flagFlush ) {
				_flags |= PendingFlush;
			}
			else {
				buffersFlushed();
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::buffersFlushed() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::bufferSent(Buffer*) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::update() {
	// Flush the outbox
	flush();

	if ( valid() /*&& readable*/ ) {
		char *buf;
		size_t len;

		 _flags |= KeepReading;

		while ( (_device->mode() & Device::Read)
		     && ((len = _parent->getBuffer(buf)) > 0) ) {
			ssize_t read = _device->read(buf, len);

			// Check if the conncection was unexpectely closed by peer
			if ( !read ) {
//				SEISCOMP_DEBUG("fd %d closed by peer, errno = %s",
//				               device()->fd(), strerror(errno));
				close();
				return;
			}
			else if ( read < 0 ) {
				if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
					SEISCOMP_DEBUG("Read error (%d): %s", errno, strerror(errno));
					close();
					return;
				}

				// Break the loop, nothing to do anymore and wait for
				// the next call when the kernel signals available data
				break;
			}

			handleReceive(buf, static_cast<size_t>(read));

			if ( (read < static_cast<ssize_t>(len)) || !valid() || !(_flags & KeepReading) ) {
				break;
			}
		}

		flush();
	}

	if ( !_device->isValid() ) {
		return;
	}

	if ( erroneous() ) {
		SEISCOMP_DEBUG("Closing erroneous session");
		close();
		return;
	}

	if ( inAvail() > 0 ) {
		_device->addMode(Device::Write);
	}
	else {
		_device->removeMode(Device::Write);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::finishReading() {
	_flags &= ~KeepReading;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::close() {
	_flags &= ~KeepReading;
	Session::close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::handleReceive(const char *buf, size_t len) {
	if ( _postDataSize > 0 ) {
		if ( len > 0 ) {
			size_t read = std::min(_postDataSize, len);
			handlePostData(buf, read);
			_postDataSize -= read;
			buf += read;
			len -= read;
		}
	}

	for ( ; len > 0; --len, ++buf ) {
		if ( *buf == '\r' ) continue;
		if ( *buf == '\n' ) {
			// If MIME unfolding is enable and a whitespace follows immediately
			// a newline then skip both characters and fold both lines.
			// See https://tools.ietf.org/html/rfc2425#section-5.8.1
			if ( (_flags & MIMEUnfolding) && (len > 1) &&
			     ((buf[1] == ' ') || (buf[1] == '\t')) ) {
				++buf;
				--len;
				continue;
			}

			_inbox[_inboxPos] = '\0';

			handleInbox(&_inbox[0], _inboxPos);
			if ( erroneous() ) {
				break;
			}

			_inboxPos = 0;
			_inbox[0] = '\0';

			if ( _postDataSize > 0 ) {
				++buf; --len;
				if ( len > 0 ) {
					size_t read = std::min(_postDataSize, len);
					handlePostData(buf, read);
					_postDataSize -= read;
					// buf and len are increased again in the for loop so
					// decrease them here again to move the read pointer to
					// the correct position in the next loop
					-- read;
					buf += read;
					len -= read;
				}
				else {
					break;
				}
			}
		}
		else {
			// If one line of data exceeds the maximum number of allowed
			// characters terminate the connection
			if ( _inboxPos >= _inbox.size() ) {
				handleInboxError(TooManyCharactersPerLine);
				break;
			}

			_inbox[_inboxPos] = *buf;
			++_inboxPos;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::handleInboxError(Error err) {
	if ( err == TooManyCharactersPerLine )
		setError("ERROR: Too many characters per line\n");
	else
		setError("ERROR\n");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::handlePostData(const char *, size_t) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::setError(const char* msg) {
	send(msg, strlen(msg));
	invalidate();
	SEISCOMP_WARNING("[client] setError('%s')", msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::handleInbox(const char *data, size_t len) {
	if ( len == 0 )
		return;
	else if ( len == 5 && strncasecmp(data, "hello", len) == 0 )
		send("TCPServer Test\n"
		     "WELCOME\n", 24);
	else if ( len == 3 && strncasecmp(data, "bye", len) == 0 )
		close();
	else
		send("WARNING: unknown command\n", 25);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::send(const char *data, size_t len) {
	if ( _currentBuffer ) {
		if ( _flags & AppendBuffer ) {
			// Append to the last buffer, even the current one
			if ( _bufferQueue.empty() ) {
				_currentBuffer->data.append(data, len);
			}
			else {
				_bufferQueue.back()->data.append(data, len);
			}
			_bufferBytesPending += len;
			/*
			SEISCOMP_DEBUG("%p appended %d bytes to private buffer",
			               static_cast<void*>(this), len);
			*/
		}
		else {
			// Create a new buffer and queue it
			auto buffer = new Buffer();
			buffer->data.assign(data, len);
			/*
			SEISCOMP_DEBUG("%p queued private buffer of %d bytes",
			               static_cast<void*>(this), buffer->data.size());
			*/
			_flags |= AppendBuffer;
			queue(buffer);
		}
		return;
	}

	// Limit to remaining quota for the current turn.
	auto chunk = std::min(len, _writeQuota);

	auto r = chunk > 0 ? _device->write(data, chunk) : 0;
	if ( r < 0 ) {
		if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
			invalidate();
			close();
			return;
		}

		// Would block, queue it.
		r = 0;
	}
	else {
		_flags |= PendingFlush;
	}

	// Adjust quota
	_writeQuota -= r;
	len -= r;

	if ( len ) {
		// Queue the remaining bytes
		auto buffer = new Buffer();
		buffer->data.assign(data + r, len);
		/*
		SEISCOMP_DEBUG("%p queued buffer overflow of %d bytes",
		               static_cast<void*>(this), buffer->data.size());
		*/
		_flags |= AppendBuffer;
		queue(buffer);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientSession::send(const char *data) {
	send(data, strlen(data));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ClientSession::send(Buffer *buf) {
	// Clear "private buffer" flag
	_flags &= ~AppendBuffer;

	if ( _currentBuffer || !_writeQuota ) {
		// If another buffer is currently active or no quota left
		// then queue this one.
		return queue(buf);
	}

	_currentBuffer = buf;
	_currentBufferHeaderOffset = 0;
	_currentBufferDataOffset = 0;
	_bufferBytesPending += buf->header.size();

	size_t buf_length = _currentBuffer->length();
	if ( buf_length == string::npos ) {
		_bufferBytesPending += _currentBuffer->data.size();
	}
	else {
		_bufferBytesPending += buf_length;
	}

	// Do not all buffersFlushed() inside this call to prevent recursion.
	// Instead set the flag PendingFlush if the buffer has been flushed
	// to call buffersFlushed() in update.
	flush(true);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ClientSession::queue(Buffer *buf) {
	_bufferBytesPending += buf->header.size();

	size_t buf_length = buf->length();
	if ( buf_length == string::npos ) {
		_bufferBytesPending += buf->data.size();
	}
	else {
		_bufferBytesPending += buf_length;
	}

	if ( !_currentBuffer ) {
		_currentBuffer = buf;
		_currentBufferHeaderOffset = 0;
		_currentBufferDataOffset = 0;

		_device->addMode(Device::Write);

		return true;
	}

	_bufferQueue.push_back(buf);

	_device->addMode(Device::Write);

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
