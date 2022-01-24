/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#define SEISCOMP_COMPONENT Processing

#include <seiscomp/logging/log.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/processing/regions.h>

#include <map>
#include <mutex>


namespace Seiscomp {
namespace Processing {

namespace {

std::mutex registryMutex;
std::map<std::string, RegionsPtr> registry;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Geo::GeoFeature *Regions::find(double lat, double lon) const {
	for ( Geo::GeoFeature *feature : featureSet.features() ) {
		if ( feature->contains(Geo::GeoCoordinate(lat, lon)) )
			return feature;
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Geo::GeoFeature *Regions::find(double lat0, double lon0,
                               double lat1, double lon1,
                               bool anyFeature,
                               double samplingDistance) const {
	double dist, az, baz;

	Geo::GeoFeature *startFeature = find(lat0, lon0);
	if ( !startFeature ) {
		// Starting point outside any region -> fail
		return nullptr;
	}

	if ( samplingDistance <= 0.0 ) {
		Geo::GeoFeature *f = find(lat1, lon1);
		if ( !f ) {
			return nullptr;
		}

		if ( !anyFeature and (f != startFeature) ) {
			return nullptr;
		}

		return startFeature;
	}

	Math::Geo::delazi_wgs84(lat0, lon0, lat1, lon1, &dist, &az, &baz);

	// Convert to km
	dist = Math::Geo::deg2km(dist);

	int steps = dist / samplingDistance;

	for ( int i = 1; i <= steps; ++i ) {
		Math::Geo::delandaz2coord(
			Math::Geo::km2deg(dist*i/steps), az,
			lat0, lon0, &lat1, &lon1
		);

		Geo::GeoFeature *f = find(lat1, lon1);
		if ( !f ) {
			// No feature of the point along the path found -> fail
			return nullptr;
		}

		if ( !anyFeature and (f != startFeature) ) {
			// Point must lie inside the same feature as the starting point
			// otherwise fail
			return nullptr;
		}
	}

	return startFeature;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Regions::contains(const Geo::GeoFeature *feature,
                       double lat0, double lon0,
                       double lat1, double lon1,
                       double samplingDistance) {
	double dist, az, baz;

	if ( !feature ) {
		return false;
	}

	if ( !feature->contains(Geo::GeoCoordinate(lat0, lon0)) ) {
		return false;
	}

	if ( samplingDistance <= 0.0 ) {
		return feature->contains(Geo::GeoCoordinate(lat1, lon1));
	}

	Math::Geo::delazi_wgs84(lat0, lon0, lat1, lon1, &dist, &az, &baz);

	// Convert to km
	dist = Math::Geo::deg2km(dist);

	int steps = dist / samplingDistance;

	for ( int i = 1; i <= steps; ++i ) {
		Math::Geo::delandaz2coord(
			Math::Geo::km2deg(dist*i/steps), az,
			lat0, lon0, &lat1, &lon1
		);

		if ( !feature->contains(Geo::GeoCoordinate(lat1, lon1)) ) {
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Regions *Regions::load(const std::string &filename) {
	std::lock_guard<std::mutex> l(registryMutex);

	auto it = registry.find(filename);
	if ( it != registry.end() )
		return it->second.get();

	RegionsPtr regions = new Regions;
	if ( !regions->featureSet.readFile(filename, nullptr) ) {
		return nullptr;
	}

	registry[filename] = regions;
	return regions.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
