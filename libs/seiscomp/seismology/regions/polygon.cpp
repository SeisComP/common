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

#include <seiscomp/seismology/regions/polygon.h>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#define SEISCOMP_COMPONENT PolyRegion
#include <seiscomp/core/system.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/logging/log.h>

namespace fs = boost::filesystem;

namespace Seiscomp::Geo {



PolyRegions::PolyRegions(const std::string &location) {
	read(location);
}


size_t PolyRegions::read(const std::string& location) {
	SEISCOMP_DEBUG("Reading FEP regions from directory: %s", location.c_str());
	fs::path directory;
	try {
		directory = SC_FS_PATH(location);
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Invalid path '%s'", location.c_str());
		return 0;
	}

	if ( !fs::exists(directory) ) {
		return regionCount();
	}

	_regions.readDir(directory.string());

	info();

	// store directory path the data was read from
	_dataDir = directory.string();

	return regionCount();
}


void PolyRegions::addRegion(GeoFeature *r) {
	_regions.addFeature(r);
}


size_t PolyRegions::regionCount() const {
	return _regions.features().size();
}


GeoFeature *PolyRegions::region(int i) const {
	return _regions.features().empty() ? nullptr : _regions.features()[i];
}


void PolyRegions::print() {
	for ( auto *f : _regions.features()) {
		std::cerr << f->name() << '\n'
		          << f->vertices().size() << '\n';
	}
	std::cerr.flush();
}


void PolyRegions::info() {
	SEISCOMP_DEBUG("Number of PolyRegions loaded: %zu",
	               _regions.features().size());

	int sum = 0;
	for ( auto *f : _regions.features() ) {
		sum += f->vertices().size();
	}

	SEISCOMP_DEBUG("Total number of vertices read in: %d", sum);
}


GeoFeature *PolyRegions::findRegion(double lat, double lon) const {
	auto gc = GeoCoordinate(lat, lon).normalize();
	for ( auto *f : _regions.features() ) {
		if ( f->contains(gc) ) {
			return f;
		}
	}

	return {};
}


std::string PolyRegions::findRegionName(double lat, double lon) const {
	auto *region = findRegion(lat, lon);
	if ( region ) {
		return region->name();
	}

	return {};
}


}
