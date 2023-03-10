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


#include <seiscomp/system/environment.h>
#include <seiscomp/system/settings.h>


using namespace std;


namespace Seiscomp {
namespace System {

namespace Generic {

namespace Detail {


template <>
int getConfig(const Config::Config *cfg, const std::string &symbol, bool) {
	return cfg->getInt(symbol);
}

template <>
size_t getConfig(const Config::Config *cfg, const std::string &symbol, bool) {
	return static_cast<size_t>(cfg->getInt(symbol));
}

template <>
double getConfig(const Config::Config *cfg, const std::string &symbol, bool) {
	return cfg->getDouble(symbol);
}

template <>
bool getConfig(const Config::Config *cfg, const std::string &symbol, bool) {
	return cfg->getBool(symbol);
}

template <>
std::string getConfig(const Config::Config *cfg, const std::string &symbol, bool asPath) {
	if ( !asPath )
		return cfg->getString(symbol);
	else
		return Environment::Instance()->absolutePath(cfg->getString(symbol));
}

template <>
std::vector<int> getConfig(const Config::Config *cfg, const std::string &symbol, bool) {
	return cfg->getInts(symbol);
}

template <>
std::vector<double> getConfig(const Config::Config *cfg, const std::string &symbol, bool) {
	return cfg->getDoubles(symbol);
}

template <>
std::vector<bool> getConfig(const Config::Config *cfg, const std::string &symbol, bool) {
	return cfg->getBools(symbol);
}

template <>
std::vector<std::string> getConfig(const Config::Config *cfg, const std::string &symbol, bool asPath) {
	if ( !asPath )
		return cfg->getStrings(symbol);

	std::vector<std::string> items = cfg->getStrings(symbol);
	for ( size_t i = 0; i < items.size(); ++i )
		items[i] = Environment::Instance()->absolutePath(items[i]);

	return items;
}


}

}

}
}
