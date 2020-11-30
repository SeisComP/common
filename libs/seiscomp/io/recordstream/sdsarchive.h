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


#ifndef SEISCOMP_IO_RECORDSTREAM_SDSARCHIVE_H
#define SEISCOMP_IO_RECORDSTREAM_SDSARCHIVE_H


#include <iostream>
#include <sstream>
#include <fstream>
#include <queue>
#include <list>
#include <set>
#include <mutex>

#include <seiscomp/core/version.h>
#include <seiscomp/io/recordstream.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(SDS);


class SDSArchive : public Seiscomp::IO::RecordStream {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		SDSArchive();
		SDSArchive(const std::string arcroot);
		virtual ~SDSArchive();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		virtual bool setSource(const std::string &source);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const Seiscomp::Core::Time &startTime,
		                       const Seiscomp::Core::Time &endTime);

		virtual bool setStartTime(const Seiscomp::Core::Time &stime);
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);
		virtual bool setTimeWindow(const Seiscomp::Core::TimeWindow &tw);

		virtual bool setTimeout(int seconds);

		virtual void close();

		virtual Seiscomp::Record *next();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		struct Index {
			Index();
			Index(const std::string& net, const std::string& sta,
			      const std::string& loc, const std::string& cha);
			Index(const std::string& net, const std::string& sta,
			      const std::string& loc, const std::string& cha,
			      const Seiscomp::Core::Time& stime,
			      const Seiscomp::Core::Time& etime);

			Index &operator=(const Index &other);
			bool operator<(const Index &other) const;
			bool operator!=(const Index &other) const;
			bool operator==(const Index &other) const;

			std::string net;
			std::string sta;
			std::string loc;
			std::string cha;
			mutable Seiscomp::Core::Time stime;
			mutable Seiscomp::Core::Time etime;
		};


		typedef std::set<Index> IndexSet;
		typedef std::list<Index> IndexList;
		typedef std::pair<std::string,bool> File;
		typedef std::queue<File> FileQueue;

		std::vector<std::string>  _arcroots;
		Seiscomp::Core::Time      _stime;
		Seiscomp::Core::Time      _etime;
		IndexList                 _orderedRequests;
		IndexSet                  _streamSet;
		IndexList::iterator       _curiter;
		const Index              *_curidx;
		FileQueue                 _fnames;
		std::set<std::string>     _readFiles;
		std::mutex                _mutex;
		bool                      _closeRequested;
		std::ifstream             _file;

		int getDoy(const Seiscomp::Core::Time &time);
		void resolveRequest();
		bool setStart(const std::string &fname, bool bsearch);

		bool resolveNet(std::string &path,
		                const std::string &net, const std::string &sta,
		                const std::string &loc, const std::string &cha,
		                const Seiscomp::Core::Time &requestStartTime,
		                int doy, int year, bool first);
		bool resolveSta(std::string &path,
		                const std::string &net, const std::string &sta,
		                const std::string &loc, const std::string &cha,
		                const Seiscomp::Core::Time &requestStartTime,
		                int doy, int year, bool first);
		bool resolveLoc(std::string &path,
		                const std::string &net, const std::string &sta,
		                const std::string &loc, const std::string &cha,
		                const Seiscomp::Core::Time &requestStartTime,
		                int doy, int year, bool first);
		bool resolveCha(std::string &path,
		                const std::string &net, const std::string &sta,
		                const std::string &loc, const std::string &cha,
		                const Seiscomp::Core::Time &requestStartTime,
		                int doy, int year, bool first);

		bool resolveFiles(const std::string &net, const std::string &sta,
		                  const std::string &loc, const std::string &cha,
		                  const Seiscomp::Core::Time &requestStartTime,
		                  int doy, int year, bool first);
};


}
}


#endif
