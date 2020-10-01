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


#ifndef SC_IO_BINARYRECORD_H
#define SC_IO_BINARYRECORD_H

#include <seiscomp/core/genericrecord.h>
#include <seiscomp/core.h>

namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(BinaryRecord);


class SC_SYSTEM_CORE_API BinaryRecord : public GenericRecord {
	public:
		//! Default Constructor
		BinaryRecord();

	public:
		void read(std::istream &in);
		void write(std::ostream &out);
};

}
}

#endif
