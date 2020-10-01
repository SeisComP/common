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


#ifndef SEISCOMP_RECORDSTREAM_DECIMATION_H
#define SEISCOMP_RECORDSTREAM_DECIMATION_H

#include <sstream>
#include <map>

#include <seiscomp/core/genericrecord.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/core.h>

namespace Seiscomp {
namespace RecordStream {

DEFINE_SMARTPOINTER(Decimation);

class SC_SYSTEM_CORE_API Decimation : public Seiscomp::IO::RecordStream {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		Decimation();
		virtual ~Decimation();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		virtual bool setSource(const std::string &source);
		virtual bool setRecordType(const char *type);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode);

		virtual bool addStream(const std::string &networkCode,
		                       const std::string &stationCode,
		                       const std::string &locationCode,
		                       const std::string &channelCode,
		                       const Seiscomp::Core::Time &stime,
		                       const Seiscomp::Core::Time &etime);

		virtual bool setStartTime(const Seiscomp::Core::Time &stime);
		virtual bool setEndTime(const Seiscomp::Core::Time &etime);
		virtual bool setTimeWindow(const Seiscomp::Core::TimeWindow &w);

		virtual bool setTimeout(int seconds);

		virtual void close();

		virtual Record *next();


	// ----------------------------------------------------------------------
	//  Private Interface
	// ----------------------------------------------------------------------
	private:
		void cleanup();

		int checkSR(Record *rec) const;

		bool push(Record *rec);
		GenericRecord *convert(Record *rec);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		typedef std::vector<double> Coefficients;

		struct ResampleStage {
			ResampleStage() : nextStage(nullptr) {}
			~ResampleStage() { if ( nextStage ) delete nextStage; }

			double targetRate;

			// Fixed source sample rate derived from the first record
			// received.
			double sampleRate;
			double dt;

			// Flag that indicates that a streams is passed through
			// without resampling.
			bool passThrough;

			bool valid;

			int N;
			int N2;
			size_t samplesToSkip;

			Coefficients *coefficients;

			// The ring buffer that holds the last samples for downsampling.
			std::vector<double> buffer;

			// The number of samples still missing in the buffer before
			// filtering can be done
			size_t missingSamples;

			// The front index of the ring buffer
			size_t front;

			// Time of front of ring buffer
			Core::Time startTime;

			// End time of last record
			Core::Time lastEndTime;

			ResampleStage *nextStage;

			void reset() {
				missingSamples = buffer.size();
				front = 0;
				samplesToSkip = 0;
				startTime = Core::Time();
				lastEndTime = Core::Time();

				if ( nextStage ) nextStage->reset();
			}
		};

		typedef std::map<int, Coefficients*> CoefficientMap;
		typedef std::map<std::string, ResampleStage*> StreamMap;

		void init(ResampleStage *stage, Record *rec);
		bool initCoefficients(ResampleStage *stage);
		GenericRecord *resample(ResampleStage *stage, Record *rec);

		IO::RecordStreamPtr _source;
		double              _targetRate;
		double              _fp;
		double              _fs;
		int                 _maxN;
		int                 _coeffScale;
		StreamMap           _streams;
		CoefficientMap      _coefficients;
		GenericRecord      *_nextRecord;
};

}
}

#endif
