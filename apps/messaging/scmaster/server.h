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


#ifndef SEISCOMP_SCMASTER_SERVER_H__
#define SEISCOMP_SCMASTER_SERVER_H__


#include <seiscomp/broker/queue.h>
#include <seiscomp/broker/messagedispatcher.h>
#include <seiscomp/broker/message.h>
#include <seiscomp/broker/client.h>

#include <seiscomp/wired/endpoint.h>
#include <seiscomp/wired/server.h>

#include "statistics/statistics.h"

#include <deque>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


class QueueWorker : public Wired::Reactor, public MessageDispatcher {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		QueueWorker(Queue *queue);


	public:
		/**
		 * @brief Acquires a lock to the idle function.
		 *
		 * This is necessary if structures needs to be accessed from within
		 * another thread. This function is blocking until the lock is acquired.
		 */
		void lock();

		/**
		 * @brief Releases the lock to the idle function.
		 */
		void unlock();


	// ----------------------------------------------------------------------
	//  Reactor interface
	// ----------------------------------------------------------------------
	public:
		bool addSession(Wired::Session *session) override;
		Wired::Device *wait() override;
		void idle() override;
		bool run() override;


	// ----------------------------------------------------------------------
	//  MessageDispatcher interface
	// ----------------------------------------------------------------------
	public:
		void sendMessage(Client *sender, Message *message) override;
		void messageAvailable(Queue *) override;


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		using ClientMessage = std::pair<Client*,Message*>;

		Queue                     *_queue;
		std::mutex                 _idleMutex;
		std::deque<ClientMessage>  _messages;
};



DEFINE_SMARTPOINTER(Server);
class Server : public Wired::Server {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Server();
		//! D'tor
		~Server();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		struct QueueItem {
			Queue          *queue{nullptr};
			Wired::IPACL    acl;
			std::string     dbURL;
			QueueWorker    *worker{nullptr};
			std::thread    *thread{nullptr};
		};

		struct DatabaseItem {
			Wired::Reactor *reactor{nullptr};
			std::thread    *thread{nullptr};
		};

		QueueItem *addQueue(const std::string &name, uint64_t maxPayloadSize);
		Queue *getQueue(const std::string &name,
		                const Wired::Socket::IPAddress &remoteAddress) const;
		const QueueItem *getQueue(const std::string &name) const;

		size_t numberOfQueues() const;

		void createStatisticsSnapshot();
		ServerStatistics &cummulatedStatistics();

		void lockStatistics() { _statsMutex.lock(); }
		void unlockStatistics() { _statsMutex.unlock(); }


	// ----------------------------------------------------------------------
	//  Reactor interface
	// ----------------------------------------------------------------------
	public:
		bool init() override;
		void shutdown() override;
		void clear() override;

		void sessionTagged(Wired::Session *session) override;


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		using Queues = std::map<std::string, QueueItem>;
		using DatabaseWorkers = std::list<DatabaseItem>;

		Queues                               _queues;
		DatabaseWorkers                      _databases;
		circular_buffer<ServerStatisticsPtr> _serverStats;
		ServerStatistics                     _cummulatedStats;
		std::mutex                           _statsMutex;
};


class WebsocketEndpoint : public Wired::AccessControlledEndpoint {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		WebsocketEndpoint(Server *server, Wired::Socket *socket,
		                  Wired::IPACL &allowAddresses);


	// ----------------------------------------------------------------------
	//  Acceptor interface
	// ----------------------------------------------------------------------
	protected:
		Wired::Session *createSession(Wired::Socket *socket) override;


	protected:
		Server *_server;
};


class SSLVerifyWebsocketEndpoint : public WebsocketEndpoint {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SSLVerifyWebsocketEndpoint(Server *server, Wired::SSLSocket *socket,
		                           Wired::IPACL &allowAddresses);


	// ----------------------------------------------------------------------
	//  Acceptor interface
	// ----------------------------------------------------------------------
	protected:
		Wired::Session *createSession(Wired::Socket *socket) override;


};


}
}
}


#endif
