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


#define SEISCOMP_COMPONENT RoutingConnection

#include "routing_private.h"

#include <seiscomp/logging/log.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/io/recordinput.h>

#include <cstdio>
#include <functional>


using namespace std;
using namespace boost;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;


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




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_RECORDSTREAM(RoutingConnection, "routing");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RoutingConnection::setSource(const string &source) {
	if ( _started )
		return false;

	_rsarray.clear();
	_rules.clear();

	/*
	 * Format of source is:
	 *  type1/source1??match=pattern;type2/source2??match=pattern;...;typeN/sourceN??match=pattern
	 * where
	 *  sourceN is either source or (source)
	 *  pattern is NET.STA.LOC.CHA and the special charactes ? * | ( ) are allowed
	 */
	string input = source;

	do {
		// Extract type1: anything until '/' (not greedy)
		static const regex re1("^(.+?)/",regex::optimize);
		smatch m1;
		if ( ! regex_search(input, m1, re1) ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': cannot find service type in '%s'",
			                source.c_str(), input.c_str());
			throw RecordStreamException("Invalid RecordStream URL");
		}
		const string type1 = m1[1].str();

		input = input.substr( m1.length() );

		// Extract source1: anything until ?? (not greedy)
		string source1;
		if ( input[0] != '(' ) {
			static const regex re2( R"(^(.+?)\?\?)",regex::optimize);
			smatch m2;
			if ( ! regex_search(input, m2, re2) ) {
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': cannot find source in '%s'",
				                source.c_str(), input.c_str());
				throw RecordStreamException("Invalid RecordStream URL");
			}
			source1 = m2[1].str();
			input = input.substr( m2.length() );
		}
		else { // Source surrounded by parentheses
			// Find closing parenthesis
			size_t p2 = findClosingParenthesis(input, 1);
			if ( p2 == string::npos ) {
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected closing parenthesis in '%s'",
				               source.c_str(), input.c_str());
				throw RecordStreamException("Invalid RecordStream URL");
			}
			source1 = input.substr(1, p2-1);
			if ( input.substr(p2, 3) != ")??" ){
				SEISCOMP_ERROR("Invalid RecordStream URL '%s': expected ?? after closing parenthesis in '%s'",
				               source.c_str(), input.c_str());
				throw RecordStreamException("Invalid RecordStream URL");
			}
			input = input.substr(p2+3);
		}

		// Extract matching rule
		static const regex re3( R"(^match=([A-Z|a-z|0-9|\?|\*|\||\(|\)]+\.)"
		                                R"([A-Z|a-z|0-9|\?|\*|\||\(|\)]+\.)"
		                                R"([A-Z|a-z|0-9|\?|\*|\||\(|\)]+\.)"
		                                R"([A-Z|a-z|0-9|\?|\*|\||\(|\)]+)"
		                                 ");?", regex::optimize);
		smatch m3;
		if ( ! regex_search(input, m3, re3) ) {
			SEISCOMP_ERROR("Invalid RecordStream URL '%s': cannot find match option in '%s'",
			               source.c_str(), input.c_str());
			throw RecordStreamException("Invalid RecordStream URL");
		}

		const string match1 = m3[1].str();

		input = input.substr( m3.length() );

		// convert user special characters (* ? .) to regex equivalent
		const string reMatch = regex_replace(
			regex_replace(
				regex_replace(
					match1,
					regex("\\."),
					string("\\."),
					match_flag_type::format_literal
				),
				regex("\\?"),
				string("."),
				match_flag_type::format_literal
			),
			regex("\\*"),
			string(".*"),
			match_flag_type::format_literal
		);

		SEISCOMP_DEBUG("Type   : %s", type1.c_str());
		SEISCOMP_DEBUG("Source : %s", source1.c_str());
		SEISCOMP_DEBUG("Match  : %s (regex %s)", match1.c_str(), reMatch.c_str());

		RecordStreamPtr rs = RecordStream::Create(type1.c_str());

		if ( rs == nullptr ) {
			SEISCOMP_ERROR("Invalid RecordStream type: %s", type1.c_str());
			return false;
		}

		if ( !rs->setSource(source1) ) {
			SEISCOMP_ERROR("Invalid RecordStream source: %s", source1.c_str());
			return false;
		}

		try {
			regex re = regex(reMatch, regex::optimize);
			_rsarray.push_back(make_pair(rs, false));
			_rules.push_back(Rule{type1, source1, match1, re});
		}
		catch ( const regex_error & ) {
			SEISCOMP_ERROR("Invalid match syntax in RecordStream URL: %s", source.c_str());
			throw RecordStreamException("Invalid RecordStream URL");
		}

	}
	while ( !input.empty() );

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RoutingConnection::getRS(const string &net, const string &sta,
                             const string &loc, const string &cha) {
	const string stream = net + "." + sta + "." + loc +  "." + cha;

	for ( size_t i = 0; i < _rules.size(); ++i) {
		if ( regex_match(stream, _rules[i].match) ) {
			SEISCOMP_DEBUG("stream %s -> recordstream %lu (%s %s %s)",
			               stream.c_str(), i, _rules[i].type.c_str(),
			               _rules[i].source.c_str(), _rules[i].matchOpt.c_str());
			return i;
		}
	}

	SEISCOMP_DEBUG("stream %s doesn't match any RecordStream", stream.c_str());
	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // private namespace
