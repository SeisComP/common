/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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

#include "session.h"
#include "version.h"
#include "strings.h"

#define SEISCOMP_COMPONENT WFAS
#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/records/mseedrecord.h>

#include <iostream>
#include <ctype.h>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace std;


namespace Seiscomp {
namespace Applications {
namespace Wfas {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool parseTime(Core::Time &time, const char *data, size_t len) {
	const char *tok;
	size_t tok_len;
	size_t tok_count = 0;

	int toks[6];

	for ( ; (tok = tokenize(data, ",", len, tok_len)) != NULL; ++tok_count ) {
		// Too many tokens
		if ( tok_count >= 6 )
			return false;

		// Too many characters per token
		if ( tok_len > 4 )
			return false;

		toks[tok_count] = 0;
		int base = 1;
		tok += tok_len-1;
		while ( tok_len-- ) {
			// Invalid format
			if ( !isdigit(*tok) )
				return false;

			toks[tok_count] += (*tok - '0') * base;
			--tok;
			base *= 10;
		}
	}

	if ( tok_count != 6 )
		return false;

	return time.set(toks[0], toks[1], toks[2], toks[3], toks[4], toks[5], 0).valid();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool validate(const string &code) {
	return code.find('?') == string::npos && code.find('*') == string::npos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ClientSession::ClientSession(Wired::Socket *s, size_t maxLen)
: Wired::ClientSession(s, maxLen) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ClientSession::inAvail() const {
	return _bytesPending + Wired::ClientSession::inAvail();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}
