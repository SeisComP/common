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


#ifndef SC_LOGGING_FILEROTATOR_H
#define SC_LOGGING_FILEROTATOR_H


#include <seiscomp/logging/file.h>
#include <mutex>


namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API FileRotatorOutput : public FileOutput {
	public:
		FileRotatorOutput(int timeSpan = 60*60*24, int historySize = 7,
		                  int maxFileSize = 100*1024*1024);
		/**
		 * Creates a new FileRotatorOutput instance
		 * @param filename The filename to log to
		 * @param timeSpan The timespan in seconds for the log time before rotating
		 * @param count The number of historic files to store
		 */
		FileRotatorOutput(const char* filename, int timeSpan = 60*60*24,
		                  int historySize = 7, int maxFileSize = 100*1024*1024);

		bool open(const char* filename);

	protected:
		/** Callback method for receiving log messages */
		void log(const char* channelName,
		         LogLevel level,
		         const char* msg,
		         time_t time);

	private:
		void rotateLogs();
		void removeLog(int index);
		void renameLog(int oldIndex, int newIndex);


	protected:
		//! time span to keep one log
		int _timeSpan;

		//! number of log files to keep
		int _historySize;

		//! maximum file size of a log file
		int _maxFileSize;

		//! last log file written to
		int _lastInterval;

		std::mutex outputMutex;
};


}
}

#endif
