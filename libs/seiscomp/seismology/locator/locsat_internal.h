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


#ifndef SEISCOMP_INTERNAL_SEISMOLOGY_LOCSAT_H
#define SEISCOMP_INTERNAL_SEISMOLOGY_LOCSAT_H


#include <seiscomp/core/exceptions.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>


namespace Seiscomp{
namespace Internal{


#include "locsat_internal_types.h"


class LocSAT {

public:
	LocSAT();
	~LocSAT();

	void reset();
	Loc* doLocation();

	void addSite(const char* station, float lat, float lon, float elev);

	void addArrival(long arrival_id, const char* station, const char* phase,
	                double time, float deltim, int defining);
	void setArrivalAzimuth(float azimuth, float delaz, int defining);
	void setArrivalSlowness(float slow, float delslo, int defining);

	void setOrigin(float lat_init, float lon_init, float depth_init);
	void setOriginTime(int year4, int month, int day, int hour, int minute, int second, int usec);
	void setOriginTime(double epoch);

	void setLocatorParams(Locator_params* params);

	void setLocatorErrors();
	void setOriginErr();

	Loc* getNewLocation();

	void printLocatorParams();

private:
	Arrival* _arrival;
	Assoc* _assoc;
	Origerr* _origerr;
	Origin* _origin;
	Site* _sites;
	Locator_params* _locator_params;
	Locator_errors* _locator_errors;
	struct date_time* _dt;
	int _num_obs;
	char* _newnet;
	int _num_sta;

	int _siteCount;
	int _arrivalCount;
	int _assocCount;

};

} // of namespace Internal
} // of namespace Seiscomp

#endif
