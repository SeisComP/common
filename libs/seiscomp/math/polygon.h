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


#ifndef SEISCOMP_MATH_GEO_POLYGON_H
#define SEISCOMP_MATH_GEO_POLYGON_H

#include <string>
#include <vector>
#include <utility>
#include <seiscomp/math/coord.h>


namespace Seiscomp
{

namespace Math
{

namespace Geo
{

template <typename T>
class Polygon : public std::vector< Coord<T> > {
	public:
		Polygon();
		~Polygon();

		/**
		 * Returns whether a location lies inside the polygon
		 * or not.
		 * @param p The location
		 * @return True, if the location lies inside, else false
		 */
		bool operator&(const Coord<T>& c);

		void addVertex(double lat, double lon);
		void addCoord(const Coord<T>& c);

		bool pointInPolygon(const T& lat, const T& lon) const;
		bool pointInPolygon(const Coord<T>& c) const;

		size_t vertexCount() const;

		const Coord<T>& vertex(int i) const;
		Coord<T>& vertex(int i);

		void print() const;

};


typedef Polygon<float> PolygonF;
typedef Polygon<double> PolygonD;


} // of ns  Geo
} // of ns  Math
} // of ns Seiscomp


template <typename T>
std::ostream& operator<<(std::ostream& os, const Seiscomp::Math::Geo::Polygon<T>& poly);


#endif
