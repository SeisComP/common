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
              "implementations based on region profiles",
              "gempa GmbH <seiscomp-devel@gempa.de>", 0, 2, 0)
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

	string initialLocatorName = "<unset>";
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

		initialLocatorName = locator;
		if ( !profile.empty() ) {
			const auto &profiles = _initialLocator->profiles();
			if ( find(profiles.begin(), profiles.end(),
			          profile) == profiles.end() ) {
				SEISCOMP_ERROR("Profile '" + profile +
				               "' not supported by initial locator '" +
				               locator + "'");
				return false;
			}

			_initialLocator->setProfile(profile);
			initialLocatorName += " (";
			initialLocatorName += profile;
			initialLocatorName += ")";
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
		SEISCOMP_ERROR("No regions found in: %s", regionFile);
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

		// cache miss: create and initialize locator
		if ( loc_it == _locators.end() ) {
			const auto *locName = profile.locatorName.c_str();
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
					SEISCOMP_WARNING("[%s] Profile '%s' not supported by locator: %s",
					                 featureName, profile.profileName, locName);
					continue;
				}
				tmpLoc->setProfile(profile.profileName);
			}

			loc_it = _locators.emplace(key, tmpLoc).first;
			_joinedCapabilities |= tmpLoc->capabilities();
		}

		profile.feature = feature;
		profile.locator = loc_it->second.get();
		_profiles.emplace_back(profile);
	}

	if ( _profiles.empty() ) {
		SEISCOMP_ERROR("No valid region profiles found in: %s", regionFile);
		return false;
	}

	// sort by rank and area, prioritize larger ranks and smaller areas
	std::sort(_profiles.begin(), _profiles.end(),
	          [](const LocatorProfile& a, const LocatorProfile& b) {
		if ( a.feature->rank() > a.feature->rank() ) {
			return true;
		}

		if ( a.feature->rank() < a.feature->rank() ) {
			return false;
		}

		auto areaA = a.feature->area();
		auto areaB = b.feature->area();

		// The area for the world defined via 90/-90 and 180/-180 computes to 0.
		// Make sure to prioritize any other polygon here.
		return areaA < areaB && areaA != 0 && areaB != 0;
	});

	SEISCOMP_INFO("Router locator initialized with %zu region profile(s) and %zu "
	              "locator-profile implementation(s), initial locator: %s",
	              _profiles.size(), _locators.size(), initialLocatorName);
/*
	if ( !_profiles.empty() ) {
		auto fmtProfiles = [&]() -> std::string {
			ostringstream ss;
			for ( const auto &profile : _profiles ) {
				const auto *f = profile.feature;
				ss << "\n  " << f->name() << " -> " << profile.locatorName;
				if ( !profile.profileName.empty() ) {
					ss << " (" << profile.profileName << ")";
				}
				ss << "\t(rank: " << f->rank() << ", area (deg²): " << f->area();
				if ( profile.minDepth && profile.maxDepth ) {
					ss << ", depth (km): ["
					   << *profile.minDepth << " - " << *profile.maxDepth << "]";
				}
				else if ( profile.minDepth ) {
					ss << ", depth >= " << *profile.minDepth << " km";
				}
				else if ( profile.maxDepth ) {
					ss << ", depth <= " << *profile.maxDepth << " km";
				}
				ss << ")";
			}
			return ss.str();
		};
		SEISCOMP_DEBUG("Configured region profiles: %s", fmtProfiles());
	}
*/

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

	auto *origin = _initialLocator->locate(pickList, initLat, initLon, initDepth,
	                                       initTime);
	return origin ? relocate(origin) : nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Origin *RouterLocator::relocate(const Origin *origin) {
	const auto *locProf = lookup(origin);
	if ( !locProf ) {
		throw LocatorException("Could not find suitable region profile for "
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
	auto fmtEpicenter = [&]() -> std::string {
		std::ostringstream ss;
		ss << Geo::OStreamFormat(epicenter);
		return ss.str();
	};

	OPT(double) depth;
	try {
		depth = origin->depth().value();
		SEISCOMP_DEBUG("Locator region lookup for hypocenter: %s / %f km",
		               fmtEpicenter(), *depth);
	}
	catch ( ValueException& ) {
		SEISCOMP_DEBUG("Locator region lookup for epicenter: %s", fmtEpicenter());
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

		// check if depth constraints are fulfilled
		if ( (profile.minDepth && depth < *profile.minDepth) ||
		     (profile.maxDepth && depth > *profile.maxDepth) ) {
			continue;
		}

		SEISCOMP_DEBUG(R"(Region profile match for %scenter:
  feature            : %s
  locator/profile    : %s/%s
  rank/area (deg²)   : %i/%f
  min/max depth (km) : %s/%s)",
		               profile.minDepth||profile.maxDepth ? "hypo" : "epi",
		               profile.feature->name(),
		               profile.locatorName,
		               profile.profileName.empty() ? "-" : profile.profileName,
		               profile.feature->rank(), profile.feature->area(),
		               profile.minDepth ? toString(*profile.minDepth) : "-",
		               profile.maxDepth ? toString(*profile.maxDepth) : "-");

		return &profile;
	}

	return {};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
