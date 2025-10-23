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


#include <seiscomp/core/strings.h>
#include <string>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::IO::MSEED {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool sid2nslc(std::string_view sid, std::string &net, std::string &sta,
              std::string &loc, std::string &cha) {
	if ( sid.compare(0, 5, "FDSN:") ) {
		return false;
	}

	sid = sid.substr(sid.rfind(':') + 1);

	int token = 0;

	for ( auto sv = Core::tokenize(sid, "_"); sv.data(); sv = Core::tokenize(sid, "_"), ++token ) {
		if ( token == 0 ) {
			net = sv;
		}
		else if ( token == 1 ) {
			sta = sv;
		}
		else if ( token == 2 ) {
			loc = sv;
		}
		else if ( token == 3 ) {
			if ( sv.length() != 1 ) {
				return false;
			}
			cha = sv;
		}
		else if ( token == 4 ) {
			if ( sv.length() != 1 ) {
				return false;
			}
			cha += sv;
		}
		else if ( token == 5 ) {
			if ( sv.length() != 1 ) {
				return false;
			}
			cha += sv;
		}
		else {
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
