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


#include <seiscomp/geo/coordinate.h>


namespace Seiscomp {
namespace Geo {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoCoordinate::ValueType GeoCoordinate::normalizeLat(ValueType lat) {
	if ( lat > 90 ) {
		int cycles = (int)((lat + 180) / 360);
		ValueType temp;

		if( cycles == 0 )
			temp = 180 - lat;
		else
			temp = lat - cycles*360;

		if ( temp > 90 )
			return 180 - temp;

		if ( temp < -90 )
			return -180 - temp;

		return temp;
	}
	else if ( lat < -90 ) {
		int cycles = (int)((lat - 180) / 360);
		ValueType temp;

		if ( cycles == 0 )
			temp = -180 - lat;
		else
			temp = lat - cycles*360;

		if ( temp > 90 )
			return +180 - temp;

		if ( temp < -90 )
			return -180 - temp;

		return temp;
	}

	return lat;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
GeoCoordinate::ValueType GeoCoordinate::normalizeLon(ValueType lon) {
	if ( lon > 180 ) {
		int cycles = (int)((lon + 180) / 360);
		return lon - cycles*360;
	}
	else if ( lon < -180 ) {
		int cycles = (int)((lon - 180) / 360);
		return lon - cycles*360;
	}

	return lon;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void GeoCoordinate::normalizeLatLon(ValueType &lat, ValueType &lon) {
	lat = normalizeLat(lat);
	lon = normalizeLon(lon);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const Seiscomp::Geo::GeoCoordinate &coords) {
	os << coords.lat << " / " << coords.lon;
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const Seiscomp::Geo::formatted_lat &fl) {
	if ( fl.v < 0 )
		os << std::abs(fl.v) << "° S";
	else
		os << fl.v << "° N";
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const Seiscomp::Geo::formatted_lon &fl) {
	if ( fl.v < 0 )
		os << std::abs(fl.v) << "° W";
	else
		os << fl.v << "° E";
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
std::ostream &operator<<(std::ostream &os, const Seiscomp::Geo::OStreamFormat<Seiscomp::Geo::GeoCoordinate> &f) {
	os << Seiscomp::Geo::formatted_lat(f.ref.lat)
	   << " / "
	   << Seiscomp::Geo::formatted_lon(f.ref.lon);
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
