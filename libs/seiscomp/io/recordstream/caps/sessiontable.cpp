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


#define SEISCOMP_COMPOMENT CAPS
#include <seiscomp/logging/log.h>

#include "sessiontable.h"
#include "utils.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>


using namespace std;


namespace Seiscomp {
namespace IO {
namespace CAPS {


namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void setDataType(const char *data, int len, SessionTableItem &item) {
	if ( CHECK_STRING(data, "FLOAT", len) ) {
		item.dataType = DT_FLOAT;
		item.dataSize = 4;
		//SEISCOMP_DEBUG("%s: raw format with 32bit floats", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "DOUBLE", len) ) {
		item.dataType = DT_DOUBLE;
		item.dataSize = 8;
		//SEISCOMP_DEBUG("%s: raw format with 64bit doubles", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "INT64", len) ) {
		item.dataType = DT_INT64;
		item.dataSize = 8;
		//SEISCOMP_DEBUG("%s: raw format with 64bit integers", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "INT32", len) ) {
		item.dataType = DT_INT32;
		item.dataSize = 4;
		//SEISCOMP_DEBUG("%s: raw format with 32bit integers", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "INT16", len) ) {
		item.dataType = DT_INT16;
		item.dataSize = 2;
		//SEISCOMP_DEBUG("%s: raw format with 16bit integers", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "INT8", len) ) {
		item.dataType = DT_INT8;
		item.dataSize = 1;
		//SEISCOMP_DEBUG("%s: raw format with 8bit integers", item.streamID.c_str());
	}
	else {
		item.dataType = DT_Unknown;
		SEISCOMP_WARNING("Unknown raw data type '%s', all packages will be ignored",
		                 string(data, len).c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}


bool SessionTableItem::splitStreamID() {
	const char *tok;
	int tok_len;
	int tok_count = 0;

	const char* str = streamID.c_str();
	int len = strlen(str);

	string *items[] = {&net, &sta, &loc, &cha};

	for ( ; (tok = tokenize(str, ".", len, tok_len)) != NULL;
	      ++tok_count ) {
		if ( tok_count > 4 ) return false;

		items[tok_count]->assign(tok, tok_len);
	}

	if ( tok_count == 3 ) {
		cha = loc;
		loc = "";
		tok_count = 4;
	}

	return tok_count == 4;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SessionTable::SessionTable() : _state(Unspecific) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SessionTable::Status SessionTable::handleResponse(const char *src_data, int data_len) {
	enum StreamHeaderToken {
		REQ_ID = 0,
		SID    = 1,
		SF     = 2,
		UOM    = 3,
		FORMAT = 4,
		COUNT  = 5
	};

	int src_len = data_len;
	const char *src = src_data;

	int len;
	const char *data;
	if ( (data = tokenize(src_data, " ", data_len, len)) == NULL ) {
		SEISCOMP_WARNING("server returned empty line, ignoring");
		return Success;
	}

	switch ( _state ) {
		case Unspecific:
			if ( CHECK_STRING(data, "STATUS", len) ) {
				if ( (data = tokenize(src_data, " ", data_len, len)) != NULL ) {
					if ( CHECK_STRING(data, "OK", len) ) {
						SEISCOMP_DEBUG("server responds OK");
					}
					else {
						SEISCOMP_ERROR("received unknown status from server: %s", data);
						return Error;
					}
				}
				else {
					SEISCOMP_ERROR("received empty status from server");
					return Error;
				}
			}
			else if ( CHECK_STRING(data, "REQUESTS", len) ) {
				if ( (data = tokenize(src_data, " ", data_len, len)) != NULL ) {
					SEISCOMP_WARNING("received unknown REQUESTS parameter: %s", data);
				}
				SEISCOMP_DEBUG("stream table update started");
				_state = Requests;
			}
			else if ( CHECK_STRING(data, "EOD", len) ) {
				SEISCOMP_DEBUG("server sent EOD");
				return EOD;
			}
			else {
				SEISCOMP_ERROR("received unknown response: %s", data);
				return Error;
			}
			break;
		case Requests:
			if ( CHECK_STRING(data, "END", len) ) {
				if ( (data = tokenize(src_data, " ", data_len, len)) != NULL ) {
					SEISCOMP_WARNING("received unknown END parameter: %s", data);
				}
				SEISCOMP_DEBUG("stream table update complete");
				_state = Unspecific;
				break;
			}

			SEISCOMP_DEBUG("request response data: %s", string(data, len).c_str());

			typedef std::pair<const char *, int> Buffer;
			const char *state = src;
			Buffer toks[COUNT];
			for ( int i = 0; i < COUNT; ++i )
				toks[i].second = 0;

			// Update request map
			while ( (data = tokenize(src, ",", src_len, len)) != NULL ) {
				trim(data, len);
				if ( !strncasecmp(data, "ID:", 3) )
					toks[REQ_ID] = Buffer(data+3, len-3);
				else if ( !strncasecmp(data, "SID:", 4) )
					toks[SID] = Buffer(data+4, len-4);
				else if ( !strncasecmp(data, "SFREQ:", 6) )
					toks[SF] = Buffer(data+6, len-6);
				else if ( !strncasecmp(data, "UOM:", 4) )
					toks[UOM] = Buffer(data+4, len-4);
				else if ( !strncasecmp(data, "FMT:", 4) )
					toks[FORMAT] = Buffer(data+4, len-4);
			}

			SessionTableItem item;
			char buffer[7];
			if ( toks[REQ_ID].second > 6 ) {
				SEISCOMP_ERROR("request state ID too high: %s", state);
				return Error;
			}
			strncpy(buffer, toks[REQ_ID].first, toks[REQ_ID].second);
			buffer[toks[REQ_ID].second] = '\0';
			int req_id;

			if ( (sscanf(buffer, "%d", &req_id) != 1) || req_id == 0 ) {
				SEISCOMP_ERROR("invalid request ID: %s", buffer);
				return Error;
			}

			if ( toks[SID].second == 0 ) {
				SEISCOMP_ERROR("missing SID in request response");
				return Error;
			}

			item.streamID.assign(toks[SID].first, toks[SID].second);

			if ( req_id < 0 ) {
				removeStream(item.streamID);
				SEISCOMP_DEBUG("stream %s has finished", item.streamID.c_str());
				return Success;
			}
			else {
				SEISCOMP_DEBUG("new request ID %d for stream %s received", req_id, item.streamID.c_str());
				std::string tmp;

				if ( (data = tokenize(toks[SF].first, "/", toks[SF].second, len)) != NULL ) {
					trim(data, len);
					tmp.assign(data, len);
					if ( !str2int(item.samplingFrequency, tmp.c_str()) ) {
						SEISCOMP_ERROR("request state 'samplingFrequency' is not a number: %s", state);
						item.samplingFrequency = 0;
					}
					else {
						item.samplingFrequencyDivider = 1;
						if ( (data = tokenize(toks[SF].first, "/", toks[SF].second, len)) != NULL ) {
							trim(data, len);
							tmp.assign(data, len);
							if ( !str2int(item.samplingFrequencyDivider, tmp.c_str()) ) {
								SEISCOMP_ERROR("request state 'samplingFrequencyDivider' "
								               "is not a number: %s", state);
							}
						}
					}
				}
				else {
					item.samplingFrequency = 0;
				}

				if ( toks[UOM].second > 4 ) {
					SEISCOMP_ERROR("request state 'unit of measurement' is invalid, "
					               "too many characters: %s", state);
					item.uom.ID = 0;
				}
				else {
					memcpy(item.uom.str, toks[UOM].first, toks[UOM].second);
				}

				if ( (data = tokenize(toks[FORMAT].first, "/", toks[FORMAT].second, len)) != NULL ) {
					trim(data, len);
					if ( CHECK_STRING(data, "RAW", len) ) {
						item.type = RawDataPacket;
						if ( (data = tokenize(toks[FORMAT].first, "/", toks[FORMAT].second, len)) != NULL ) {
							setDataType(data, len, item);
						}

						SEISCOMP_DEBUG("%s: samplingFrequency=%d/%d",
						               item.streamID.c_str(), item.samplingFrequency,
						               item.samplingFrequencyDivider);
					}
					else if ( CHECK_STRING(data, "FIXEDRAW", len) ) {
						item.type = FixedRawDataPacket;
						if ( (data = tokenize(toks[FORMAT].first, "/", toks[FORMAT].second, len)) != NULL ) {
							setDataType(data, len, item);
						}

						SEISCOMP_DEBUG("%s: samplingFrequency=%d/%d",
						               item.streamID.c_str(), item.samplingFrequency,
						               item.samplingFrequencyDivider);
					}
					else if ( CHECK_STRING(data, "MSEED", len) ) {
						item.type = MSEEDPacket;
					}
					else if ( CHECK_STRING(data, "META", len ) ) {
						item.type = MetaDataPacket;
					}
					else {
						string tmp(data, len);
						item.type = ANYPacket;
					}
				}

				// Register item
				registerItem(req_id, item);
			}

			break;
	}

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void setDataType(const char *data, int len, SessionTableItem &item) {
	if ( CHECK_STRING(data, "FLOAT", len) ) {
		item.dataType = DT_FLOAT;
		//SEISCOMP_DEBUG("%s: raw format with 32bit floats", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "DOUBLE", len) ) {
		item.dataType = DT_DOUBLE;
		//SEISCOMP_DEBUG("%s: raw format with 64bit doubles", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "INT64", len) ) {
		item.dataType = DT_INT64;
		//SEISCOMP_DEBUG("%s: raw format with 64bit integers", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "INT32", len) ) {
		item.dataType = DT_INT32;
		//SEISCOMP_DEBUG("%s: raw format with 32bit integers", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "INT16", len) ) {
		item.dataType = DT_INT16;
		//SEISCOMP_DEBUG("%s: raw format with 16bit integers", item.streamID.c_str());
	}
	else if ( CHECK_STRING(data, "INT8", len) ) {
		item.dataType = DT_INT8;
		//SEISCOMP_DEBUG("%s: raw format with 8bit integers", item.streamID.c_str());
	}
	else {
		string tmp;
		item.dataType = DT_Unknown;
		tmp.assign(data, len);
		SEISCOMP_WARNING("Unknown raw data type '%s', all packages will be ignored",
		                 tmp.c_str());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SessionTable::registerItem(int id, SessionTableItem &item) {
	item.fSamplingFrequency = (double)item.samplingFrequency /
	                          (double)item.samplingFrequencyDivider;

	SessionTable::iterator it = find(id);
	if ( it == end() ) {
		if ( !item.splitStreamID() ) {
			SEISCOMP_WARNING("invalid streamID received: %s", item.streamID.c_str());
			return;
		}

		// Copy the item
		SessionTableItem &target = operator[](id);
		target = item;

		_streamIDLookup[item.streamID] = id;

		if ( _itemAddedFunc ) _itemAddedFunc(&target);
	}
	else {
		bool res = item.splitStreamID();

		// streamID changed for the same sessionID?
		if ( it->second.streamID != item.streamID ) {
			SEISCOMP_WARNING("inconsistent state: streamID '%s' for id %d, "
			                 "but streamID '%s' has not been closed before",
			                 item.streamID.c_str(), id, it->second.streamID.c_str());
			// Update lookup table
			_streamIDLookup.erase(_streamIDLookup.find(it->second.streamID));

			if ( !res ) {
				SEISCOMP_WARNING("invalid streamID received: %s", item.streamID.c_str());
				erase(it);
				return;
			}

			_streamIDLookup[item.streamID] = id;

			// TODO: How to update the request list?
		}

		it->second = item;

		if ( _itemAddedFunc ) _itemAddedFunc(&it->second);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SessionTable::removeStream(const std::string &streamID) {
	StreamIDLookupTable::iterator it = _streamIDLookup.find(streamID);
	if ( it == _streamIDLookup.end() ) {
		SEISCOMP_WARNING("internal: tried to remove unknown stream '%s'",
		                 streamID.c_str());
		return;
	}

	SessionTable::iterator sessionIt = find(it->second);
	if ( sessionIt != end() ) {
		// Remove session table row
		if ( _itemAboutToBeRemovedFunc )
			_itemAboutToBeRemovedFunc(&sessionIt->second);
		erase(sessionIt);
	}

	// Remove lookup entry
	_streamIDLookup.erase(it);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SessionTable::reset() {
	clear();
	_streamIDLookup.clear();
	_state = Unspecific;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
