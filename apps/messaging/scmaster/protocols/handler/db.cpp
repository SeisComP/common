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


#define SEISCOMP_COMPONENT BROKER

#include <seiscomp/logging/log.h>
#include <seiscomp/core/endianess.h>

#include "db.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Wired;


namespace {


enum Command {
	CMD_START          = 1,
	CMD_COMMIT         = 2,
	CMD_ROLLBACK       = 3,
	CMD_EXECUTE        = 4,
	CMD_QUERY          = 5,
	CMD_QUERY_END      = 6,
	CMD_LAST_ID        = 7,
	CMD_AFFECTED_ROWS  = 8,
	CMD_FETCH          = 9
};


}


namespace Seiscomp {
namespace Messaging {
namespace Protocols {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DBHandler::DBHandler(WebsocketSession *session, Seiscomp::IO::DatabaseInterfacePtr db)
: WebsocketHandler(session), _db(db) {
	_session->setTag(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DBHandler::~DBHandler() {
	if ( _db ) {
		_db->disconnect();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DBHandler::addUpgradeHeader() {
	_session->send("X-DB-Backend: ");
	_session->send(_db->backend().toString());
	_session->send("\r\n");
	_session->send("X-DB-Prefix: ");
	_session->send(_db->columnPrefix().c_str(), _db->columnPrefix().size());
	_session->send("\r\n");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DBHandler::start() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DBHandler::handleFrame(Websocket::Frame &frame) {
	if ( frame.type != Websocket::Frame::BinaryFrame ) {
		sendClose();
		return;
	}

	if ( frame.data.empty() ) {
		sendClose();
		return;
	}

	Command cmd = Command(*reinterpret_cast<uint8_t*>(frame.data.data()));
	switch ( cmd ) {
		case CMD_START:
			// SEISCOMP_DEBUG("[cmd] start");
			_db->start();
			sendResult(CMD_START, 0, "OK");
			break;
		case CMD_COMMIT:
			// SEISCOMP_DEBUG("[cmd] commit");
			_db->commit();
			sendResult(CMD_COMMIT, 0, "OK");
			break;
		case CMD_ROLLBACK:
			// SEISCOMP_DEBUG("[cmd] rollback");
			_db->rollback();
			sendResult(CMD_ROLLBACK, 0, "OK");
			break;
		case CMD_EXECUTE:
			// SEISCOMP_DEBUG("[cmd] execute");
			if ( frame.data.size() < 2 ) {
				sendClose();
			}
			else {
				if ( _db->execute(&frame.data[1]) ) {
					sendResult(CMD_EXECUTE, 0, "OK");
				}
				else {
					sendResult(CMD_EXECUTE, 1, "Error");
				}
			}
			break;
		case CMD_QUERY:
			// SEISCOMP_DEBUG("[cmd] query");
			if ( frame.data.size() < 2 ) {
				sendClose();
			}
			else {
				if ( _db->beginQuery(&frame.data[1]) ) {
					int fieldCount = _db->getRowFieldCount();

					BufferPtr resp = new Buffer;
					resp->data.resize(1+1+4);
					*reinterpret_cast<uint8_t*>(&resp->data[0]) = CMD_QUERY;
					*reinterpret_cast<uint8_t*>(&resp->data[1]) = 0;
					*reinterpret_cast<int*>(&resp->data[2]) = fieldCount;
					// Convert field count to little endian
					Endianess::ByteSwapper<Endianess::Current::LittleEndian,4>::Take(&resp->data[2], 1);

					// SEISCOMP_DEBUG("[query] %d fields", fieldCount);
					for ( int i = 0; i < fieldCount; ++i ) {
						auto name = _db->getRowFieldName(i);
						int l = strlen(name);
						// SEISCOMP_DEBUG("[query] field.%i (%d): %s", i, l, name);
						auto offset = resp->data.size();
						resp->data.resize(offset + 4 + l);
						*reinterpret_cast<int32_t*>(&resp->data[offset]) = l;
						Endianess::ByteSwapper<Endianess::Current::LittleEndian,4>::Take(&resp->data[offset], 1);
						memcpy(&resp->data[offset + 4], name, l);

					}

					Websocket::Frame::finalizeBuffer(resp.get(), Websocket::Frame::BinaryFrame);
					_session->send(resp.get());
				}
				else {
					sendResult(CMD_QUERY, 1, "Error");
				}
			}
			break;
		case CMD_QUERY_END:
			// SEISCOMP_DEBUG("[cmd] query end");
			_db->endQuery();
			sendResult(CMD_QUERY_END, 0, "OK");
			break;
		case CMD_FETCH:
			if ( !_db->fetchRow() ) {
				sendResult(CMD_FETCH, 1, "Error");
			}
			else {
				int fieldCount = _db->getRowFieldCount();

				BufferPtr resp = new Buffer;
				resp->data.resize(1+1+4);
				*reinterpret_cast<uint8_t*>(&resp->data[0]) = CMD_FETCH;
				*reinterpret_cast<uint8_t*>(&resp->data[1]) = 0;
				*reinterpret_cast<int*>(&resp->data[2]) = fieldCount;
				// Convert field count to little endian
				Endianess::ByteSwapper<Endianess::Current::LittleEndian,4>::Take(&resp->data[2], 1);

				for ( int i = 0; i < fieldCount; ++i ) {
					auto content = _db->getRowField(i);
					auto l = _db->getRowFieldSize(i);
					auto offset = resp->data.size();

					if ( !content ) {
						// null pointer = DB NULL
						l = -1;
						resp->data.resize(offset + 4);
					}
					else {
						resp->data.resize(offset + 4 + l);
						memcpy(&resp->data[offset + 4], content, l);
					}

					*reinterpret_cast<int32_t*>(&resp->data[offset]) = l;
					Endianess::ByteSwapper<Endianess::Current::LittleEndian,4>::Take(&resp->data[offset], 1);
				}

				Websocket::Frame::finalizeBuffer(resp.get(), Websocket::Frame::BinaryFrame);
				_session->send(resp.get());
			}
			break;
		case CMD_LAST_ID:
		{
			// SEISCOMP_DEBUG("[cmd] last id");
			uint64_t oid = _db->lastInsertId(&frame.data[1]);
			BufferPtr resp = new Buffer;
			resp->data.resize(1+1+8);
			*reinterpret_cast<uint8_t*>(&resp->data[0]) = CMD_LAST_ID;
			*reinterpret_cast<uint8_t*>(&resp->data[1]) = 0;
			*reinterpret_cast<uint64_t*>(&resp->data[2]) = oid;
			// Convert field count to little endian
			Endianess::ByteSwapper<Endianess::Current::LittleEndian,8>::Take(&resp->data[2], 1);
			Websocket::Frame::finalizeBuffer(resp.get(), Websocket::Frame::BinaryFrame);
			_session->send(resp.get());
			break;
		}
		case CMD_AFFECTED_ROWS:
		{
			// SEISCOMP_DEBUG("[cmd] affected rows");
			uint64_t nAffectedRows = _db->numberOfAffectedRows();
			BufferPtr resp = new Buffer;
			resp->data.resize(1+1+8);
			*reinterpret_cast<uint8_t*>(&resp->data[0]) = CMD_AFFECTED_ROWS;
			*reinterpret_cast<uint8_t*>(&resp->data[1]) = 0;
			*reinterpret_cast<uint64_t*>(&resp->data[2]) = nAffectedRows;
			// Convert field count to little endian
			Endianess::ByteSwapper<Endianess::Current::LittleEndian,8>::Take(&resp->data[2], 1);
			Websocket::Frame::finalizeBuffer(resp.get(), Websocket::Frame::BinaryFrame);
			_session->send(resp.get());
			break;
		}
		default:
			sendClose();
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DBHandler::buffersFlushed() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DBHandler::close() {
	// Response request initiated from the client
	if ( _db ) {
		_db->disconnect();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DBHandler::sendClose() {
	BufferPtr resp = new Buffer;
	Websocket::Frame::finalizeBuffer(resp.get(),
	                                 Websocket::Frame::ConnectionClose,
	                                 Websocket::CloseProtocolError);
	_session->send(resp.get());
	_session->invalidate();
	_session->request().state = HttpRequest::FINISHED;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DBHandler::sendResult(uint8_t command, uint8_t code, const char *message) {
	BufferPtr resp = new Buffer;
	resp->data.resize(2);
	*reinterpret_cast<uint8_t*>(&resp->data[0]) = command;
	*reinterpret_cast<uint8_t*>(&resp->data[1]) = code;
	if ( message ) {
		resp->data += message;
	}
	Websocket::Frame::finalizeBuffer(resp.get(), Websocket::Frame::BinaryFrame);
	_session->send(resp.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
