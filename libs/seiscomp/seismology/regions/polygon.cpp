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
#include <seiscomp/logging/log.h>
#include <seiscomp/core/system.h>
#include <seiscomp/core/strings.h>

namespace fs = boost::filesystem;

namespace Seiscomp::Geo {



PolyRegions::PolyRegions(const std::string &location) {
	read(location);
}


PolyRegions::~PolyRegions() {
	for ( auto *region : _regions ) {
		delete region;
	}
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

	fs::directory_iterator end_itr;
	std::vector<std::string> files;

	try {
		for ( fs::directory_iterator itr(directory); itr != end_itr; ++itr ) {

			if ( fs::is_directory(*itr) ) {
				continue;
			}

			if ( boost::regex_match(SC_FS_IT_LEAF(itr), boost::regex(".*\\.(?:fep)")) ) {
				files.push_back(SC_FS_IT_STR(itr));
			}
		}
	}
	catch ( const std::exception &ex ) {
		SEISCOMP_ERROR("Reading regions: %s", ex.what());
		return regionCount();
	}

	std::sort(files.begin(), files.end());

	for ( auto &file : files ) {
		if ( !readFepBoundaries(file) ) {
			SEISCOMP_ERROR("Error reading file: %s", file.c_str());
		}
	}

	info();

	// store directory path the data was read from
	_dataDir = directory.string();

	return regionCount();
}


bool PolyRegions::readFepBoundaries(const std::string& filename) {
	SEISCOMP_DEBUG("Reading FEP regions from file: %s", filename.c_str());
	std::ifstream infile(filename.c_str());

	if ( infile.bad() ) {
		return false;
	}

	auto closePolygon = [](GeoFeature *&pr, size_t &vertexSize, size_t lineNum,
	        const std::string &filename, const char *msg) {
		if ( !pr ) {
			return;
		}

		SEISCOMP_WARNING("%s on line %zu of file: %s", msg, lineNum, filename);
		delete pr;
		pr = nullptr;
	};

	boost::regex vertexLine(R"(^([-+]?[0-9]*\.?[0-9]+)\s+([-+]?[0-9]*\.?[0-9]+)(?:\s+([^\d\s].*)$|$))");
	boost::regex sizeLine(R"(^99*\.?[0-9]+\s+99*\.?[0-9]+\s+([0-9]+)$)");
	boost::regex LLine(R"(^L\s+(.*)$)");
	boost::smatch what;

	std::string line;
	GeoFeature *pr = nullptr;
	size_t lineNum = 0;
	size_t vertexSize = 0;
	GeoCoordinate lastCoord;

	while ( std::getline(infile, line) ) {
		++lineNum;

		Core::trim(line);

		if ( line.empty() || line[0] == '#' ) {
			continue;
		}

		// vertex line: longitude latitude
		if ( boost::regex_match(line, what, vertexLine) ) {
			if ( pr && vertexSize ) {
				closePolygon(pr, vertexSize, lineNum, filename,
				             "Incomplete polygon, "
				             "found vertex but expected name definition");
			}

			if ( pr ) {
				pr->addVertex(lastCoord);
			}
			else {
				pr = new GeoFeature();
			}
			lastCoord = GeoCoordinate(atof(what.str(2).c_str()),
			                          atof(what.str(1).c_str())).normalize();

			continue;
		}

		// skip line if no vertex line has been read so far
		if ( !pr ) {
			SEISCOMP_WARNING("No vertex read so far, ignoring line %zu of "
			                 "file: %s", lineNum, filename);
			continue;
		}

		// vertex size (optional): 99.0 99.0 SIZE
		if ( boost::regex_match(line, what, sizeLine) ) {
			if ( vertexSize ) {
				SEISCOMP_WARNING("Ignoring unexpected polygon size definition "
				                 "on line %zu of file: %s", lineNum, filename);
				continue;
			}

			vertexSize = atoi(what.str(1).c_str());
			if ( vertexSize != pr->vertices().size() + 1 ) {
				SEISCOMP_WARNING("Polygon size definition on line %zu does not "
				                 "match vertexes read (%zu != %zu), file: %s",
				                 lineNum, vertexSize, pr->vertices().size() + 1,
				                 filename);
			}
			continue;
		}

		// polygon name: L NAME
		if ( boost::regex_match(line, what, LLine) ) {
			if ( lastCoord != pr->vertices().front() ) {
				pr->addVertex(lastCoord);
			}

			if ( pr->vertices().size() < 3 ) {
				closePolygon(pr, vertexSize, lineNum, filename,
				             "Too few vertices for polygon");
				continue;
			}

			pr->setName(what.str(1));
			pr->setClosedPolygon(true);
			pr->updateBoundingBox();

			if ( pr->area() < 0 ) {
				pr->invertOrder();
			}

			addRegion(pr);
			pr = nullptr;
			vertexSize = 0;
			continue;
		}

		closePolygon(pr, vertexSize, lineNum, filename, "Incomplete polygon");
	}

	closePolygon(pr, vertexSize, lineNum, filename, "Incomplete polygon");

	return true;
}


void PolyRegions::addRegion(GeoFeature *r) {
	_regions.push_back(r);
}


size_t PolyRegions::regionCount() const {
	return _regions.size();
}


GeoFeature *PolyRegions::region(int i) const {
	return _regions.empty() ? nullptr : _regions[i];
}


void PolyRegions::print() {
	for ( auto *region : _regions) {
		std::cerr << region->name() << '\n'
		          << region->vertices().size() << '\n';
	}
	std::cerr.flush();
}


void PolyRegions::info() {
	SEISCOMP_DEBUG("Number of PolyRegions loaded: %zu", _regions.size());

	int sum = 0;
	for ( auto *region : _regions ) {
		sum += region->vertices().size();
	}

	SEISCOMP_DEBUG("Total number of vertices read in: %d", sum);
}


GeoFeature *PolyRegions::findRegion(double lat, double lon) const {
	auto gc = GeoCoordinate(lat, lon).normalize();
	for ( auto *region : _regions ) {
		if ( region->contains(gc) ) {
			return region;
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
