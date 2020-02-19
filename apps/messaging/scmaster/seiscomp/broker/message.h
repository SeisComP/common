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


#ifndef GEMPA_BROKER_MESSAGE_H__
#define GEMPA_BROKER_MESSAGE_H__


#include <string>
#include <stdint.h>

#include <seiscomp/core/enumeration.h>
#include <seiscomp/wired/buffer.h>

#include <seiscomp/broker/api.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


class Group;


typedef uint64_t SequenceNumber;
#define INVALID_SEQUENCE_NUMBER Seiscomp::Messaging::Broker::SequenceNumber(-1)


MAKEENUM(
	ContentEncoding,
	EVALUES(
		Identity,
		Deflate,
		GZip,
		LZ4
	),
	ENAMES(
		"identity",
		"deflate",
		"gzip",
		"lz4"
	)
);

MAKEENUM(
	MimeType,
	EVALUES(
		Binary,
		JSON,
		BSON,
		XML,
		IMPORTED_XML,
		Text
	),
	ENAMES(
		"application/x-sc-bin",
		"text/json",
		"application/x-sc-bson",
		"application/x-sc-xml",
		"text/xml",
		"text/plain"
	)
);


DEFINE_SMARTPOINTER(Message);
/**
 * @brief The Message class implements the message structure.
 *
 * A message contains meta data and a payload. Since each protocol has to
 * encode the message differently a cached version for each protocol is also
 * stored. That buffer can be sent without further modifications. This is
 * in particular helpful if a message is going to be sent to hundreds of
 * clients connected through the same protocol. The message has to be encoded
 * only once and not hundred times. This cache is lazy and will only be
 * populated at the first send operation.
 */
class SC_BROKER_API Message : public Seiscomp::Core::BaseObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Message();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Decodes a message if object is NULL according to the payload
		 *        and format
		 * @return true if msg->object is a valid pointer or false otherwise.
		 */
		bool decode();

		/**
		 * @brief Encodes a message if object is not NULL and saves the
		 *        encoded buffer in payload.
		 * @return true if payload is not empty, false otherwise.
		 */
		bool encode();


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	public:
		enum struct Type {
			Unspecified,
			Regular,
			Transient, // From this enumeration messages are not processed
			Status
		};

		std::string                   sender;      //!< The sender
		std::string                   target;      //!< The target group/topic
		std::string                   encoding;    //!< The encoding of the data
		std::string                   mimeType;    //!< The mime type of the data
		std::string                   payload;     //!< The payload bytes
		Core::BaseObjectPtr           object;      //!< The decoded object
		Core::Version                 schemaVersion; //!< The schema version of the payload after decoding
		Seiscomp::Core::Time          timestamp;   //!< The received time
		Type                          type; //!< The message type
		bool                          selfDiscard; //!< Whether self discard should be checked or not
		bool                          processed;
		/** The assigned sequence number */
		SequenceNumber                sequenceNumber;

		/** Cached encoded version for different protocols */
		Wired::BufferPtr              encodingWebSocket;

		/** Cache of the target group */
		Group                         *_internalGroupPtr;
};


}
}
}


#endif
