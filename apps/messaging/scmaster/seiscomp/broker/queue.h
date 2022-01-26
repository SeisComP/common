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


#ifndef SEISCOMP_BROKER_QUEUE_H__
#define SEISCOMP_BROKER_QUEUE_H__


#include <seiscomp/core/enumeration.h>
#include <seiscomp/core/message.h>

#include <seiscomp/broker/messageprocessor.h>
#include <seiscomp/broker/hashset.h>
#include <seiscomp/broker/group.h>
#include <seiscomp/broker/message.h>
#include <seiscomp/broker/statistics.h>

#include <seiscomp/broker/utils/utils.h>
#include <seiscomp/broker/utils/circular.h>

#include <thread>
#include <vector>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


class Client;
class MessageDispatcher;

DEFINE_SMARTPOINTER(MessageProcessor);


/**
 * @brief The Queue class implements the central messaging service.
 *
 * The Queue receives messages, queues them and distributes them to subscribed
 * clients.
 */
class SC_BROKER_API Queue {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		using StringList = std::vector<std::string>;
		using MessageProcessors = std::vector<MessageProcessorPtr>;

		using KeyValueCStrPair = MessageProcessor::KeyValueCStrPair;
		using KeyCStrValues = MessageProcessor::KeyCStrValues;

		using KeyValuePair = MessageProcessor::KeyValuePair;
		using KeyValues = MessageProcessor::KeyValues;

		enum Constants {
			MaxAdditionalParams = MessageProcessor::MaxAdditionalParams
		};

		MAKEENUM(
			Result,
			EVALUES(
				Success,
				InternalError,
				ClientNameNotUnique,
				ClientNotAccepted,
				GroupNameNotUnique,
				GroupDoesNotExist,
				GroupAlreadySubscribed,
				GroupNotSubscribed,
				MessageNotAccepted,
				MessageDecodingFailed,
				MessageEncodingFailed,
				NotEnoughClientHeap
			),
			ENAMES(
				"Success",
				"Internal error",
				"Client name is not unique",
				"Client was not accepted",
				"Group name is not unique",
				"Group does not exist",
				"Already subscribed to group",
				"Not subscribed to group",
				"Message not accepted",
				"Message could not be decoded",
				"Message could not be encoded",
				"Not enough client heap"
			)
		);

		const std::string StatusGroup = "STATUS_GROUP";


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Queue(const std::string &name, uint64_t maxPayloadSize);
		~Queue();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @return The queue name
		 */
		const std::string &name() const;

		/**
		 * @brief Adds a message processor to the list of processors.
		 * @param proc The processor instance which is managed by the queue
		 *             with a smart pointer.
		 * @return Success flag
		 */
		bool add(MessageProcessor *proc);

		/**
		 * @brief Adds a group/topic to the queue.
		 * @param name The name of the group
		 * @return true on success, false otherwise
		 */
		Result addGroup(const std::string &name);

		/**
		 * @brief Returns a list of available group names
		 * @return The list of names
		 */
		const StringList &groups() const { return _groupNames; }

		/**
		 * @brief Return the sender name of the queue.
		 * @return A NULL terminated const string
		 */
		const char *senderName() const;

		/**
		 * @brief Sets the message dispatcher for thread synchronisation.
		 *
		 * The queue runs a thread to process messages via plugins. If the
		 * message is processed the thread notifies the queue about it. The
		 * queue could now call publish but that is probably not thread-safe
		 * and inefficient to implement on each subscriber. The message
		 * dispatcher receives a notification about a new message and can then
		 * implement any inter-thread communication to publish the message in
		 * the same context as it has been created.
		 *
		 * @param dispatcher The dispatcher instance not managed by the queue.
		 */
		void setMessageDispatcher(MessageDispatcher *dispatcher);

		/**
		 * @brief Subscribe a client to a particular group
		 * @param client The client
		 * @param group The name of the group
		 * @return The result code
		 */
		Result subscribe(Client *client, const std::string &group);

		/**
		 * @brief Unsubscribes a client from a particular group
		 * @param client The client
		 * @param group The name of the group
		 * @return The result code
		 */
		Result unsubscribe(Client *client, const std::string &group);

		/**
		 * @brief Returns a buffered message after a particular sequence number
		 * @param sequenceNumber The sequence number to continue with.
		 *
		 *        The returned message must have a sequence number greater than
		 *        this parameter or lower if a wrap has occured but never the
		 *        same.
		 * @param client The client instance to filter subscriptions for
		 * @return A message pointer or NULL if no message is available
		 */
		Message *getMessage(SequenceNumber sequenceNumber,
		                    const Client *client) const;

		/**
		 * @brief Pushes a message from a client to the queue
		 *
		 * This method is called from Client subclasses that received a message
		 * through their transport protocol. The message pointer will either
		 * be managed in a smart pointer or deleted. If false is returned the
		 * caller must take care of deleting the message.
		 *
		 * @param sender The sender instance
		 * @param msg The message
		 * @param packetLength The size in bytes of the received packet including
		 *                     protocol specific header data. This is only
		 *                     used for statistics.
		 * @return The result code
		 */
		Result push(Client *sender, Message *msg, int packetSize = 0);

		/**
		 * @brief Activates the queue and starts the processing thread.
		 */
		void activate();

		/**
		 * @brief Shutdown the queue and finished the processing thread if
		 *        running.
		 *
		 * This will also shutdown all processors associated with the queue.
		 *
		 * Note that this call can block depending how many plugins are
		 * running and currently processing a message. This method waits until
		 * the processing thread is finished.
		 */
		void shutdown();

		/**
		 * @brief Callback to notify the queue about some timeout.
		 *
		 * This function is used to check expiration of outstanding
		 * acknowledgement messages. This function is not thread-safe and
		 * must be called from within the thread the queue is running in.
		 */
		void timeout();

		/**
		 * @brief Populates the passed statistics structure.
		 * @param stats[out] The target structure
		 * @param reset[in] Whether to reset the internal statistics or not.
		 */
		void getStatisticsSnapshot(QueueStatistics &stats, bool reset = true);


	// ----------------------------------------------------------------------
	//  Client memory interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Allocates additional client heap. Once allocated the heap
		 *        cannot be free'd anymore. This is mainly used for plugins
		 *        that are initialized once and need to store additional
		 *        data in a client structure.
		 * @param bytes The number of bytes to allocate
		 * @return An offset to the local client heap or a negative number
		 *         in case of an error. The absolute value (-result) of the
		 *         error translates to a status code (@Result).
		 */
		int allocateClientHeap(int bytes);


	// ----------------------------------------------------------------------
	//  Publisher interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Registers a client in the queue and sets up the PubSub
		 *        connections.
		 *
		 * This is called when the client calls connect and is part of the
		 * PublisherBase interface.
		 * @param client The client to be registered
		 * @param slot The slot
		 * @return The result code
		 */
		Result connect(Client *client, const KeyCStrValues params, int paramCount,
		               KeyValues &outParams);

		/**
		 * @brief Deregisters a client from the queue and clears the PubSub
		 *        connections.
		 *
		 * This is called when the client calls disconnect and is part of the
		 * PublisherBase interface.
		 * @param client The client to be deregistered
		 * @return The result code
		 */
		Result disconnect(Client *client);


	// ----------------------------------------------------------------------
	//  Settings interface
	// ----------------------------------------------------------------------
	public:
		uint64_t maxPayloadSize() const;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		using ProcessingTask = std::pair<Client*,Message*>;
		using TaskQueue = Utils::BlockingDequeue<ProcessingTask>;

		/**
		 * @brief Publishes a message from a client to all registered clients
		 *
		 * This method is called from Client subclasses that received a message
		 * through their transport protocol.
		 *
		 * @param sender The sender instance
		 * @param msg The message
		 * @return true on success, false otherwise
		 */
		bool publish(Client *sender, Message *msg);

		/**
		 * @brief Pops all messages from the processing queue and publishes them.
		 *
		 * This call does not block.
		 */
		void flushProcessedMessages();

		/**
		 * @brief The processing loop running in a different thread.
		 */
		void processingLoop();

		/**
		 * @brief Processes a message e.g. via plugins.
		 * @param task The task to be processed
		 */
		void process(ProcessingTask &task);

		/**
		 * @brief Called from the processing thread informing the queue that
		 *        the message is processed and can be forwarded to clients.
		 * @param task The task
		 */
		void taskReady(const ProcessingTask &task);

		/**
		 * @brief Replaces the incoming message with a response
		 * @param task The task to be updated
		 */
		void returnToSender(Message *msg, Core::BaseObject *obj);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		using Groups = std::map<std::string, GroupPtr>;
		using MessageRing = circular_buffer<MessagePtr>;
		using ClientNames = KHashSet<const char*>;
		using Clients = KHashMap<const char*, Client*>;

		std::string          _name;
		MessageProcessors    _processors;
		MessageProcessors    _connectionProcessors;
		MessageProcessors    _messageProcessors;
		MessageDispatcher   *_processedMessageDispatcher;
		SequenceNumber       _sequenceNumber;
		Groups               _groups;
		StringList           _groupNames;
		MessageRing          _messages;
		Clients              _clients;
		std::thread         *_messageProcessor;
		TaskQueue            _tasks;
		TaskQueue            _results;
		Core::Time           _created;
		Core::Time           _lastSOHTimestamp;
		int                  _allocatedClientHeap;
		int                  _sohInterval;
		int                  _inactivityLimit;
		uint64_t             _maxPayloadSize;
		mutable Tx           _txMessages;
		mutable Tx           _txBytes;
		mutable Tx           _txPayload;


	friend class MessageDispatcher;
};


inline const std::string &Queue::name() const {
	return _name;
}


inline uint64_t Queue::maxPayloadSize() const {
	return _maxPayloadSize;
}


}
}
}


#endif
