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
#include "strings.h"
#include "version.h"

#define SEISCOMP_COMPONENT WFAS
#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/io/recordstream/sdsarchive.h>
#include <seiscomp/io/records/mseedrecord.h>

#include <iostream>
#include <ctype.h>
#include <cerrno>


using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace std;


namespace Seiscomp {
namespace Applications {
namespace Wfas {


class ArclinkSession : public ClientSession {
	public:
		ArclinkSession(Requests *requests, Wired::Socket *sock);
		~ArclinkSession();

		virtual void update();
		virtual void flush();
		virtual size_t inAvail() const;


	protected:
		virtual void handleInbox(const char *data, int len);


	private:
		bool initRecordStream();

		void sendResponse(const char *data);
		void sendResponse(const char *data, int len);
		void sendError(const char *data);
		void clearError();

		void collectData();
		bool purgeRequest(size_t id);

		bool resolveWildcards(Request &, const RequestItem &) const;


	private:
		Requests                     *_requests;
		RequestPtr                    _newRequest;
		std::string                   _errorMessage;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkListener::ArclinkListener(const Wired::IPACL &allowedIPs,
                                 const Wired::IPACL &deniedIPs,
                                 Wired::Socket *socket)
: Wired::AccessControlledEndpoint(socket, allowedIPs, deniedIPs) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Wired::Session *ArclinkListener::createSession(Wired::Socket *socket) {
	socket->setMode(Wired::Socket::Read);
	return new ArclinkSession(&_requests, socket);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkSession::ArclinkSession(Requests *requests, Wired::Socket *sock)
: ClientSession(sock, 200)
, _requests(requests) {
	_state = Unspecific;
	_bytesPending = _currentHeaderOffset = _currentDataOffset = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArclinkSession::~ArclinkSession() {
	if ( _currentRequest ) purgeRequest(_currentRequest->id);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkSession::resolveWildcards(Request &req, const RequestItem &item) const {
	/*
	Client::Inventory *gInv = Client::Inventory::Instance();
	DataModel::Inventory *inv;
	if ( gInv )
		inv = gInv->inventory();
	else
		inv = NULL;

	// No inventory -> skip wildcard request
	if ( !inv ) {
		if ( !validate(item.net) || !validate(item.sta) ||
			 !validate(item.loc) || !validate(item.cha) )
			return false;
	}

	// Network wildcard
	if ( !validate(item.net) ) {
		for ( size_t i = 0; i < inv->networkCount(); ++i ) {
			DataModel::Network *net = inv->network(i);
			if ( !Core::wildcmp(net->code(), item.net) ) continue;
			resolve(req, item, inv, net, net->code());
		}
	}
	else
		resolve(req, item, inv, NULL, item.net);

	return true;
	*/

	/*
	if ( !validate(item.net) || !validate(item.sta) || !validate(item.loc) ||
	     !validate(item.cha) )
		return false;
	*/

	req.items.push_back(item);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkSession::update() {
	ClientSession::update();

	if ( _recordInput && _bytesPending == 0 )
		collectData();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkSession::handleInbox(const char *src_data, int data_len) {
	if ( data_len == 0 )
		return;

	SEISCOMP_DEBUG("$ %s", src_data);

	int len;
	const char *data;
	if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
		sendError("empty line");
		return;
	}

	switch ( _state ) {
		case Unspecific:
			if ( len == 5 && strncasecmp(data, "hello", len) == 0 ) {
				sendResponse("scwfas v" SCWFAS_VERSION_NAME "\r\n");
				sendResponse(global.dcid.c_str(), global.dcid.size());
				sendResponse("\r\n");
			}
			else if ( len == 3 && strncasecmp(data, "bye", len) == 0 )
				close();
			else if ( len == 4 && strncasecmp(data, "user", len) == 0 ) {
				clearError();
				sendResponse("OK\r\n");
			}
			else if ( len == 7 && strncasecmp(data, "request", len) == 0 ) {
				if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
					sendError("REQUEST without type");
					return;
				}

				if ( len != 8 || strncasecmp(data, "waveform", len) != 0 ) {
					sendError("unsupported REQUEST type");
					return;
				}

				if ( (data = tokenize(src_data, " ", data_len, len)) != NULL ) {
					// Parse for "format=MSEED"
					int tlen; const char *tdata;
					if ( (tdata = tokenize(data, "=", len, tlen)) == NULL ) {
						sendError("invalid argument to REQUEST WAVEFORM");
						return;
					}

					if ( tlen == 6 && strncasecmp(tdata, "format", tlen) == 0 ) {
						tdata = tokenize(data, "=", len, tlen);
						if ( tdata == NULL || strncasecmp(tdata, "mseed", 5) != 0 ) {
							sendError("unsupported waveform format");
							return;
						}
					}
					else {
						sendError("invalid argument to REQUEST WAVEFORM");
						return;
					}
				}

				sendResponse("OK\r\n");

				// Remove current request if a new one should be started
				if ( _currentRequest ) {
					purgeRequest(_currentRequest->id);
					_currentRequest = NULL;
				}

				_state = StreamRequest;
				_newRequest = new Request;
			}
			else if ( (len == 8 && strncasecmp(data, "download", len) == 0) ||
			          (len == 9 && strncasecmp(data, "bdownload", len) == 0) ) {
				sendError("unsupported command, use BCDOWNLOAD");
			}
			else if ( len == 10 && strncasecmp(data, "bcdownload", len) == 0 ) {
				if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
					sendError("BCDOWNLOAD expects parameters");
					return;
				}

				size_t id;
				if ( !fromString(id, std::string(data, len)) ) {
					sendError("invalid request id");
					return;
				}

				if ( (data = tokenize(src_data, " ", data_len, len)) != NULL ) {
					sendError("BCDOWNLOAD does not support resumed downloads");
					return;
				}

				Requests::iterator it = _requests->find(id);
				if ( it == _requests->end() )
					sendResponse("0\r\nEND\r\n");
				else {
					if ( _currentRequest ) purgeRequest(_currentRequest->id);
					_currentRequest = it->second;
					if ( !initRecordStream() ) {
						sendError("internal server error");
						return;
					}
				}
			}
			else if ( (len == 5 && strncasecmp(data, "purge", len) == 0) ) {
				if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
					sendError("PURGE expects one parameter");
					return;
				}

				string tmp;
				tmp.assign(data, len);
				size_t id;
				if ( !fromString(id, tmp) ) {
					sendError("PURGE: invalid ID format");
					return;
				}

				if ( !purgeRequest(id) )
					sendError("PURGE: invalid ID");
				else {
					clearError();
					sendResponse("OK\r\n");
				}
			}
			// Debugging stuff
			else if ( len == 7 && strncasecmp(data, "showerr", len) == 0 ) {
				sendResponse(_errorMessage.c_str());
				sendResponse("\r\n", 2);
			}
			else
				sendError("unknown command");
			break;

		case StreamRequest:
			if ( len == 3 && strncasecmp(data, "end", len) == 0 ) {
				_state = Unspecific;

				// TODO: register new request
				if ( _requests->empty() )
					_newRequest->id = 1;
				else {
					Requests::reverse_iterator it = _requests->rbegin();
					_newRequest->id = it->first+1;
				}

				if ( _newRequest->items.empty() )
					SEISCOMP_WARNING("Enqueuing empty request");

				(*_requests)[_newRequest->id] = _newRequest;
				string id = toString(_newRequest->id);

				_newRequest = NULL;

				clearError();
				sendResponse(id.c_str(), id.size());
				sendResponse("\r\n", 2);
			}
			else {
				RequestItem item;

				// start time
				if ( !parseTime(item.startTime, data, len) ) {
					SEISCOMP_WARNING("invalid request start time");
					//sendError("invalid request start time");
					return;
				}

				if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
					SEISCOMP_WARNING("wrong request format");
					//sendError("wrong request format");
					return;
				}

				// end time
				if ( !parseTime(item.endTime, data, len) ) {
					SEISCOMP_WARNING("invalid request end time");
					//sendError("invalid request end time");
					return;
				}

				if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
					SEISCOMP_WARNING("wrong request format");
					//sendError("wrong request format");
					return;
				}

				item.net.assign(data, len);

				if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
					SEISCOMP_WARNING("wrong request format");
					//sendError("wrong request format");
					return;
				}

				item.sta.assign(data, len);

				if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
					SEISCOMP_WARNING("wrong request format");
					//sendError("wrong request format");
					return;
				}

				item.cha.assign(data, len);

				if ( (data = tokenize(src_data, " ", data_len, len)) != NULL ) {
					item.loc.assign(data, len);
					if ( item.loc == "." ) item.loc = "";
				}

				if ( !resolveWildcards(*_newRequest, item) ) {
					SEISCOMP_WARNING("invalid wildcard request: %s.%s.%s.%s",
					                 item.net.c_str(), item.sta.c_str(),
					                 item.loc.c_str(), item.cha.c_str());
					return;
				}
			}
			break;

		default:
			sendError("internal server error");
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkSession::flush() {
	// Try to empty the outbox
	if ( _currentChunk.data.empty() ) {
		_bytesPending = 0;
		Wired::ClientSession::flush();
		return;
	}

	while ( !_currentChunk.data.empty() ) {
		int remaining = _currentChunk.header.size()-_currentHeaderOffset;
		int written;

		if ( remaining > 0 ) {
			written = _device->write(&_currentChunk.header[_currentHeaderOffset],
			                         remaining);
			// Error on socket?
			if ( written < 0 ) {
				if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
					// Close the session
					_currentChunk = Chunk();
					close();
					break;
				}
			}
			// No non-blocking writing possible?
			else if ( written == 0 ) {
			}
			else {
				_currentHeaderOffset += written;
				_bytesPending -= written;
			}
		}

		// header not yet completely sent
		if ( _currentHeaderOffset < _currentChunk.header.size() ) return;


		remaining = _currentChunk.data.size()-_currentDataOffset;
		if ( remaining > 0 ) {
			written = _device->write(&_currentChunk.data[_currentDataOffset],
			                         remaining);

			// Error on socket?
			if ( written < 0 ) {
				if ( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
					_currentChunk = Chunk();
					// Close the session
					close();
					break;
				}
			}
			// No non-blocking writing possible?
			else if ( written == 0 ) {
			}
			else {
				_currentDataOffset += written;
				_bytesPending -= written;
			}
		}

		// Finished current?
		if ( _currentDataOffset == _currentChunk.data.size() ) {
			_currentChunk = Chunk();
			_currentHeaderOffset = 0;
			_currentDataOffset = 0;

			// Finish for now and give the control back to the
			// reactor which needs to run in LevelTriggered mode
		}
		else
			// No all data has been written
			break;
	}

	//cout << "ClientSession::flush(): inAvail = " << inAvail() << endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t ArclinkSession::inAvail() const {
	if ( _recordInput ) return 1;
	return ClientSession::inAvail();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkSession::initRecordStream() {
	_recordStream = NULL;
	_recordInput = NULL;

	if ( _currentRequest == NULL )
		return false;

	if ( global.sdsBackend.empty() )
		_recordStream = new RecordStream::SDSArchive();
	else
		_recordStream = IO::RecordStream::Create(global.sdsBackend.c_str());

	if ( _recordStream == NULL ) {
		if ( global.sdsBackend.empty() )
			SEISCOMP_ERROR("Failed to allocate SDS handler");
		else
			SEISCOMP_ERROR("Failed to create SDS handler '%s'", global.sdsBackend.c_str());
		return false;
	}

	_recordStream->setSource(global.filebase);

	if ( _currentRequest->items.empty() )
		SEISCOMP_DEBUG("empty stream list");

	for ( RequestItems::iterator it = _currentRequest->items.begin();
	      it != _currentRequest->items.end(); ++it ) {
		SEISCOMP_DEBUG("add stream: %s %s %s %s %s %s",
		               it->startTime.toString("%Y,%m,%d,%H,%M,%S").c_str(),
		               it->endTime.toString("%Y,%m,%d,%H,%M,%S").c_str(),
		               it->net.c_str(), it->sta.c_str(), it->cha.c_str(), it->loc.c_str());
		_recordStream->addStream(it->net, it->sta, it->loc, it->cha,
		                         it->startTime, it->endTime);
	}

	_recordInput = new Seiscomp::IO::RecordInput(
	      _recordStream.get(), Seiscomp::Array::INT, Seiscomp::Record::SAVE_RAW
	);

	return _recordInput != NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkSession::collectData() {
	if ( !_recordInput ) return;

	_currentChunk.header.clear();
	_currentChunk.data.clear();
	_currentHeaderOffset = _currentDataOffset = 0;

	try {
		Seiscomp::RecordPtr rec;
		while ( _currentChunk.data.size() < MAX_CHUNK_SIZE ) {
			rec = _recordInput->next();
			if ( rec == NULL ) {
				// End of stream: close stream and input
				_recordInput = NULL;
				_recordStream = NULL;
				break;
			}

			Seiscomp::IO::MSeedRecord *mseed =
				Seiscomp::IO::MSeedRecord::Cast(rec.get());

			// Skip non mseed records
			if ( mseed == NULL ) continue;

			const Array *data = mseed->raw();
			if ( data == NULL ) continue;

			_currentChunk.data.append((const char*)data->data(), data->size()*data->elementSize());
		}

		if ( _currentChunk.data.empty() )
			_currentChunk.header = "0\r\n";
		else {
			_currentChunk.header = "CHUNK ";
			_currentChunk.header += toString(_currentChunk.data.size());
			_currentChunk.header += "\r\n";
		}

		if ( _recordInput == NULL ) {
			_currentChunk.data += "END\r\n";
		}

		_bytesPending = _currentChunk.header.size() + _currentChunk.data.size();

		flush();
		return;
	}
	catch ( ... ) {}

	// Close connection
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArclinkSession::purgeRequest(size_t id) {
	Requests::iterator it = _requests->find(id);
	if ( it == _requests->end() ) return false;
	_requests->erase(it);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkSession::sendResponse(const char *data) {
	sendResponse(data, strlen(data));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkSession::sendResponse(const char *data, int len) {
	send(data, len);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkSession::sendError(const char *data) {
	_errorMessage = data;
	send("ERROR\r\n");
	//setError("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArclinkSession::clearError() {
	_errorMessage = "success";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}
