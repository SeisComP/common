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


/**
 * This code is taken from libmseed [1] and transformed to C++. The original
 * code structure is preserved to allow easy mapping of fixes in the original
 * code and this one.
 *
 * [1] https://github.com/EarthScope/libmseed
 */


#define SEISCOMP_COMPONENT core/io/records/mseed/decoder


#include <seiscomp/logging/log.h>
#include <seiscomp/core/endianess.h>
#include <cstddef>
#include <cstdint>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::IO::MSEED {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define EXTRACTBITRANGE(VALUE, STARTBIT, LENGTH) (((VALUE) >> (STARTBIT)) & ((1U << (LENGTH)) - 1))
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
__attribute__((optimize("O3", "unroll-loops")))
int64_t decodeSteim1(int32_t *input, size_t inputLength, size_t sampleCount,
                     int32_t *output, size_t outputLength, bool swapflag) {
	uint32_t frame[16]; /* Frame, 16 x 32-bit quantities = 64 bytes */
	int32_t diff[60];   /* Difference values for a frame, max is 15 x 4 (8-bit samples) */
	int32_t Xn = 0;     /* Reverse integration constant, aka last sample */
	uint64_t outputidx;
	uint64_t maxframes = inputLength / 64;
	uint64_t frameidx;
	int diffidx;
	int startnibble;
	int nibble;
	int widx;
	int idx;

	union dword {
		int8_t d8[4];
		int16_t d16[2];
		int32_t d32;
	} *word;

	if ( !maxframes ) {
		return 0;
	}

	if ( !input || !output || (outputLength == 0) ) {
		return -1;
	}

	#if DECODE_DEBUG
	ms_log (0, "Decoding %" PRIu64 " Steim1 frames, swapflag: %d\n", maxframes, swapflag);
	#endif

	for ( frameidx = 0, outputidx = 0; frameidx < maxframes && outputidx < sampleCount; frameidx++ ) {
		// Copy frame, each is 16x32-bit quantities = 64 bytes
		memcpy (frame, input + (16 * frameidx), 64);
		diffidx = 0;

		// Save forward integration constant (X0) and reverse integration constant (Xn)
		// and set the starting nibble index depending on frame.
		if ( frameidx == 0 ) {
			if ( swapflag ) {
				Core::Endianess::swap(&frame[1]);
				Core::Endianess::swap(&frame[2]);
			}

			output[0] = frame[1];
			outputidx++;
			Xn = frame[2];

			startnibble = 3; // First frame: skip nibbles, X0, and Xn

			#if DECODE_DEBUG
			ms_log (0, "Frame %" PRIu64 ": X0=%d  Xn=%d\n", frameidx, output[0], Xn);
			#endif
		}
		else {
			startnibble = 1; // Subsequent frames: skip nibbles

			#if DECODE_DEBUG
			ms_log (0, "Frame %" PRIu64 "\n", frameidx);
			#endif
		}

		// Swap 32-bit word containing the nibbles
		if ( swapflag ) {
			Core::Endianess::swap(&frame[0]);
		}

		// Decode each 32-bit word according to nibble
		for (widx = startnibble; widx < 16; widx++) {
			// W0: the first 32-bit contains 16 x 2-bit nibbles for each word
			nibble = EXTRACTBITRANGE (frame[0], (30 - (2 * widx)), 2);
			word = (union dword *)&frame[widx];

			switch ( nibble ) {
				case 0: // 00: Special flag, no differences
					#if DECODE_DEBUG
					ms_log (0, "  W%02d: 00=special\n", widx);
					#endif
					break;

				case 1: // 01: Four 1-byte differences
					for ( idx = 0; idx < 4; idx++ ) {
						diff[diffidx++] = word->d8[idx];
					}
					#if DECODE_DEBUG
					ms_log(0, "  W%02d: 01=4x8b  %d  %d  %d  %d\n", widx, diff[diffidx - 4], diff[diffidx - 3],
						   diff[diffidx - 2], diff[diffidx - 1]);
					#endif
					break;

				case 2: // 10: Two 2-byte differences
					for ( idx = 0; idx < 2; idx++ ) {
						if ( swapflag ) {
							Core::Endianess::swap(&word->d16[idx]);
						}

						diff[diffidx++] = word->d16[idx];
					}

					#if DECODE_DEBUG
					ms_log (0, "  W%02d: 10=2x16b  %d  %d\n", widx, diff[diffidx - 2], diff[diffidx - 1]);
					#endif
					break;

				case 3: // 11: One 4-byte difference
					if ( swapflag ) {
						Core::Endianess::swap(&word->d32);
					}

					diff[diffidx++] = word->d32;

					#if DECODE_DEBUG
					ms_log (0, "  W%02d: 11=1x32b  %d\n", widx, diff[diffidx - 1]);
					#endif
					break;
			} // Done with decoding 32-bit word based on nibble
		} // Done looping over nibbles and 32-bit words

		// Apply differences in this frame to calculate output samples,
		// ignoring first difference for first frame
		for ( idx = (frameidx == 0) ? 1 : 0; idx < diffidx && outputidx < sampleCount;
		      idx++, outputidx++ ) {
			output[outputidx] = output[outputidx - 1] + diff[idx];
		}
	} // Done looping over frames

	// Check data integrity by comparing last sample to Xn (reverse integration constant)
	if ( outputidx == sampleCount && output[outputidx - 1] != Xn ) {
		SEISCOMP_WARNING("Data integrity check for Steim1 failed, Last sample=%d, Xn=%d\n",
		                 output[outputidx - 1], Xn);
	}

	return outputidx;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
