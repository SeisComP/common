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

#include <seiscomp/core/endianess.h>
#include <seiscomp/logging/log.h>
#include <iostream>
#include <string.h>

#include "websocket.h"


namespace Seiscomp {
namespace Wired {
namespace Websocket {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Frame::Frame() : _maxPayloadSize(0) {
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline bool Frame::next(int nBytes, void *dst, ItemCallback cb) {
	_bytesToRead = nBytes;
	_buffer = (char*)dst;
	_func = cb;
	//SEISCOMP_DEBUG("next %d", nBytes);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Frame::reset() {
	_isFinished = false;
	data = std::string();
	status = NoStatus;
	payloadLength = 0;
	next(1, &_control, &Frame::readControl);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Frame::setMaxPayloadSize(uint64_t size) {
	_maxPayloadSize = size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Frame::feed(const char *data, int len) {
	const char *src = data;

	//SEISCOMP_DEBUG("handle %d", len);
	while ( (len > 0) && (_bytesToRead > 0) ) {
		int toCopy = std::min(_bytesToRead, len);
		memcpy(_buffer, data, toCopy);
		data += toCopy;
		_buffer += toCopy;
		len -= toCopy;
		_bytesToRead -= toCopy;
		//SEISCOMP_DEBUG("copy %d, %d, %d", toCopy, len, _bytesToRead);
		if ( !_bytesToRead ) {
			if ( !(this->*_func)() )
				return -1;
			//else
			//	SEISCOMP_DEBUG("got next");
		}
	}
	//SEISCOMP_DEBUG("endhandle %d", len);

	return data-src;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Frame::readControl() {
	int tp = _control & 0x0F;
	switch ( tp ) {
		case 0x00:
			type = ContinuationFrame;
			break;
		case 0x01:
			type = TextFrame;
			break;
		case 0x02:
			type = BinaryFrame;
			break;
		case 0x08:
			type = ConnectionClose;
			break;
		case 0x09:
			type = Ping;
			break;
		case 0x0A:
			type = Pong;
			break;
		default:
			// error
			SEISCOMP_ERROR("invalid frame control byte: %x", tp);
			return false;
	}

	finalFragment = _control & 0x80;

	return next(1, &payloadLength, &Frame::readPayload1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Frame::readPayload1() {
	isMasked = payloadLength & 0x80;
	payloadLength &= ~0x80;

	if ( payloadLength <= 125 ) {
		if ( isMasked )
			return next(4, &mask, &Frame::readMask);
		else
			return readData();
	}
	else if ( payloadLength == 126 )
		return next(2, &payloadLength, &Frame::readPayload16);
	else if ( payloadLength == 127 )
		return next(8, &payloadLength, &Frame::readPayload64);

	SEISCOMP_ERROR("invalid frame payload: %ld", payloadLength);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Frame::readPayload16() {
	Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,2>::Take(&payloadLength, 1);
	if ( isMasked )
		return next(4, &mask, &Frame::readMask);
	else
		return readData();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Frame::readPayload64() {
	Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,8>::Take(&payloadLength, 1);
	if ( isMasked )
		return next(4, &mask, &Frame::readMask);
	else
		return readData();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Frame::readMask() {
	return readData();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Frame::dataComplete() {
	if ( isMasked ) {
		// Unmask data
		int l = data.size() >> 2;
		uint32_t *b = (uint32_t *)data.data();
		for ( int i = 0; i < l; ++i, ++b )
			*b ^= mask;

		l = data.size() & 0x03;
		uint8_t *bb = (uint8_t*)b;
		uint8_t *mb = (uint8_t*)&mask;
		for ( int i = 0; i < l; ++i )
			bb[i] ^= mb[i];
	}

	_isFinished = true;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Frame::readStatus() {
	Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,2>::Take(&status, 1);

	if ( payloadLength > 0 ) {
		if ( _maxPayloadSize && payloadLength > _maxPayloadSize ) {
			SEISCOMP_ERROR("payload limit exceeded %ld > %ld",
			               payloadLength, _maxPayloadSize);
			return false;
		}

		data.resize(payloadLength);
		return next((int)payloadLength, (void*)data.data(), &Frame::dataComplete);
	}
	else
		_isFinished = true;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Frame::readData() {
	if ( type == ConnectionClose ) {
		// According to https://tools.ietf.org/id/draft-ietf-hybi-thewebsocketprotocol-09.html#closeframe
		// this is optional
		if ( payloadLength >= 2 ) {
			payloadLength -= 2;
			return next(2, (void*)&status, &Frame::readStatus);
		}
	}

	if ( payloadLength > 0 ) {
		if ( _maxPayloadSize && payloadLength > _maxPayloadSize ) {
			SEISCOMP_ERROR("payload limit exceeded %ld > %ld",
			               payloadLength, _maxPayloadSize);
			return false;
		}

		data.resize(payloadLength);
		return next((int)payloadLength, (void*)data.data(), &Frame::dataComplete);
	}
	else
		_isFinished = true;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Frame::finalizeBuffer(Buffer *buf, Type type, Status statusCode) {
	uint8_t control = 0x80 | type;
	uint8_t plc;
	uint64_t pl = buf->data.size();
	size_t headerOffset = 0;

	// Add two bytes for the status code
	if ( statusCode != NoStatus ) {
		pl += 2;
		headerOffset = 2;
	}

	if ( pl > 125 && pl <= 65535 ) {
		plc = 126;
		Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,2>::Take(&pl, 1);
		buf->header.resize(4+headerOffset);
		memcpy((char*)buf->header.data()+2, &pl, 2);
	}
	else if ( pl > 65535 ) {
		plc = 127;
		Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,8>::Take(&pl, 1);
		buf->header.resize(10+headerOffset);
		memcpy((char*)buf->header.data()+2, &pl, 8);
	}
	else {
		plc = (uint8_t)pl;
		buf->header.resize(2+headerOffset);
	}

	memcpy((char*)buf->header.data(), &control, 1);
	memcpy((char*)buf->header.data()+1, &plc, 1);

	if ( statusCode != NoStatus ) {
		uint16_t sc = statusCode;
		Core::Endianess::ByteSwapper<Core::Endianess::Current::LittleEndian,2>::Take(&sc, 1);
		memcpy((char*)buf->header.data()+buf->header.size()-2, &sc, 2);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
