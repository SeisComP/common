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


#include <fstream>
#include <queue>
#include <list>
#include <set>
#include <mutex>

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
		~SDSArchive() override;


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(const std::string &source) override;

		bool addStream(const std::string &networkCode,
		               const std::string &stationCode,
		               const std::string &locationCode,
		               const std::string &channelCode) override;

		bool addStream(const std::string &networkCode,
		               const std::string &stationCode,
		               const std::string &locationCode,
		               const std::string &channelCode,
		               const OPT(Core::Time) &startTime,
		               const OPT(Core::Time) &endTime) override;

		bool setStartTime(const OPT(Core::Time) &stime) override;
		bool setEndTime(const OPT(Core::Time) &etime) override;
		bool setTimeWindow(const Core::TimeWindow &tw) override;

		bool setTimeout(int seconds) override;

		void close() override;

		Record *next() override;


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
			      const OPT(Core::Time) &stime,
			      const OPT(Core::Time) &etime);

			Index &operator=(const Index &other);
			bool operator<(const Index &other) const;
			bool operator!=(const Index &other) const;
			bool operator==(const Index &other) const;

			std::string net;
			std::string sta;
			std::string loc;
			std::string cha;

			mutable OPT(Core::Time) stime;
			mutable OPT(Core::Time) etime;
		};


		using IndexSet = std::set<Index>;
		using IndexList = std::list<Index>;
		using File = std::pair<std::string, bool>;
		using FileQueue = std::queue<File>;

		std::vector<std::string>  _arcroots;
		OPT(Core::Time)           _stime;
		OPT(Core::Time)           _etime;
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
