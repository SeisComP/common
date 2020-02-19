/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#include <seiscomp/broker/statistics.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


QueueStatistics &QueueStatistics::operator+=(const QueueStatistics &stats) {
	if ( name.empty() )
		name = stats.name;
	else {
		// Check name
	}

	messages += stats.messages;
	bytes += stats.bytes;
	payload += stats.payload;

	groups.resize(stats.groups.size());
	for ( size_t i = 0; i < stats.groups.size(); ++i ) {
		if ( groups[i].name.empty() )
			groups[i].name = stats.groups[i].name;
		else {
			// Check name
		}

		groups[i].messages += stats.groups[i].messages;
		groups[i].bytes += stats.groups[i].bytes;
		groups[i].payload += stats.groups[i].payload;
	}

	return *this;
}


}
}
}
