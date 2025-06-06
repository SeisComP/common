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

	auto closePolygon = [](GeoFeature *&f, GeoFeatureSet &gfs, size_t lineNum,
	                       const string &path) {
		if ( !f ) {
			return;
		}

		if ( f->name().empty() ) {
			SEISCOMP_WARNING("No name defined for polygon on line %zu of file "
			                 "file: %s", f->name(), lineNum, path);
			delete f;
			f = nullptr;
			return;
		}

		if ( f->vertices().size() < 3 ) {
			SEISCOMP_WARNING("Too few vertices for polygon '%s' on line %zu of "
			                 "file: %s", f->name(), lineNum, path);
			delete f;
			f = nullptr;
			return;
		}

		f->setClosedPolygon(true);
		f->updateBoundingBox();

		if ( f->area() < 0 ) {
			f->invertOrder();
		}

		gfs.addFeature(f);
		f = nullptr;
	};

	boost::regex vertexLine(R"(^([-+]?[0-9]*\.?[0-9]+)\s+([-+]?[0-9]*\.?[0-9]+)(?:\s+([^\d\s].*)$|$))");
	boost::regex sizeLine(R"(^99*\.?[0-9]+\s+99*\.?[0-9]+\s+([0-9]+)$)");
	boost::regex LLine(R"(^L\s+(.*)$)");
	boost::smatch what;

	string line;
	GeoFeature *f = nullptr;
	size_t lineNum = 0;
	bool endOfVertices = false;
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
			if ( endOfVertices ) {
				closePolygon(f, featureSet, lineNum, path);
				endOfVertices = false;
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

		// stop reading vertices
		if ( f->vertices().empty() || lastCoord != f->vertices().front() ) {
			f->addVertex(lastCoord);
		}
		endOfVertices = true;

		// vertex size (optional): 99.0 99.0 SIZE
		if ( boost::regex_match(line, what, sizeLine) ) {
			size_t vertexSize = atoi(what.str(1).c_str());
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
			f->setName(what.str(1));
		}
	}

	closePolygon(f, featureSet, lineNum, path);

	return featureSet.features().size() - currentNumberOfFeatures;
}


}
