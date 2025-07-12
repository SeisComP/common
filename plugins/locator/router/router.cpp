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


#define SEISCOMP_COMPONENT RouterLocator

#include "router.h"

#include <seiscomp/logging/log.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/system/environment.h>

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Seismology;


namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ADD_SC_PLUGIN("Meta locator routing location requests to actual locator "
              "implementations",
              "gempa GmbH <seiscomp-devel@gempa.de>", 0, 1, 0)
REGISTER_LOCATOR(RouterLocator, "Router");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string locProcIndex(const string &locator, const string &profile) {
	return locator + "_" + profile;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RouterLocator::init(const Seiscomp::Config::Config &config) {
	string locator;
	try {
	 	locator = config.getString("RouterLocator.initial.locator");
	}
	catch ( ... ) {}

	string profile;
	try {
	 	profile = config.getString("RouterLocator.initial.profile");
	}
	catch ( ... ) {}

	if ( !locator.empty() ) {
		_initialLocator = LocatorInterface::Create(locator.c_str());
		if ( !_initialLocator ) {
			SEISCOMP_ERROR("Could not load initial locator: " + locator);
			return false;
		}

		if ( !_initialLocator->init(config) ) {
			SEISCOMP_ERROR("Could not initialize initial locator: " + locator);
			return false;
		}

		if ( !profile.empty() ) {
			const auto &profiles = _initialLocator->profiles();
			if ( find(profiles.begin(), profiles.end(), profile) == profiles.end() ) {
				SEISCOMP_ERROR("Profile '" + profile +
				               "' not supported by initial locator '" +
				               locator + "'");
				return false;
			}

			_initialLocator->setProfile(profile);
		}

		_locators[locProcIndex(locator, profile)] = _initialLocator;
		_joinedCapabilities |= _initialLocator->capabilities();
	}

	auto *env = Seiscomp::Environment::Instance();
	if ( !env ) {
		SEISCOMP_ERROR("Could not load SeisComP environment");
		return false;
	}

	string regionFile;
	try {
		regionFile = env->absolutePath(config.getString("RouterLocator.regions"));
	}
	catch ( ... ) {}

	if ( regionFile.empty() ) {
		SEISCOMP_ERROR("No region file specified");
		return false;
	}

	if ( !_geoFeatureSet.readFile(regionFile, nullptr) ) {
		SEISCOMP_ERROR("No regions found in: %s", regionFile.c_str());
		return false;
	}

	for ( const auto &feature : _geoFeatureSet.features() ) {
		LocatorProfile profile;
		const auto *featureName = feature->name().c_str();

		double depth;
		for ( const auto &[key, value] : feature->attributes() ) {
			if ( key == "locator") {
				profile.locatorName = value;
			}
			else if ( key == "profile" ) {
				profile.profileName = value;
			}
			else if ( key == "minDepth" ) {
				if ( fromString(depth, value) ) {
					profile.minDepth = depth;
				}
				else {
					SEISCOMP_WARNING("[%s] Invalid minDepth", featureName);
				}
			}
			else if ( key == "maxDepth" ) {
				if ( fromString(depth, value) ) {
					profile.maxDepth = depth;
				}
				else {
					SEISCOMP_WARNING("[%s] Invalid maxDepth", featureName);
				}
			}
		}

		if ( profile.locatorName.empty() ) {
			SEISCOMP_WARNING("[%s] Missing locator name", featureName);
			continue;
		}

		// lookup profile locator
		string key = locProcIndex(profile.locatorName, profile.profileName);
		auto loc_it = _locators.find(key);
		const auto *locName = profile.locatorName.c_str();

		// cache miss: create and initialize locator
		if ( loc_it == _locators.end() ) {
			LocatorInterfacePtr tmpLoc = LocatorInterface::Create(locName);
			if ( !tmpLoc ) {
				SEISCOMP_WARNING("[%s] Could not load locator: %s",
				                 featureName, locName);
				continue;
			}

			if ( !tmpLoc->init(config) ) {
				SEISCOMP_WARNING("[%s] Could not initialize locator: %s",
				                 featureName, locName);
				continue;
			}

			if ( !profile.profileName.empty() ) {
				const auto &profiles = tmpLoc->profiles();
				if ( find(profiles.begin(), profiles.end(),
				          profile.profileName) == profiles.end() ) {
					SEISCOMP_WARNING("[%s] Profile '%s' not supported by "
					                 "locator: %s", featureName,
					                 profile.profileName.c_str(), locName);
					continue;
				}
				tmpLoc->setProfile(profile.profileName);
			}

			_locators[key] = tmpLoc;
			_joinedCapabilities |= tmpLoc->capabilities();

			profile.feature = feature;
			profile.locator = tmpLoc.get();
			_profiles.emplace_back(profile);
		}
	}

	if ( _profiles.empty() ) {
		SEISCOMP_ERROR("No valid profiles found in: %s", regionFile.c_str());
		return false;
	}

	// sort by rank and area, prioritize larger ranks and smaller areas
	std::sort(_profiles.begin(), _profiles.end(),
	          [](const LocatorProfile& a, const LocatorProfile& b) {
		return a.feature->rank() > a.feature->rank() ||
		       ( a.feature->rank() == a.feature->rank() &&
		         a.feature->area() < b.feature->area() );
	});

	SEISCOMP_DEBUG("Router locator initialized with %lu profiles",
	               _profiles.size());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RouterLocator::IDList RouterLocator::profiles() const {
	return {};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RouterLocator::setProfile(const string &name) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RouterLocator::capabilities() const {
	return _joinedCapabilities;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *RouterLocator::locate(PickList &pickList) {
	if ( !_initialLocator ) {
		return nullptr;
	}

	auto *origin = _initialLocator->locate(pickList);
	return origin ? relocate(origin) : nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *RouterLocator::locate(PickList &pickList,
                              double initLat, double initLon, double initDepth,
                              const Time &initTime) {
	if ( !_initialLocator ) {
		return nullptr;
	}

	auto *origin =_initialLocator->locate(pickList, initLat, initLon, initDepth,
	                                      initTime);
	return origin ? relocate(origin) : nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Origin *RouterLocator::relocate(const Origin *origin) {
	const auto *locProf = lookup(origin);
	if ( !locProf ) {
		throw LocatorException("Could not find suitable locator profile for "
		                       "initial location");
	}

	return locProf->locator->relocate(origin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RouterLocator::LocatorProfile*
RouterLocator::lookup(const scdm::Origin *origin) const {
	if ( !origin ) {
		return {};
	}

	auto epicenter = Geo::GeoCoordinate(origin->latitude().value(),
	                                    origin->longitude().value());
	OPT(double) depth;
	try {
		depth = origin->depth().value();
		SEISCOMP_DEBUG("Locator lookup for hypocenter: %f/%f/%f",
		               epicenter.latitude(), epicenter.latitude(), *depth);
	}
	catch ( ValueException& ) {
		SEISCOMP_DEBUG("Locator lookup for epicenter: %f/%f",
		               epicenter.latitude(), epicenter.latitude());
	}

	for ( const auto &profile : _profiles ) {
		// test if epicenter is contained in geofeature
		if ( !profile.feature->contains(epicenter) ) {
			continue;
		}

		// skip if depth is not available but profile defines depth contrains
		if ( !depth && ( profile.minDepth || profile.maxDepth ) ) {
			continue;
		}

		// check if depth constrains are fulfilled
		if ( (profile.minDepth && depth < *profile.minDepth) ||
		     (profile.maxDepth && depth > *profile.maxDepth) ) {
			continue;
		}

		SEISCOMP_DEBUG("Feature '%s' matches %scenter, locator: %s/%s",
		               profile.feature->name().c_str(),
		               profile.minDepth||profile.maxDepth ? "hypo" : "epi",
		               profile.locatorName.c_str(),
		               profile.profileName.empty() ?
		                   "-" : profile.profileName.c_str());

		return &profile;
	}

	return {};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
