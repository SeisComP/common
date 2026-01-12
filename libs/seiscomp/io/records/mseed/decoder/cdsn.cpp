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
#define MAX14 0x1FFFul
#define CDSN_MANTISSA_MASK 0x3FFFul  // mask for mantissa
#define CDSN_GAINRANGE_MASK 0xC000ul // mask for gainrange factor
#define CDSN_SHIFT 14                // # bits in mantissa
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int64_t decodeCDSN(const char *net, const char *sta, const char *loc, const char *cha,
                   int16_t *input, size_t sampleCount, int32_t *output, size_t outputLength,
                   bool swapflag) {
	uint32_t idx = 0;
	int32_t mantissa;  // mantissa
	int32_t gainrange; // gain range factor
	int32_t mult = -1; // multiplier for gain range
	uint16_t sint;
	int32_t sample;

	if ( !sampleCount ) {
		return 0;
	}

	if ( !input || !output || (outputLength == 0) ) {
		return -1;
	}

	for ( idx = 0; idx < sampleCount && outputLength >= sizeof (int32_t); idx++ ) {
		memcpy(&sint, &input[idx], sizeof (int16_t));
		if ( swapflag ) {
			Core::Endianess::swap(&sint);
		}

		// Recover mantissa and gain range factor
		mantissa = (sint & CDSN_MANTISSA_MASK);
		gainrange = (sint & CDSN_GAINRANGE_MASK) >> CDSN_SHIFT;

		// Determine multiplier from the gain range factor and format definition
		// because shift operator is used later, these are powers of two
		if ( gainrange == 0 ) {
			mult = 0;
		}
		else if ( gainrange == 1 ) {
			mult = 2;
		}
		else if ( gainrange == 2 ) {
			mult = 4;
		}
		else if ( gainrange == 3 ) {
			mult = 7;
		}

		// Unbias the mantissa
		mantissa -= MAX14;

		// Calculate sample from mantissa and multiplier using left shift
		// mantissa << mult is equivalent to mantissa * (2 exp (mult))
		sample = ((uint32_t)mantissa << mult);

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
