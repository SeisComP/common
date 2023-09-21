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



#ifndef SEISCOMP_TTT_LOCSAT_H
#define SEISCOMP_TTT_LOCSAT_H


#include <string>
#include <vector>
#include <seiscomp/seismology/ttt.h>


namespace Seiscomp {
namespace TTT {


/**
 * TTTLibTau
 *
 * A class to compute seismic travel times for 1D models like "iasp91".
 */
class SC_SYSTEM_CORE_API Locsat : public TravelTimeTableInterface {
	public:
		struct Velocity {
			Velocity() {}
			Velocity(float z, float p, float s) : depth(z), vp(p), vs(s) {}
			float depth;
			float vp;
			float vs;
		};

	public:
		/**
		 * Construct a TravelTimeTable object for the specified model.
		 * Currently only "iasp91" is supported.
		 */
		Locsat();
		~Locsat();

		Locsat(const Locsat &other);
		Locsat &operator=(const Locsat &other);


	public:
		bool setModel(const std::string &model) override;
		const std::string &model() const override;


		/**
		 * @brief Compute the traveltime(s) for the model selected using
		 *        setModel().
		 *
		 * Note that altitude correction is currently not implemented! The
		 * respective parameters are ignored.
		 * @param dep1 The source depth in km
		 *
		 * @returns A TravelTimeList of travel times sorted by time.
		 */
		TravelTimeList *compute(double lat1, double lon1, double dep1,
		                        double lat2, double lon2, double alt2 = 0.,
		                        int ellc = 1) override;

		/**
		 * Compute the traveltime and a given phase. The default implementation
		 * computes the complete travel time list and searches for them
		 * requested phase.
		 * @param dep1 The source depth in km
		 *
		 * @returns A TravelTime
		 */
		TravelTime compute(const char *phase,
		                   double lat1, double lon1, double dep1,
		                   double lat2, double lon2, double alt2=0.,
		                   int ellc = 1) override;

		/**
		 * @brief Compute the traveltime for the model selected using setModel()
		 *        and the first (fastest) phase.
		 *
		 * Note that altitude correction is currently not implemented! The
		 * respective parameters are ignored.
		 * @param dep1 The source depth in km
		 * @returns A TravelTime
		 */
		TravelTime computeFirst(double lat1, double lon1, double dep1,
		                        double lat2, double lon2, double alt2 = 0.,
		                        int ellc = 1) override;


		/**
		 * Compute the travel time for a given phase. This is intended
		 * to be a faster call than 'compute' since only the travel time
		 * is returned.
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
		double computeTime(const char *phase,
		                   double lat1, double lon1, double dep1,
		                   double lat2, double lon2, double elev2=0.,
		                   int ellc = 1) override;


	private:
		TravelTimeList *compute(double delta, double depth);
		TravelTime compute(const char *phase, double delta, double depth);
		TravelTime computeFirst(double delta, double depth);
		double computeTime(const char *phase, double delta, double depth);

		bool initTables();


	private:
		std::string _model;
		std::string _tablePrefix;
		int         _Pindex;
};


}
}


#endif
