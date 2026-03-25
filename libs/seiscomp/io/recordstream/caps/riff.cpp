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


#include "riff.h"
#include "endianess.h"

#include <cstdio>
#include <cstring>


namespace Seiscomp {
namespace IO {
namespace CAPS {
namespace RIFF {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Chunk::~Chunk() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <int SIZE_T, bool BigEndian>
VectorChunk<SIZE_T,BigEndian>::VectorChunk(std::vector<char> &d)
: data(d), startOfs(-1), len(-1) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <int SIZE_T, bool BigEndian>
VectorChunk<SIZE_T,BigEndian>::VectorChunk(std::vector<char> &d, int ofs, int count)
: data(d), startOfs(ofs), len(count) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <int SIZE_T, bool BigEndian>
int VectorChunk<SIZE_T,BigEndian>::chunkSize() const {
	return data.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <int SIZE_T, bool BigEndian>
bool VectorChunk<SIZE_T,BigEndian>::get(std::streambuf &input, int size) {
	int count = size/SIZE_T;

	if ( len >= 0 ) {
		if ( len > count )
			return false;

		count = len;
	}

	// Skip first samples (bytes = samples*sizeof(T))
	if ( startOfs > 0 )
		input.pubseekoff(startOfs*SIZE_T, std::ios_base::cur);

	Endianess::Reader r(input);

	data.resize(count*SIZE_T);
	r(data.data(), data.size());

	// Convert array to little endian
	Endianess::ByteSwapper<BigEndian,SIZE_T>::Take(data.data(), count);

	// Go the end of chunk
	if ( (int)data.size() < size )
		input.pubseekoff(size-data.size(), std::ios_base::cur);

	return r.good;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template struct VectorChunk<1,false>;
template struct VectorChunk<1,true>;
template struct VectorChunk<2,false>;
template struct VectorChunk<2,true>;
template struct VectorChunk<4,false>;
template struct VectorChunk<4,true>;
template struct VectorChunk<8,false>;
template struct VectorChunk<8,true>;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
}
