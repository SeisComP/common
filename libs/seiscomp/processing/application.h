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


#ifndef SEISCOMP_PROCESSING_STREAMPROCESSOR_APPLICATION_H
#define SEISCOMP_PROCESSING_STREAMPROCESSOR_APPLICATION_H


#include <seiscomp/client/streamapplication.h>
#include <seiscomp/datamodel/waveformstreamid.h>
#include <seiscomp/processing/waveformprocessor.h>
#include <seiscomp/processing/timewindowprocessor.h>
#include <seiscomp/processing/streambuffer.h>


namespace Seiscomp {
namespace Processing {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Application class for stream processing commandline applications.
  */
class SC_SYSTEM_CLIENT_API Application : public Client::StreamApplication {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Application(int argc, char **argv);

		//! D'tor
		~Application();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		StreamBuffer& streamBuffer();
		const StreamBuffer& streamBuffer() const;

		void addProcessor(const std::string& networkCode,
		                  const std::string& stationCode,
		                  const std::string& locationCode,
		                  const std::string& channelCode,
		                  WaveformProcessor *wp);
		void addProcessor(const DataModel::WaveformStreamID &wfid,
				  WaveformProcessor *wp);

		void addProcessor(const std::string& networkCode,
		                  const std::string& stationCode,
		                  const std::string& locationCode,
		                  const std::string& channelCode,
		                  TimeWindowProcessor *twp);
		void addProcessor(const DataModel::WaveformStreamID &wfid,
				  TimeWindowProcessor *twp);

		void removeProcessors(const std::string& networkCode,
		                      const std::string& stationCode,
		                      const std::string& locationCode,
		                      const std::string& channelCode);
		void removeProcessors(const DataModel::WaveformStreamID &wfid);

		void removeProcessor(WaveformProcessor *wp);

		size_t processorCount() const;


	// ----------------------------------------------------------------------
	//  Protected methods
	// ----------------------------------------------------------------------
	protected:
		void addObject(const std::string& parentID, DataModel::Object* o) override;
		void removeObject(const std::string& parentID, DataModel::Object* o) override;
		void updateObject(const std::string& parentID, DataModel::Object* o) override;

		void handleRecord(Record *rec) override;

		void enableStation(const std::string& code, bool enabled);
		void enableStream(const std::string& code, bool enabled);

		virtual void handleNewStream(const Record *) {}
		virtual void processorFinished(const Record *, WaveformProcessor *) {}

		void done() override;


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		void registerProcessor(const std::string& networkCode,
		                       const std::string& stationCode,
		                       const std::string& locationCode,
		                       const std::string& channelCode,
		                       WaveformProcessor *wp);
		void registerProcessor(const DataModel::WaveformStreamID &wfid,
				       WaveformProcessor *twp);

		void registerProcessor(const std::string& networkCode,
		                       const std::string& stationCode,
		                       const std::string& locationCode,
		                       const std::string& channelCode,
		                       TimeWindowProcessor *twp);
		void registerProcessor(const DataModel::WaveformStreamID &wfid,
				       TimeWindowProcessor *twp);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef std::multimap<std::string, WaveformProcessorPtr> StationProcessors;
		typedef std::multimap<std::string, WaveformProcessorPtr> ProcessorMap;
		typedef DataModel::WaveformStreamID                      WID;
		typedef std::pair<WID, WaveformProcessorPtr>             WaveformProcessorItem;
		typedef std::pair<WID, TimeWindowProcessorPtr>           TimeWindowProcessorItem;
		typedef std::list<WaveformProcessorItem>                 WaveformProcessorQueue;
		typedef std::list<WaveformProcessorPtr>                  WaveformProcessorRemovalQueue;
		typedef std::list<TimeWindowProcessorItem>               TimeWindowProcessorQueue;

		ProcessorMap                    _processors;
		StationProcessors               _stationProcessors;

		StreamBuffer                    _waveformBuffer;

		WaveformProcessorQueue          _waveformProcessorQueue;
		WaveformProcessorRemovalQueue   _waveformProcessorRemovalQueue;
		TimeWindowProcessorQueue        _timeWindowProcessorQueue;
		bool                            _registrationBlocked;
};


}
}

#endif
