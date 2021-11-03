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


#ifndef SEISCOMP_SERVICES_RECORDSTREAM_ROUTING_H
#define SEISCOMP_SERVICES_RECORDSTREAM_ROUTING_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>
#include <thread>

#include <seiscomp/core/datetime.h>
#include <seiscomp/core/timewindow.h>
#include <seiscomp/io/recordstream.h>
#include <seiscomp/core.h>
#include <seiscomp/client/queue.h>

#include "balanced.h"

namespace Seiscomp {
namespace RecordStream {
namespace Balanced {
namespace _private {

DEFINE_SMARTPOINTER(RoutingConnection);

class SC_SYSTEM_CORE_API RoutingConnection : public BalancedConnection {
	DECLARE_SC_CLASS(RoutingConnection)

	public:
		//! C'tor
		RoutingConnection();

		//! Initializing Constructor
		RoutingConnection(std::string serverloc);

		//! Destructor
		virtual ~RoutingConnection();

	public:

		//! Initialize the combined connection.
		virtual bool setSource(const std::string &serverloc);
 
	protected:
		virtual int getRS(const std::string &networkCode,
		                      const std::string &stationCode,
		                      const std::string &locationCode,
		                      const std::string &channelCode); 

	private:
		struct Rule {
			const std::string type;
			const std::string source;
			const std::string matchOpt;
			const std::regex match;
		};
		std::vector<Rule> _rules;
};

} // namespace _private
} // namesapce Balanced
} // namespace RecordStream
} // namespace Seiscomp

#endif

