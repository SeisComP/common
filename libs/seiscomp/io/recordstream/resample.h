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


#ifndef SEISCOMP_RECORDSTREAM_RESAMPLE_H
#define SEISCOMP_RECORDSTREAM_RESAMPLE_H


#include <sstream>
#include <map>
#include <deque>

#include <seiscomp/io/recordstream.h>
#include <seiscomp/io/recordfilter/demux.h>


namespace Seiscomp {
namespace RecordStream {


DEFINE_SMARTPOINTER(Resample);

class SC_SYSTEM_CORE_API Resample : public Seiscomp::IO::RecordStream {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		Resample();
		virtual ~Resample();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		virtual bool setSource(const std::string &source) override;
		virtual bool setRecordType(const char *type) override;

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode) override;

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const OPT(Core::Time) &startTime,
		                       const OPT(Core::Time) &endTime) override;

		virtual bool setStartTime(const OPT(Core::Time) &stime) override;
		virtual bool setEndTime(const OPT(Core::Time) &etime) override;
		virtual bool setTimeWindow(const Core::TimeWindow &w) override;

		virtual bool setTimeout(int seconds) override;

		virtual void close() override;

		virtual Record *next() override;


	// ----------------------------------------------------------------------
	//  Private Interface
	// ----------------------------------------------------------------------
	private:
		void push(Record *rec);
		void cleanup();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		typedef std::deque<GenericRecord*> OutputQueue;

		IO::RecordStreamPtr   _source;
		bool                  _debug;
		IO::RecordDemuxFilter _demuxer;
		OutputQueue           _queue;

		double                _targetRate;
		double                _fp;
		double                _fs;
		int                   _lanczosKernelWidth;
		int                   _coeffScale;
};

}
}

#endif
