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


#ifndef SEISCOMP_CLIENT_PROTOCOL_H
#define SEISCOMP_CLIENT_PROTOCOL_H


#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/enumeration.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/core/optional.h>
#include <seiscomp/messaging/packet.h>

#include <boost/thread/mutex.hpp>

#include <deque>
#include <map>
#include <set>
#include <string>


namespace Seiscomp {
namespace Client {


PREPAREENUM(Result,
	EVALUES(
		/** Everything went find */
		OK = 0,
		/** The value/string passed is not an URL */
		InvalidURL,
		/** The URL parameters are not correct / not understood */
		InvalidURLParameters,
		/** Protocol implementation is not available */
		InvalidProtocol,
		/** No content encoding specified e.g. when sending messages */
		ContentEncodingRequired,
		/** No content type specified e.g. when sending messages */
		ContentTypeRequired,
		/** An unknown content encoding was received */
		ContentEncodingUnknown,
		/** An unknown content type was received */
		ContentTypeUnknown,
		/** There is already an active connection */
		AlreadyConnected,
		/** There is currently no active connection to handle */
		NotConnected,
		/** The server closed the connection */
		ConnectionClosedByPeer,
		/** A system error, check errno */
		SystemError,
		/** The connection timed out */
		TimeoutError,
		/** A network error, check errno */
		NetworkError,
		/** An application protocol error occurred */
		NetworkProtocolError,
		/** The requested username is connected already */
		DuplicateUsername,
		/** The remote group does not exist */
		GroupDoesNotExist,
		/** There are no inbox messages */
		InboxUnderflow,
		/** Too many inbox messages which are not processed */
		InboxOverflow,
		/** Too many unacknowledged outbox messages */
		OutboxOverflow,
		/** You can't subscribe again to groups you are already subscribed to */
		AlreadySubscribed,
		/** You can't unsubscribe from groups you are not subscribed to */
		NotSubscribed,
		/** Messages could not be encoded into a packet */
		EncodingError,
		/** Package could not be decoded into a message */
		DecodingError,
		/** Missing group, e.g. when sending data */
		MissingGroup,
		/** Invalid message type enumeration e.g. when sending messages/data */
		InvalidMessageType,
		/** Message too large */
		MessageTooLarge,
		/** Unspecified error */
		Error
	),
	ENAMES(
		"OK",
		"InvalidURL",
		"InvalidURLParameters",
		"InvalidProtocol",
		"ContentEncodingRequired",
		"ContentTypeRequired",
		"ContentEncodingUnknown",
		"ContentTypeUnknown",
		"AlreadyConnected",
		"NotConnected",
		"ConnectionClosedByPeer",
		"SystemError",
		"TimeoutError",
		"NetworkError",
		"NetworkProtocolError",
		"DuplicateUsername",
		"GroupDoesNotExist",
		"InboxUnderflow",
		"InboxOverflow",
		"OutboxOverflow",
		"AlreadySubscribed",
		"NotSubscribed",
		"EncodingError",
		"DecodingError",
		"MissingGroup",
		"InvalidMessageType",
		"MessageTooLarge",
		"Error"
	)
);

class Result : public ENUMWRAPPERCLASS(Result) {
	public:
		Result(Type value = Type(0)) : ENUMWRAPPERCLASS(Result)(value) {}
		Type code() const { return _value; }
		operator bool() const { return _value == OK; }

	// Disable implicit casts to the enumeration and therefore to
	// a bool value. A special bool operator is used that maps true to
	// OK.
	private:
		operator Type() const { return ENUMWRAPPERCLASS(Result)::operator Type(); }
};



DEFINE_SMARTPOINTER(Protocol);

/**
 * @brief The abstract class Protocol implements the low-level message
 *        transport protocol.
 *
 * The protocol handles one connection to a messaging broker and supports
 * the publish/subscribe pattern. It must implement the methods to subscribe
 * to a group, unsubscribe from a group and to receive and send messages.
 * Messages are just binary blobs with associated metadata. The message
 * content is not interpreted or parsed in any way.
 *
 * @code
 * ProtocolPtr proto = Protocol::Create("scmp");
 * if ( !proto ) exit(1);
 * if ( proto->connect("localhost") ) {
 *   cerr << "Connection failed" << endl;
 *   exit(1);
 * }
 *
 * cout << "Connected as " << proto->clientName() << endl;
 * proto->subscribe("PICK");
 *
 * PacketPtr p;
 * while ( (p = c->recv() ) {
 *   cout << "Packet from " << p->sender() << " to " << p->target << endl;
 * }
 *
 * proto->disconnect();
 * @endcode
 *
 * ## Message Types
 * ### Regular
 *
 * Regular messages will be queued, receive a sequence number and their
 * content will not be touched by the server unless certain processing
 * profiles are enabled. That is the default mode of the messaging API.
 *
 * ### Transient
 *
 * Transient messages are like regular messages but they are not queued and
 * will not be processed. A client that connects to continue with a sequence
 * number will not receive such a message.
 *
 * ### Service
 *
 * Service messages are like transient messages but are evaluated by the
 * server. They are mainly used to interact with the server. State-of-health
 * messages and sync messages are typical service messages.
 */
class SC_SYSTEM_CLIENT_API Protocol : public Core::InterruptibleObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		struct State {
			State();
			uint64_t      localSequenceNumber;
			OPT(uint64_t) sequenceNumber;
			uint64_t      receivedMessages;
			uint64_t      sentMessages;
			uint64_t      bytesSent;
			uint64_t      bytesReceived;
			uint64_t      bytesBuffered;
			uint64_t      maxBufferedBytes;
			uint64_t      maxInboxSize;
			uint64_t      maxOutboxSize;
			uint64_t      systemReadCalls;
			uint64_t      systemWriteCalls;
		};

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
			ContentType,
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

		MAKEENUM(
			MessageType,
			EVALUES(
				Regular,
				Transient,
				Status
			),
			ENAMES(
				"regular",
				"transient",
				"status"
			)
		);

		static const std::string STATUS_GROUP;
		static const std::string LISTENER_GROUP;
		static const std::string IMPORT_GROUP;

		//! A list of group names
		using Groups = std::set<std::string>;
		using KeyValueStore = std::map<std::string, std::string>;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Protocol();
		virtual ~Protocol();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Sets whether membership info of clients should be received.
		 *        This information informs about when another client enters
		 *        one of the groups this connection is subscribed to, when
		 *        another clients leaves one of the groups this connection is
		 *        subscribed to and when a client disconnects from the
		 *        broker.
		 *        This method has to be called prior to connect to have an
		 *        affect.
		 * @param enable true if membership info should be received, false
		 *               otherwise.
		 */
		void setMembershipInfo(bool enable);

		/**
		 * @brief connect
		 * @param address The connection address, e.g. host:port/queue
		 * @param timeoutMs The timeout in milliseconds
		 * @param clientName The desirec client name. If it is nullptr then the
		 *                   server will choose a random client name.
		 * @return
		 */
		virtual Result connect(const char *address, unsigned int timeoutMs,
		                       const char *clientName = nullptr) = 0;

		Result connect(const std::string &address, unsigned int timeoutMs,
		               const std::string &clientName = std::string());

		/**
		 * @brief Returns the schema version supported by the remote end.
		 * This requires a successfull connection to be valid.
		 * @return The version of the remote schema
		 */
		Core::Version schemaVersion() const;

		/**
		 * @brief Returns configuration parameters as key-value store
		 *        returned by the broker.
		 * Those parameters can be used or not, it is up to the user of
		 * the connection.
		 * @return The key-value store with additional parameters
		 */
		const KeyValueStore &extendedParameters() const;

		/**
		 * @brief Returns the client name. Either the one given during connect
		 *        or the one assigned by the server.
		 * @return The client name
		 */
		const std::string &clientName() const;

		/**
		 * @brief Subscribes to a group which must exist
		 * @param group The group name
		 * @return Result code
		 */
		virtual Result subscribe(const std::string &group) = 0;

		/**
		 * @brief Unsubscribes from a group where the client was subscribed to.
		 * @param group The group name
		 * @return Result code
		 */
		virtual Result unsubscribe(const std::string &group) = 0;

		/**
		 * @brief Sends data with a particular content type.
		 * @param targetGroup The group name to send the message to
		 * @param data The data pointer to the octett stream.
		 * @param len Length in bytes of the octett stream.
		 * @param encoding The data content encoding.
		 * @param type The data content type.
		 * @return Result code
		 */
		virtual Result sendData(const std::string &targetGroup,
		                        const char *data, size_t len,
		                        MessageType type,
		                        ContentEncoding contentEncoding,
		                        ContentType contentType) = 0;

		/**
		 * @brief Sends a message. A message is actually a binary stream
		 *        of certain length. Optionally a content type can be
		 *        associated with it.
		 * @param targetGroup The group name to send the message to
		 * @param msg The message pointer.
		 * @param contentType An optional content type which will be or
		 *                    won't be associated with the packet. This is
		 *                    implementation specific.
		 * @return Result code
		 */
		virtual Result sendMessage(const std::string &targetGroup,
		                           const Core::Message *msg,
		                           MessageType type = Regular,
		                           OPT(ContentEncoding) contentEncoding = Core::None,
		                           OPT(ContentType) contentType = Core::None) = 0;

		/**
		 * @brief Receives a data packet.
		 * The call will block if the local inbox is empty.
		 * @param p The packet instance which will hold the packet
		 *          information and the payload.
		 * @return Result code
		 */
		virtual Result recv(Packet &p) = 0;

		/**
		 * @brief Receives a data packet.
		 * The call will block if the local inbox is empty.
		 * @param result An optional storage for the result code
		 * @return A packet pointer which must be handled by the caller.
		 */
		virtual Packet *recv(Result *result = nullptr) = 0;

		/**
		 * @brief Waits for a new message to arrive so that a subsequent call
		 *        to \ref recv will return immediately.
		 * @return Result code
		 */
		virtual Result fetchInbox() = 0;

		/**
		 * @brief Synchronizes the outbox with the remote server.
		 * @details This means if this method returns successfully then all
		 *          sent messages have been acknowledged by the remote end.
		 *          This operation blocks until all messages have been
		 *          acknowledged.
		 *          Writing is not possible while the function is running even
		 *          from another thread.
		 *          Usually this function does not need to be called from
		 *          user code. It is there for completeness to allow clients
		 *          to be sure that all messages were handled by the server
		 *          at a certain point in time without having to disconnect.
		 * @return Result code
		 */
		virtual Result syncOutbox() = 0;

		/**
		 * @brief Disconnects gracefully from the broker. It sends a disconnect
		 *        message and wait for the receipt. In contrast to close, this
		 *        is the nice way to say 'good bye'.
		 * @return Result code
		 */
		virtual Result disconnect() = 0;

		/**
		 * @brief Checks for an active connection.
		 * @return True if a connection is established, false otherwise
		 */
		virtual bool isConnected() = 0;

		/**
		 * @brief Closes the underlying socket and resources. This is the
		 *        hard way to close a connection.
		 * @return Result code
		 */
		virtual Result close() = 0;

		/**
		 * @brief Sets the read timeout for receiving a message. If the
		 *        timeout hits then TimeoutError is returned.
		 * @param milliseconds The timeout in milliseconds. Zero or a negative
		 *                     value disabled the timeout.
		 * @return Result code
		 */
		virtual Result setTimeout(int milliseconds) = 0;

		/**
		 * @brief Returns the names of the available message groups.
		 * @details This call only returns valid content after a connection
		 *          was established. If a reconnect occurred during the
		 *          lifetime of the connection and the groups of the queue
		 *          have changed then this will be reflected in the returned
		 *          vector.
		 * @return The available groups
		 */
		const Groups &groups() const;

		bool erroneous() const;

		/**
		 * @brief Returns the error message corresponding to the last
		 *        erroneous call.
		 * @return The message or empty.
		 */
		const std::string &lastErrorMessage() const;

		const State &state() const;

		/**
		 * @brief Queries the local message inbox size. It is safe to call this
		 *        method from separate threads.
		 * @return The number of messages queued in the local inbox.
		 */
		size_t inboxSize() const;

		/**
		 * @brief Queries the local message outbox size. If a message is still
		 *        in the outbox does not mean that is hasn't been sent over the
		 *        wire. It just means that the server hasn't acknowledged the
		 *        message yet and that in case of a reconnect the outbox will
		 *        be sent again.
		 * @return The number of unacknowledged messages.
		 */
		virtual size_t outboxSize() const = 0;

		void setCertificate(const std::string &cert);

		static Core::Message *decode(const std::string &blob,
		                             ContentEncoding encoding,
		                             ContentType type);

		static Core::Message *decode(const char *blob, size_t blob_length,
		                             ContentEncoding encoding,
		                             ContentType type);

		static bool encode(std::string &blob, const Core::Message *msg,
		                   ContentEncoding encoding, ContentType type,
		                   int schemaVersion);


	// ----------------------------------------------------------------------
	//  Interruptible interface
	// ----------------------------------------------------------------------
	protected:
		virtual void handleInterrupt(int) override;


	// ----------------------------------------------------------------------
	//  Protected methods
	// ----------------------------------------------------------------------
	protected:
		void queuePacket(Packet *p);

		/**
		 * Clears all messages in the inbox. This method is not intended for
		 * public use. Note that it does not lock the read mutex.
		 */
		void clearInbox();


	// ----------------------------------------------------------------------
	//  Private types and members
	// ----------------------------------------------------------------------
	protected:
		using PacketQueue = std::deque<Packet*>;

		bool                   _wantMembershipInfo;
		Groups                 _groups;
		PacketQueue            _inbox;
		std::string            _errorMessage;
		State                  _state;
		std::string            _registeredClientName;
		Core::Version          _schemaVersion; //!< The schema version the
		                                       //!< server supports
		KeyValueStore          _extendedParameters;
		std::string            _certificate;   //!< Optional client certificate

		// Mutexes to synchronize read access from separate threads.
		mutable boost::mutex   _readMutex;
};


DEFINE_INTERFACE_FACTORY(Protocol);

#define REGISTER_CONNECTION_PROTOCOL(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Client::Protocol, Class> __##Class##InterfaceFactory__(Service)


inline Core::Version Protocol::schemaVersion() const {
	return _schemaVersion;
}

inline const Protocol::KeyValueStore &Protocol::extendedParameters() const {
	return _extendedParameters;
}

inline const std::string &Protocol::clientName() const {
	return _registeredClientName;
}

inline bool Protocol::erroneous() const {
	return !_errorMessage.empty();
}

inline const std::string &Protocol::lastErrorMessage() const {
	return _errorMessage;
}

inline const Protocol::State &Protocol::state() const {
	return _state;
}


}
}


#endif
