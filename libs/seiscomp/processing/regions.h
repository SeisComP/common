/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#ifndef SEISCOMP_PROCESSING_REGIONS_H
#define SEISCOMP_PROCESSING_REGIONS_H


#include <seiscomp/config/config.h>
#include <seiscomp/geo/featureset.h>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(Regions);
class Regions : public Core::BaseObject {
	public:
		/**
		 * @brief Returns the feature which contains the given point
		 * @param lat The latitude of the reference point
		 * @param lon The longitude of the reference point
		 * @return The feature which contains the reference point or null
		 */
		Geo::GeoFeature *find(double lat, double lon) const;

		/**
		 * @brief Checks whether a given path is contained in
		 *        the region set.
		 * @param lat0 The latitude of the starting point
		 * @param lon0 The longitude of the starting point
		 * @param lat1 The latitude of the end point
		 * @param lon1 The longitude of the end point
		 * @param anyFeature If true then the path is allowed to be contained
		 *                   in multiple features. If false then the path must
		 *                   be contained in a single feature.
		 * @param samplingDistance The distance of the path sampling. The default
		 *                         is 10 km which means that a point each 10 km
		 *                         along the path will be checked for containment.
		 *                         The lower this value the more precise the
		 *                         output but the more expensive in terms of
		 *                         CPU cycles. If a value lower than zero or
		 *                         equal to zero is passed then only the
		 *                         starting point and end point will be checked.
		 * @return The feature of the starting point or null
		 */
		Geo::GeoFeature *find(double lat0, double lon0,
		                      double lat1, double lon1,
		                      bool anyFeature = true,
		                      double samplingDistance = 10) const;

		/**
		 * @brief Checks whether a path is contained completely in a given
		 *        feature.
		 * @param feature The feature to test again.
		 * @param lat0 The latitude of the starting point
		 * @param lon0 The longitude of the starting point
		 * @param lat1 The latitude of the end point
		 * @param lon1 The longitude of the end point
		 * @param samplingDistance The distance of the path sampling. The default
		 *                         is 10 km which means that a point each 10 km
		 *                         along the path will be checked for containment.
		 *                         The lower this value the more precise the
		 *                         output but the more expensive in terms of
		 *                         CPU cycles. If a value lower than zero or
		 *                         equal to zero is passed then only the
		 *                         starting point and end point will be checked.
		 * @return true if contained, false otherwise
		 */
		static bool contains(const Geo::GeoFeature *feature,
		                     double lat0, double lon0,
		                     double lat1, double lon1,
		                     double samplingDistance = 10);

		static const Regions *load(const std::string& filename);

	public:
		Geo::GeoFeatureSet featureSet;
};


}
}


#endif
