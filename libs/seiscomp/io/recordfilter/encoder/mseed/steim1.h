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


#ifndef SEISCOMP_IO_ENCODER_MSEED_STEIM1_H
#define SEISCOMP_IO_ENCODER_MSEED_STEIM1_H


#include "encoder.h"


namespace Seiscomp {
namespace IO {
namespace MSEED {


/**
 * @brief A Steim1 frame with 64 bytes payload
 */
struct Steim1Frame {
	u_int32_t nibbleWord;
	u_int32_t sampleWord[15];
}
#if defined (__GNUC__)
__attribute__((packed))
#endif
;


template<typename T>
class Steim1 : public Encoder {
	public:
		Steim1(Format *format, int freqn, int freqd)
		    : Encoder(format, EncodingType::STEIM1, freqn, freqd) {}

	public:
		void flush() override;
		void push(size_t n, const void *samples) override;

	private:
		void updateSpw(int bp);
		void store(int32_t value);
		void initPacket();
		void finishPacket();
		void updatePacket();
		void queuePacket();
		int numberOfFrames();

	private:
		int       _frameCount{0};
		int       _bp{0};
		int       _fp{0};
		int       _spw{4};
		int32_t   _lastSample{0};
		int32_t   _buf[5];
		u_int32_t _nibbleWord{0};
};


}
}
}


#include "steim1.ipp"


#endif
