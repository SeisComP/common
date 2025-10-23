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


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::IO::MSEED {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define SRO_MANTISSA_MASK 0x0FFFul  // mask for mantissa
#define SRO_GAINRANGE_MASK 0xF000ul // mask for gainrange factor
#define SRO_SHIFT 12                // # bits in mantissa

#define MAX12 0x7FFul
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int64_t decodeSRO(int16_t *input, size_t sampleCount, int32_t *output, size_t outputLength,
                  bool swapflag) {
	uint32_t idx = 0;
	int32_t mantissa;   // mantissa
	int32_t gainrange;  // gain range factor
	int32_t add2gr;     // added to gainrage factor
	int32_t mult;       // multiplier for gain range
	int32_t add2result; // added to multiplied gain rage
	int32_t exponent;   // total exponent
	uint16_t sint;
	int32_t sample;

	if ( !sampleCount ) {
		return 0;
	}

	if ( !input || !output || (outputLength == 0) ) {
		return -1;
	}

	add2gr = 0;
	mult = -1;
	add2result = 10;

	for ( idx = 0; idx < sampleCount && outputLength >= sizeof (int32_t); idx++ ) {
		memcpy(&sint, &input[idx], sizeof (int16_t));
		if ( swapflag ) {
			Core::Endianess::swap(&sint);
		}

		// Recover mantissa and gain range factor
		mantissa = (sint & SRO_MANTISSA_MASK);
		gainrange = (sint & SRO_GAINRANGE_MASK) >> SRO_SHIFT;

		// Take 2's complement for mantissa
		if ( (unsigned long)mantissa > MAX12 ) {
			mantissa -= 2 * (MAX12 + 1);
		}

		// Calculate exponent, SRO exponent = 0..10
		exponent = (mult * (gainrange + add2gr)) + add2result;

		if ( (exponent < 0) || (exponent > 10) ) {
			SEISCOMP_ERROR("SRO gain ranging exponent out of range: %d", exponent);
			return -1;
		}

		// Calculate sample as mantissa * 2^exponent
		sample = mantissa * ((uint64_t)1 << exponent);

		// Save sample in output array
		output[idx] = sample;
		outputLength -= sizeof (int32_t);
	}

	return idx;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
