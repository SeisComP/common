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
#include <cstring>

#include "./format.h"


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::IO::MSEED {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define MAX24 0x7FFFFFul // maximum 24 bit positive
#define GEOSCOPE_MANTISSA_MASK 0x0FFFul /* mask for mantissa */
#define GEOSCOPE_GAIN3_MASK 0x7000ul    /* mask for gainrange factor */
#define GEOSCOPE_GAIN4_MASK 0xf000ul    /* mask for gainrange factor */
#define GEOSCOPE_SHIFT 12               /* # bits in mantissa */
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int64_t decodeGEOSCOPE(char *input, size_t sampleCount, float *output, size_t outputLength,
                       EncodingType encoding, bool swapflag) {
  uint32_t idx = 0;
  int32_t mantissa;  /* mantissa from SEED data */
  int32_t gainrange; /* gain range factor */
  int32_t exponent;  /* total exponent */
  int32_t k;
  uint64_t exp2val;
  int16_t sint;
  double dsample = 0.0;

	union {
		uint8_t b[4];
		uint32_t i;
	} sample32;

	if ( !sampleCount ) {
		return 0;
	}

	if ( !input || !output || outputLength == 0 ) {
		return -1;
	}

	// Make sure we recognize this as a GEOSCOPE encoding format
	if ( encoding != EncodingType::GEOSCOPE24 && encoding != EncodingType::GEOSCOPE163 &&
	     encoding != EncodingType::GEOSCOPE164 ) {
		SEISCOMP_ERROR("unrecognized GEOSCOPE encoding: %d", static_cast<int>(encoding));
		return -1;
	}

	for ( idx = 0; idx < sampleCount && outputLength >= sizeof (float); idx++ ) {
		switch ( encoding ) {
			case EncodingType::GEOSCOPE24:
				sample32.i = 0;
				if ( swapflag ) {
					for ( k = 0; k < 3; k++ ) {
						sample32.b[2 - k] = input[k];
					}
				}
				else {
					for ( k = 0; k < 3; k++ ) {
						sample32.b[1 + k] = input[k];
					}
				}

				mantissa = sample32.i;

				// Take 2's complement for mantissa for overflow
				if ( (unsigned long)mantissa > MAX24 ) {
					mantissa -= 2 * (MAX24 + 1);
				}

				// Store
				dsample = (double)mantissa;
				break;

			case EncodingType::GEOSCOPE163:
				memcpy(&sint, input, sizeof (int16_t));
				if ( swapflag ) {
					Core::Endianess::swap(&sint);
				}

				// Recover mantissa and gain range factor
				mantissa = (sint & GEOSCOPE_MANTISSA_MASK);
				gainrange = (sint & GEOSCOPE_GAIN3_MASK) >> GEOSCOPE_SHIFT;

				// Exponent is just gainrange for GEOSCOPE
				exponent = gainrange;

				// Calculate sample as mantissa / 2^exponent
				exp2val = (uint64_t)1 << exponent;
				dsample = ((double)(mantissa - 2048)) / exp2val;

				break;

			case EncodingType::GEOSCOPE164:
				memcpy(&sint, input, sizeof (int16_t));
				if ( swapflag ) {
					Core::Endianess::swap(&sint);
				}

				// Recover mantissa and gain range factor
				mantissa = (sint & GEOSCOPE_MANTISSA_MASK);
				gainrange = (sint & GEOSCOPE_GAIN4_MASK) >> GEOSCOPE_SHIFT;

				// Exponent is just gainrange for GEOSCOPE
				exponent = gainrange;

				// Calculate sample as mantissa / 2^exponent
				exp2val = (uint64_t)1 << exponent;
				dsample = ((double)(mantissa - 2048)) / exp2val;

				break;
			default:
				break;
		}

		// Save sample in output array
		output[idx] = (float)dsample;
		outputLength -= sizeof (float);

		// Increment edata pointer depending on size
		switch ( encoding ) {
			case EncodingType::GEOSCOPE24:
				input += 3;
				break;
			case EncodingType::GEOSCOPE163:
			case EncodingType::GEOSCOPE164:
				input += 2;
				break;
			default:
				break;
		}
	}

	return idx;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
