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


#include <seiscomp/datamodel/network.h>
#include <seiscomp/datamodel/utils.h>
#include <seiscomp/utils/bindings.h>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

Core::MetaValue empty;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Bindings::Bindings() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::init(const DataModel::ConfigModule *cfg, const std::string &setupName,
                    bool allowGlobal) {
	_bindings.clear();

	if ( cfg == nullptr )
		return false;

	for ( size_t i = 0; i < cfg->configStationCount(); ++i ) {
		DataModel::ConfigStation *sta_cfg = cfg->configStation(i);
		if ( !cfg->enabled() ) continue;

		DataModel::Setup *setup = DataModel::findSetup(sta_cfg, setupName, allowGlobal);
		if ( (setup == nullptr) || !setup->enabled() ) continue;

		DataModel::ParameterSet* ps = DataModel::ParameterSet::Find(setup->parameterSetID());
		if ( ps == nullptr ) {
			/*
			SEISCOMP_WARNING("%s.%s: parameter set '%s' not found",
			                 sta_cfg->networkCode().c_str(),
			                 sta_cfg->stationCode().c_str(),
			                 setup->parameterSetID().c_str());
			*/
			continue;
		}

		KeyValuesPtr params = new KeyValues;
#if SC_API_VERSION < 0x012000
		params->readFrom(ps);
#else
		params->init(ps);
#endif

		_bindings[sta_cfg->networkCode()][sta_cfg->stationCode()].keys = params;
	}

	return !_bindings.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const KeyValues *Bindings::getKeys(const std::string &networkCode,
                                   const std::string &stationCode) const {
	NetworkMap::const_iterator it = _bindings.find(networkCode);
	if ( it == _bindings.end() )
		return nullptr;

	StationMap::const_iterator it2 = it->second.find(stationCode);
	if ( it2 == it->second.end() )
		return nullptr;

	return it2->second.keys.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const KeyValues *Bindings::getKeys(const DataModel::Station *station) const {
	if ( station->network() == nullptr )
		return nullptr;

	return getKeys(station->network()->code(), station->code());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::remove(const std::string &networkCode,
                      const std::string &stationCode) {
	NetworkMap::iterator it = _bindings.find(networkCode);
	if ( it == _bindings.end() )
		return false;

	StationMap::iterator it2 = it->second.find(stationCode);
	if ( it2 == it->second.end() )
		return false;

	it->second.erase(it2);
	if ( it->second.empty() )
		_bindings.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::remove(const DataModel::Station *station) {
	if ( !station->network() )
		return false;

	return remove(station->network()->code(), station->code());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::setData(const std::string &networkCode,
                       const std::string &stationCode,
                       const Core::MetaValue &value) {
	NetworkMap::iterator it = _bindings.find(networkCode);
	if ( it == _bindings.end() )
		return false;

	StationMap::iterator it2 = it->second.find(stationCode);
	if ( it2 == it->second.end() )
		return false;

	it2->second.data = value;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::setData(const DataModel::Station *station,
                       const Core::MetaValue &value) {
	if ( station->network() == nullptr )
		return false;

	return setData(station->network()->code(), station->code(), value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Core::MetaValue &Bindings::data(const std::string &networkCode,
                                      const std::string &stationCode) {
	NetworkMap::iterator it = _bindings.find(networkCode);
	if ( it == _bindings.end() )
		return empty;

	StationMap::iterator it2 = it->second.find(stationCode);
	if ( it2 == it->second.end() )
		return empty;

	return it2->second.data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Core::MetaValue &Bindings::data(const DataModel::Station *station) {
	if ( station->network() == nullptr )
		return empty;

	return data(station->network()->code(), station->code());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Bindings::const_iterator Bindings::begin() const {
	const_iterator it;
	it._networks = &_bindings;
	it._stations = nullptr;
	it._nIt = _bindings.begin();

	if ( it._nIt != _bindings.end() ) {
		it._stations = &it._nIt->second;
		it._sIt = it._nIt->second.begin();
	}
	else
		it = end();

	return it;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Bindings::const_iterator Bindings::end() const {
	return const_iterator();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
