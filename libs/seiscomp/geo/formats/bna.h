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


#ifndef SEISCOMP_GEO_FORMATS_BNA_H
#define SEISCOMP_GEO_FORMATS_BNA_H


#include <seiscomp/geo/featureset.h>


namespace Seiscomp {
namespace Geo {


/**
 * @brief Reads one BNA file. A BNA file may contain multiple segments
 *        consisting of multiple points which define a non closed polyline.
 *
 * A new segment is introduced by the following line:
 *   '"arbitrary segment name","rank <#rank>",<#points>'
 * e.g.
 *   '"test segment","rank 1",991'
 *
 * A point or vertex is defined by the following line:
 *   '<latitude>,<longitude>'
 * e.g.
 *   '31.646944,25.151389'
 *
 * In addition the BNA file format supports complex areas such as main land and
 * islands. The coordinate pairs representing the islands are separated from
 * each other by repeating the initial coordinate of the main area. The
 * following sample file illustrates a complex area:
 *
 *   "Test Area","rank 1",17
 *   2,2 --begin main area
 *   2,1
 *   1,1
 *   1,2
 *   2,2 --end of main area
 *   4,4 --begin island #1
 *   4,3
 *   3,3
 *   3,4
 *   4,4 --end island #1
 *   2,2 --end main area
 *   7,7 --begin island #2
 *   7,5
 *   6,4
 *   5,6
 *   7,7 --end of island #2
 *   2,2 --end of main area
 *
 * @param featureSet The target feature that will hold the read features
 * @param filename The path to the GeoJSON file
 * @param category An optional category attached to all read features
 * @return The number of features read
 */
size_t readBNA(GeoFeatureSet &featureSet, const std::string &path,
               const Category *category = nullptr);


}
}


#endif
