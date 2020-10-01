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


#ifndef SEISCOMP_CLIENT_PACKET_H
#define SEISCOMP_CLIENT_PACKET_H


#include <seiscomp/core/message.h>
#include <seiscomp/client.h>
#include <string>
#include <stdint.h>


namespace Seiscomp {
namespace Client {


DEFINE_SMARTPOINTER(Packet);

/**
 * @brief The Packet struct provides access to a data packet received
 *        from a messaging backend.
 *
 * Apart from metadata a package only contains a vector of octetts which
 * are parsed in the next layer. A packet is usually transformed to and
 * from a message.
 */
struct SC_SYSTEM_CLIENT_API Packet : Core::BaseObject {
	DECLARE_CASTS(Packet)

	Packet() : type(Undefined), seqNo(-1) {}

	enum Type {
		Undefined    = -1,
		Data         = 0,
		Enter        = 1,
		Leave        = 2,
		Status       = 3,
		Disconnected = 4
	};

	Type        type; //!< The message type
	std::string headerContentEncoding; //!< The encoding raw string
	std::string headerContentType; //!< The content type raw string
	std::string sender; //!< The sender's client name
	std::string target; //!< The target group or peer
	std::string subject; //!< For ENTER, LEAVE, STATE and DISCONNECTED messages
	uint64_t    seqNo; //!< The optional sequence number
	std::string payload; //!< The actual payload or *the* message

	Packet &swap(Packet &other);
};


inline Packet &Packet::swap(Packet &other) {
	std::swap(type, other.type);
	sender.swap(other.sender);
	target.swap(other.target);
	headerContentEncoding.swap(other.headerContentEncoding);
	headerContentType.swap(other.headerContentType);
	subject.swap(other.subject);
	std::swap(seqNo, other.seqNo);
	payload.swap(other.payload);
	return *this;
}


}
}


#endif
