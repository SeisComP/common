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



#ifndef SEISCOMP_SEISMOLOGY_REGIONS_H
#define SEISCOMP_SEISMOLOGY_REGIONS_H

#include <seiscomp/core.h>
#include <seiscomp/seismology/regions/polygon.h>
#include <string>


namespace Seiscomp {

class SC_SYSTEM_CORE_API Regions {
	public:
		static std::string getFlinnEngdahlRegion(double lat, double lon, int *id = nullptr);

		/**
		 * @return The number of available Flinn-Engdahl regions.
		 */
		static int getFlinnEngdahlRegionsCount();

		/**
		 * @brief Returns the Flinn-Engdahl region by id.
		 * Note that the id starts at 1.
		 * @param id The Flinn-Engdahl region id
		 * @return The Flinn-Engdahl region name. If the id is out of
		 *         bounds then an empty string will be returned.
		 */
		static std::string getFlinnEngdahlRegionById(int id);

		static void load();
		static std::string getRegionName(double lat, double lon);
		static Seiscomp::Geo::PolyRegions &polyRegions();

	private:
		Regions() = default;
};


} // of ns Seiscomp

#endif
