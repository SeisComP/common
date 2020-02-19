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


#define SEISCOMP_COMPONENT Utils/Replace
#include <seiscomp/utils/replace.h>
#include <stdlib.h>

/*
#include <seiscomp/core/platform/platform.h>
#ifdef MACOSX
    // On OSX HOST_NAME_MAX is not define in sys/params.h. Therefoere we use MAXHOSTNAMELEN
    // instead on this platform.
    #include <sys/param.h>
    #define HOST_NAME_MAX MAXHOSTNAMELEN
#endif
*/
#include <seiscomp/system/hostinfo.h>

#include <seiscomp/logging/log.h>
#include <iostream>
#include <errno.h>
#include <cstdlib>

namespace Seiscomp {
namespace Util {

bool VariableResolver::resolve(std::string& variable) const {
	if ( variable == "hostname" )
		variable = System::HostInfo().name();
	else if ( variable == "user" ) {
		variable = System::HostInfo().login();
		if ( variable.empty() ) return false;
	}
	else
		return false;

	return true;
}

namespace {

VariableResolver defaultResolver;

}


std::string replace(const std::string& input) {
	return replace(input, defaultResolver);
}


std::string replace(const std::string &input,
                    const VariableResolver &resolver) {
	return replace(input, resolver, "@", "@", "@");
}


std::string replace(const std::string &input,
                    const VariableResolver &resolver,
                    const std::string &prefix, const std::string &postfix,
                    const std::string &emptyValue) {
	std::string::size_type lastPos = 0, pos = 0;
	std::string result;

	while ( (pos = input.find(prefix, pos)) != std::string::npos ) {
		std::string::size_type endPos;
		endPos = input.find(postfix, pos+prefix.size());
		if ( endPos == std::string::npos )
			break;

		// skip leading '@'
		std::string placeHolder = input.substr(pos+prefix.size(), endPos-pos-prefix.size());

		if ( placeHolder.empty() )
			placeHolder = emptyValue;
		else if ( !resolver.resolve(placeHolder) ) {
			pos = endPos+postfix.size();
			continue;
		}

		result.append(input, lastPos, pos-lastPos);
		result += placeHolder;

		pos = endPos+postfix.size();
		lastPos = pos;
	}

	result.append(input, lastPos, input.size()-lastPos);

	return result;
}


std::string replace(const char* input,
                    const VariableResolver& resolver) {
	return replace(std::string(input), resolver);
}


}
}
