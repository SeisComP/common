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
	}

	string regionFile;
	try {
		regionFile = config.getString("RouterLocator.regions");
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

		double depth;
		for ( const auto &att : feature->attributes() ) {
			if ( att.first == "locator") {
				profile.locator = att.second;
			}
			else if ( att.first == "profile" ) {
				profile.profile = att.second;
			}
			else if ( att.first == "minDepth" ) {
				if ( fromString(depth, att.second) ) {
					profile.minDepth = depth;
				}
				else {
					SEISCOMP_WARNING("Invalid minDepth value in feature: %s",
					                 feature->name().c_str());
				}
			}
			else if ( att.first == "maxDepth" ) {
				if ( fromString(depth, att.second) ) {
					profile.maxDepth = depth;
				}
				else {
					SEISCOMP_WARNING("Invalid maxDepth value in feature: %s",
					                 feature->name().c_str());
				}
			}
		}

		if ( profile.locator.empty() ) {
			SEISCOMP_WARNING("Missing locator name in feature: %s",
			                 feature->name().c_str());
			continue;
		}

		profile.feature = feature;
		_profiles.emplace_back(profile);
	}

	// sort by rank and area, prioritize larger ranks and smaller areas
	std::sort(_profiles.begin(), _profiles.end(),
	          [](const LocatorProfile& a, const LocatorProfile& b) {
		return a.feature->rank() > a.feature->rank() ||
		       ( a.feature->rank() == a.feature->rank() &&
		         a.feature->area() < b.feature->area() );
	});

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
	return InitialLocation | DistanceCutOff | FixedDepth | IgnoreInitialLocation;
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
	auto *locProf = lookup(origin);
	if ( !locProf ) {
		throw LocatorException("Could not find suitable locator profile for "
		                       "initial location");
	}

	auto *locator = LocatorInterface::Create(locProf->locator.c_str());
	if ( !locator ) {
		throw LocatorException("Could not load locator: " + locProf->locator);
	}

	if ( !locProf->profile.empty() ) {
		const auto &profiles = locator->profiles();
		if ( find(profiles.begin(), profiles.end(),
		          locProf->profile) == profiles.end() ) {
			throw LocatorException("profile '" + locProf->profile +
			                       "' not supported by locator '" +
			                       locProf->locator + "'");
		}

		locator->setProfile(locProf->profile);
	}

	return locator->relocate(origin);
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
	}
	catch ( ValueException& ) {}

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

		return &profile;
	}

	return {};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
