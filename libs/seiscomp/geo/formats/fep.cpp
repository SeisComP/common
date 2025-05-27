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


#define SEISCOMP_COMPONENT Geo

#include <fstream>

#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <seiscomp/core/strings.h>
#include <seiscomp/geo/formats/fep.h>
#include <seiscomp/logging/log.h>


using namespace std;


namespace Seiscomp::Geo {

size_t readFEP(GeoFeatureSet &featureSet, const string &path,
               const Category *category) {
	SEISCOMP_DEBUG("Reading polygons from file: %s", path.c_str());

	ifstream infile(path.c_str());

	if ( infile.bad() ) {
		return 0;
	}

	auto closePolygon = [](GeoFeature *&f, size_t &vertexSize, size_t lineNum,
	                       const string &path, const char *msg) {
		if ( !f ) {
			return;
		}

		SEISCOMP_WARNING("%s on line %zu of file: %s", msg, lineNum, path);
		delete f;
		f = nullptr;
	};

	boost::regex vertexLine(R"(^([-+]?[0-9]*\.?[0-9]+)\s+([-+]?[0-9]*\.?[0-9]+)(?:\s+([^\d\s].*)$|$))");
	boost::regex sizeLine(R"(^99*\.?[0-9]+\s+99*\.?[0-9]+\s+([0-9]+)$)");
	boost::regex LLine(R"(^L\s+(.*)$)");
	boost::smatch what;

	string line;
	GeoFeature *f = nullptr;
	size_t lineNum = 0;
	size_t vertexSize = 0;
	GeoCoordinate lastCoord;
	size_t currentNumberOfFeatures = featureSet.features().size();

	while ( getline(infile, line) ) {
		++lineNum;

		Core::trim(line);

		if ( line.empty() || line[0] == '#' ) {
			continue;
		}

		// vertex line: longitude latitude
		if ( boost::regex_match(line, what, vertexLine) ) {
			if ( f && vertexSize ) {
				closePolygon(f, vertexSize, lineNum, path,
				             "Incomplete polygon, "
				             "found vertex but expected name definition");
			}

			if ( f ) {
				f->addVertex(lastCoord);
			}
			else {
				f = new GeoFeature(category);
			}
			lastCoord = GeoCoordinate(atof(what.str(2).c_str()),
			                          atof(what.str(1).c_str())).normalize();

			continue;
		}

		// skip line if no vertex line has been read so far
		if ( !f ) {
			SEISCOMP_WARNING("No vertex read so far, ignoring line %zu of "
			                 "file: %s", lineNum, path);
			continue;
		}

		// vertex size (optional): 99.0 99.0 SIZE
		if ( boost::regex_match(line, what, sizeLine) ) {
			if ( vertexSize ) {
				SEISCOMP_WARNING("Ignoring unexpected polygon size definition "
				                 "on line %zu of file: %s", lineNum, path);
				continue;
			}

			vertexSize = atoi(what.str(1).c_str());
			if ( vertexSize != f->vertices().size() + 1 ) {
				SEISCOMP_WARNING("Polygon size definition on line %zu does not "
				                 "match vertexes read (%zu != %zu), file: %s",
				                 lineNum, vertexSize, f->vertices().size() + 1,
				                 path);
			}
			continue;
		}

		// polygon name: L NAME
		if ( boost::regex_match(line, what, LLine) ) {
			if ( lastCoord != f->vertices().front() ) {
				f->addVertex(lastCoord);
			}

			if ( f->vertices().size() < 3 ) {
				closePolygon(f, vertexSize, lineNum, path,
				             "Too few vertices for polygon");
				continue;
			}

			f->setName(what.str(1));
			f->setClosedPolygon(true);
			f->updateBoundingBox();

			if ( f->area() < 0 ) {
				f->invertOrder();
			}

			featureSet.addFeature(f);

			f = nullptr;
			vertexSize = 0;
			continue;
		}

		closePolygon(f, vertexSize, lineNum, path, "Incomplete polygon");
	}

	closePolygon(f, vertexSize, lineNum, path, "Incomplete polygon");

	return featureSet.features().size() - currentNumberOfFeatures;
}


}
