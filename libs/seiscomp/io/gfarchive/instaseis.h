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


#ifndef SEISCOMP_IO_GFARCHIVE_INSTASEIS_H
#define SEISCOMP_IO_GFARCHIVE_INSTASEIS_H


#include <seiscomp/io/gfarchive.h>
#include <seiscomp/io/socket.h>

#include <string>
#include <vector>
#include <list>


namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API Instaseis : public GFArchive {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Instaseis();
		Instaseis(const std::string &url);

		//! D'tor
		~Instaseis();


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
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		bool getInfo() const;


	// ----------------------------------------------------------------------
	//  Private member
	// ----------------------------------------------------------------------
	private:
		struct Request {
			Core::TimeSpan timeSpan;
			std::string    id;
			double         distance;
			double         depth;
		};

		typedef std::list<Request> RequestList;

		std::string          _host;
		std::string          _path;
		int                  _timeout;
		Core::TimeSpan       _defaultTimespan;
		RequestList          _requests;

		mutable Socket       _socket;
		mutable std::string  _model;

		mutable double       _dt;

		mutable int          _maxLength;
		mutable double       _srcShift;

		mutable double       _minDepth;
		mutable double       _maxDepth;

		mutable double       _minDist;
		mutable double       _maxDist;

		mutable bool         _hasInfo;
};


}
}


#endif
