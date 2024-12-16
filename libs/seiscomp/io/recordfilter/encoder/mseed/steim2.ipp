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


namespace Seiscomp {
namespace IO {
namespace MSEED {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim2<T>::updateSpw(int bp) {
	assert(bp < 7);

	if ( _buf[bp] < -536870912 ) {
		std::cerr << _format->networkCode << "." << _format->stationCode << "."
		          << _format->locationCode << "." << _format->channelCode << ": "
		             "value " << _buf[bp] << " is too large for Steim2 encoding"
		          << std::endl;
		_buf[bp] = -536870912;
		_spw = 1;
		return;
	}

	if ( _buf[bp] > 536870911 ) {
		std::cerr << _format->networkCode << "." << _format->stationCode << "."
		          << _format->locationCode << "." << _format->channelCode << ": "
		             "value " << _buf[bp] << " is too large for Steim2 encoding"
		          << std::endl;
		_buf[bp] = 536870911;
		_spw = 1;
		return;
	}

	int spw1 = 7;
	if ( _buf[bp] < -16384 || _buf[bp] > 16383 ) {
		spw1 = 1;
	}
	else if ( _buf[bp] < -512 || _buf[bp] > 511 ) {
		spw1 = 2;
	}
	else if ( _buf[bp] < -128 || _buf[bp] > 127 ) {
		spw1 = 3;
	}
	else if ( _buf[bp] < -32 || _buf[bp] > 31 ) {
		spw1 = 4;
	}
	else if ( _buf[bp] < -16 || _buf[bp] > 15 ) {
		spw1 = 5;
	}
	else if ( _buf[bp] < -8  || _buf[bp] > 7 ) {
		spw1 = 6;
	}
	if ( spw1 < _spw ) {
		_spw = spw1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim2<T>::store(int32_t value) {
	assert(_bp < 7);
	_buf[_bp] = value - _lastSample;
	_lastSample = value;
	updateSpw(_bp);
	++_bp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim2<T>::initPacket() {
	int i;
	int32_t beginSample = _lastSample;

	for ( i = 1; i < _bp; ++i ) {
		beginSample -= _buf[i];
	}

	reset();
	Steim2Frame *frames = reinterpret_cast<Steim2Frame*>(_currentData);
	frames[0].sampleWord[0] = Core::Endianess::Converter::ToBigEndian<uint32_t>(beginSample);
	_frameCount = 0;
	_nibbleWord = 0;
	_fp = 2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim2<T>::finishPacket() {
	int i;
	int32_t endSample = _lastSample;

	for ( i = 0; i < _bp; ++i ) {
		endSample -= _buf[i];
	}

	Steim2Frame *frames = reinterpret_cast<Steim2Frame*>(_currentData);
	frames[0].sampleWord[1] = Core::Endianess::Converter::ToBigEndian<uint32_t>(endSample);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim2<T>::updatePacket() {
	unsigned int nibble = 0;
	u_int32_t sampleWord = 0;

	assert(_bp < 8);

	int used = _bp;

	while ( used > _spw ) {
		--used;
		_spw = 7;
		for ( int i = 0; i < used; ++i ) {
			updateSpw(i);
		}
	}

	_spw = used;

	switch ( _spw ) {
		case 7:
			nibble = 3;
			sampleWord = (2U << 30) | ((_buf[0] & 0xf) << 24) |
				((_buf[1] & 0xf) << 20) | ((_buf[2] & 0xf) << 16) |
				((_buf[3] & 0xf) << 12) | ((_buf[4] & 0xf) << 8) |
				((_buf[5] & 0xf) << 4) | (_buf[6] & 0xf);
			break;
		case 6:
			nibble = 3;
			sampleWord = (1U << 30) | ((_buf[0] & 0x1f) << 25) |
				((_buf[1] & 0x1f) << 20) | ((_buf[2] & 0x1f) << 15) |
				((_buf[3] & 0x1f) << 10) | ((_buf[4] & 0x1f) << 5) |
				(_buf[5] & 0x1f);
			break;
		case 5:
			nibble = 3;
			sampleWord = ((_buf[0] & 0x3f) << 24) | ((_buf[1] & 0x3f) << 18) |
				((_buf[2] & 0x3f) << 12) | ((_buf[3] & 0x3f) << 6) |
				(_buf[4] & 0x3f);
			break;
		case 4:
			nibble = 1;
			sampleWord = ((_buf[0] & 0xff) << 24) | ((_buf[1] & 0xff) << 16) |
				((_buf[2] & 0xff) <<  8) | (_buf[3] & 0xff);
			break;
		case 3:
			nibble = 2;
			sampleWord = (3U << 30) | ((_buf[0] & 0x3ff) << 20) |
				((_buf[1] & 0x3ff) << 10) | (_buf[2] & 0x3ff);
			break;
		case 2:
			nibble = 2;
			sampleWord = (2U << 30) | ((_buf[0] & 0x7fff) << 15) |
				(_buf[1] & 0x7fff);
			break;
		case 1:
			nibble = 2;
			sampleWord = (1U << 30) | (_buf[0] & 0x3fffffff);
			break;
		default:
			assert(0);
			break;
	}

	_nibbleWord |= (nibble << (30 - ((_fp + 1) << 1)));

	_spw = 7;
	for ( int i = 0; i < _bp - used; ++i ) {
		_buf[i] = _buf[i + used];
		updateSpw(i);
	}

	_bp -= used;
	_sampleCount += used;

	Steim2Frame *frames = reinterpret_cast<Steim2Frame*>(_currentData);
	frames[_frameCount].nibbleWord = Core::Endianess::Converter::ToBigEndian<uint32_t>(_nibbleWord);
	frames[_frameCount].sampleWord[_fp] = Core::Endianess::Converter::ToBigEndian<uint32_t>(sampleWord);

	if ( ++_fp < 15 ) {
		return;
	}

	_nibbleWord = 0;
	_fp = 0;
	++_frameCount;
	return;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim2<T>::queuePacket() {
	_format->updateBuffer(_currentPacket.get(), _sampleCount, _frameCount + (_fp > 0));
	_bufferQueue.push_back(_currentPacket);
	_currentPacket.reset();
	_currentData = nullptr;
	_currentDataLen = 0;
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim2<T>::push(size_t n, const void *samples) {
	for ( size_t i = 0; i < n; ++i ) {
		int32_t sample = static_cast<const T*>(samples)[i];
		store(sample);
		tick();

		while ( _bp >= _spw ) {
			if( !_currentPacket ) {
				_currentPacket = _format->getBuffer(getTime(_bp),
				                                    _timingQuality,
				                                    &_currentData,
				                                    &_currentDataLen);
				initPacket();
			}

			updatePacket();
			if ( _frameCount == numberOfFrames() ) {
				finishPacket();
				queuePacket();
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim2<T>::flush() {
	while ( _bp ) {
		if ( !_currentPacket ) {
			_currentPacket = _format->getBuffer(getTime(_bp),
			                                    _timingQuality,
			                                    &_currentData,
			                                    &_currentDataLen);
			initPacket();
		}

		updatePacket();
		if( _frameCount == numberOfFrames() ) {
			finishPacket();
			queuePacket();
		}
	}

	if ( _currentPacket ) {
		finishPacket();
		queuePacket();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
inline int Steim2<T>::numberOfFrames() {
	return _currentDataLen >> 6;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
