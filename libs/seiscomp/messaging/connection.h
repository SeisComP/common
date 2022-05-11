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


#ifndef SEISCOMP_CLIENT_CONNECTION_H
#define SEISCOMP_CLIENT_CONNECTION_H


#include <seiscomp/core/message.h>
#include <seiscomp/messaging/protocol.h>
#include <seiscomp/utils/timer.h>

#include <openssl/x509.h>

#include <mutex>
#include <functional>


namespace Seiscomp {
namespace Client {


DEFINE_SMARTPOINTER(Connection);

/**
 * @brief The Connection class implements the high level connection to a
 *        messaging backend. It requires a protocol implementation and
 *        parses the message from the wire into an object tree.
 *
 * @code
 * ConnectionPtr c = new Connection;
 * if ( c->setSource("scmp://app@localhost/testing") != OK ) {
 *   cerr << "Invalid source" <<endl;
 *   exit(1);
 * }
 *
 * if ( c->connect() != OK ) {
 *   cerr << "Could not open connection" <<endl;
 *   exit(1);
 * }
 *
 * cout << "Connected as " << c->clientName() << endl;
 * Message *nmsg;
 * while ( msg = c->receive(&nmsg) ) {
 *   cout << "Received message from " << nmsg->sender << endl;
 * }
 *
 * c->close();
 * @endcode
 */
class SC_SYSTEM_CLIENT_API Connection : public Core::BaseObject {
	public:
		typedef std::list<Connection*> Connections;
		typedef std::function<void (const Core::Time &, std::ostream&)> InfoCallback;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Connection(Protocol *proto = nullptr);
		//! D'tor
		~Connection();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Sets the default content encoding passed to the final
		 *        implementation of \ref Protocol.sendMessage.
		 *
		 * This is taken as a hint and implementations can or cannot respect
		 * it. The default encoding is deflate.
		 *
		 * @param enc The content encoding
		 */
		void setContentEncoding(Protocol::ContentEncoding enc);

		/**
		 * @brief Sets the default content type passed to the final
		 *        implementation of \ref Protocol.sendMessage.
		 *
		 * This is taken as a hint and implementations can or cannot respect
		 * it. The default type is binary.
		 *
		 * @param type The content type
		 */
		void setContentType(Protocol::ContentType type);

		/**
		 * @brief Creates a connection to the given URL.
		 *
		 * The protocol implementation is created from the given protocol.
		 * The default is 'scmp' and can be omitted: 'localhost/testing' and
		 * 'scmp://localhost/testing' are equal.
		 * If a username is passed then it will be required for registration
		 * otherwise a random username (clientname) will be created and
		 * assigned to the connection.
		 *
		 * Only the protocol implementation will be instantiated and the
		 * address will be saved. A real connection attempt isn't being
		 * made at this point.
		 *
		 * If a protocol has been set already in the constructor then it will
		 * be used if the protocol is omitted in the URL. Otherwise it will be
		 * replaced by the given protocol.
		 *
		 * @param url The URL in format proto://user@host/queue
		 * @return An unmanaged connection pointer or nullptr in case of error.
		 */
		Result setSource(const char *URL);
		Result setSource(const std::string &URL);

		//! Returns the configured source
		const std::string &source() const;

		/**
		 * @brief Sets whether membership info of clients should be received.
		 *        This information informs about when another client enters
		 *        one of the groups this connection is subscribed to, when
		 *        another clients leaves one of the groups this connection is
		 *        subscribed to and when a client disconnects from the
		 *        broker.
		 *        This method has to be called prior to connect and after
		 *        setSource to have an affect.
		 * @param enable true if membership info should be received, false
		 *               otherwise.
		 */
		Result setMembershipInfo(bool enable);

		/**
		 * @brief Attempts to connect to the messaging backend located at the
		 *        given source (@setSource).
		 * @param clientName An optional clientname.
		 * @param primaryGroup The primary messaging group. If empty then the
		 *                     connection is treated as read-only connection.
		 * @param timeoutMs The handshake timeout in milliseconds. If zero then
		 *                  no timeout is being used.
		 * @return Result code
		 */
		Result connect(const std::string &clientName,
		               const std::string &primaryGroup,
		               unsigned int timeoutMs = 3000);

		//! Disconnects from the backend
		Result disconnect();

		//! Returns whether a connection is established or not
		bool isConnected() const;

		/**
		 * @brief Reconnects the last established connection.
		 *
		 * If the connection is still established then it will disconnect
		 * first.
		 *
		 * @return Result code
		 */
		Result reconnect();

		//! Closes the connection to the backend the hard way
		Result close();

		/**
		 * @brief Sets the read timeout for receiving a message. If the
		 *        timeout hits then TimeoutError is returned.
		 * @param milliseconds The timeout in milliseconds. Zero or a negative
		 *                     value disabled the timeout.
		 * @return Result code
		 */
		Result setTimeout(int milliseconds);

		Result subscribe(const char *group);
		Result subscribe(const std::string &group);
		Result unsubscribe(const char *group);
		Result unsubscribe(const std::string &group);

		/**
		 * @brief Waits for a new message to arrive so that a subsequent call
		 *        to \ref recv() will return immediately.
		 * @return Result code
		 */
		Result fetchInbox();

		/**
		 * @brief Synchronizes the outbox with the remote server.
		 *        See Protocol::syncOutbox().
		 * @return Result code
		 */
		Result syncOutbox();

		/**
		 * @brief Reads a message from the backend. If no message is available
		 *        locally the call will block until a message arrives.
		 * @param packet An optional storage pointer to the transmitted packet
		 *               from which the message has been created. The ownership
		 *               goes to the caller.
		 * @param status An optional status storage which holds the result of
		 *               the operation.
		 * @return The message pointer. nullptr is returned if either an error
		 *         occurred or a system packet was received. This requires a
		 *         package storage pointer to be passed.
		 */
		Core::Message *recv(Packet **packet = nullptr, Result *status = nullptr);
		Core::Message *recv(PacketPtr &packet, Result *status = nullptr);

		Result sendMessage(const Core::Message *msg);
		Result sendMessage(const std::string &targetGroup, const Core::Message *msg);

		//! The following three functions are actually convenience wrapper for
		//! backward compatibility
		bool send(const Core::Message *msg);
		bool send(const std::string &targetGroup, const Core::Message *msg);

		size_t inboxSize() const;

		const Protocol::State *state() const;

		Result lastError() const;
		const std::string lastErrorMessage() const;

		Protocol *protocol() const;

		void setInfoCallback(InfoCallback);

		void getInfo(const Core::Time &timestamp, std::ostream &os);

		Result setCertificate(const std::string &cert);

	// ----------------------------------------------------------------------
	//  Query functions
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns the client name. Either the one given during connect
		 *        or the one assigned by the server.
		 * @return The client name
		 */
		const std::string &clientName() const;

		/**
		 * @brief Returns the schema version supported by the remote end.
		 * This requires a successfull connection to be valid.
		 * @return The version of the remote schema
		 */
		Core::Version schemaVersion() const;

		// See Protocoll::configurationParameters
		const Protocol::KeyValueStore *extendedParameters() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		static void registerConnection(Connection *con);
		static void unregisterConnection(Connection *con);


	protected:
		ProtocolPtr               _protocol;
		Protocol::ContentEncoding _defaultContentEncoding;
		Protocol::ContentType     _defaultContentType;
		std::string               _address;
		std::string               _clientName;
		std::string               _primaryGroup;
		unsigned int              _timeoutMs;
		Result                    _lastError;
		InfoCallback              _infoCallback;
		Connections::iterator     _poolIterator;
};


inline Result Connection::lastError() const {
	return _lastError;
}

inline const std::string &Connection::source() const {
	return _address;
}

inline bool Connection::send(const Core::Message *msg) {
	return sendMessage(msg) == OK;
}

inline bool Connection::send(const std::string &targetGroup, const Core::Message *msg) {
	return sendMessage(targetGroup, msg) == OK;
}


}
}


#endif
