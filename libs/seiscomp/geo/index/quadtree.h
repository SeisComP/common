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


#ifndef SEISCOMP_GEO_INDEX_QUADTREE_H
#define SEISCOMP_GEO_INDEX_QUADTREE_H


#include <seiscomp/geo/featureset.h>

#include <boost/function.hpp>

#include <vector>
#include <ostream>


namespace Seiscomp {
namespace Geo {


class SC_SYSTEM_CORE_API QuadTree {
	public:
		typedef boost::function<bool (const GeoFeature *)> VisitFunc;

	public:
		QuadTree();

	public:
		/**
		 * @brief Adds a feature to the tree.
		 * @param feature The feature const pointer. The ownership is not transferred.
		 */
		void addItem(const GeoFeature *feature);

		//! Adds all features of the feature set
		void add(const GeoFeatureSet &featureSet);

		const GeoBoundingBox &bbox() const;

		void query(const GeoCoordinate &gc, const VisitFunc &) const;
		void query(const GeoBoundingBox &bb, const VisitFunc &, bool clipOnlyNodes = false) const;

		/**
		 * Visits the features of a quadtree by descending from the root to
		 * the leaves or from the leaves to the root. The visitor is an
		 * arbitrary class that should implement the following methods:
		 *
		 * bool accept(const BoundingBox &box) const;
		 * bool visit(const GeoFeature *feature) const;
		 *
		 * If either of those methods returns false, then traversing the
		 * tree is aborted.
		 *
		 * @param visitor The visitor instance
		 * @param topDown Either traverse the quadtree top-down (from root
		 *        to leaves) or bottom-up (from leaves to root).
		 */
		template <typename VISITOR>
		void accept(const VISITOR &visitor, bool topDown = true);

		const GeoFeature *findFirst(const GeoCoordinate &gc);
		const GeoFeature *findLast(const GeoCoordinate &gc);

		//! Dumps the tree to an output stream
		std::ostream &dump(std::ostream &os) const;

	private:
		typedef std::vector<const GeoFeature*> Features;

		enum NodeIndex {
			InvalidIndex = -1,
			UpperRight = 0,
			UpperLeft = 1,
			LowerLeft = 2,
			LowerRight = 3
		};

		DEFINE_SMARTPOINTER(Node);
		struct Node : public Core::BaseObject {
			Node();

			NodeIndex findAndCreateNode(const GeoFeature *f);
			void addItem(const GeoFeature *f, int depth);
			void dump(std::ostream &os, int indent) const;

			void visit(const GeoCoordinate &gc, const VisitFunc &) const;
			void visit(const GeoBoundingBox &bb, const VisitFunc &, bool clipOnlyNodes = false) const;

			template <typename VISITOR>
			bool acceptTopDown(const VISITOR &visitor);

			template <typename VISITOR>
			bool acceptButtomUp(const VISITOR &visitor);

			const GeoFeature *findFirst(const GeoCoordinate &gc);
			const GeoFeature *findLast(const GeoCoordinate &gc);

			GeoBoundingBox  bbox;
			Features        features;
			bool            isLeaf;
			NodePtr         children[4];
		};

	private:
		Node _root;
};


#include <seiscomp/geo/index/quadtree.ipp>


std::ostream &operator<<(std::ostream &os, const Seiscomp::Geo::QuadTree &tree);


}
}


#endif
