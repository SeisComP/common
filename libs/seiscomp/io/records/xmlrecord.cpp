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


#include <seiscomp/io/records/xmlrecord.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <stdio.h>


namespace Seiscomp {

namespace IO {


REGISTER_RECORD(XMLRecord, "xml");


XMLRecord::XMLRecord() {}

void XMLRecord::read(std::istream &in) {
	XMLArchive ar(in.rdbuf(), true);
	ar & NAMED_OBJECT("GenericRecord", *this);
	// Setting the eof bit causes the input to abort the reading
	if ( in.rdbuf()->sgetc() == EOF )
		in.setstate(std::ios_base::eofbit);
}

void XMLRecord::write(std::ostream &out) {
	XMLArchive ar(out.rdbuf(), false);
	ar & NAMED_OBJECT("GenericRecord", *this);
}

}
}
