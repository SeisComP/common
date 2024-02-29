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
#include "handler/broker.h"
#include "handler/db.h"


using namespace std;
using namespace Seiscomp::IO;
using namespace Seiscomp::Wired;


using string_sink = Seiscomp::Core::ContainerSink<string>;
using osstream = boost::iostreams::stream<string_sink>;


namespace {


string ERR_QUEUE_DOES_NOT_EXIST   = "The queue does not exist.";
// string ERR_QUEUE_ACCESS_FORBIDDEN = "The queue is not allowed to be accessed from your IP address.";


}


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
bool HttpSession::handleRequest(HttpRequest &req) {
	if ( !isAuthenticated() ) {
		sendStatus(HTTP_401, "Authentication required");
		return true;
	}

	if ( !isAuthorized() ) {
		sendStatus(HTTP_403, "Access is not authorized");
		return true;
	}

	return Wired::HttpSession::handleRequest(req);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleGETRequest(Wired::HttpRequest &req) {
	req.state = HttpRequest::FINISHED;

	// First check WS Upgrade of the broker URL
	if ( req.upgrade ) {
		if ( req.path.compare(0, global.http.brokerPath.size(), global.http.brokerPath) ) {
			sendStatus(HTTP_404);
			return true;
		}

		if ( !global.http.brokerPath.empty() ) {
			req.path.erase(0, global.http.brokerPath.size());
		}

		URLInsituPath path(req.path);
		if ( path.next() ) {
			auto requestQueue = path.savePart();

			if ( path.next() ) {
				sendStatus(HTTP_404);
				return true;
			}

			if ( req.secWebsocketProtocol == DBHandler::PROTOCOL_NAME ) {
				auto q = _server->getQueue(requestQueue);
				if ( !q ) {
					sendStatus(HTTP_404);
					return true;
				}

				if ( !q->acl.check(static_cast<Socket*>(device())->address()) ) {
					// Sending 403 means to expose internal information about the
					// existence of the queue. This is in accordance with the web socket
					// implementation. At least we have the option to discreminate
					// between those two cases.

					// sendStatus(HTTP_403, ERR_QUEUE_ACCESS_FORBIDDEN);
					sendStatus(HTTP_404, ERR_QUEUE_DOES_NOT_EXIST);
					return true;
				}

				return handleDatabaseUpgrade(q->dbURL);
			}
			else if ( req.secWebsocketProtocol == BrokerHandler::PROTOCOL_NAME ) {
				return handleBrokerUpgrade(requestQueue);
			}
		}
		else if ( req.secWebsocketProtocol == BrokerHandler::PROTOCOL_NAME ) {
			return handleBrokerUpgrade(string());
		}

		// No support for protocols currently
		static string error = "400 Unsupported websocket protocol";
		sendStatus(HTTP_400, error);
		return true;
	}

	if ( req.path.compare(0, global.http.staticPath.size(), global.http.staticPath) ) {
		sendStatus(HTTP_404);
		return true;
	}

	if ( !global.http.staticPath.empty() ) {
		req.path.erase(0, global.http.staticPath.size());
	}

	URLInsituPath path(req.path);
	const char *filename = path.part_start;

	if ( !path.next() ) {
		filename = "/";
	}
	else if ( path == "api" ) {
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

	// Here we serve static files
	if ( global.http.filebase.empty() ) {
		sendStatus(HTTP_404);
		return true;
	}

	// Not more than 1kb as buffer
	FileBufferPtr file = new FileBuffer(1024);
	string fn = global.http.filebase + filename;
	if ( fn.empty() ) {
		sendResponse(HTTP_404);
		return true;
	}

	const char *mimeType = nullptr;

	if ( fn[fn.size()-1] == '/' ) {
		fn += "index.html";
		mimeType = FileBuffer::mimeType(FileBuffer::HTML);
	}
	else {
		size_t pdot = fn.rfind('.');
		if ( pdot != string::npos )
			mimeType = FileBuffer::mimeType(&fn[pdot+1]);
	}

	if ( !file->open(fn, "r") )
		sendStatus(HTTP_404);
	else
		sendResponse(file.get(), HTTP_200, mimeType);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handlePOSTRequest(Wired::HttpRequest &req) {
	req.state = HttpRequest::FINISHED;

	URLInsituPath path(req.path);

	if ( path.next() ) {
		if ( path == "api" ) {
			if ( path.next() ) {
				if ( path == "db" ) {
					if ( _request.upgrade ) {
						return handleDatabaseUpgrade(req.data);
					}
					else {
						sendStatus(HTTP_426);
						return true;
					}
				}
			}
		}
	}

	// All failed API requests result in 404.
	sendResponse(HTTP_404);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleBrokerUpgrade(const string &) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool HttpSession::handleDatabaseUpgrade(const string &) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
