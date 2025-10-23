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


#ifndef SEISCOMP_IO_RECORDS_MSEED_ENCODER_UNCOMPRESSED_H
#define SEISCOMP_IO_RECORDS_MSEED_ENCODER_UNCOMPRESSED_H


#include "encoder.h"
#include "format.h"


namespace Seiscomp::IO::MSEED {


template<typename T>
class Uncompressed : public Encoder {
	public:
		Uncompressed(Format *format, int freqn, int freqd)
		: Encoder(format, getEncoding<T>(), freqn, freqd) {}

	public:
		void flush() override {
			if ( _format->bigEndian ) {
				Core::Endianess::Converter::ToBigEndian((T*)_currentData, _sampleCount);
			}
			else {
				Core::Endianess::Converter::ToLittleEndian((T*)_currentData, _sampleCount);
			}

			if ( _currentPacket ) {
				queuePacket();
			}
		}

		void push(size_t n, const void *samples) override {
			for ( size_t i = 0; i < n; ++i ) {
				if ( !_currentPacket )
					_currentPacket = _format->getBuffer(getTime(),
					                                    _timingQuality,
					                                    &_currentData,
					                                    &_currentDataLen);
				else if ( _sampleCount * static_cast<int>(sizeof(T)) >= _currentDataLen ) {
					flush();
					_currentPacket = _format->getBuffer(getTime(),
					                                    _timingQuality,
					                                    &_currentData,
					                                    &_currentDataLen);
				}
				reinterpret_cast<T*>(_currentData)[_sampleCount] = static_cast<const T*>(samples)[i];
				++_sampleCount;
				tick();
			}
		}

	private:
		void queuePacket() {
			_format->updateBuffer(_currentPacket.get(), _sampleCount, 1);
			queue();
			_currentPacket.reset();
			_currentData = nullptr;
			_currentDataLen = 0;
			reset();
		}
};


}


#endif
