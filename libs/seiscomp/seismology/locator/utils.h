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


#ifndef SEISCOMP_SEISMOLOGY_LOCATOR_UTILS_H
#define SEISCOMP_SEISMOLOGY_LOCATOR_UTILS_H


namespace Seiscomp{

namespace DataModel {

class Origin;
class OriginQuality;

}


/**
 * @brief Compiles an origin quality object from an origin.
 * Computed attributes:
 *  * minimum distance
 *  * median distance
 *  * maximum distance
 *  * azimuthal gap
 *  * associated phase count
 *  * used phase count
 *  * depth phase count
 *  * associated station count
 *  * used station count
 *
 * Preconditions are:
 *  * Arrivals must be present to generate phase counts
 *  * Arrivals must have azimuth and distance set to generate azimuthal gap and
 *    min/max/median distances
 *  * Picks must be resolvable (Pick::Find) to update station counts
 *
 * Values that cannot be determined will not be written into the output quality,
 * meaning that this function will never set an output attribute to None.
 *
 * @param quality The output quality
 * @param origin The input origin
 */
void compile(DataModel::OriginQuality &quality, const DataModel::Origin *origin);


/**
 * @brief Convenience function which populates the origin quality of an origin
 *        object.
 * If the origin holds already a quality object then it will be updated.
 * Otherwise a new quality object will be set.
 * @param origin The origin that will receive the quality object.
 */
void populateQuality(DataModel::Origin *origin);



}


#endif
