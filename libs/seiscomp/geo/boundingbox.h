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


#ifndef SEISCOMP_GEO_BOUNDINGBOX_H
#define SEISCOMP_GEO_BOUNDINGBOX_H


#include <seiscomp/geo/coordinate.h>
#include <ostream>


namespace Seiscomp {
namespace Geo {


class SC_SYSTEM_CORE_API GeoBoundingBox {
	public:
		typedef GeoCoordinate::ValueType ValueType;

		enum Relation {
			Disjunct,
			Contains,
			Intersects
		};

	public:
		GeoBoundingBox();
		GeoBoundingBox(ValueType south, ValueType west,
		               ValueType north, ValueType east);

	public:
		bool operator==(const GeoBoundingBox &other) const;
		GeoBoundingBox &operator+=(const GeoBoundingBox &other);
		GeoBoundingBox operator+(const GeoBoundingBox &other) const;

		bool operator&(const GeoBoundingBox &other) const;

	public:
		GeoBoundingBox &normalize();

		bool isEmpty() const;
		bool isNull() const;

		/**
		 * @brief Sets this box empty. After that call @isEmpty() will
		 *        return true.
		 */
		void reset();

		bool coversFullLongitude() const;

		ValueType width() const;
		ValueType height() const;

		bool crossesDateLine() const;
		static bool crossesDateLine(ValueType east, ValueType west);

		GeoCoordinate center() const;

		bool contains(const GeoCoordinate &v) const;

		/**
		 * @brief Checks whether other is fully contained in this. If false
		 *        is returned then both boxes can still intersect.
		 * @param other The boundingbox that is being checked to be contained
		 *              in this.
		 * @return true if other is contained in this, false otherwise
		 */
		bool contains(const GeoBoundingBox &other) const;

		/**
		 * @brief Categorizes the relation of this and other. This basically
		 *        returns whether the boxes are disjunct, contain each other
		 *        or just intersect.
		 * @param other The other box to test
		 * @return A Relation enumeration value
		 */
		Relation relation(const GeoBoundingBox &other) const;

		/**
		 * @brief Extends this boundingbox to contain also the other.
		 * @param other The boundingbox to be merged with this
		 */
		void merge(const GeoBoundingBox &other);

		/**
		 * @brief intersects
		 * @param other The boundingbox to check against
		 * @return true if this and other intersect, false otherwise
		 */
		bool intersects(const GeoBoundingBox &other) const;

		void fromPolygon(size_t n, const GeoCoordinate *coords,
		                 bool isClosed = true);


	public:
		union {
			ValueType north;
			ValueType latMax;
		};

		union {
			ValueType south;
			ValueType latMin;
		};

		union {
			ValueType east;
			ValueType lonMax;
		};

		union {
			ValueType west;
			ValueType lonMin;
		};

		static GeoBoundingBox Empty;
};


#include <seiscomp/geo/boundingbox.ipp>


std::ostream &operator<<(std::ostream &os, const GeoBoundingBox &box);

template <>
std::ostream &operator<<(std::ostream &os, const OStreamFormat<GeoBoundingBox> &);


}
}


#endif
