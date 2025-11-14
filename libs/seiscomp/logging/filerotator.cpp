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


#define SEISCOMP_COMPONENT log
#include <seiscomp/logging/filerotator.h>

#include <cstdarg>
#include <cstdio>
#include <sstream>
#include <unistd.h>

#ifndef WIN32
#include <sys/stat.h>
#endif


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Logging {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FileRotatorOutput::FileRotatorOutput(int timeSpan, int historySize, int maxFileSize)
: _timeSpan(timeSpan)
, _historySize(historySize)
, _maxFileSize(maxFileSize)
, _lastInterval(-1) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FileRotatorOutput::FileRotatorOutput(const char* filename, int timeSpan, int historySize, int maxFileSize)
: FileOutput(filename)
, _timeSpan(timeSpan)
, _historySize(historySize)
, _maxFileSize(maxFileSize)
, _lastInterval(-1) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FileRotatorOutput::open(const char* filename) {
	if ( !FileOutput::open(filename) ) {
		return false;
	}

#ifndef WIN32
	struct stat st;
	if ( stat(filename, &st) == 0 ) {
		_lastInterval = st.st_mtime / _timeSpan;
	}

#endif
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FileRotatorOutput::log(const char* channelName,
                            LogLevel level,
                            const char* msg,
                            time_t time) {
	std::lock_guard<std::mutex> l(outputMutex);

	int currentInterval = (int)(time / (time_t)_timeSpan);

	if ( _lastInterval == -1 )
		_lastInterval = currentInterval;

	if ( _lastInterval != currentInterval ) {
		rotateLogs();
		_lastInterval = currentInterval;
	}
	else if ( _maxFileSize > 0 ) {
		if ( _stream.tellp() > (std::ofstream::pos_type)_maxFileSize ) {
			rotateLogs();
		}
	}

	FileOutput::log(channelName, level, msg, time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FileRotatorOutput::removeLog(int index) {
	std::stringstream ss;
	ss << _filename << "." << index;
	unlink(ss.str().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FileRotatorOutput::renameLog(int oldIndex, int newIndex) {
	std::stringstream oldFile, newFile;
	oldFile << _filename;
	if ( oldIndex > 0 ) {
		oldFile << "." << oldIndex;
	}

	newFile << _filename;
	if ( newIndex > 0 ) {
		newFile << "." << newIndex;
	}

	rename(oldFile.str().c_str(), newFile.str().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FileRotatorOutput::rotateLogs() {
	// Close current stream
	if ( _stream.is_open() ) {
		_stream.close();
	}

	// Rotate the log files
	removeLog(_historySize);

	for ( int i = _historySize-1; i >= 0; --i ) {
		renameLog(i, i+1);
	}

	// Open the new stream
	int tmp(_lastInterval);
	open(_filename.c_str());
	_lastInterval = tmp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
