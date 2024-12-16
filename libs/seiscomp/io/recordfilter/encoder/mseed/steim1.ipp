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
void Steim1<T>::updateSpw(int bp) {
	int spw1 = 4;

	assert(bp < 4);
	if ( _buf[bp] < -32768 || _buf[bp] > 32767 ) {
		spw1 = 1;
	}
	else if ( _buf[bp] < -128 || _buf[bp] > 127 ) {
		spw1 = 2;
	}

	if ( spw1 < _spw ) {
		_spw = spw1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim1<T>::store(int32_t value) {
	assert(_bp < 4);
	_buf[_bp] = value - _lastSample;
	_lastSample = value;
	updateSpw(_bp);
	++_bp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim1<T>::initPacket() {
	int i;
	int32_t beginSample = _lastSample;

	for ( i = 1; i < _bp; ++i ) {
		beginSample -= _buf[i];
	}

	reset();
	Steim1Frame *frames = reinterpret_cast<Steim1Frame*>(_currentData);
	frames[0].sampleWord[0] = Core::Endianess::Converter::ToBigEndian<uint32_t>(beginSample);
	_frameCount = 0;
	_nibbleWord = 0;
	_fp = 2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim1<T>::finishPacket() {
	int i;
	int32_t endSample = _lastSample;

	for ( i = 0; i < _bp; ++i ) {
		endSample -= _buf[i];
	}

	Steim1Frame *frames = reinterpret_cast<Steim1Frame*>(_currentData);
	frames[0].sampleWord[1] = Core::Endianess::Converter::ToBigEndian<uint32_t>(endSample);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim1<T>::updatePacket() {
	unsigned int nibble = 0;
	u_int32_t sampleWord = 0;

	assert(_bp < 5);

	int used = _bp;

	while ( used > _spw ) {
		--used;
		_spw = 4;
		for ( int i = 0; i < used; ++i ) {
			updateSpw(i);
		}
	}

	while (used < _spw) {
		_spw >>= 1;
	}

	used = _spw;

	switch ( _spw ) {
		case 4:
			nibble = 1;
			sampleWord = ((_buf[0] & 0xff) << 24) | ((_buf[1] & 0xff) << 16) |
			              ((_buf[2] & 0xff) <<  8) | (_buf[3] & 0xff);
			break;
		case 2:
			nibble = 2;
			sampleWord = ((_buf[0] & 0xffff) << 16) | (_buf[1] & 0xffff);
			break;
		case 1:
			nibble = 3;
			sampleWord = _buf[0];
			break;
		default:
			assert(0);
	}

	_nibbleWord |= (nibble << (30 - ((_fp + 1) << 1)));

	_spw = 4;
	for ( int i = 0; i < _bp - used; ++i) {
		_buf[i] = _buf[i + used];
		updateSpw(i);
	}

	_bp -= used;
	_sampleCount += used;

	Steim1Frame *frames = reinterpret_cast<Steim1Frame*>(_currentData);
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
void Steim1<T>::queuePacket() {
	_format->updateBuffer(_currentPacket.get(), _sampleCount, _frameCount + (_fp > 0));
	_bufferQueue.push_back(_currentPacket);
	_currentPacket.reset();
	_currentData = 0;
	_currentDataLen = 0;
	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void Steim1<T>::push(size_t n, const void *samples) {
	for ( size_t i = 0; i < n; ++i ) {
		int32_t sample = static_cast<const T*>(samples)[i];
		store(sample);
		tick();

		while ( _bp >= _spw ) {
			if ( !_currentPacket ) {
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
void Steim1<T>::flush() {
	while ( _bp ) {
		if ( !_currentPacket ) {
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

	if ( _currentPacket ) {
		finishPacket();
		queuePacket();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
inline int Steim1<T>::numberOfFrames() {
	return _currentDataLen >> 6;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
