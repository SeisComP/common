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
#define MAX16 0x7FFFul
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int64_t decodeDWWSSN(int16_t *input, size_t sampleCount, int32_t *output, size_t outputLength,
                     bool swapflag) {
	uint32_t idx = 0;
	int32_t sample;
	uint16_t sint;

	if ( !sampleCount ) {
		return 0;
	}

	if ( !input || !output || outputLength == 0 ) {
		return -1;
	}

	for ( idx = 0; idx < sampleCount && outputLength >= sizeof (int32_t); idx++ ) {
		memcpy(&sint, &input[idx], sizeof (uint16_t));
		if ( swapflag ) {
			Core::Endianess::swap(&sint);
		}
		sample = (int32_t)sint;

		// Take 2's complement for sample
		if ( (unsigned long)sample > MAX16 ) {
			sample -= 2 * (MAX16 + 1);
		}

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
