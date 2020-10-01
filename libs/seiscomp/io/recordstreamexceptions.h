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


#ifndef SEISCOMP_IO_RECORDSTREAMEXCEPTIONS_H
#define SEISCOMP_IO_RECORDSTREAMEXCEPTIONS_H

#include <seiscomp/core/exceptions.h>
#include <seiscomp/core.h>

namespace Seiscomp {
namespace IO {


class SC_SYSTEM_CORE_API RecordStreamException: public Seiscomp::Core::StreamException {
	public:
		RecordStreamException(): Seiscomp::Core::StreamException("RecordStream exception") {}
		RecordStreamException(const std::string& what): StreamException(what) {}
};


class SC_SYSTEM_CORE_API RecordStreamTimeout: public RecordStreamException {
	public:
		RecordStreamTimeout(): RecordStreamException("timeout") {}
		RecordStreamTimeout(const std::string& what): RecordStreamException(what) {}
};


}
}

#endif
