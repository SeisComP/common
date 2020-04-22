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


#ifndef SEISCOMP_WIRED_PROTOCOLS_WEBSOCKET_H
#define SEISCOMP_WIRED_PROTOCOLS_WEBSOCKET_H


#include <string>
#include <stdint.h>

#include <seiscomp/wired/buffer.h>


namespace Seiscomp {
namespace Wired {
namespace Websocket {


enum Status {
	// No status set
	NoStatus                 = 0,
	// Normal closure; the connection successfully completed whatever
	// purpose for which it was created.
	CloseNormal              = 1000,
	// The endpoint is going away, either because of a server failure
	// or because the browser is navigating away from the page that
	// opened the connection.
	CloseGoingAway           = 1001,
	// The endpoint is terminating the connection due to a protocol
	// error.
	CloseProtocolError       = 1002,
	// The connection is being terminated because the endpoint received
	// data of a type it cannot accept (for example, a text-only
	// endpoint received binary data).
	CloseUnsupported         = 1003,
	// Reserved. Indicates that no status code was provided even though
	// one was expected.
	CloseNoStatus            = 1005,
	// Reserved. Used to indicate that a connection was closed
	// abnormally (that is, with no close frame being sent) when a
	// status code is expected.
	CloseAbnormal            = 1006,
	// The endpoint is terminating the connection because a message
	// was received that contained inconsistent data (e.g., non-UTF-8
	// data within a text message).
	CloseInconsistentData    = 1007,
	// The endpoint is terminating the connection because it received
	// a message that violates its policy. This is a generic status code,
	// used when codes 1003 and 1009 are not suitable.
	CloseViolatePolicy       = 1008,
	// The endpoint is terminating the connection because a data frame
	// was received that is too large.
	CloseTooLarge            = 1009,
	// The client is terminating the connection because it expected
	// the server to negotiate one or more extension, but the server
	// didn't.
	CloseExpectedExts        = 1010,
	// The server is terminating the connection because it encountered
	// an unexpected condition that prevented it from fulfilling the
	// request.
	CloseUnexpectedCondition = 1011
};


DEFINE_SMARTPOINTER(Frame);
class Frame : public Seiscomp::Core::BaseObject {
	public:
		enum Type {
			ContinuationFrame = 0x00,
			TextFrame         = 0x01,
			BinaryFrame       = 0x02,
			ConnectionClose   = 0x08,
			Ping              = 0x09,
			Pong              = 0x0A
		};


	public:
		Frame();

		//! Resets the header
		void reset();

		void setMaxPayloadSize(uint64_t size);

		//! Returns the number of bytes read from the input buffer
		//! If an protocol error occured, -1 is returned.
		ssize_t feed(const char *data, size_t len);

		bool isFinished() { return _isFinished; }

		static void finalizeBuffer(Buffer *buf, Type type, Status statusCode = NoStatus);


	private:
		typedef bool (Frame::*ItemCallback)();
		bool next(size_t nBytes, void *dst, ItemCallback cb);

		bool readControl();
		bool readPayload1();
		bool readPayload16();
		bool readPayload64();
		bool readMask();
		bool dataComplete();

		bool readStatus();
		bool readData();


	private:
		size_t        _bytesToRead;
		uint8_t      *_buffer;
		ItemCallback  _func;
		bool          _isFinished;
		uint8_t       _control;
		uint64_t      _maxPayloadSize;

	public:
		Type          type;
		uint16_t      status;
		bool          isMasked;
		bool          finalFragment;
		uint64_t      payloadLength;
		uint32_t      mask;
		std::string   data;
};


}
}
}


#endif
