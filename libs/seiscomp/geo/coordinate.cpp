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

namespace {

template <typename T>
T sub(T a, T b) {
	return GeoCoordinate::normalizeLon(a-b);
}

}
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
bool contains(const GeoCoordinate &v, const GeoCoordinate *polygon, size_t sides) {
	// should not happen since when reading the BNA files the last point if it
	// equals the first point
	if ( polygon[0] == polygon[sides-1] )
		--sides;

	// no polygon if less than 3 sites
	if ( sides < 3 ) return false;

	size_t i, j;
	bool oddCrossings = false;

	for ( i = 0, j = sides-1; i < sides; j = i++ ) {
		GeoCoordinate::ValueType relLonLeft = sub(polygon[i].lon, v.lon);
		GeoCoordinate::ValueType relLonRight = sub(polygon[j].lon, v.lon);
		GeoCoordinate::ValueType relWidth = relLonLeft-relLonRight;
		if ( fabs(relWidth) > 180 ) {
			if ( relWidth < -180 )
				relWidth += 360;
			else
				relWidth -= 360;

			relLonLeft = relLonRight+relWidth;
		}

		if ( (relLonLeft > 0) == (relLonRight > 0) ) continue;
		if ( v.lat < (polygon[j].lat-polygon[i].lat) * sub(v.lon, polygon[i].lon) / sub(polygon[j].lon, polygon[i].lon) + polygon[i].lat )
			oddCrossings = !oddCrossings;
	}

	return oddCrossings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool contains(const GeoCoordinate &v, const GeoCoordinate *polygon, size_t sides, double &area) {
	if ( polygon[0] == polygon[sides-1] )
		--sides;

	area = 0;

	// no polygon if less than 3 sites
	if ( sides < 3 ) return false;

	size_t i, j;
	bool oddCrossings = false;

	GeoCoordinate::ValueType ref_lon = polygon[0].lon;

	for ( i = 0, j = sides-1; i < sides; j = i++ ) {
		GeoCoordinate::ValueType relLonLeft = sub(polygon[i].lon, v.lon);
		GeoCoordinate::ValueType relLonRight = sub(polygon[j].lon, v.lon);
		GeoCoordinate::ValueType relWidth = relLonLeft-relLonRight;
		if ( fabs(relWidth) > 180 ) {
			if ( relWidth < -180 )
				relWidth += 360;
			else
				relWidth -= 360;

			relLonLeft = relLonRight+relWidth;
		}

		GeoCoordinate::ValueType l0 = sub(polygon[j].lon, ref_lon);
		GeoCoordinate::ValueType l1 = sub(polygon[i].lon, ref_lon);

		area += l1*polygon[j].lat - l0*polygon[i].lat;

		if ( (relLonLeft > 0) == (relLonRight > 0) ) continue;
		if ( v.lat < (polygon[j].lat-polygon[i].lat) * sub(v.lon, polygon[i].lon) / sub(polygon[j].lon, polygon[i].lon) + polygon[i].lat )
			oddCrossings = !oddCrossings;
	}

	area *= 0.5;
	return oddCrossings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double area(const GeoCoordinate *polygon, size_t sides) {
	if ( polygon[0] == polygon[sides-1] )
		--sides;

	// no polygon if less than 3 sites
	if ( sides < 3 ) return 0;

	size_t i, j;
	double A = 0.0;

	GeoCoordinate::ValueType ref_lon = polygon[0].lon;

	for ( i = 0, j = sides - 1; i < sides; j = i++ ) {
		GeoCoordinate::ValueType l0 = sub(polygon[j].lon, ref_lon);
		GeoCoordinate::ValueType l1 = sub(polygon[i].lon, ref_lon);

		A += l0*polygon[i].lat- l1*polygon[j].lat;
	}

	return A*0.5;
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
