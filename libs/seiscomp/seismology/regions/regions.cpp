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



#include <seiscomp/seismology/regions.h>
#include <seiscomp/seismology/regions/ferdata.h>
#include <seiscomp/seismology/regions/polygon.h>

#include <seiscomp/system/environment.h>

using namespace Seiscomp;


namespace {


Geo::PolyRegions regions;


}


Regions::Regions() {
}


void Regions::load() {
	if ( !regions.read(Environment::Instance()->configDir() + "/fep") )
		regions.read(Environment::Instance()->shareDir() + "/fep");
}


Geo::PolyRegions &Regions::polyRegions() {
	return regions;
}


std::string Regions::getRegionName(double lat, double lon) {
	while ( lon < -180 ) lon += 360;
	while ( lon > 180 ) lon -= 360;

	std::string name = getRegionalName(lat, lon);
	return name.empty()?getFeGeoRegionName(lat, lon):name;
}


std::string Regions::getFeGeoRegionName(double lat, double lon) {
	std::string name;

	int _lat = int(lat);
	int _lon = int(lon);

	if (lat >= 0.0) _lat += 1;
	if (lon >= 0.0) _lon += 1;

	if (lat >= -90 && lat <= +90 && lon >= -180 && lon <= +180)
		name = feGeoRegionsNames[feGeoRegionsArray[_lat + 90][_lon + 180] - 1];
	else
		name = "unknown Region";

	return name;
}


std::string Regions::getRegionalName(double lat, double lon) {
	return regions.findRegionName(lat, lon);
}
