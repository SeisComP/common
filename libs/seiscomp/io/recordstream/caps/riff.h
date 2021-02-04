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


#ifndef SEISCOMP_IO_CAPS_RIFF_H
#define SEISCOMP_IO_CAPS_RIFF_H


#include <vector>
#include <stdint.h>

#include "packet.h"


namespace Seiscomp {
namespace IO {
namespace CAPS {
namespace RIFF {


struct Chunk {
	virtual ~Chunk();

	bool read(std::istream &input, int size) { return get(*input.rdbuf(), size); }

	virtual bool get(std::streambuf &input, int size) = 0;

	virtual int chunkSize() const = 0;
};


template <int SIZE_T, bool BigEndian>
struct VectorChunk : Chunk {
	std::vector<char> &data;

	VectorChunk(std::vector<char> &d);

	// sampleOfs and sampleCount are not byte offsets but elements of
	// type T
	VectorChunk(std::vector<char> &d, int sampleOfs, int sampleCount);
	virtual ~VectorChunk() {}

	int chunkSize() const;

	bool get(std::streambuf &input, int size);

	int startOfs;
	int len;
};


}
}
}
}


#endif
