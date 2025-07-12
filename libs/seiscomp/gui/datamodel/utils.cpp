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


#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/sensorlocation.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/gui/datamodel/utils.h>


namespace Seiscomp {
namespace Gui {


double elevation(const DataModel::Station *sta) {
	try {
		return sta->elevation();
	}
	catch ( ... ) {
		return 0;
	}
}


double elevation(const DataModel::SensorLocation *sloc) {
	try {
		return sloc->elevation();
	}
	catch ( ... ) {
		if ( sloc->station() ) {
			return elevation(sloc->station());
		}
	}
	return 0;
}


double computeDistance(const DataModel::Origin *org,
                       const DataModel::Station *sta,
                       double defaultDepth,
                       double *az, double *baz,
                       double *epicentral) {
	double salt = elevation(sta);
	double edep = defaultDepth;
	try {
		edep = org->depth();
	}
	catch ( ... ) {}

	return computeDistance(org->latitude(), org->longitude(), edep,
	                       sta->latitude(), sta->longitude(), salt,
	                       az, baz, epicentral);
}


double computeDistance(const DataModel::Origin *org,
                       const DataModel::SensorLocation *sloc,
                       double defaultDepth, double *az, double *baz,
                       double *epicentral) {
	double salt = elevation(sloc);
	double edep = defaultDepth;
	try {
		edep = org->depth();
	}
	catch ( ... ) {}

	return computeDistance(org->latitude(), org->longitude(), edep,
	                       sloc->latitude(), sloc->longitude(), salt,
	                       az, baz, epicentral);
}


}
}
