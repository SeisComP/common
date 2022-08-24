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


#define SEISCOMP_COMPONENT BalancedConnection

#include <cstdio>
#include <string>
#include <iostream>
#include <functional>

#include <seiscomp/logging/log.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/recordinput.h>

#include "balanced_private.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::IO;
using namespace Seiscomp::RecordStream;


namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t findClosingParenthesis(const string &s, size_t p) {
	int cnt = 1;
	for ( size_t i = p; i < s.size(); ++i ) {
		if ( s[i] == '(' ) ++cnt;
		else if ( s[i] == ')' ) --cnt;
		if ( !cnt ) return i;
	}

	return string::npos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_RECORDSTREAM(BalancedConnection, "balanced");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BalancedConnection::setSource(const string &source) {
	if ( _started )
		return false;

	reset();

	_rsarray.clear();

	size_t p1,p2;

	/*
	 * Format of source is:
	 *  type1/source1;type2/source2;...;typeN/sourceN
	 * where
	 *  sourceN is either source or (source)
	 */

	string serverloc = source;

	while (true) {
		// Find first slash
		p1 = serverloc.find('/');
		string type1;

		if ( p1 == string::npos ) {
			type1 = "slink";
			p1 = 0;
		}
		else {
			type1 = serverloc.substr(0, p1);
			// Move behind '/'
			++p1;
		}

		string source1;

		// Extract source1
		if ( p1 >= serverloc.size() ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': missing source",
				       serverloc.c_str());
			throw RecordStreamException("Invalid RecordStream URL");
		}

		// Source surrounded by parentheses
		if ( serverloc[p1] == '(' ) {
			++p1;
			// Find closing parenthesis
			p2 = findClosingParenthesis(serverloc, p1);
			if ( p2 == string::npos ) {
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected closing parenthesis",
					       serverloc.c_str());
				throw RecordStreamException("Invalid RecordStream URL");
			}

			source1 = serverloc.substr(p1, p2-p1);
			++p2;
		}
		else {
			p2 = serverloc.find(';', p1);
			if ( p2 == string::npos ) {
				p2 = serverloc.length();
			}

			source1 = serverloc.substr(p1, p2-p1);
		}

		SEISCOMP_DEBUG("Type   : %s", type1.c_str());
		SEISCOMP_DEBUG("Source : %s", source1.c_str());

		RecordStreamPtr rs = RecordStream::Create(type1.c_str());

		if ( rs == nullptr ) {
			SEISCOMP_ERROR("Invalid RecordStream type: %s", type1.c_str());
			return false;
		}

		if ( !rs->setSource(source1) ) {
			SEISCOMP_ERROR("Invalid RecordStream source: %s", source1.c_str());
			return false;
		}

		_rsarray.push_back(make_pair(rs, false));

		if ( p2 == serverloc.length() )
			break;

		serverloc = serverloc.substr(p2 + 1, string::npos);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int BalancedConnection::getRS(const string &net, const string &sta,
                              const string &loc, const string &cha) {
	// create hash on stastion code
	size_t i = 0;
	for ( const char* p = sta.c_str(); *p != 0; ++p ) i += *p;

	return i % _rsarray.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
