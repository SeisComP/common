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
		virtual Wired::Device *wait();
		virtual void idle();
		virtual bool run();


	// ----------------------------------------------------------------------
	//  MessageDispatcher interface
	// ----------------------------------------------------------------------
	public:
		virtual void sendMessage(Client *sender, Message *message);
		virtual void messageAvailable(Queue *);


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
		Queue *addQueue(const std::string &name, uint64_t maxPayloadSize,
		                const Wired::IPACL &acl);
		Queue *getQueue(const std::string &name,
		                Client *enquirer,
		                const Wired::Socket::IPAddress &remoteAddress) const;

		size_t numberOfQueues() const;

		void createStatisticsSnapshot();
		ServerStatistics &cummulatedStatistics();

		void lockStatistics() { _statsMutex.lock(); }
		void unlockStatistics() { _statsMutex.unlock(); }


	// ----------------------------------------------------------------------
	//  Reactor interface
	// ----------------------------------------------------------------------
	public:
		virtual bool init();
		virtual void shutdown();
		virtual void clear();

		virtual void sessionTagged(Wired::Session *session);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		struct QueueItem {
			Queue          *queue;
			Wired::IPACL    acl;
			QueueWorker    *worker;
			std::thread    *thread;
		};

		typedef std::map<std::string, QueueItem> Queues;
		Queues                               _queues;
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
		virtual Wired::Session *createSession(Wired::Socket *socket);


	private:
		Server *_server;
};


}
}
}


#endif
