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



#ifndef SEISCOMP_IO_GFARCHIVE_HELMBERGER_H
#define SEISCOMP_IO_GFARCHIVE_HELMBERGER_H


#include <seiscomp/io/gfarchive.h>

#include <string>
#include <map>
#include <list>
#include <set>


namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API HelmbergerArchive : public GFArchive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		HelmbergerArchive();
		HelmbergerArchive(const std::string &baseDirectory);

		//! D'tor
		~HelmbergerArchive();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(std::string) override;
		void close() override;

		std::list<std::string> availableModels() const override;
		std::list<double> availableDepths(const std::string &model) const override;

		bool setTimeSpan(const Core::TimeSpan &span) override;

		//! Adds a request for a greensfunction.
		bool addRequest(const std::string &id,
		                const std::string &model,
		                const GFSource &source,
                        const GFReceiver &receiver) override;

		bool addRequest(const std::string &id,
		                const std::string &model,
		                const GFSource &source,
                        const GFReceiver &receiver,
		                const Core::TimeSpan &span) override;

		Core::GreensFunction* get() override;

		OPT(double) getTravelTime(const std::string &phase,
		                          const std::string &model,
		                          const GFSource &source,
		                          const GFReceiver &receiver) override;


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

		struct ModelConfig {
			double velocity;
			DoubleList distances;
			DoubleList depths;
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
