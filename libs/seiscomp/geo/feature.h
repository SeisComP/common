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

#ifndef SEISCOMP_GEO_FEATURE_H
#define SEISCOMP_GEO_FEATURE_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/geo/coordinate.h>
#include <seiscomp/geo/boundingbox.h>

#include <map>
#include <ostream>

namespace Seiscomp {
namespace Geo {


DEFINE_SMARTPOINTER(Category);
DEFINE_SMARTPOINTER(GeoFeature);


struct SC_SYSTEM_CORE_API Category {
	unsigned int id;
	std::string name;
	std::string localName;
	const Category *parent;
	std::string dataDir;

	Category(unsigned int id, std::string name = "",
	         const Category *parent = nullptr) :
	    id(id), name(std::move(name)), parent(parent) {}
};


class SC_SYSTEM_CORE_API GeoFeature : public Core::BaseObject {
	public:
		using GeoCoordinates = std::vector<GeoCoordinate>;
		using SubFeatures = std::vector<size_t>;
		using Attributes = std::map<std::string, std::string>;

		GeoFeature(const Category  *category = nullptr, unsigned int rank = 1);
		GeoFeature(std::string name, const Category *category,
		           unsigned int rank);
		GeoFeature(std::string name, const Category *category,
		           unsigned int rank, Attributes attributes);
		virtual ~GeoFeature();

	public:
		bool operator==(const GeoFeature &other) const;
		bool operator!=(const GeoFeature &other) const;

		void setName(const std::string &name) { _name = name; }
		const std::string &name() const { return _name; }

		const Category *category() const { return _category; }

		void setRank(unsigned int rank);
		unsigned int rank() const { return _rank; }

		void setAttribute(const std::string &name, const std::string &value);
		const Attributes &attributes() const { return _attributes; }

		/** Adds a vertex to the GeoFeature and changes the BBox if
		  * applicable. If newSubFeature is set to true */
		void addVertex(const GeoCoordinate &vertex, bool newSubFeature = false);
		void addVertex(float lat, float lon, bool newSubFeature = false) {
			addVertex(GeoCoordinate(lat, lon), newSubFeature);
		}

		bool closedPolygon() const { return _closedPolygon; }
		void setClosedPolygon(bool closed) { _closedPolygon = closed; }

		void updateBoundingBox();

		// Inverts the point order from counter-clockwise to clockwise or
		// vice versa.
		void invertOrder();

		/**
		 * @brief Sorts all subfeatures according to their area and containment
		 *        from largest to smallest.
		 */
		void sort();

		/**
		 * @brief Sets an arbitrary pointer for user data. It is not touched
		 *        and/or utilized by the geo library.
		 */
		void setUserData(void*);
		void *userData() const;

		const GeoCoordinates &vertices() const { return _vertices; }
		const GeoBoundingBox &bbox() const { return _bbox; }
		const SubFeatures &subFeatures() const { return _subFeatures; }

		bool contains(const GeoCoordinate &v) const;

		double area() const;

		/**
		 * @deprecated This method is deprecated and will be removed in future
		 * releases. Use Seiscomp::Geo::area instead.
		 */
		static double area(const GeoCoordinate *polygon, size_t sides) __attribute__((deprecated));

	private:
		std::string          _name;
		const Category      *_category{nullptr};
		void                *_userData{nullptr};
		unsigned int         _rank{1};
		GeoFeature::Attributes   _attributes;

		GeoCoordinates       _vertices;
		bool                 _closedPolygon{false};
		GeoBoundingBox       _bbox;

		/** Index of verticies marking the start of a sub feature.
		 *  E.g. if the GeoFeature defines a main area and a group of
		 *  islands this vector would contain the indices of the start
		 *  point of each island */
		SubFeatures _subFeatures;
};


std::ostream& operator<<(std::ostream& os, const GeoFeature &gf);


inline void *GeoFeature::userData() const {
	return _userData;
}


} // of ns Geo
} // of ns Seiscomp


#endif // SEISCOMP_GEO_GEOFEATURE_H__
