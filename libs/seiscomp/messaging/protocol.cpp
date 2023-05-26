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


#define SEISCOMP_COMPONENT Messaging

#include <seiscomp/logging/log.h>
#include <seiscomp/messaging/protocol.h>

#include <seiscomp/core/interfacefactory.ipp>

#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/archive/binarchive.h>
#include <seiscomp/io/archive/jsonarchive.h>
#include <seiscomp/io/archive/bsonarchive.h>
#include <seiscomp/io/streams/filter/lz4.h>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <iostream>


using namespace std;
namespace bio = boost::iostreams;


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Client::Protocol, SC_SYSTEM_CLIENT_API);


namespace Seiscomp {
namespace Client {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string Protocol::LISTENER_GROUP = "";
const std::string Protocol::IMPORT_GROUP   = "IMPORT_GROUP";
const std::string Protocol::STATUS_GROUP   = "STATUS_GROUP";
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


class ImportXMLArchive : public IO::XMLArchive {
	public:
		ImportXMLArchive(std::streambuf* buf, bool isReading = true,
		                 int forceWriteVersion = -1)
		: XMLArchive(buf, isReading, forceWriteVersion) {
			setRootName("");
		}
};


template <typename AR>
inline void parse(Core::Message *&msg, const char *blob, size_t blob_length,
                  Protocol::ContentEncoding encoding) {
	bio::stream_buffer<bio::array_source> buf(blob, blob_length);

	if ( encoding == Protocol::Identity ) {
		AR ar(&buf, true);
		ar >> msg;
		return;
	}

	bio::filtering_istreambuf filtered_buf;

	switch ( encoding ) {
		case Protocol::Deflate:
			filtered_buf.push(boost::iostreams::zlib_decompressor());
			break;
		case Protocol::GZip:
			filtered_buf.push(boost::iostreams::zlib_decompressor());
			break;
		case Protocol::LZ4:
			filtered_buf.push(ext::boost::iostreams::lz4_decompressor());
			break;
		default:
			throw runtime_error("Invalid encoding type");
	}

	filtered_buf.push(buf);
	AR ar(&filtered_buf, true);
	ar >> msg;
}


template <typename AR>
inline bool write(std::string &blob, const Core::Message *&msg,
                  Protocol::ContentEncoding encoding, int schemaVersion) {
	bio::stream_buffer<boost::iostreams::back_insert_device<std::string> > buf(blob);

	if ( encoding == Protocol::Identity ) {
		AR ar(&buf, false, schemaVersion);
		Core::Message *tmp = const_cast<Core::Message*>(msg);
		ar << tmp;
		return ar.success();
	}

	bio::filtering_ostreambuf filtered_buf;

	switch ( encoding ) {
		case Protocol::Deflate:
			filtered_buf.push(boost::iostreams::zlib_compressor());
			break;
		case Protocol::GZip:
			filtered_buf.push(boost::iostreams::zlib_compressor());
			break;
		case Protocol::LZ4:
			filtered_buf.push(ext::boost::iostreams::lz4_compressor());
			break;
		default:
			return false;
	}

	filtered_buf.push(buf);

	AR ar(&filtered_buf, false, schemaVersion);
	Core::Message *tmp = const_cast<Core::Message*>(msg);
	ar << tmp;
	return ar.success();
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>}
Protocol::State::State() {
	localSequenceNumber = 0;
	sequenceNumber = Core::None;
	sentMessages = 0;
	receivedMessages = 0;
	bytesSent = 0;
	bytesReceived = 0;
	bytesBuffered = 0;
	maxBufferedBytes = 0;
	maxInboxSize = 0;
	maxOutboxSize = 0;
	systemReadCalls = 0;
	systemWriteCalls = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Protocol::Protocol() {
	_schemaVersion = 0;
	_wantMembershipInfo = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Protocol::~Protocol() {
	clearInbox();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Protocol::setMembershipInfo(bool enable) {
	_wantMembershipInfo = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Result Protocol::connect(const std::string &address,
                         unsigned int timeoutMs,
                         const std::string &clientName) {
	return connect(address.c_str(), timeoutMs,
	               clientName.empty() ? nullptr : clientName.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Protocol::Groups &Protocol::groups() const {
	return _groups;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Protocol::handleInterrupt(int) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Protocol::queuePacket(Packet *p) {
	_inbox.push_back(p);

	if ( _state.maxInboxSize < _inbox.size() )
		_state.maxInboxSize = _inbox.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Protocol::clearInbox() {
	for ( PacketQueue::iterator it = _inbox.begin();
		  it != _inbox.end(); ++it )
		delete *it;
	_inbox.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>}
Core::Message *Protocol::decode(const char *blob, size_t blob_length,
                                ContentEncoding encoding, ContentType type) {
	Core::Message* msg = nullptr;

	try {

		switch ( type ) {
			case Binary:
				parse<IO::VBinaryArchive>(msg, blob, blob_length, encoding);
				break;
			case JSON:
				parse<IO::JSONArchive>(msg, blob, blob_length, encoding);
				break;
			case BSON:
				parse<IO::BSONArchive>(msg, blob, blob_length, encoding);
				break;
			case XML:
				parse<IO::XMLArchive>(msg, blob, blob_length, encoding);
				break;
			case IMPORTED_XML:
				parse<ImportXMLArchive>(msg, blob, blob_length, encoding);
				break;
			default:
				break;
		}

	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("message decoding failed: %s", e.what());
		if ( msg ) {
			delete msg;
			msg = nullptr;
		}
	}

	return msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>}
Core::Message *Protocol::decode(const std::string &blob,
                                ContentEncoding encoding,
                                ContentType type) {
	return decode(blob.c_str(), blob.size(), encoding, type);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>}
bool Protocol::encode(std::string &blob, const Core::Message *msg,
                      ContentEncoding encoding,
                      ContentType type,
                      int schemaVersion) {
	switch ( type ) {
		case Binary:
			return write<IO::VBinaryArchive>(blob, msg, encoding, schemaVersion);
		case JSON:
			return write<IO::JSONArchive>(blob, msg, encoding, schemaVersion);
		case BSON:
			return write<IO::BSONArchive>(blob, msg, encoding, schemaVersion);
		case XML:
			return write<IO::XMLArchive>(blob, msg, encoding, schemaVersion);
		case IMPORTED_XML:
			return write<ImportXMLArchive>(blob, msg, encoding, schemaVersion);
		default:
			break;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>}
size_t Protocol::inboxSize() const {
	lock_guard<mutex> l(_readMutex);
	return _inbox.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Protocol::setCertificate(const string &cert) {
	_certificate = cert;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
