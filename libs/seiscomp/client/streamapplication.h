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


#ifndef SEISCOMP_CLIENT_STREAM_APPLICATION_H
#define SEISCOMP_CLIENT_STREAM_APPLICATION_H


#include <seiscomp/client/application.h>
#include <seiscomp/core/record.h>
#include <seiscomp/io/recordstream.h>

#include <mutex>


namespace Seiscomp {
namespace Client {


class SC_SYSTEM_CLIENT_API StreamApplication : public Application {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		StreamApplication(int argc, char **argv);
		~StreamApplication();


	public:
		bool openStream();
		void closeStream();

		IO::RecordStream* recordStream() const;

		bool addStation(const std::string& networkCode,
		                const std::string& stationCode);
		bool addStream(const std::string& networkCode,
		               const std::string& stationCode,
		               const std::string& locationCode,
		               const std::string& channelCode);

		void setStartTime(const Seiscomp::Core::Time&);
		void setEndTime(const Seiscomp::Core::Time&);
		bool setTimeWindow(const Seiscomp::Core::TimeWindow&);

		//! Sets whether to start the acquisition automatically
		//! before the run loop or not. This method has to be called before run().
		//! The default is true. If set to false then the acquisition needs
		//! to be started with readRecords or startRecordThread and
		//! autoCloseOnAcquisitionFinished is also set to false.
		void setAutoAcquisitionStart(bool);

		//! Sets the application close flag when acquisition is finished.
		//! The default is true as auto start is true. If setAutoAcquisitionStart
		//! is changed this flag is set as well.
		void setAutoCloseOnAcquisitionFinished(bool);

		//! Sets the storage hint of incoming records.
		//! The default is: DATA_ONLY
		void setRecordInputHint(Record::Hint hint);

		//! Sets the data type of read records.
		//! The default is: FLOAT
		void setRecordDatatype(Array::DataType datatype);

		//! Returns the data type of the internal record sample buffer
		Array::DataType recordDataType() const { return _recordDatatype; }

		void startRecordThread();
		void waitForRecordThread();
		bool isRecordThreadActive() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		bool init();
		bool run();
		void done();
		void exit(int returnCode);

		bool dispatch(Core::BaseObject* obj);

		void readRecords(bool sendEndNotification);

		//! This method gets called when the acquisition is finished
		//! The default implementation closes the objects queue and
		//! finishes the application
		virtual void acquisitionFinished();

		//! This method gets called when a new record has been received
		//! by recordstream thread.
		//! The default implementation stores it in the threaded object
		//! queue which gets read by the main thread.
		//! The input record is not managed and ownership is transferred
		//! to this method.
		virtual bool storeRecord(Record *rec);

		//! This method gets called when a record has been popped from
		//! the event queue in the main thread. The ownership of the
		//! pointer is transferred to this method. An empty function
		//! body override would cause a memory leak.
		virtual void handleRecord(Record *rec) = 0;

		//! Logs the received records for the last period
		virtual void handleMonitorLog(const Core::Time &timestamp);


	private:
		bool                _startAcquisition;
		bool                _closeOnAcquisitionFinished;
		Record::Hint        _recordInputHint;
		Array::DataType     _recordDatatype;
		IO::RecordStreamPtr _recordStream;
		std::thread        *_recordThread;
		size_t              _receivedRecords;
		ObjectLog          *_logRecords;
};


}
}


#endif
