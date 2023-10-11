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



#ifndef SEISCOMP_CORE_SEISMOLOGY_TTT_H
#define SEISCOMP_CORE_SEISMOLOGY_TTT_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/exceptions.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/core.h>

#include <string>
#include <list>


namespace Seiscomp {


class SC_SYSTEM_CORE_API FileNotFoundError : public Core::GeneralException {
	public:
		FileNotFoundError(std::string const &filename) : Core::GeneralException("File not found: " + filename) {}
};

class SC_SYSTEM_CORE_API MultipleModelsError : public Core::GeneralException {
	public:
		MultipleModelsError(std::string const &model) : Core::GeneralException("Multiple models at the same time are not supported ('" + model +
		                                                                   "' instantiated previously)") {}
};

class SC_SYSTEM_CORE_API NoPhaseError : public Core::GeneralException {
	public:
		NoPhaseError() : Core::GeneralException("No phase available") {}
};



/**
 * TravelTime
 *
 * Representation of a seismic travel time
 */
class SC_SYSTEM_CORE_API TravelTime {
	public:
		TravelTime();
		TravelTime(const std::string &_phase,
		           double _time, double _dtdd, double _dtdh, double _dddp,
		           double _takeoff);

		bool operator==(const TravelTime &other) const;
		bool operator<(const TravelTime &other) const;

		std::string phase; //!< phase code like "P", "pP", "PKiKP" etc.
		double time;       //!< actual travel time in seconds
		double dtdd;       //!< dt/dd angular slowness in sec/deg
		double dtdh;       //!< dt/dh dependence of time on source depth
		double dddp;       //!< dd/dp
		double takeoff;    //!< take-off angle at source
		OPT(double) azi;   //!< azimuth angle at source (for 3D models)
};


/**
 * TravelTimeList
 *
 * A list of TravelTime objects, sorted by travel time value.
 */
class SC_SYSTEM_CORE_API TravelTimeList : public  std::list<TravelTime> {
	public:
		bool isEmpty() { return size()==0; } // XXX temporary hack

		void sortByTime();

		double depth, delta;
};


/**
 * TravelTimeTableInterface
 *
 * An interface class to compute seismic travel times for arbitrary models.
 */

class SC_SYSTEM_CORE_API TravelTimeTableInterface : public Core::BaseObject {
	public:
		TravelTimeTableInterface();
		virtual ~TravelTimeTableInterface();


	public:
		/**
		 * Instantiates a TTT interface and returns the pointer to
		 * be freed by the caller. If name is not valid, nullptr is
		 * returned. Available interfaces: libtau, LOCSAT
		 */
		static TravelTimeTableInterface *Create(const char *name);

		/**
		 * Sets the model to use. Implementations can use this model
		 * string for whatever purpose and must not be validated by
		 * the caller.
		 * Computation of travel times should also work if the
		 * model has not been set from outside.
		 */
		virtual bool setModel(const std::string &model) = 0;

		//! Returns the currently set model
		virtual const std::string &model() const = 0;


		/**
		 * Computes a list of all supported phase.
		 *
		 * @param lat1 Latitude of source
		 * @param lon1 Longitude of source
		 * @param dep1 The source depth in km
		 * @param lat2 Latitude of receiver
		 * @param lon2 Longitude of receiver
		 * @param elev2 Elevation of receiver in m
		 * @param ellc Apply ellipticity correction (1 = on, 0 = off)
		 * @returns A TravelTimeList of travel times sorted by time.
		 */
		virtual TravelTimeList *
		compute(double lat1, double lon1, double dep1,
		        double lat2, double lon2, double elev2=0.,
		        int ellc = 1) = 0;


		/**
		 * Compute the traveltime and a given phase. The default implementation
		 * computes the complete travel time list and searches for them
		 * requested phase.
		 *
		 * @param lat1 Latitude of source
		 * @param lon1 Longitude of source
		 * @param dep1 The source depth in km
		 * @param lat2 Latitude of receiver
		 * @param lon2 Longitude of receiver
		 * @param elev2 Elevation of receiver in m
		 * @param ellc Apply ellipticity correction (1 = on, 0 = off)
		 * @returns A TravelTime
		 */
		virtual TravelTime
		compute(const char *phase,
		        double lat1, double lon1, double dep1,
		        double lat2, double lon2, double elev2=0.,
		        int ellc = 1);


		/**
		 * Compute the traveltime for the first (fastest) phase.
		 *
		 * @param lat1 Latitude of source
		 * @param lon1 Longitude of source
		 * @param dep1 The source depth in km
		 * @param lat2 Latitude of receiver
		 * @param lon2 Longitude of receiver
		 * @param elev2 Elevation of receiver in m
		 * @param ellc Apply ellipticity correction (1 = on, 0 = off)
		 * @returns A TravelTime
		 */
		virtual TravelTime
		computeFirst(double lat1, double lon1, double dep1,
		             double lat2, double lon2, double elev2=0.,
		             int ellc = 1) = 0;

		/**
		 * Compute the travel time for a given phase. This is intended
		 * to be a faster call than 'compute' since only the travel time
		 * is returned. However the default implementation call 'compute'
		 * and return the travel time for the requested phase.
		 *
		 * @param lat1 Latitude of source
		 * @param lon1 Longitude of source
		 * @param dep1 The source depth in km
		 * @param lat2 Latitude of receiver
		 * @param lon2 Longitude of receiver
		 * @param elev2 Elevation of receiver in m
		 * @param ellc Apply ellipticity correction (1 = on, 0 = off)
		 * @returns The travel time for the phase
		 */
		virtual double
		computeTime(const char *phase,
		            double lat1, double lon1, double dep1,
		            double lat2, double lon2, double elev2=0.,
		            int ellc = 1);
};


DEFINE_INTERFACE_FACTORY(TravelTimeTableInterface);
DEFINE_SMARTPOINTER(TravelTimeTableInterface);


class SC_SYSTEM_CORE_API TravelTimeTable : public TravelTimeTableInterface {
	public:
		TravelTimeTable();


	public:
		bool setModel(const std::string &model);
		const std::string &model() const;

		TravelTimeList *
		compute(double lat1, double lon1, double dep1,
		        double lat2, double lon2, double elev2 = 0.,
		        int ellc = 1);

		TravelTime
		compute(const char *phase,
		        double lat1, double lon1, double dep1,
		        double lat2, double lon2, double elev2 = 0.,
		        int ellc = 1);

		TravelTime
		computeFirst(double lat1, double lon1, double dep1,
		             double lat2, double lon2, double elev2 = 0.,
		             int ellc = 1);

	private:
		static TravelTimeTableInterfacePtr _interface;
};



// Some helpers

SC_SYSTEM_CORE_API
bool ellipcorr(const std::string &phase,
               double lat1, double lon1,
               double lat2, double lon2,
               double depth, double &corr);

// Retrieve traveltime for the specified phase. Returns true if phase was
// found, false otherwise.
SC_SYSTEM_CORE_API
const TravelTime* getPhase(const TravelTimeList *, const std::string &phaseCode);

SC_SYSTEM_CORE_API
const TravelTime* firstArrivalP(const TravelTimeList *);


}


#define REGISTER_TRAVELTIMETABLE(Class, Service) \
static Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::TravelTimeTableInterface, Class> __##Class##InterfaceFactory__(Service)


#endif
