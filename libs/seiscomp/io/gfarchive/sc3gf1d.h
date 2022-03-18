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


#ifndef SEISCOMP_IO_GFARCHIVE_SAUL_H
#define SEISCOMP_IO_GFARCHIVE_SAUL_H


#include <seiscomp/io/gfarchive.h>
#include <seiscomp/seismology/ttt.h>

#include <string>
#include <map>
#include <list>
#include <set>


namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API SC3GF1DArchive : public GFArchive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SC3GF1DArchive();
		SC3GF1DArchive(const std::string &baseDirectory);

		//! D'tor
		~SC3GF1DArchive();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(std::string);
		void close();

		std::list<std::string> availableModels() const;
		std::list<double> availableDepths(const std::string &model) const;

		bool setTimeSpan(const Core::TimeSpan &span);

		//! Adds a request for a greensfunction.
		bool addRequest(const std::string &id,
		                const std::string &model,
		                const GFSource &source,
		                const GFReceiver &receiver);

		bool addRequest(const std::string &id,
		                const std::string &model,
		                const GFSource &source,
		                const GFReceiver &receiver,
		                const Core::TimeSpan &span);

		Core::GreensFunction* get();

		OPT(double) getTravelTime(const std::string &phase,
		                          const std::string &model,
		                          const GFSource &source,
		                          const GFReceiver &receiver);


	// ----------------------------------------------------------------------
	//  Private member
	// ----------------------------------------------------------------------
	private:
		bool hasModel(const std::string &) const;
		Core::GreensFunction* read(const std::string &file,
		                           const Core::TimeSpan &ts, double timeOfs);


	// ----------------------------------------------------------------------
	//  Private member
	// ----------------------------------------------------------------------
	private:
		struct Request {
			Core::TimeSpan timeSpan;
			std::string    id;
			std::string    model;
			double         distance;
			double         depth;
		};

		typedef std::list<Request> RequestList;
		typedef std::set<double> DoubleList;
		typedef std::map<double, double> TTDepth;
		typedef std::map<double, TTDepth> TTDistance;
		typedef std::map<std::string, TTDistance> TTPhases;

		struct ModelConfig {
			ModelConfig() : travelTimesInitialized(false) {}

			DoubleList                  distances;
			DoubleList                  depths;
			TTPhases                    travelTimes;
			bool                        travelTimesInitialized;
			std::string                 travelTimeInterfaceName{"LOCSAT"};
			std::string                 travelTimeInterfaceProfile{"iasp91"};
			TravelTimeTableInterfacePtr travelTimeTable;
		};

		typedef std::map<std::string, ModelConfig> ModelMap;

		ModelMap           _models;
		std::string        _baseDirectory;
		Core::TimeSpan     _defaultTimespan;
		RequestList        _requests;
};

}
}

#endif
