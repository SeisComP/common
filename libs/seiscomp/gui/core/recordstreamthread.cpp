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


#define SEISCOMP_COMPONENT RecordStreamThread
#include <seiscomp/logging/log.h>

#include <seiscomp/gui/core/recordstreamthread.h>

#include <iostream>

#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/record.h>
#include <seiscomp/core/strings.h>

Q_DECLARE_METATYPE(Seiscomp::RecordPtr)

namespace Seiscomp {
namespace Gui {


int RecordStreamThread::_numberOfThreads = 0;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamThread::RecordStreamThread(const std::string& recordStreamURL)
: QThread()
, _id(_numberOfThreads)
, _recordStreamURL(recordStreamURL) {
	_requestedClose = false;
	_readingStreams = false;
	_dataType = Array::FLOAT;
	_recordHint = Record::DATA_ONLY;

	qRegisterMetaType<Seiscomp::RecordPtr>("Seiscomp::RecordPtr");

	++_numberOfThreads;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamThread::~RecordStreamThread() {
	--_numberOfThreads;
	stop(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordStreamThread::ID() const {
	return _id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::connect() {
	_requestedClose = false;
	_readingStreams = false;

	SEISCOMP_DEBUG("[rthread %d] trying to open stream '%s'", ID(), _recordStreamURL.c_str());

	_recordStream = IO::RecordStream::Open(_recordStreamURL.c_str());

	if ( !_recordStream ) {
		SEISCOMP_ERROR("[rthread %d] could not create stream from URL %s", ID(), _recordStreamURL.c_str());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setStartTime(const OPT(Core::Time) &t) {
	if ( !_recordStream ) {
		return;
	}

	SEISCOMP_DEBUG("[rthread %d] setting start time = %s",
	               ID(), t ? t->iso() : "<null>");

	_recordStream->setStartTime(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setEndTime(const OPT(Core::Time) &t) {
	if ( !_recordStream ) {
		return;
	}

	SEISCOMP_DEBUG("[rthread %d] setting end time = %s",
	               ID(), t ? t->iso() : "<null>");

	_recordStream->setEndTime(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setTimeWindow(const Core::TimeWindow &tw) {
	if ( !_recordStream ) {
		return;
	}

	_recordStream->setTimeWindow(tw);

	SEISCOMP_DEBUG("[rthread %d] setting time window: start = %s, end = %s",
	               ID(), tw.startTime().iso(), tw.endTime().iso());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::setTimeout(int seconds) {
	if ( !_recordStream ) {
		return false;
	}

	return _recordStream->setTimeout(seconds);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::addStation(const std::string &network, const std::string &station) {
	if ( !_recordStream ) {
		return false;
	}

	SEISCOMP_DEBUG("[rthread %d] adding stream %s.%s.??.???", ID(), network.c_str(), station.c_str());
	return _recordStream->addStream(network, station, "??", "???");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::addStream(const std::string &network, const std::string &station,
                                   const std::string &location, const std::string &channel) {
	if ( !_recordStream ) {
		return false;
	}

	SEISCOMP_DEBUG("[rthread %d] adding stream %s.%s.%s.%s", ID(), network.c_str(), station.c_str(), location.c_str(), channel.c_str());
	return _recordStream->addStream(network, station, location, channel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordStreamThread::addStream(const std::string& network, const std::string& station,
                                   const std::string& location, const std::string& channel,
                                   const OPT(Core::Time) &stime, const OPT(Core::Time) &etime) {
	if ( !_recordStream ) {
		return false;
	}

	SEISCOMP_DEBUG("[rthread %d] adding stream %s.%s.%s.%s - %s~%s", ID(),
	               network, station, location, channel,
	               stime ? stime->iso() : "",
	               etime ? etime->iso() : "");
	return _recordStream->addStream(network, station, location, channel, stime, etime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::run() {
	if ( !_recordStream ) {
		SEISCOMP_DEBUG("[rthread %d] no stream source set, running aborted", ID());
		return;
	}

	SEISCOMP_DEBUG("[rthread %d] running record acquisition", ID());

	_mutex.lock();
	_readingStreams = true;
	_requestedClose = false;
	_mutex.unlock();

	RecordStreamState::Instance().openedConnection(this);

	_mutex.lock();
	IO::RecordInput recInput(_recordStream.get(), _dataType, _recordHint);
	_mutex.unlock();
	try {
		for ( Record *rec : recInput ) {
			bool stopAcquisition;
			_mutex.lock();
			stopAcquisition = _requestedClose;
			_mutex.unlock();
			if ( stopAcquisition ) {
				SEISCOMP_DEBUG("[rthread %d] close request leads to breaking the acquisition loop", ID());
				break;
			}
			if ( rec ) {
				try {
					rec->endTime();
					emit receivedRecord(rec);
				}
				catch ( ... ) {
					SEISCOMP_ERROR("[rthread %d] Skipping invalid record for %s.%s.%s.%s (fsamp: %0.2f, nsamp: %d)",
					               ID(),
					               rec->networkCode().c_str(), rec->stationCode().c_str(), rec->locationCode().c_str(),
					               rec->channelCode().c_str(), rec->samplingFrequency(), rec->sampleCount());
				}
			}
		}
	}
	catch ( Core::OperationInterrupted &e ) {
		SEISCOMP_INFO("[rthread %d] acquisition exception: %s", ID(), e.what());
	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("[rthread %d] acquisition exception: %s", ID(), e.what());
		emit handleError(QString(e.what()));
	}

	SEISCOMP_DEBUG("[rthread %d] finished record acquisition", ID());

	RecordStreamState::Instance().closedConnection(this);

	_mutex.lock();
	_readingStreams = false;
	_mutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::stop(bool waitForTermination) {
	if ( isFinished() ) {
		// Thread is finished, nothing to do ...
		return;
	}

	_mutex.lock();
	_requestedClose = true;
	if ( !_readingStreams ) {
		SEISCOMP_DEBUG("[rthread %d] actually no stream are being read", ID());
	}

	if (_recordStream && _readingStreams) {
		SEISCOMP_DEBUG("[rthread %d] about to close record acquisition stream", ID());
		_recordStream->close();
		SEISCOMP_DEBUG("[rthread %d] closed record acquisition stream", ID());
	}
	_mutex.unlock();

	if ( !isRunning() ) {
		SEISCOMP_DEBUG("[rthread %d] not running now", ID());
		wait();
	}

	if ( waitForTermination ) {
		SEISCOMP_DEBUG("waiting for thread %d to finish", ID());
		wait(5000);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &RecordStreamThread::recordStreamURL() const {
	return _recordStreamURL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Array::DataType RecordStreamThread::dataType() const {
	return _dataType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setDataType(Array::DataType dataType) {
	_dataType = dataType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record::Hint RecordStreamThread::recordHint() const {
	return _recordHint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamThread::setRecordHint(Record::Hint hint) {
	_recordHint = hint;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamState RecordStreamState::_instance;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamState::RecordStreamState() : QObject() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamState &RecordStreamState::Instance() {
	return _instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordStreamState::connectionCount() const {
	return _connectionCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<RecordStreamThread*> RecordStreamState::connections() const {
	return _activeThreads;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamState::openedConnection(RecordStreamThread *thread) {
	++_connectionCount;
	_activeThreads.removeAll(thread);
	_activeThreads.append(thread);

	emit connectionEstablished(thread);

	if ( _connectionCount == 1 ) {
		SEISCOMP_DEBUG("First connection established");
		emit firstConnectionEstablished();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordStreamState::closedConnection(RecordStreamThread *thread) {
	--_connectionCount;
	_activeThreads.removeAll(thread);

	if ( _connectionCount < 0 ) {
		assert(false);
		_connectionCount = 0;
	}

	emit connectionClosed(thread);

	if ( !_connectionCount ) {
		SEISCOMP_DEBUG("Last connection closed");
		emit lastConnectionClosed();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

} // namespace Gui
} // namespace Seiscomp
