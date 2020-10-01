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


#ifndef SC_LOGGING_OUTPUT_H
#define SC_LOGGING_OUTPUT_H

#include <seiscomp/logging/node.h>
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace Logging {

/**
 * \brief Logging output class
 *
 * To implement a special kind of logging output,
 * derive from this class and implement the
 * method Output::log(...) to receive logging
 * messages.
 * \code
 * MyOutput log;
 * log.subscribe(GetAll());
 * \endcode
 */
class SC_SYSTEM_CORE_API Output : public Node {
	protected:
		Output();

	public:
		virtual ~Output() {}

	public:
		/** Subscribe to a particular channel */
		bool subscribe(Channel* channel);
		bool unsubscribe(Channel* channel);
		void logComponent(bool e) { _logComponent = e; }
		void logContext(bool e) { _logContext = e; }
		void setUTCEnabled(bool e) { _useUTC = e; }

	protected:
		/** Callback method for receiving log messages */
		virtual void log(const char* channelName,
		                 LogLevel level,
		                 const char* msg,
		                 time_t time) = 0;

		/** The following methods calls are only valid inside the
		    log(...) method */

		/** Returns the current component name */
		const char* component() const;
		/** Returns the sourcefile of the current log entry */
		const char* fileName() const;
		/** Returns the function name of the current log entry */
		const char* functionName() const;
		/** Returns the line number of the current log entry */
		int lineNum() const;

	private:
		void publish(const Data &data);

	protected:
		bool _logComponent;
		bool _logContext;
		bool _useUTC;

	private:
		PublishLoc* _publisher;
};

}
}

#endif
