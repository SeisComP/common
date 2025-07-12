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

#include "session.h"
#include "settings.h"
#include "version.h"
#include "strings.h"

#define SEISCOMP_COMPONENT WFAS

#include <seiscomp/logging/log.h>
#include <seiscomp/core/datetime.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/io/recordstream/sdsarchive.h>
#include <seiscomp/io/records/mseedrecord.h>
#include <seiscomp/wired/protocols/http.h>

#include <iostream>
#include <ctype.h>
#include <cerrno>
#include <sstream>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace std;


namespace Seiscomp {
namespace Applications {
namespace Wfas {


string wadlDataselectPre = "\
<application xmlns=\"http://wadl.dev.java.net/2009/02\"\n\
		xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n\
	<resources base=\"";
string wadlDataselectPost = "/dataselect/1\">\n\
		<resource path=\"query\">\n\
			<method name=\"POST\">\n\
				<response status=\"200\">\n\
					<representation mediaType=\"application/vnd.fdsn.mseed\"/>\n\
				</response>\n\
				<response status=\"204 400 401 403 404 413 414 500 503\">\n\
					<representation mediaType=\"text/plain\"/>\n\
				</response>\n\
			</method>\n\
		</resource>\n\
		<resource path=\"version\">\n\
			<method name=\"GET\">\n\
				<response>\n\
					<representation mediaType=\"text/plain\"/>\n\
				</response>\n\
			</method>\n\
		</resource>\n\
		<resource path=\"application.wadl\">\n\
			<method name=\"GET\">\n\
				<response>\n\
					<representation mediaType=\"application/xml\"/>\n\
				</response>\n\
			</method>\n\
		</resource>\n\
	</resources>\n\
</application>";

const char *TimeFormats[] = {
	"%FT%T.%f",    // YYYY-MM-DDThh:mm:ss.ssssss
	"%Y-%jT%T.%f", // YYYY-DDDThh:mm:ss.ssssss"
	"%FT%T",       // YYYY-MM-DDThh:mm:ss
	"%Y-%jT%T",    // YYYY-DDDThh:mm:ss
	"%FT%R",       // YYYY-MM-DDThh:mm
	"%Y-%jT%R",    // YYYY-DDDThh:mm
	"%FT%H",       // YYYY-MM-DDThh
	"%Y-%jT%H",    // YYYY-DDDThh
	"%F",          // YYYY-MM-DD
	"%Y-%j",       // YYYY-DDD
	"%Y"           // YYYY
};
const size_t TimeFormatsLen = sizeof(TimeFormats) / sizeof(TimeFormats[0]);



namespace {


bool parseFDSNWSTime(Core::Time &time, const char *str) {
	for ( size_t i = 0; i < TimeFormatsLen; ++i ) {
		if ( time.fromString(str, TimeFormats[i] ) ) {
			return true;
		}
	}
	return false;
}

char * toUpper(char *str) {
	char *s = str;
	while ( *s ) {
		*s = toupper(*s);
		++s;
	}
	return str;
}


} // ns anonymous


DEFINE_SMARTPOINTER(ArchiveBuffer);
struct ArchiveBuffer : Wired::Buffer {
	ArchiveBuffer(IO::RecordStream *in,
	              Array::DataType dt = Array::DOUBLE,
	              Record::Hint h = Record::SAVE_RAW)
	: stream(in), input(in, dt, h) {
		format = Wired::Buffer::Octetts;
	}

	virtual bool updateBuffer() {
		if ( !stream )
			return false;

		data.clear();

		if ( buffered ) {
			const Array *rec_data = buffered->raw();
			data.append((const char*)rec_data->data(), rec_data->size()*rec_data->elementSize());
			buffered = NULL;
		}

		try {
			RecordPtr rec;
			while ( data.size() < MAX_CHUNK_SIZE ) {
				rec = input.next();
				if ( rec == NULL ) {
					// End of stream: close stream and input
					stream = NULL;
					break;
				}

				IO::MSeedRecord *mseed = IO::MSeedRecord::Cast(rec.get());

				// Skip non mseed records
				if ( mseed == NULL ) continue;

				const Array *rec_data = mseed->raw();
				if ( rec_data == NULL ) continue;

				data.append((const char*)rec_data->data(), rec_data->size()*rec_data->elementSize());
			}
		}
		catch ( ... ) {}

		if ( data.empty() )
			header = "0\r\n\r\n";
		else {
			char tmp[10]; tmp[0] = '\0';
			sprintf(tmp, "%X\r\n", (int)data.size());
			header = tmp;
			data += "\r\n";

			// Close transfer block
			if ( !stream )
				data += "0\r\n\r\n";
		}

		return true;
	}

	virtual size_t length() const {
		return string::npos;
	}

	/**
	 * @brief Checks whether the stream has any record or not.
	 *
	 *        This method reads the first valid miniSEED record
	 *        and stores it in an internal variable. That is used
	 *        as first record in the response later.
	 * @return The boolean value indicating data availability
	 */
	bool hasData() {
		if ( !stream )
			return false;

		try {
			while ( !buffered ) {
				RecordPtr rec = input.next();
				if ( rec ) {
					buffered = IO::MSeedRecord::Cast(rec.get());
					const Array *rec_data = buffered->raw();

					if ( rec_data == NULL )
						buffered = NULL;
				}
				else
					break;
			}
		}
		catch ( ... ) {}

		return buffered != NULL;
	}

	IO::RecordStreamPtr stream;
	IO::RecordInput     input;
	IO::MSeedRecordPtr  buffered;
};


class FDSNWSSession : public Wired::HttpSession {
	public:
		FDSNWSSession(Wired::Socket *sock);
		~FDSNWSSession();


	public:
		virtual bool handleGETRequest(Wired::HttpRequest &req) override;
		virtual bool handlePOSTRequest(Wired::HttpRequest &req) override;


	private:
		ArchiveBuffer *createStreamBuffer();
		void sendError(const string &path, const string &options,
		               Wired::HttpStatus status, const char *msg = NULL);
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSListener::FDSNWSListener(const Wired::IPACL &allowedIPs,
                               const Wired::IPACL &deniedIPs,
                               Wired::Socket *socket)
: Wired::AccessControlledEndpoint(socket, allowedIPs, deniedIPs)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Wired::Session *FDSNWSListener::createSession(Wired::Socket *socket) {
	socket->setMode(Wired::Socket::Read);
	return new FDSNWSSession(socket);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSSession::FDSNWSSession(Wired::Socket *sock)
: Wired::HttpSession(sock, "https") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FDSNWSSession::~FDSNWSSession() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArchiveBuffer *FDSNWSSession::createStreamBuffer() {
	IO::RecordStreamPtr stream;

	if ( global.sdsBackend.empty() )
		stream = new RecordStream::SDSArchive();
	else
		stream = IO::RecordStream::Create(global.sdsBackend.c_str());

	if ( stream == NULL ) {
		if ( global.sdsBackend.empty() )
			SEISCOMP_ERROR("Failed to allocate SDS handler");
		else
			SEISCOMP_ERROR("Failed to create SDS handler '%s'", global.sdsBackend.c_str());
		return NULL;
	}

	stream->setSource(global.filebase);
	return new ArchiveBuffer(stream.get(), Seiscomp::Array::INT, Seiscomp::Record::SAVE_RAW);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FDSNWSSession::sendError(const string &path, const string &options,
                              Wired::HttpStatus status, const char *msg) {
	stringstream ss;
	ss << "Error " << status.toString() << endl << endl;

	if ( msg != NULL )
		ss << msg << endl << endl;
	else if ( status == Wired::HTTP_404 )
		ss << "The requested resource does not exist on this server." << endl << endl;

	ss << "Request:" << endl << path;
	if ( !options.empty() ) ss << "?" << options;
	ss << endl << endl
	   << "Request Submitted:" << endl << Core::Time::UTC().iso() << endl << endl
	   << "Service Version:" << endl << SCWFAS_VERSION_NAME;
	sendResponse(ss.str().c_str(), status, "text/plain");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSSession::handleGETRequest(Wired::HttpRequest &req) {
	string options = req.options;
	Wired::URLPath path(req.path);

	req.state = Wired::HttpRequest::FINISHED;

	if ( !path.next() ) {
		sendError(req.path, options, Wired::HTTP_404);
		return true;
	}

	if ( path != "fdsnws" ) {
		sendError(req.path, options, Wired::HTTP_404);
		return true;
	}

	if ( !path.next() ) {
		sendError(req.path, options, Wired::HTTP_404);
		return true;
	}

	if ( path != "dataselect" ) {
		sendError(req.path, options, Wired::HTTP_404);
		return true;
	}

	if ( !path.next() ) {
		sendError(req.path, options, Wired::HTTP_404);
		return true;
	}

	if ( path != "1" ) {
		sendError(req.path, options, Wired::HTTP_404);
		return true;
	}

	if ( !path.next() ) {
		sendError(req.path, options, Wired::HTTP_404);
		return true;
	}

	if ( path == "version" ) {
		static string version = "scwfas v" SCWFAS_VERSION_NAME;

		sendResponse(version, Wired::HTTP_200, "text/plain");
		return true;
	}
	else if ( path == "application.wadl" ) {
		static string wadl;

		if ( wadl.empty() )
			wadl = wadlDataselectPre + global.fdsnws.baseUrl + wadlDataselectPost;

		sendResponse(wadl, Wired::HTTP_200, "text/plain");
		return true;
	}
	else if ( path == "query" ) {
		Wired::URLInsituOptions opts(req.options);
		Core::Time startTime;
		Core::Time endTime = Core::Time::UTC();
		set<string> networks;
		set<string> stations;
		set<string> locations;
		set<string> channels;
		bool noData404 = false;

		while ( opts.next() ) {
			opts.val_len = Wired::HttpSession::urldecode(opts.val, opts.val_len);

			if ( opts == Wired::URLOptionName("net") ||
			     opts == Wired::URLOptionName("network")) {
				vector<string> toks;
				Core::split(toks, toUpper(opts.val), ",");
				networks.insert(toks.begin(), toks.end());
			}
			else if ( opts == Wired::URLOptionName("sta") ||
			          opts == Wired::URLOptionName("station")) {
				vector<string> toks;
				Core::split(toks, toUpper(opts.val), ",");
				stations.insert(toks.begin(), toks.end());
			}
			else if ( opts == Wired::URLOptionName("loc") ||
			          opts == Wired::URLOptionName("location")) {
				vector<string> toks;
				// Do not compress since empty location codes are valid
				Core::split(toks, toUpper(opts.val), ",", false);
				// Replace -- by empty location code
				for ( vector<string>::const_iterator it = toks.begin();
				      it != toks.end(); ++it )
					locations.insert(*it == "--" ? string("") : *it);
			}
			else if ( opts == Wired::URLOptionName("cha") ||
			          opts == Wired::URLOptionName("channel")) {
				vector<string> toks;
				Core::split(toks, toUpper(opts.val), ",");
				channels.insert(toks.begin(), toks.end());
			}
			else if ( opts == Wired::URLOptionName("start") ||
			          opts == Wired::URLOptionName("starttime")) {
				if ( !parseFDSNWSTime(startTime, opts.val) ) {
					sendError(req.path, options, Wired::HTTP_400, "Invalid start time");
					return true;
				}
			}
			else if ( opts == Wired::URLOptionName("end") ||
			          opts == Wired::URLOptionName("endtime")) {
				if ( !parseFDSNWSTime(endTime, opts.val) ) {
					sendError(req.path, options, Wired::HTTP_400, "Invalid end time");
					return true;
				}
			}
			else if ( opts == Wired::URLOptionName("nodata") ) {
				if ( opts.val_len == 3 && !strncasecmp(opts.val, "404", 3) )
					noData404 = true;
				else if ( opts.val_len != 3 || strncasecmp(opts.val, "204", 3) ) {
					sendError(req.path, options, Wired::HTTP_400, "Invalid nodata value");
					return true;
				}
			}
		}

		// Since we have no inventory access, we have to insist on network,
		// station and channel codes. The location code is set to "" by default.
		if ( networks.empty() ) {
			sendError(req.path, options, Wired::HTTP_400, "No network specified");
			return true;
		}
		if ( stations.empty() ) {
			stations.insert("*");
		}
		if ( locations.empty() ) {
			locations.insert("");
		}
		if ( channels.empty() ) {
			sendError(req.path, options, Wired::HTTP_400, "No channels specified");
			return true;
		}

		if ( !startTime.valid() ) {
			sendError(req.path, options, Wired::HTTP_400, "Start time not specified");
			return true;
		}
		else if ( startTime > endTime ) {
			sendError(req.path, options, Wired::HTTP_400, "Start time larger than endtime");
			return true;
		}

		if ( global.fdsnws.maxTimeWindow > 0 ) {
			double timeSpan = static_cast<double>(endTime - startTime);
			size_t numCha = networks.size() * stations.size() *
			                locations.size() * channels.size();
			if ( timeSpan * numCha > global.fdsnws.maxTimeWindow ) {
				sendError(req.path, options, Wired::HTTP_400,
				          string("Maximum allowed request time window of " +
				                 Core::toString(global.fdsnws.maxTimeWindow) +
				                 "s exceeded").c_str());
				return true;
			}
		}

		ArchiveBufferPtr buf = createStreamBuffer();
		if ( buf.get() == NULL ) {
			sendError(req.path, options, Wired::HTTP_500, "Data backend not available");
			return true;
		}

		for ( set<string>::const_iterator net = networks.begin();
		      net != networks.end(); ++net ) {
			for ( set<string>::const_iterator sta = stations.begin();
			      sta != stations.end(); ++sta ) {
				for ( set<string>::const_iterator loc = locations.begin();
				      loc != locations.end(); ++loc ) {
					for ( set<string>::const_iterator cha = channels.begin();
					      cha != channels.end(); ++cha ) {
						SEISCOMP_DEBUG("add stream: %s %s %s %s %s %s",
						               startTime.toString("%Y,%m,%d,%H,%M,%S").c_str(),
						               endTime.toString("%Y,%m,%d,%H,%M,%S").c_str(),
						               net->c_str(), sta->c_str(),
						               loc->c_str(), cha->c_str());
						buf->stream->addStream(net->c_str(), sta->c_str(),
						                       loc->c_str(), cha->c_str(),
						                       startTime, endTime);
					}
				}
			}
		}

		if ( !buf->hasData() ) {
			if ( noData404 )
				sendError(req.path, options, Wired::HTTP_404);
			else
				sendResponse(Wired::HTTP_204);
		}
		else
			sendResponse(buf.get(), Wired::HTTP_200, "application/vnd.fdsn.mseed");
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FDSNWSSession::handlePOSTRequest(Wired::HttpRequest &req) {
	Wired::URLPath path(req.path);

	req.state = Wired::HttpRequest::FINISHED;

	if ( !path.next() ) {
		sendError(req.path, req.options, Wired::HTTP_404);
		return true;
	}

	if ( path != "fdsnws" ) {
		sendError(req.path, req.options, Wired::HTTP_404);
		return true;
	}

	if ( !path.next() ) {
		sendError(req.path, req.options, Wired::HTTP_404);
		return true;
	}

	if ( path != "dataselect" ) {
		sendError(req.path, req.options, Wired::HTTP_404);
		return true;
	}

	if ( !path.next() ) {
		sendError(req.path, req.options, Wired::HTTP_404);
		return true;
	}

	if ( path != "1" ) {
		sendError(req.path, req.options, Wired::HTTP_404);
		return true;
	}

	if ( !path.next() ) {
		sendError(req.path, req.options, Wired::HTTP_404);
		return true;
	}

	if ( path != "query" ) {
		sendError(req.path, req.options, Wired::HTTP_404);
		return true;
	}

	char *tok, *src = &req.data[0];
	size_t len_tok, len_src = req.data.size();

	double timeWindow = 0;
	bool noData404 = false;
	vector<RequestItem> requestItems;
	while ( (tok = tokenize(src, "\n", len_src, len_tok)) ) {
		trim(tok, len_tok);
		char *sep = strnchr(tok, len_tok, '=');
		// Currently we ignore parameters
		if ( sep ) {
			*sep = '\0';
			char* val = sep + 1;
			size_t val_len = len_tok - static_cast<size_t>(val - tok);
			if ( !strcmp(tok, "minimumlength") ) {
				//
			}
			else if ( !strcmp(tok, "longestonly") ) {
				//
			}
			else if ( !strcmp(tok, "nodata") ) {
				if ( val_len == 3 && !strncasecmp(val, "404", 3) )
					noData404 = true;
				else if ( val_len != 3 || strncasecmp(val, "204", 3) ) {
					sendError(req.path, req.options, Wired::HTTP_400,
					          "Invalid nodata value");
					return true;
				}
			}

			continue;
		}

		char *col;
		size_t len_col = len_tok;
		size_t num_col = 0;

		RequestItem item;
		while ( (col = tokenize(tok, " ", len_tok, len_col)) ) {
			switch ( num_col ) {
				case 0:
					item.net.assign(col, len_col);
					break;
				case 1:
					item.sta.assign(col, len_col);
					break;
				case 2:
					if ( strncmp(col, "--", len_col) != 0 )
						item.loc.assign(col, len_col);
					break;
				case 3:
					item.cha.assign(col, len_col);
					break;
				case 4:
					col[len_col] = '\0';
					if ( !parseFDSNWSTime(item.startTime, col) ) {
						sendError(req.path, req.options, Wired::HTTP_400,
						          "Invalid start time");
						return true;
					}
					break;
				case 5:
					col[len_col] = '\0';
					if ( !parseFDSNWSTime(item.endTime, col) ) {
						sendError(req.path, req.options, Wired::HTTP_400,
						          "Invalid end time");
						return true;
					}
					break;
				default:
					sendError(req.path, req.options, Wired::HTTP_400,
					          "Invalid request line");
					return true;
			}

			++num_col;
		}

		if ( item.startTime > item.endTime ) {
			sendError(req.path, req.options, Wired::HTTP_400,
			          "Start time larger than endtime");
			return true;
		}

		if ( global.fdsnws.maxTimeWindow > 0 ) {
			timeWindow += static_cast<double>(item.endTime - item.startTime);
			if ( timeWindow > global.fdsnws.maxTimeWindow ) {
				sendError(req.path, req.options, Wired::HTTP_400,
				          string("Maximum allowed request time window of " +
				                 Core::toString(global.fdsnws.maxTimeWindow) +
				                 "s exceeded").c_str());
				return true;
			}
		}
		requestItems.push_back(item);
	}


	ArchiveBufferPtr buf = createStreamBuffer();
	if ( buf.get() == NULL ) {
		sendError(req.path, req.options, Wired::HTTP_500,
		          "Data backend not available");
		return true;
	}

	for ( vector<RequestItem>::const_iterator item = requestItems.begin();
	      item != requestItems.end(); ++item ) {
		SEISCOMP_DEBUG("add stream: %s %s %s %s %s %s",
		               item->startTime.toString("%Y,%m,%d,%H,%M,%S").c_str(),
		               item->endTime.toString("%Y,%m,%d,%H,%M,%S").c_str(),
		               item->net.c_str(), item->sta.c_str(),
		               item->loc.c_str(), item->cha.c_str());
		buf->stream->addStream(item->net, item->sta, item->loc, item->cha,
		                       item->startTime, item->endTime);
	}

	if ( !buf->hasData() ) {
		if ( noData404 ) {
			sendError(req.path, req.options, Wired::HTTP_404);
		}
		else {
			sendResponse(Wired::HTTP_204);
		}
	}
	else {
		sendResponse(buf.get(), Wired::HTTP_200, "application/vnd.fdsn.mseed");
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}
