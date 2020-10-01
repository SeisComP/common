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


#ifndef SEISCOMP_GUI_SLCONNECTION_H
#define SEISCOMP_GUI_SLCONNECTION_H

#include <string>

#include <QThread>
#include <QtCore>
#ifndef Q_MOC_RUN
#include <seiscomp/core/record.h>
#include <seiscomp/io/recordinput.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {

//! \brief This class provides a thread to receive records from
//! \brief a stream source.
class SC_GUI_API RecordStreamThread : public QThread {
	Q_OBJECT

	public:
		RecordStreamThread(const std::string& recordStreamURL);
		~RecordStreamThread();


	public:
		bool connect();

		void setStartTime(const Seiscomp::Core::Time&);
		void setEndTime(const Seiscomp::Core::Time&);
		void setTimeWindow(const Seiscomp::Core::TimeWindow&);

		bool setTimeout(int seconds);

		bool addStation(const std::string& network, const std::string& station);

		// Needs to be called after connect()
		bool addStream(const std::string& network, const std::string& station,
		               const std::string& location, const std::string& channel);
		// Needs to be called after connect()
		bool addStream(const std::string& network, const std::string& station,
		               const std::string& location, const std::string& channel,
		               const Seiscomp::Core::Time &stime, const Seiscomp::Core::Time &etime);

		// Needs to be called after connect()
		bool addStream(const std::string& network, const std::string& station,
		               const std::string& location, const std::string& channel,
		               double gain);

		void stop(bool waitForTermination);

		//! Returns the used stream URL
		const std::string& recordStreamURL() const;

		//! Returns the array data type this record stream thread is
		//! configured to use (default: Array::FLOAT).
		Array::DataType dataType() const;

		//! Set the array data type this record stream thread should
		//! use (default: Array::FLOAT). NOTE: The data type must be
		//! set before calling run() to have any impact.
		void setDataType(Array::DataType dataType);

		//! Returns the array data type this record stream thread is
		//! configured to use (default: Array::FLOAT).
		Record::Hint recordHint() const;

		//! Set the record save hint (default: Record::DATA_ONLY).
		//! NOTE: The hint must be set before calling run() to have any impact.
		void setRecordHint(Record::Hint hint);

		//! Returns the current recordthread ID
		int ID() const;


	signals:
		//! This signal will be fired whenever a new record has been
		//! read from the stream source. The receiver has to take care
		//! to store the record inside a RecordPtr to prevent memory leaks.
		//! The RecordStreamThread does not destroy any objects read from
		//! the stream source.
		void receivedRecord(Seiscomp::Record*);

		//! This signal will be fired if an error occurs.
		void handleError(const QString &);


	protected:
		void run();


	private:
		typedef std::map<std::string, double> GainMap;
		int                                   _id;
		std::string                           _recordStreamURL;
		bool                                  _requestedClose;
		bool                                  _readingStreams;
		Seiscomp::IO::RecordStreamPtr         _recordStream;
		QMutex                                _mutex;
		static int                            _numberOfThreads;
		GainMap                               _gainMap;
		Array::DataType                       _dataType;
		Record::Hint                          _recordHint;
};


class SC_GUI_API RecordStreamState : public QObject {
	Q_OBJECT

	protected:
		RecordStreamState();

	public:
		static RecordStreamState& Instance();

		int connectionCount() const;
		QList<RecordStreamThread*> connections() const;

	private:
		void openedConnection(RecordStreamThread*);
		void closedConnection(RecordStreamThread*);

	signals:
		void connectionEstablished(RecordStreamThread*);
		void connectionClosed(RecordStreamThread*);

		void firstConnectionEstablished();
		void lastConnectionClosed();

	private:
		static RecordStreamState _instance;

		int _connectionCount;
		QList<RecordStreamThread*> _activeThreads;

	friend class RecordStreamThread;
};


} // namespace Gui
} // namespace Seiscomp

#endif
