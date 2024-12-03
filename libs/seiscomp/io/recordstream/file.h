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


#ifndef SEISCOMP_SERVICES_RECORDSTREAM_RECORDFILE_H
#define SEISCOMP_SERVICES_RECORDSTREAM_RECORDFILE_H

#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <seiscomp/io/recordstream.h>
#include <seiscomp/core.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(File);


class SC_SYSTEM_CORE_API File : public Seiscomp::IO::RecordStream {
	public:
		enum SeekDir {
			Begin = std::ios_base::beg,
			Current = std::ios_base::cur,
			End = std::ios_base::end
		};

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		File() = default;
		File(std::string name);
		File(const File &f);
		~File() override;


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		File &operator=(const File &f);


	// ----------------------------------------------------------------------
	//  Public RecordStream interface
	// ----------------------------------------------------------------------
	public:
		bool setSource(const std::string &filename) override;

		bool addStream(const std::string &networkCode,
		               const std::string &stationCode,
		               const std::string &locationCode,
		               const std::string &channelCode) override;

		bool addStream(const std::string &networkCode,
		               const std::string &stationCode,
		               const std::string &locationCode,
		               const std::string &channelCode,
		               const OPT(Seiscomp::Core::Time) &startTime,
		               const OPT(Seiscomp::Core::Time) &endTime) override;

		bool setStartTime(const OPT(Seiscomp::Core::Time) &startTime)  override;
		bool setEndTime(const OPT(Seiscomp::Core::Time) &endTime) override;

		void close() override;

		bool setRecordType(const char *type) override;

		Record *next() override;


	// ----------------------------------------------------------------------
	//  Public file specific interface
	// ----------------------------------------------------------------------
	public:
		std::string name() const;
		size_t tell();

		File &seek(size_t pos);
		File &seek(int off, SeekDir dir);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		struct TimeWindowFilter {
			TimeWindowFilter() {}
			TimeWindowFilter(const OPT(Core::Time) &stime,
			                 const OPT(Core::Time) &etime)
			: start(stime), end(etime) {}

			OPT(Core::Time) start;
			OPT(Core::Time) end;
		};

		using FilterMap = std::map<std::string, TimeWindowFilter>;
		using ReFilterList = std::vector<std::pair<std::string,TimeWindowFilter> >;

		const TimeWindowFilter *findTimeWindowFilter(Record *rec);

		RecordFactory   *_factory{nullptr};
		std::string      _name;
		bool             _closeRequested;
		std::fstream     _fstream;
		std::istream    *_current{&_fstream};
		FilterMap        _filter;
		ReFilterList     _reFilter;
		OPT(Core::Time)  _startTime;
		OPT(Core::Time)  _endTime;
};


}
}

#endif
