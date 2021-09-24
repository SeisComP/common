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


#include <seiscomp/geo/featureset.h>


namespace Seiscomp {
namespace Geo {


/**
 * @brief Reads a GeoJSON file and adds found features to the feature set.
 *        In case of an error an exception is thrown.
 * @param featureSet The target feature that will hold the read features
 * @param filename The path to the GeoJSON file
 * @param category An optional category attached to all read features
 * @return The number of features read
 */
size_t readGeoJSON(GeoFeatureSet &featureSet, const std::string &path,
                   const Category *category = nullptr);


}
}


#endif
