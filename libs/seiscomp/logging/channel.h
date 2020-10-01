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


#ifndef SC_LOGGING_CHANNEL_H
#define SC_LOGGING_CHANNEL_H

#include <string>
#include <map>

#include <seiscomp/logging/log.h>
#include <seiscomp/logging/node.h>

namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Channel;

typedef std::map<std::string, Channel*> ComponentMap;

// documentation in implementation file
class SC_SYSTEM_CORE_API Channel : public Node {
	public:
		Channel(const std::string &name, LogLevel level);
		virtual ~Channel();

		virtual void publish(const Data &data);
		const std::string &name() const;

		LogLevel logLevel() const;
		void setLogLevel(LogLevel level);

	protected:
		Channel *getComponent(Channel *componentParent,
		                      const char *component);

	private:
		//! the full channel name
		std::string _name;
		LogLevel _level;

		ComponentMap subChannels;
		ComponentMap components;

		Channel(const Channel &);
		Channel &operator=(const Channel&);

	friend Channel *Seiscomp::Logging::getComponentChannel(
		const char *component,
		const char *path,
		LogLevel level);
};

}
}

#endif
