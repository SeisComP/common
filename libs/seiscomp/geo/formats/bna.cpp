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

#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/geo/formats/bna.h>

#include <fstream>
#include <sstream>


using namespace std;


namespace Seiscomp {
namespace Geo {

namespace {


bool readBNAHeader(string &segment, unsigned int &rank,
                   GeoFeature::Attributes &attributes,
                   unsigned int &points, bool &isClosed,
                   string &error, const string &line) {
	rank = 1;
	attributes.clear();
	points = 0;
	vector<string> fields;

	// read 3-5 header fields separated by a comma where the first 2-4 fields
	// must start and end on a quote (") and the last field represent a number
	if ( Core::splitExt(fields, line.c_str(), ",", false, true) < 3 ) {
		error = "BNA requires at least 2 header fields";
		return false;
	}
	else if ( fields.size() > 5 ) {
		error = "BNA allows at most 5 header fields";
		return false;
	}

	// read segment name from first field
	vector<string>::iterator it = fields.begin();
	segment = Core::trim(*it);

	// read rank and attributes from next fields
	for ( ; it != fields.end()-1; ++it ) {
		// rank is a special identifier
		if ( it->length() >= 6 && strncmp(it->c_str(), "rank ", 5) == 0 ) {
			int tmp(0);
			tmp = atoi(it->c_str()+5);
			if ( tmp > 1 ) {
				rank = static_cast<unsigned int>(tmp);
				continue;
			}
		}

		// read list of key value parameter into parameter map, e.g.
		// "foo1: bar1, foo2: bar2"
		char *source = const_cast<char *>(it->c_str());
		size_t sourceLen = it->size();
		const char *key, *value;
		size_t keyLen, valueLen;
		char delimFound = 0;
		while ( sourceLen > 0 ) {
			key = Core::tokenizeUnescape(keyLen, sourceLen, source, delimFound, ":");
			if ( key == nullptr || !sourceLen || !delimFound )
				break;
			value = Core::tokenizeUnescape(valueLen, sourceLen, source, delimFound, ",");
			if ( value != nullptr )
				attributes[string(key, keyLen)] = string(value, valueLen);
			else
				attributes[string(key, keyLen)] = "";
		}
	}

	// points
	int p;
	if ( !Core::fromString(p, *it) ) {
		error = "invalid number format in length field";
		return false;
	}
	if ( p >= 0 ) {
		points = static_cast<unsigned int>(p);
		isClosed = true;
	}
	else {
		points = static_cast<unsigned int>(-p);
		isClosed = false;
	}
	return true;
}


}


size_t readBNA(GeoFeatureSet &featureSet, const std::string &filename,
               const Category *category) {
	SEISCOMP_DEBUG("Reading segments from file: %s", filename.c_str());

	ifstream infile(filename.c_str());

	if ( infile.fail() ) {
		SEISCOMP_WARNING("Could not open segment file for reading: %s",
		                 filename.c_str());
		return false;
	}

	vector<GeoFeature*> features;
	GeoFeature *feature;
	unsigned int lineNum = 0, rank, points;
	string line, segment, error;
	const char *nptr;
	char *endptr;
	bool isClosed;
	GeoCoordinate v;
	bool startSubFeature;
	GeoFeature::Attributes attributes;

	bool fileValid = true;

	while ( infile.good() && fileValid ) {
		++lineNum;

		// read BNA header
		getline(infile, line);
		if ( Core::trim(line).empty() ) {
			continue;
		}

		if ( !readBNAHeader(segment, rank, attributes, points, isClosed, error, line) ) {
			SEISCOMP_ERROR("error reading BNA header in file %s at line %i: %s",
			               filename.c_str(), lineNum, error.c_str());
			fileValid = false;
			break;
		}
		startSubFeature = false;

		feature = new GeoFeature(segment, category, rank, attributes);
		features.push_back(feature);
		if ( isClosed )
			feature->setClosedPolygon(true);

		// read vertices, expected format:
		//   "lon1,lat1 lon2,lat2 ... lon_i,lat_i\n"
		//   "lon_i+1,lat_i+1 lon_i+2,lat_i+2 ... \n
		nptr = nullptr;
		unsigned int pi = 0;
		while ( true ) {
			if ( nptr == nullptr ) {
				// stop if all points have been read
				if ( pi == points ) break;

				// read next line
				if ( infile.good() ) {
					++lineNum;
					getline(infile, line);
					nptr = line.c_str();
				}
				else {
					SEISCOMP_ERROR("to few vertices (%i/%i) for feature "
					               "starting at line %i",
					               pi, points, lineNum - pi);
					fileValid = false;
					break;
				}
			}

			// advance nptr to next none white space
			while ( isspace(*nptr) ) ++nptr;

			// read next line if end of line is reached
			if ( *nptr == '\0' ) {
				nptr = nullptr;
				continue;
			}

			// file invalid if extra characters are found after last vertex
			if ( pi == points ) {
				SEISCOMP_ERROR("extra characters after last vertex (%i) of "
				              "feature starting at line %i",
				               pi, lineNum - pi);
				fileValid = false;
				break;
			}

			// read longitude
			endptr = nullptr;
			errno = 0;
			v.lon = strtof(nptr, &endptr);
			if ( errno != 0 || endptr == nullptr || endptr == nptr ||
			     v.lon < -180 || v.lon > 180) {
				SEISCOMP_ERROR("invalid longitude in file %s at line %i",
				               filename.c_str(), lineNum);
				fileValid = false;
				break;
			}

			// search for comma
			nptr = strchr(endptr, ',');
			if ( nptr == nullptr ) {
				SEISCOMP_ERROR("invalid coordinate separator in file %s at line %i",
				               filename.c_str(), lineNum);
				fileValid = false;
				break;
			}

			// read latitude
			endptr = nullptr; nptr += 1;
			v.lat = strtof(nptr, &endptr);
			if ( errno != 0 || endptr == nullptr || endptr == nptr ||
			     v.lat < -90 || v.lat > 90) {
				SEISCOMP_ERROR("invalid latitude in file %s at line %i",
				               filename.c_str(), lineNum);
				fileValid = false;
				break;
			}
			nptr = endptr;

			while ( isspace(*nptr) ) ++nptr;

			// Skip comments
			if ( strncmp(nptr, "--", 2) == 0 )
				nptr = nullptr;

			// increase number of succesfully read points
			pi += 1;

			if ( !feature->vertices().empty() ) {
				// check if the current vertex marks the end of a (sub-) or
				// feature and if so don't add it to the vertex vector but mark
				// the next vertex as the starting point of a new sub feature
				if ( v == feature->vertices().front() ) {
					startSubFeature = true;
					continue;
				}
				// Don't add the vertex if it is equal to the start point of
				// the current subfeature
				else if ( !startSubFeature &&
				          !feature->subFeatures().empty() &&
				          v == feature->vertices()[feature->subFeatures().back()] ) {
					continue;
				}
			}

			feature->addVertex(v, startSubFeature);
			startSubFeature = false;
		}

		if ( fileValid ) {
			feature->updateBoundingBox();
			if ( feature->area() < 0 )
				feature->invertOrder();
		}
	}

	if ( fileValid ) {
		for ( GeoFeature *f : features )
			featureSet.addFeature(f);
		return features.size();
	}
	else {
		for ( size_t i = 0; i < features.size(); ++i ) {
			delete features[i];
		}
		return 0;
	}
}


}
}
