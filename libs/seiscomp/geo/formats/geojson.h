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


#ifndef SEISCOMP_GEO_FORMATS_GEOJSON_H
#define SEISCOMP_GEO_FORMATS_GEOJSON_H


#include <seiscomp/geo/feature.h>
#include <seiscomp/geo/featureset.h>

#include <ostream>


namespace Seiscomp::Geo {


/**
 * @brief Reads a GeoJSON file and adds found features to the feature set.
 *        In case of an error an exception is thrown.
 * @param featureSet The target feature that will hold the read features
 * @param path The path to the GeoJSON file
 * @param category An optional category attached to all read features
 * @return The number of features read
 */
size_t readGeoJSON(GeoFeatureSet &featureSet, const std::string &path,
                   const Category *category = nullptr);


/**
 * @brief Write a GeoFeature to a ostream.
 * @param os The ostream to write to
 * @param feature The feature to serialize
 * @param indent Indentation depth for pretty printing. Disabled if less than 0.
 * @return true if the feature was written
 */
bool writeGeoJSON(std::ostream &os, const GeoFeature &feature, int indent = -1);

/**
 * @brief Write a GeoFeature to a GeoJSON file.
 * @param path The path to the GeoJSON file
 * @param feature The feature to serialize
 * @param indent Indentation depth for pretty printing. Disabled if less than 0.
 * @return true if the feature was written
 */
bool writeGeoJSON(const std::string &path, const GeoFeature &feature,
                  int indent = -1);

/**
 * @brief Write a GeoFeature vector to a ostream.
 * @param os The ostream to write to
 * @param gfs The geo feature vector to serialize
 * @param indent Indentation depth for pretty printing. Disabled if less than 0.
 * @return Number of feature written
 */
size_t writeGeoJSON(std::ostream &os, const GeoFeatureSet::Features &gfs,
                    int indent = -1);

/**
 * @brief Write a GeoFeature vector to a GeoJSON file.
 * @param path The path to the GeoJSON file
 * @param gfs The geo feature vector to serialize
 * @param indent Indentation depth for pretty printing. Disabled if less than 0.
 * contained in gfs are appended to the GeoJSON file. When appending the
 * @return Number of feature written
 */
size_t writeGeoJSON(const std::string &path, const GeoFeatureSet::Features &gfs,
                    int indent = -1);

}


#endif
