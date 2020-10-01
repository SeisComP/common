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

#ifndef SC_LOGGING_DEFS_H
#define SC_LOGGING_DEFS_H

#include <seiscomp/core.h>

namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Channel;

SC_SYSTEM_CORE_API extern Channel *_SCDebugChannel;
SC_SYSTEM_CORE_API extern Channel *_SCInfoChannel;
SC_SYSTEM_CORE_API extern Channel *_SCWarningChannel;
SC_SYSTEM_CORE_API extern Channel *_SCErrorChannel;
SC_SYSTEM_CORE_API extern Channel *_SCNoticeChannel;


}
}


#endif
