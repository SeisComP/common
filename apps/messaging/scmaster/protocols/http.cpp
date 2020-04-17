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


#include <seiscomp/io/archive/jsonarchive.h>
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

	if ( !global.http.staticPath.empty() ) {
		req.path.erase(0, global.http.staticPath.size());
	}

	Wired::URLInsituPath path(req.path);
	const char *filename = path.part_start;

	if ( path.next() ) {
		if ( path == "api" ) {
			if ( path.next() ) {
				if ( path == "stats.json" ) {
					Wired::BufferPtr response = new Wired::Buffer;
					_server->lockStatistics();
					{
						boost::iostreams::stream<Core::ContainerSink<string>> os(response->data);
						IO::JSONArchive json;
						json.create(&os);
						json << NAMED_OBJECT("stats", _server->cummulatedStatistics());
						json.close();
					}
					_server->unlockStatistics();
					sendResponse(response.get(), Wired::HTTP_200, "application/json");
					return true;
				}
			}

			sendResponse(Wired::HTTP_404);
			return true;
		}
	}

	// Not more than 1kb as buffer
	Wired::FileBufferPtr file = new Wired::FileBuffer(1024);
	string fn = global.http.filebase + filename;
	if ( fn.empty() ) {
		sendResponse(Wired::HTTP_404);
		return true;
	}

	const char *mimeType = nullptr;

	if ( fn[fn.size()-1] == '/' ) {
		fn += "index.html";
		mimeType = Wired::FileBuffer::mimeType(Wired::FileBuffer::HTML);
	}
	else {
		size_t pdot = fn.rfind('.');
		if ( pdot != string::npos )
			mimeType = Wired::FileBuffer::mimeType(&fn[pdot+1]);
	}

	if ( !file->open(fn, "r") )
		sendStatus(Wired::HTTP_404);
	else
		sendResponse(file.get(), Wired::HTTP_200, mimeType);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
