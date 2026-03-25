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


#ifndef SEISCOMP_IO_RECORDS_ENCODER_MSEED_ENCODER_H
#define SEISCOMP_IO_RECORDS_ENCODER_MSEED_ENCODER_H


#include <seiscomp/core/datetime.h>
#include <seiscomp/core/endianess.h>
#include <seiscomp/core/optional.h>

#include <deque>
#include <functional>

#include "format.h"


namespace Seiscomp::IO::MSEED {


DEFINE_SMARTPOINTER(Encoder);
class Encoder : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Encoder(Format *format, EncodingType type, int freqn, int freqd)
		: _format(format), _freqn(freqn), _freqd(freqd) {
			_format->packType = type;
		}

		virtual ~Encoder() {
			if ( _format ) {
				delete _format;
			}
		}


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		using BufferCallback = std::function<void (CharArray*)>;

		void setBufferCallback(BufferCallback callback) { _bufferCallback = callback; }

		virtual void push(size_t n, const void *samples) = 0;
		virtual void flush() = 0;
		virtual void reset() { _sampleCount = 0; }

		void setTime(const Core::Time &time) {
			_itime = time;
			_ticks = 0;
		}

		Core::Time getTime(int tickDiff = 0) const {
			auto micros = Core::Time::Storage(_ticks - tickDiff) * _freqd * 1000000 / _freqn;
			return _itime + Core::TimeSpan(0, micros);
		}

		bool contiguous(const Core::TimeSpan &diff) const {
			return diff.count() * _freqn * 2 / _freqd / 1000000 == 0;
		}

		const Format *format() const { return _format; }

		void setTimingQuality(int quality) { _timingQuality = quality; }
		int timingQuality() const { return _timingQuality; }

		CharArrayPtr pop() {
			if ( _bufferQueue.empty() ) {
				return nullptr;
			}

			CharArrayPtr buf = _bufferQueue.front();
			_bufferQueue.pop_front();
			return buf;
		}

	protected:
		void tick() {
			++_ticks;
		}

		void queue() {
			if ( _currentPacket ) {
				if ( _bufferCallback ) {
					_bufferCallback(_currentPacket.get());
				}
				else {
					_bufferQueue.push_back(_currentPacket);
				}
			}
		}

	protected:
		using BufferQueue = std::deque<CharArrayPtr>;

		Format         *_format{nullptr};
		void           *_currentData{nullptr};
		int             _currentDataLen{0};
		CharArrayPtr    _currentPacket;
		int             _sampleCount{0};
		int             _timingQuality{-1};

	private:
		BufferQueue     _bufferQueue;
		BufferCallback  _bufferCallback;

	private:
		Core::Time    _itime;
		int           _ticks{0};
		const int     _freqn{0};
		const int     _freqd{0};
};


}


#endif
