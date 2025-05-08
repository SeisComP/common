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


#ifndef SEISCOMP_SEISMOLOGY_LOCSAT_H
#define SEISCOMP_SEISMOLOGY_LOCSAT_H


#include <seiscomp/core/exceptions.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/seismology/locatorinterface.h>
#include <seiscomp/core.h>

#include "db3/db_arrival.h"
#include "db3/db_assoc.h"
#include "db3/db_origerr.h"
#include "db3/db_origin.h"
#include "db3/db_site.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <map>


namespace {


using namespace Seiscomp;


struct LocatorParams {
	LocatorParams() {
		outfile_name = prefix = nullptr;
	}

	~LocatorParams() {
		if ( outfile_name ) {
			delete [] outfile_name;
		}
		if ( prefix ) {
			delete [] prefix;
		}
	}

	/* DEFAULT - DESCRIPTION                     */
	int     num_dof;         /* 9999    - number of degrees of freedom    */
	float   est_std_error;   /* 1.0     - estimate of data std error      */
	float   conf_level;      /* 0.9     - confidence level    	     */
	float   damp;            /* -1.0    - damping (-1.0 means no damping) */
	int     max_iterations;  /* 20      - limit iterations to convergence */
	char    fix_depth;       /* true    - use fixed depth ?               */
	float   fixing_depth;    /* 0.0     - fixing depth value              */
	float   lat_init;        /* modifiable - initial latitude             */
	float   lon_init;        /* modifiable - initial longitude            */
	float   depth_init;      /* modifiable - initial depth                */
	int     use_location;    /* true    - use current origin data ?       */
	char    verbose;         /* true    - verbose output of data ?        */
	int     cor_level;       /* 0       - correction table level          */
	char   *outfile_name;    /* nullptr    - name of file to print data      */
	char   *prefix;          /* nullptr    - dir name & prefix of tt tables  */
};


struct LocatorError {
	int arid;
	int time;
	int az;
	int slow;
};


class SC_SYSTEM_CORE_API LOCSAT : public Seismology::LocatorInterface {
	public:
		LOCSAT();


	public:
		bool init(const Config::Config &config) override;

		//! Returns supported parameters to be changed.
		IDList parameters() const override;

		//! Returns the value of a parameter.
		std::string parameter(const std::string &name) const override;

		//! Sets the value of a parameter.
		bool setParameter(const std::string &name,
		                  const std::string &value) override;

		IDList profiles() const override;
		void setProfile(const std::string &name) override;

		int capabilities() const override;

		DataModel::Origin* locate(PickList& pickList) override;
		DataModel::Origin* locate(PickList& pickList,
		                          double initLat, double initLon, double initDepth,
		                          const Seiscomp::Core::Time& initTime) override;

		DataModel::Origin* relocate(const DataModel::Origin* origin) override;


	private:
		void setLocatorParams(int param, const char* value);
		std::string getLocatorParams(int param) const;
		void setDefaultLocatorParams();

		bool loadArrivals(const DataModel::Origin *origin);
		DataModel::Origin *fromPicks(PickList &pickList);

		double stationCorrection(const std::string &staid, const std::string &stacode,
		                         const std::string &phase) const;


	private:
		void reset();
		DataModel::Origin *locate();

		void addSite(const char* station, float lat, float lon, float elev);

		void addArrival(long arrival_id, const char* station, const char* phase,
		                double time, float deltim, int defining);
		void setArrivalAzimuth(float azimuth, float delaz, int defining);
		void setArrivalSlowness(float slow, float delslo, int defining);

		void setOrigin(float lat_init, float lon_init, float depth_init);
		void setOriginTime(double epoch);


	private:
		using PhaseCorrectionMap = std::map<std::string, double>;
		using StationCorrectionMap = std::map<std::string, PhaseCorrectionMap>;

		static const std::string   _defaultTablePrefix;
		static const IDList        _allowedParameters;

		StationCorrectionMap       _stationCorrection;
		std::string                _tablePrefix;
		bool                       _computeConfidenceEllipsoid;
		double                     _minArrivalWeight{0.5};
		double                     _defaultPickUncertainty;
		bool                       _usePickUncertainties{false};
		bool                       _usePickBackazimuth{true};
		bool                       _usePickSlowness{true};

		bool                       _enableDebugOutput;

		IDList                     _profiles;

		std::vector<Arrival>       _arrivals;
		std::vector<Assoc>         _assocs;
		std::vector<Site>          _sites;
		Origerr                    _origerr;
		Origin                     _origin;
		LocatorParams              _params;
		std::vector<LocatorError>  _errors;
};


}


#endif
