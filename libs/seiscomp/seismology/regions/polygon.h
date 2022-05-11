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



#ifndef SEISCOMP_SEISMOLOGY_REGIONS_POLY_H
#define SEISCOMP_SEISMOLOGY_REGIONS_POLY_H

#include <seiscomp/core.h>
#include <seiscomp/geo/feature.h>
#include <vector>


namespace Seiscomp {
namespace Geo {


class SC_SYSTEM_CORE_API PolyRegions {
	public:
		PolyRegions() = default;
		PolyRegions(const std::string &location);
		~PolyRegions();

	public:
		void print();
		void info();

		GeoFeature *findRegion(double lat, double lon) const;
		std::string findRegionName(double lat, double lon) const;

		size_t regionCount() const;
		void addRegion(GeoFeature* r);
		GeoFeature *region(int i) const;

		size_t read(const std::string& location);

		const std::string& dataDir() const { return _dataDir; }

	private:
		bool readFepBoundaries(const std::string& filename);

	private:
		std::vector<GeoFeature*> _regions;
		std::string _dataDir;
};


} // of ns Regions
} // of ns Seiscomp


#endif
