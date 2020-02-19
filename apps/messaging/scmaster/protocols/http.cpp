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


#include <seiscomp/wired/buffers/file.h>

#include "http.h"
#include "../settings.h"


using namespace std;


namespace Seiscomp {
namespace Messaging {
namespace Protocols {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
HttpSession::HttpSession(Wired::Socket *sock, Broker::Server *server)
: Wired::HttpSession(sock, "http")
, _server(server) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleGETRequest(Wired::HttpRequest &req) {
	if ( global.http.filebase.empty() ) {
		sendStatus(Wired::HTTP_404);
		return true;
	}

	if ( req.path.compare(0, global.http.staticPath.size(), global.http.staticPath) ) {
		sendStatus(Wired::HTTP_404);
		return true;
	}

	string localPath = req.path.substr(global.http.staticPath.size());

	// Not more than 1kb as buffer
	Wired::FileBufferPtr file = new Wired::FileBuffer(1024);
	string fn = global.http.filebase + localPath;

	if ( fn[fn.length()-1] == '/' )
		fn += "index.html";

	std::cerr << fn << std::endl;

	if ( !file->open(fn, "r") )
		sendStatus(Wired::HTTP_404);
	else
		sendResponse(file.get(), Wired::HTTP_200, NULL);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
