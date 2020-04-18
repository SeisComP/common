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


#ifndef GEMPA_BROKER_PROTOCOL_H__
#define GEMPA_BROKER_PROTOCOL_H__


namespace Seiscomp {
namespace Messaging {
namespace Broker {
namespace Protocol {


// Defines do not make much sense inside namespaces but they are placed here
// to stay close to future variable declarations.

/**
 * It follows a list of definitions for all protocol commands and replies and
 * their headers. They are being used in the code and changing them here will
 * cause a change in behaviour of the server.
 */

/**
 * ```
 * CONNECT
 * Ack-Window: [number of messages after which an ack will be send from the server]
 * Membership-Info: 1
 * Queue: [name of queue]
 * Client-Name: [name of client]
 * Subscriptions: [list of groups]
 * Seq-No: [last seen sequence number]
 *
 * ^@
 * ```
 *
 * The *Seq-No* header contains the last sequence number the client has seen from
 * that queue. That header is optional. If subscriptions are given then the
 * client will receive an **ENTER** frame for each group it subscribed to. If any
 * of the requested groups does not exist, an **ERROR** frame is sent and the
 * connection is closed.
 *
 * The order of messages looks as follows:
 *
 * 1. CONNECT
 * 2. CONNECTED
 * 3. ENTER
 * 4. RECV
 *
 * Step 3 repeats for as many groups as given in the subscription list. Step 4
 * repeats for all messages received during the lifetime of the connection.
 */
#define SCMP_PROTO_CMD_CONNECT        "CONNECT"
#define SCMP_PROTO_CMD_CONNECT_HEADER_QUEUE           "Queue"
#define SCMP_PROTO_CMD_CONNECT_HEADER_CLIENT_NAME     "Client-Name"
#define SCMP_PROTO_CMD_CONNECT_HEADER_MEMBERSHIP_INFO "Membership-Info"
#define SCMP_PROTO_CMD_CONNECT_HEADER_SELF_DISCARD    "Self-Discard"
#define SCMP_PROTO_CMD_CONNECT_HEADER_ACK_WINDOW      "Ack-Window"
#define SCMP_PROTO_CMD_CONNECT_HEADER_SEQ_NUMBER      "Seq-No"
#define SCMP_PROTO_CMD_CONNECT_HEADER_SUBSCRIPTIONS   "Subscriptions"

/**
 * ```
 * DISCONNECT
 * Receipt: [id]
 *
 * ^@
 * ```
 *
 * The DISCONNECT command ask the server to gracefully shutdown the connection
 * and free all associated resources.
 */
#define SCMP_PROTO_CMD_DISCONNECT     "DISCONNECT"
#define SCMP_PROTO_CMD_DISCONNECT_HEADER_RECEIPT      "Receipt"

/**
 * ```
 * SUBSCRIBE
 * Groups: [list of groups]
 *
 * ^@
 * ```
 * Subscribes to a specific group which must exist on the server. In response
 * either an **ENTER** or **ERROR** frame will be received.
 */
#define SCMP_PROTO_CMD_SUBSCRIBE      "SUBSCRIBE"
#define SCMP_PROTO_CMD_SUBSCRIBE_HEADER_GROUPS        "Groups"

/**
 * ```
 * UNSUBSCRIBE
 * Groups: [list of groups]
 *
 * ^@
 * ```
 *
 * Unsubscribes from a specific group which must exist on the server. In
 * response either a **LEAVE** or **ERROR** frame will be received.
 */
#define SCMP_PROTO_CMD_UNSUBSCRIBE    "UNSUBSCRIBE"
#define SCMP_PROTO_CMD_UNSUBSCRIBE_HEADER_GROUPS      SCMP_PROTO_CMD_SUBSCRIBE_HEADER_GROUPS

/**
 * Sends a message to a group or a client (peer-to-peer).
 *
 * ```
 * SEND
 * D: [name of group or the client]
 * T: [MIME type]
 * E: [transfer encoding]
 * L: [length of content]
 *
 * [payload]^@
 * ```
 *
 * Each message sent will increase the private sequence number counter for this
 * connection starting with 0. So the first message will get assigned the
 * sequence number 1. That counter must be maintained by the client and the
 * server to correctly synchronize acknowledgements. If the message is rejected
 * an **ERROR** frame will be sent to the client and the connection will be
 * closed.
 */
#define SCMP_PROTO_CMD_SEND           "SEND"
#define SCMP_PROTO_CMD_SEND_HEADER_DESTINATION        "D"
#define SCMP_PROTO_CMD_SEND_HEADER_CONTENT_LENGTH     "L"
#define SCMP_PROTO_CMD_SEND_HEADER_ENCODING           "E"
#define SCMP_PROTO_CMD_SEND_HEADER_MIMETYPE           "T"
#define SCMP_PROTO_CMD_SEND_HEADER_TRANSIENT          "Transient"


/**
 * A member notifies the server about its state including memory consumption,
 * cpu usage, uptime and so on. The payload is always a key-value list
 * separated by '&'.
 *
 * ```
 * STATE
 * D: [name of group or the client]
 * L: [length of content]
 *
 * hostname=localhost&totalmemory=8589934592&clientmemoryusage=68891443...^@
 * ```
 */
#define SCMP_PROTO_CMD_STATE        "STATE"
#define SCMP_PROTO_CMD_STATE_HEADER_DESTINATION       "D"
#define SCMP_PROTO_CMD_STATE_HEADER_CONTENT_LENGTH    "L"

#define SCMP_PROTO_CMD_FIRST_CHARS  "CDSU"

/**
 * ```
 * CONNECTED
 * Queue: [name of queue]
 * Server: SeisComP/2017.334
 * Version: [server protocol version]
 * Client-Name: [client name, either auto assigned or requested by the client]
 * Authentication: [16 byte hex random NaCL nonce prefix]
 *                 [32 byte hex NaCL public server key]
 *                 [16 byte hex encrypted buffer: "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"]
 * Groups: [list of available groups]
 *
 * ^@
 * ```
 *
 * In return to a **CONNECT** frame the server responds with a **CONNECTED**
 * frame. It reports the client name in use for this connection and a list of
 * available groups.
 */
#define SCMP_PROTO_REPLY_CONNECT      "CONNECTED"
#define SCMP_PROTO_REPLY_CONNECT_HEADER_VERSION        "Version"
#define SCMP_PROTO_REPLY_CONNECT_HEADER_SCHEMA_VERSION "Schema-Version"
#define SCMP_PROTO_REPLY_CONNECT_HEADER_QUEUE          SCMP_PROTO_CMD_CONNECT_HEADER_QUEUE
#define SCMP_PROTO_REPLY_CONNECT_HEADER_CLIENT_NAME    SCMP_PROTO_CMD_CONNECT_HEADER_CLIENT_NAME
#define SCMP_PROTO_REPLY_CONNECT_HEADER_ACK_WINDOW     SCMP_PROTO_CMD_CONNECT_HEADER_ACK_WINDOW
#define SCMP_PROTO_REPLY_CONNECT_HEADER_GROUPS         "Groups"

/**
 * The probably most important part of the protocol is receiving a message from
 * a group or a client (peer-to-peer).
 *
 * ```
 * RECV
 * C: [client name of sender]
 * D: [name of group or the client]
 * T: [MIME type]
 * E: [transfer encoding]
 * N: [message sequence number]
 * L: [length of content]
 *
 * [payload]^@
 * ```
 *
 * The payload can be anything, binary data or text. Optionally the *Content-Type*
 * header is set to inform the client about the format.
 */
#define SCMP_PROTO_REPLY_SEND         "RECV"
#define SCMP_PROTO_REPLY_SEND_HEADER_SENDER           "C"
#define SCMP_PROTO_REPLY_SEND_HEADER_SEQ_NUMBER       "N"
#define SCMP_PROTO_REPLY_SEND_HEADER_DESTINATION      SCMP_PROTO_CMD_SEND_HEADER_DESTINATION
#define SCMP_PROTO_REPLY_SEND_HEADER_CONTENT_LENGTH   SCMP_PROTO_CMD_SEND_HEADER_CONTENT_LENGTH
#define SCMP_PROTO_REPLY_SEND_HEADER_ENCODING         SCMP_PROTO_CMD_SEND_HEADER_ENCODING
#define SCMP_PROTO_REPLY_SEND_HEADER_MIMETYPE         SCMP_PROTO_CMD_SEND_HEADER_MIMETYPE

/**
 * ```
 * ACK
 * N: [connection specific sequence number]
 *
 * ^@
 * ```
 *
 * The server sends according to the configured acknoledgement window an
 * acknowledgement frame to signal that all messages prior to the given sequence
 * number have been processed and that the current sequence number is expected
 * to be the one sent. It will do that as well after 1 second the client
 * hasn't sent any further messages.
 */
#define SCMP_PROTO_REPLY_ACK          "ACK"
#define SCMP_PROTO_REPLY_ACK_HEADER_SEQ_NUMBER        SCMP_PROTO_REPLY_SEND_HEADER_SEQ_NUMBER

/**
 * ```
 * RECEIPT
 * Receipt-Id: [id]
 *
 * ^@
 * ```
 *
 * A receipt can basically be sent for anything which has an id. A receipt is
 * being sent definitely after a disconnect request. In that case the receipt
 * id is the username.
 */
#define SCMP_PROTO_REPLY_RECEIPT      "RECEIPT"
#define SCMP_PROTO_REPLY_RECEIPT_HEADER_ID            "Receipt-Id"

/**
 * A members enters a group. In response to a **SUBSCRIBE** command, the complete
 * group information will be sent to the client. Specifically if
 * ```Member == self```. Otherwise only the group and member information will be
 * sent from the server to all clients that are subscribed to that group. The
 * client needs to update its internal cache and the frame body is empty in that
 * case.
 *
 * ```
 * ENTER
 * D: [name of group]
 * C: [name of client]
 *
 * clientA, clientB, ...
 * }^@
 * ```
 */
#define SCMP_PROTO_REPLY_ENTER        "ENTER"
#define SCMP_PROTO_REPLY_ENTER_HEADER_GROUP           "D"
#define SCMP_PROTO_REPLY_ENTER_HEADER_MEMBER          "C"

/**
 * A member leaves a group. This message will sent from the server to all clients
 * that are subscribed to the group in question.
 *
 * ```
 * LEAVE
 * D: [name of group]
 * C: [name of client]
 *
 * ^@
 * ```
 */
#define SCMP_PROTO_REPLY_LEAVE        "LEAVE"
#define SCMP_PROTO_REPLY_LEAVE_HEADER_GROUP           SCMP_PROTO_REPLY_ENTER_HEADER_GROUP
#define SCMP_PROTO_REPLY_LEAVE_HEADER_MEMBER          SCMP_PROTO_REPLY_ENTER_HEADER_MEMBER

/**
 * A member state of health information or simply its state including
 * memory consumption, cpu usage, uptime and so on. The payload is always
 * a key-value list separated by '&'.
 *
 * ```
 * STATE
 * L: [length of content]
 * D: [name of group or the client]
 * C: [name of client]
 *
 * hostname=localhost&totalmemory=8589934592&clientmemoryusage=68891443...^@
 * ```
 */
#define SCMP_PROTO_REPLY_STATE        "STATE"
#define SCMP_PROTO_REPLY_STATE_HEADER_DESTINATION     "D"
#define SCMP_PROTO_REPLY_STATE_HEADER_CLIENT          "C"
#define SCMP_PROTO_REPLY_STATE_HEADER_CONTENT_LENGTH  SCMP_PROTO_CMD_SEND_HEADER_CONTENT_LENGTH

/**
 * A client was disconnected. This message will sent from the server to all
 * clients currently connected.
 *
 * ```
 * DISCONNECTED
 * C: [name of client]
 *
 * ^@
 * ```
 */
#define SCMP_PROTO_REPLY_DISCONNECTED "DISCONNECTED"
#define SCMP_PROTO_REPLY_DISCONNECTED_HEADER_CLIENT   SCMP_PROTO_REPLY_STATE_HEADER_CLIENT

/**
 * ```
 * ERROR
 * N: [connection specific sequence number]
 *
 * Error message ...^@
 * ```
 */
#define SCMP_PROTO_REPLY_ERROR        "ERROR"
#define SCMP_PROTO_REPLY_ERROR_HEADER_SEQ_NUMBER      SCMP_PROTO_REPLY_SEND_HEADER_SEQ_NUMBER


}
}
}
}


#endif
