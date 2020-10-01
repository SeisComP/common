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



#ifndef SC_LOGGING_PUBLISHER_H
#define SC_LOGGING_PUBLISHER_H

#include <seiscomp/logging/common.h>
#include <seiscomp/logging/node.h>

#include <stdarg.h>

namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Channel;

// documentation in implementation file
class SC_SYSTEM_CORE_API Publisher : public Node {
	public:
		Publisher(PublishLoc *src);
		Publisher();
		virtual ~Publisher();

		// metadata about the publisher and its publication
		PublishLoc *src;

		static void Publish(PublishLoc *, Channel *,
		                    const char *format, ...);
		static void PublishVA(PublishLoc *, Channel *,
		                      const char *format, va_list args);

	protected:
		virtual void setEnabled(bool newState);

		Publisher(const Publisher &);
		Publisher & operator=(const Publisher &);
};


}
}


#endif
