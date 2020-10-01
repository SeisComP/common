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
		Regions();

		static void load();
		static std::string getRegionName(double lat, double lon);
		static Seiscomp::Geo::PolyRegions &polyRegions();

	private:
		static std::string getFeGeoRegionName(double lat, double lon);
		static std::string getRegionalName(double lat, double lon);

};


} // of ns Seiscomp

#endif
