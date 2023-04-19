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


/*! @class Seiscomp::Logging::Channel <seiscomp/logging/channel.h>
  @brief Implements a hierarchical logging channel

  You should not need to use Channel directly under normal
  circumstances. See COMPONENT_CHANNEL() macro, GetComponentChannel() and
  getGlobalChannel()

  Channel implements channel logging support.  A channel is a named
  logging location which is global to the program.  Channels are
  hierarchically related.

  For example, if somewhere in your program a message is logged to
  "debug/foo/bar", then it will be delived to any subscribers to
  "debug/foo/bar", or subscribers to "debug/foo", or subscribers to
  "debug".   Subscribing to a channel means you will receive anything
  published on that channel or sub-channels.

  As a special case, subscribing to the channel "" means you will receive
  all messages - as every message has a channel and the empty string "" is
  considered to mean the root of the channel tree.

  In addition, componentized channels are all considered sub channels of
  the global channel hierarchy.  All rDebug(), rWarning(), and rError()
  macros publish to the componentized channels (component defined by
  SEISCOMP_COMPONENT).

  @code
      // get the "debug" channel for our component.  This is the same as
      // what rDebug() publishes to.
      Channel *node = COMPONENT_CHANNEL("debug", Log_Debug);
      // equivalent to
      Channel *node = GetComponentChannel(SEISCOMP_COMPONENT, "debug");

      // Or, get the global "debug" channel, which will have messages from
      // *all* component's "debug" channels.
      Channel *node = getGlobalChannel( "debug", Log_Debug );
  @endcode

  @author Valient Gough
  @see COMPONENT_CHANNEL()
  @see getComponentChannel()
  @see getGlobalChannel()
*/
class SC_SYSTEM_CORE_API Channel : public Node {
	public:
		using ComponentMap = std::map<std::string, Channel*>;

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
