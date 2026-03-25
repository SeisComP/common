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


#define SEISCOMP_COMPONENT MASTER

#include <seiscomp/logging/log.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/archive/binarchive.h>
#include <seiscomp/io/archive/jsonarchive.h>
#include <seiscomp/io/streams/filter/lz4.h>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <iostream>

#include "message.h"


using namespace std;
namespace bio = boost::iostreams;


namespace Seiscomp {
namespace Messaging {
namespace Broker {
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
inline Core::Version parse(Core::BaseObjectPtr &obj, const string &blob,
                           ContentEncoding encoding) {
	bio::stream_buffer<bio::array_source> buf(blob.c_str(), blob.size());

	if ( encoding == Identity ) {
		AR ar(&buf, true);
		ar >> obj;
		return ar.version();
	}

	bio::filtering_istreambuf filtered_buf;

	switch ( encoding ) {
		case Deflate:
			filtered_buf.push(boost::iostreams::zlib_decompressor());
			break;
		case GZip:
			filtered_buf.push(boost::iostreams::zlib_decompressor());
			break;
		case LZ4:
			filtered_buf.push(ext::boost::iostreams::lz4_decompressor());
			break;
		default:
			throw runtime_error("Invalid encoding type");
	}

	filtered_buf.push(buf);
	AR ar(&filtered_buf, true);
	ar >> obj;
	return ar.version();
}


template <typename AR>
inline bool write(std::string &blob, const Core::BaseObject *obj,
                  ContentEncoding encoding, Core::Version schemaVersion) {
	bio::stream_buffer<boost::iostreams::back_insert_device<std::string> > buf(blob);

	if ( encoding == Identity ) {
		AR ar(&buf, false, schemaVersion.packed);
		Core::BaseObject *tmp = const_cast<Core::BaseObject*>(obj);
		ar << tmp;
		return ar.success();
	}

	bio::filtering_ostreambuf filtered_buf;

	switch ( encoding ) {
		case Deflate:
			filtered_buf.push(boost::iostreams::zlib_compressor());
			break;
		case GZip:
			filtered_buf.push(boost::iostreams::zlib_compressor());
			break;
		case LZ4:
			filtered_buf.push(ext::boost::iostreams::lz4_compressor());
			break;
		default:
			return false;
	}

	filtered_buf.push(buf);

	AR ar(&filtered_buf, false, schemaVersion.packed);
	Core::BaseObject *tmp = const_cast<Core::BaseObject*>(obj);
	ar << tmp;
	return ar.success();
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Message::Message()
: type(Type::Unspecified)
, selfDiscard(true)
, processed(false)
, sequenceNumber(INVALID_SEQUENCE_NUMBER)
, _internalGroupPtr(NULL)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Message::decode() {
	// Already decoded -> done
	if ( object )
		return true;

	ContentEncoding ce(Identity);
	if ( !encoding.empty() && !ce.fromString(encoding) ) {
		// Encoding unknown
		return false;
	}

	MimeType mt;
	if ( !mt.fromString(mimeType) ) {
		// Mimetype unknown
		return false;
	}

	schemaVersion = Core::Version();

	try {
		switch ( mt ) {
			case Binary:
				schemaVersion = parse<IO::VBinaryArchive>(object, payload, ce);
				break;
			case JSON:
				schemaVersion = parse<IO::JSONArchive>(object, payload, ce);
				break;
			case XML:
				schemaVersion = parse<IO::XMLArchive>(object, payload, ce);
				break;
			case IMPORTED_XML:
				schemaVersion = parse<ImportXMLArchive>(object, payload, ce);
				break;
			default:
				break;
		}

	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("message decoding failed: %s", e.what());
		object = NULL;
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Message::encode() {
	payload = string();

	// Already decoded -> done
	if ( !object )
		return true;

	ContentEncoding ce(Identity);
	if ( !encoding.empty() && !ce.fromString(encoding) ) {
		// Encoding unknown
		return false;
	}

	MimeType mt;
	if ( !mt.fromString(mimeType) ) {
		// Mimetype unknown
		return false;
	}

	switch ( mt ) {
		case Binary:
			return write<IO::VBinaryArchive>(payload, object.get(), ce, schemaVersion);
		case JSON:
			return write<IO::JSONArchive>(payload, object.get(), ce, schemaVersion);
		case XML:
			return write<IO::XMLArchive>(payload, object.get(), ce, schemaVersion);
		case IMPORTED_XML:
			return write<ImportXMLArchive>(payload, object.get(), ce, schemaVersion);
		default:
			break;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
