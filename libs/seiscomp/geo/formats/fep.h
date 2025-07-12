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


#ifndef SEISCOMP_GEO_FORMATS_FEP_H
#define SEISCOMP_GEO_FORMATS_FEP_H


#include <seiscomp/geo/featureset.h>


namespace Seiscomp::Geo {


/**
 * @brief Reads a FEP plus file and adds found features to the feature set.
 *        In case of an error an exception is thrown. Only closed polygons
 *        are supported.
 *
 * Example:
 *   13.0 52.0
 *   13.0 53.0
 *   14.0 53.0
 *   14.0 52.0
 *   99.0 99.0 4
 *   L Germany
 *
 * Format definition:
 *   longtitude1 latitude1
 *   ...
 *   longtitudeN latitudeN
 *   99.0 99.0 VERTEX_COUNT
 *   L POLYGON_NAME
 *
 *   A polygon starts with a number vertex lines. A vetex contains 2 floats,
 *   longitude and latitude, and may be followed by a comment which must not
 *   start on a digit.
 *
 *   A polygon is required to declare at least 3 vertices. If the last vertex
 *   matches the first one a minimum of 4 vertices are required.
 *
 *   After the vertex definition an option vertex count line may follow
 *   expressed with a longitude and latitude value of 99 followed by the
 *   vertex count. If the count does not match the vertices read a warning
 *   is reported.
 *
 *   A polygon is finalized by an mandatory L line defining the polygon name.
 *
 *   All lines read are stipped first. Empty lines and lines starting on '#'
 *   are ignored.
 *
 * @param featureSet The target feature that will hold the read features
 * @param filename The path to the GeoJSON file
 * @param category An optional category attached to all read features
 * @return The number of features read
 */
size_t readFEP(GeoFeatureSet &featureSet, const std::string &path,
               const Category *category = nullptr);


}


#endif
