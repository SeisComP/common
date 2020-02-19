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


#include "stringfirewall.h"

#include <seiscomp/core/strings.h>


using namespace std;


namespace Seiscomp {
namespace Util {


namespace {


bool passes(const StringFirewall::StringSet &ids, const std::string &id) {
	StringFirewall::StringSet::iterator it;
	for ( it = ids.begin(); it != ids.end(); ++it ) {
		if ( Core::wildcmp(*it, id) )
			return true;
	}

	return false;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StringFirewall::isAllowed(const std::string &s) const {
	return (allow.empty()?true:allow.find(s) != allow.end())
	    && (deny.empty()?true:deny.find(s) == deny.end());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StringFirewall::isDenied(const std::string &s) const {
	return !isAllowed(s);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WildcardStringFirewall::WildcardStringFirewall()
: _enableCaching(true) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WildcardStringFirewall::clearCache() const {
	_cache.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void WildcardStringFirewall::setCachingEnabled(bool enable) {
	_enableCaching = enable;
	if ( !_enableCaching )
		_cache.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool WildcardStringFirewall::isAllowed(const std::string &s) const {
	// If no rules are configured, don't cache anything and just return true
	if ( allow.empty() && deny.empty() ) return true;

	if ( _enableCaching ) {
		StringPassMap::const_iterator it = _cache.find(s);

		// Not yet cached, evaluate the string
		if ( it == _cache.end() ) {
			bool check = (allow.empty()?true:passes(allow, s))
			          && (deny.empty()?true:!passes(deny, s));
			_cache[s] = check;
			return check;
		}

		// Return cached result
		return it->second;
	}
	else {
		return (allow.empty()?true:passes(allow, s))
		    && (deny.empty()?true:!passes(deny, s));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
