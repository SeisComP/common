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


#ifndef SEISCOMP_GEO_GEOCOORDINATE_H
#define SEISCOMP_GEO_GEOCOORDINATE_H


#include <seiscomp/math/geo.h>
#include <ostream>


namespace Seiscomp {
namespace Geo {


class SC_SYSTEM_CORE_API GeoCoordinate {
	public:
		typedef float ValueType;

	public:
		GeoCoordinate();
		GeoCoordinate(ValueType lat_, ValueType lon_);

	public:
		void set(ValueType lat, ValueType lon);

		ValueType latitude() const;
		ValueType longitude() const;

		bool operator==(const GeoCoordinate &other) const;
		bool operator!=(const GeoCoordinate &other) const;

		GeoCoordinate &normalize();

		static ValueType width(ValueType lon0, ValueType lon1);
		static ValueType normalizeLat(ValueType lat);
		static ValueType normalizeLon(ValueType lon);
		static void normalizeLatLon(ValueType &lat, ValueType &lon);

		static ValueType distanceLon(ValueType lon0, ValueType lon1);

	public:
		ValueType lat;
		ValueType lon;
};


//! Added with API 14.1
bool contains(const GeoCoordinate &v, const GeoCoordinate *polygon, size_t sides);
//! Added with API 14.1
bool contains(const GeoCoordinate &v, const GeoCoordinate *polygon, size_t sides, double &area);

//! Added with API 14.1
double area(const GeoCoordinate *polygon, size_t sides);


// For backwards compatibility, define Vertex as GeoCoordinate
typedef GeoCoordinate Vertex;


#include <seiscomp/geo/coordinate.ipp>


struct formatted_lat {
	formatted_lat(double lat) : v(lat) {}
	double v;
};

struct formatted_lon {
	formatted_lon(double lon) : v(lon) {}
	double v;
};

template <typename T>
struct OStreamFormat {
	OStreamFormat(const T &r) : ref(r) {}
	const T &ref;
};


template <typename T>
OStreamFormat<T> format(const T &ref) { return OStreamFormat<T>(ref); }


std::ostream &operator<<(std::ostream &os, const GeoCoordinate &);
std::ostream &operator<<(std::ostream &os, const formatted_lat &);
std::ostream &operator<<(std::ostream &os, const formatted_lon &);

template <typename T>
std::ostream &operator<<(std::ostream &os, const OStreamFormat<T> &f) {
	os << f.ref;
	return os;
}

template <>
std::ostream &operator<<(std::ostream &os, const OStreamFormat<GeoCoordinate> &);


}
}


#endif
