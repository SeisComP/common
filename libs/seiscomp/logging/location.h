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



#ifndef SC_LOGGING_LOCATION_H
#define SC_LOGGING_LOCATION_H

#include <string>

#include <seiscomp/logging/log.h>
#include <seiscomp/logging/node.h>

namespace Seiscomp {
namespace Logging {

// documentation in implementation file
class SC_SYSTEM_CORE_API FileNode : public Node {
	public:
		FileNode(const char *componentName, const char *fileName);
		FileNode(const char *fileName);
		virtual ~FileNode();

		static FileNode *Lookup(const char *componentName,
		                        const char *fileName);
		static FileNode *Lookup(const char *fileName);

		std::string componentName;
		std::string fileName;

	private:
        FileNode(const FileNode&);
		FileNode & operator = (const FileNode&);
};

}
}

#endif

