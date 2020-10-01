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



#ifndef SEISCOMP_TTT_LIBTAU_H
#define SEISCOMP_TTT_LIBTAU_H


#include <string>
#include <seiscomp/seismology/ttt.h>

extern "C" {

#include <libtau/tau.h>

}


namespace Seiscomp {
namespace TTT {


/**
 * TTTLibTau
 *
 * A class to compute seismic travel times for 1D models like "iasp91".
 */
class SC_SYSTEM_CORE_API LibTau : public TravelTimeTableInterface {
	public:
		/**
		 * Construct a TravelTimeTable object for the specified model.
		 * Currently only "iasp91" is supported.
		 */
		LibTau();
		~LibTau();

		LibTau(const LibTau &other);
		LibTau &operator=(const LibTau &other);


	public:
		bool setModel(const std::string &model);
		const std::string &model() const;


		/**
		 * Select a branch of traveltimes to be computed. Currently only
		 * "all" is supported. Use the default!
		 */
		void setBranch(const std::string &branch="all");


		/**
		 * Compute the traveltime(s) for the branch selected using setBranch()
		 * @param dep1 The source depth in km
		 *
		 * @returns A TravelTimeList of travel times sorted by time.
		 *
		 * It should be noted that in this implementation it is extremely
		 * important to compute as many travel times for the same focal
		 * depth as possible. Changing the depth will always result in
		 * the time-consuming re-computation of some internal tables,
		 * which can be avoided by as many consecutive compute() calls as
		 * possible, for the same depth.
		 *
		 * XXX However:
		 * XXX NEITHER ellipticity NOR altitude correction is currently
		 * XXX implemented! The respective parameters are ignored.
		 */
		TravelTimeList *compute(double lat1, double lon1, double dep1,
		                        double lat2, double lon2, double alt2 = 0.,
		                        int ellc = 1);


		/**
		 * Compute the traveltime for the branch selected using setBranch()
		 * and the first (fastest) phase.
		 * @param dep1 The source depth in km
		 *
		 * @returns A TravelTime
		 *
		 * XXX However:
		 * XXX NEITHER ellipticity NOR altitude correction is currently
		 * XXX implemented! The respective parameters are ignored.
		 */
		TravelTime computeFirst(double lat1, double lon1, double dep1,
		                        double lat2, double lon2, double alt2 = 0.,
		                        int ellc = 1);


	private:
		TravelTimeList *compute(double delta, double depth);
		TravelTime computeFirst(double delta, double depth);

		/**
		 * Sets the source depth.
		 * @param depth The source depth in km
		 */
		void setDepth(double depth);

		void initPath(const std::string &model);

		libtau _handle;

		double _depth;
		std::string _model;

		bool _initialized;
};


}
}


#endif
