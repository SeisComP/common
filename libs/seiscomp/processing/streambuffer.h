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


#ifndef SEISCOMP_PROCESSING_STREAMBUFFER_H
#define SEISCOMP_PROCESSING_STREAMBUFFER_H


#include <string>
#include <list>

#include <seiscomp/core/recordsequence.h>
#include <seiscomp/client.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API StreamBuffer {
	// ----------------------------------------------------------------------
	//  Public Types
	// ----------------------------------------------------------------------
	public:
		struct SC_SYSTEM_CLIENT_API WaveformID {
			WaveformID(const std::string &net, const std::string &sta,
			           const std::string &loc, const std::string &cha)
			: networkCode(net), stationCode(sta)
			, locationCode(loc), channelCode(cha)
			{}
		
			WaveformID(const Record *rec)
			: networkCode(rec->networkCode())
			, stationCode(rec->stationCode())
			, locationCode(rec->locationCode())
			, channelCode(rec->channelCode())
			{}


			bool operator<(const WaveformID& other) const {
				if ( networkCode < other.networkCode ) return true;
				if ( networkCode > other.networkCode ) return false;
		
				if ( stationCode < other.stationCode ) return true;
				if ( stationCode > other.stationCode ) return false;
		
				if ( locationCode < other.locationCode ) return true;
				if ( locationCode > other.locationCode ) return false;
		
				return channelCode < other.channelCode;
			}
		
		
			std::string networkCode;
			std::string stationCode;
			std::string locationCode;
			std::string channelCode;
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! Creates streambuffer with no type yet
		//! use setTimeWindow or setTimeSpan before using it
		StreamBuffer();

		//! Creates a timewindow buffer
		StreamBuffer(const Core::TimeWindow &timeWindow);

		//! Creates a ringbuffer
		StreamBuffer(const Core::TimeSpan &timeSpan);

		~StreamBuffer();


	// ----------------------------------------------------------------------
	//  Interface
	// ----------------------------------------------------------------------
	public:
		void setTimeWindow(const Core::TimeWindow &timeWindow);
		void setTimeSpan(const Core::TimeSpan &timeSpan);

		RecordSequence *sequence(const WaveformID &wid) const;
		RecordSequence *feed(const Record *rec);

		bool addedNewStream() const;

		void printStreams(std::ostream &os = std::cout) const;
		std::list<std::string> getStreams() const;

		//! Clears the streambuffer and removes all cached records
		void clear();


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		enum Mode {
			TIME_WINDOW,
			RING_BUFFER
		};

		using SequenceMap = std::map<WaveformID, RecordSequence*>;

		Mode                     _mode;
		Seiscomp::Core::Time     _timeStart;
		Seiscomp::Core::TimeSpan _timeSpan;
		SequenceMap              _sequences;
		bool                     _newStreamAdded;
};


}
}


#endif
