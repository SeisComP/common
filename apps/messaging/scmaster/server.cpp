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


#define SEISCOMP_COMPONENT BROKER

#include <seiscomp/logging/log.h>
#include <seiscomp/io/archive/jsonarchive.h>

#include <iostream>
#include <functional>

#include "server.h"
#include "protocols/websocket.h"


using namespace std;


namespace Seiscomp {
namespace Messaging {
namespace Broker {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QueueWorker::QueueWorker(Queue *queue)
: _queue(queue) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QueueWorker::lock() {
	_idleMutex.lock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QueueWorker::unlock() {
	_idleMutex.unlock();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QueueWorker::addSession(Wired::Session *session) {
	if ( !Wired::Reactor::addSession(session) ) {
		return false;
	}

	static_cast<Protocols::WebsocketSession*>(session)->welcome();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Wired::Device *QueueWorker::wait() {
	unlock();
	Wired::Device *dev = Reactor::wait();
	lock();
	return dev;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QueueWorker::idle() {
	// If an interrupt occured, flush the messages.
	if ( interrupted() ) {
		flushMessages(_queue);

		while ( !_messages.empty() ) {
			auto item = _messages.front();
			_messages.pop_front();
			unlock();
			if ( _queue->push(item.first, item.second) != Queue::Success ) {
				delete item.second;
			}
			lock();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QueueWorker::run() {
	if ( !setTimer(1, 0, bind(&Queue::timeout, _queue)) ) {
		SEISCOMP_ERROR("[worker@%s] Failed to add timer", _queue->name().c_str());
		return false;
	}

	lock();
	bool r = Reactor::run();
	unlock();
	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QueueWorker::sendMessage(Client *sender, Message *message) {
	lock();
	_messages.push_back(ClientMessage(sender, message));
	unlock();
	interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QueueWorker::messageAvailable(Queue *q) {
	assert(_queue == q);
	interrupt();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Server::Server() {
	// Queue one hour of statistics
	_serverStats.reserve(360);

	setTriggerMode(Wired::DeviceGroup::LevelTriggered);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Server::~Server() {
	for ( auto &item : _queues ) {
		delete item.second.worker;
		delete item.second.queue;
	}

	for ( auto &item : _databases ) {
		delete item.reactor;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Server::QueueItem *Server::addQueue(const std::string &name, uint64_t maxPayloadSize) {
	if ( _queues.find(name) != _queues.end() )
		return nullptr;

	Queue *newQueue = new Queue(name, maxPayloadSize);
	QueueItem &item = _queues[name];
	item.queue = newQueue;
	item.worker = new QueueWorker(item.queue);
	item.worker->setTriggerMode(Wired::DeviceGroup::LevelTriggered);
	item.thread = nullptr;
	item.queue->setMessageDispatcher(item.worker);

	return &item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


const char *makeString(const Wired::Socket::IPAddress &addr) {
	static char buf[Wired::Socket::IPAddress::MAX_IP_STRING_LEN];
	addr.toString(buf);
	return buf;
}


}

Queue *Server::getQueue(const std::string &name,
                        const Wired::Socket::IPAddress &remoteAddress) const {
	Queues::const_iterator it = _queues.find(name);
	if ( it == _queues.end() )
		return nullptr;

	if ( !it->second.acl.check(remoteAddress) ) {
		SEISCOMP_INFO("Access blocked to queue %s for IP %s",
		              name.c_str(), makeString(remoteAddress));
		return nullptr;
	}

	return it->second.queue;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Server::QueueItem *Server::getQueue(const std::string &name) const {
	auto it = _queues.find(name);
	if ( it == _queues.end() )
		return nullptr;

	return &it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Server::numberOfQueues() const {
	return _queues.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::createStatisticsSnapshot() {
	ServerStatisticsPtr stats = new ServerStatistics;

	stats->timestamp = Core::Time::UTC();
	stats->queues.resize(numberOfQueues());

	lockStatistics();

	_cummulatedStats.queues.resize(numberOfQueues());

	size_t idx;
	Queues::iterator it;
	for ( it = _queues.begin(), idx = 0; it != _queues.end(); ++it, ++idx ) {
		it->second.worker->lock();
		it->second.queue->getStatisticsSnapshot(stats->queues[idx], true);
		it->second.worker->unlock();

		_cummulatedStats.queues[idx] += stats->queues[idx];

		stats->messages += stats->queues[idx].messages;
		stats->bytes += stats->queues[idx].bytes;
		stats->payload += stats->queues[idx].payload;
	}

	if ( _serverStats.empty() )
		stats->sequenceNumber = 0;
	else
		stats->sequenceNumber = _serverStats.back()->sequenceNumber + 1;

	_cummulatedStats.sequenceNumber = -1;
	_cummulatedStats.timestamp = stats->timestamp;
	_cummulatedStats.messages += stats->messages;
	_cummulatedStats.bytes += stats->bytes;
	_cummulatedStats.payload += stats->payload;

	_serverStats.push_back(stats);

	/*
	IO::JSONArchive json;
	json.create("-");
	json.setFormattedOutput(true);
	json << NAMED_OBJECT("stats", *stats);
	json.close();
	*/

	unlockStatistics();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ServerStatistics &Server::cummulatedStatistics() {
	return _cummulatedStats;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Server::init() {
	if ( !Wired::Server::init() )
		return false;

	// Start all reactor threads
	Queues::iterator it;
	for ( it = _queues.begin(); it != _queues.end(); ++it ) {
		QueueItem &item = it->second;
		item.queue->activate();
		if ( !item.worker->setup() )
			return false;

		item.thread = new thread(bind(&QueueWorker::run, item.worker));
	}

	createStatisticsSnapshot();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::shutdown() {
	Wired::Server::shutdown();

	for ( auto it = _queues.begin(); it != _queues.end(); ++it ) {
		SEISCOMP_DEBUG("Shutdown sequence for queue %s", it->first.c_str());
		QueueItem &item = it->second;
		SEISCOMP_DEBUG("* Shutdown worker");
		item.worker->shutdown();
		SEISCOMP_DEBUG("* Wait for thread");
		if ( item.thread ) {
			item.thread->join();
			delete item.thread;
			item.thread = nullptr;
		}

		SEISCOMP_DEBUG("* Shutdown queue itself");
		item.queue->shutdown();
	}

	for ( auto &item : _databases ) {
		SEISCOMP_DEBUG("Shutdown sequence for database handler");
		SEISCOMP_DEBUG("* Shutdown worker");
		item.reactor->shutdown();
		SEISCOMP_DEBUG("* Wait for thread");
		if ( item.thread ) {
			item.thread->join();
			delete item.thread;
			item.thread = nullptr;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::clear() {
	Wired::Server::clear();

	createStatisticsSnapshot();

	double plmb = _cummulatedStats.payload.received / 1024.0 / 1024.0;
	double dmb = _cummulatedStats.bytes.received / 1024.0 / 1024.0;

	std::cerr << "[Reception]" << std::endl
	          << "MSG : " << _cummulatedStats.messages.received << std::endl
	          << "DATA: " << _cummulatedStats.bytes.received << std::endl
	          << "BODY: " << _cummulatedStats.payload.received << std::endl
	          << "ERAT: " << (plmb*100/dmb) << "%" << std::endl;

	plmb = _cummulatedStats.payload.sent / 1024.0 / 1024.0;
	dmb = _cummulatedStats.bytes.sent / 1024.0 / 1024.0;

	std::cerr << "[Dispatch]" << std::endl
	          << "MSG : " << _cummulatedStats.messages.sent << std::endl
	          << "DATA: " << _cummulatedStats.bytes.sent << std::endl
	          << "BODY: " << _cummulatedStats.payload.sent << std::endl
	          << "ERAT: " << (plmb*100/dmb) << "%" << std::endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Server::sessionTagged(Wired::Session *session) {
	Protocols::WebsocketSession *ws;
	ws = static_cast<Protocols::WebsocketSession*>(session);

	session->setTag(false);

	auto q = ws->queue();

	if ( q ) {
		auto it = _queues.find(q->name());
		if ( it == _queues.end() )
			return;

		if ( it->second.worker ) {
			// Move session to the corresponding queue worker
			moveTo(it->second.worker, session);
			it->second.worker->interrupt();
		}
	}
	else {
		// Database WS session
		if ( _databases.empty() ) {
			DatabaseItem db;
			db.reactor = new Wired::Reactor;
			if ( !db.reactor->setup() ) {
				delete db.reactor;
				SEISCOMP_ERROR("Setup database handler thread failed, skip connection");
				ws->sendStatus(Wired::HTTP_503);
				ws->invalidate();
				return;
			}

			db.thread = new thread(bind(&Wired::Reactor::run, db.reactor));
			_databases.push_back(db);
		}

		moveTo(_databases.back().reactor, session);
		_databases.back().reactor->interrupt();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WebsocketEndpoint::WebsocketEndpoint(Server *server, Wired::Socket *socket, Wired::IPACL &allowAddresses)
: Wired::AccessControlledEndpoint(socket, allowAddresses, Wired::IPACL())
, _server(server) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Wired::Session *WebsocketEndpoint::createSession(Wired::Socket *socket) {
	return new Protocols::WebsocketSession(socket, _server);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


class SSLWebsocketSession : public Protocols::WebsocketSession {
	public:
		SSLWebsocketSession(Wired::Socket *sock, Broker::Server *server)
		: Protocols::WebsocketSession(sock, server) {}

	public:
		void accepted() override {
			auto socket = static_cast<Wired::SSLSocket*>(device());
			setAuthenticated(socket->peerCertificate());
			if ( isAuthenticated() ) {
				long ret = SSL_get_verify_result(socket->ssl());
				switch( ret ) {
					case X509_V_OK:
					{
						setAuthorized(true);
						break;
					}
					case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
					{
						setAuthorized(true);
						char buf[Wired::Socket::IPAddress::MAX_IP_STRING_LEN];
						socket->address().toString(buf);
						SEISCOMP_WARNING(
							"[%s:%d] %s",
							buf, socket->port(),
							X509_verify_cert_error_string(SSL_get_verify_result(socket->ssl()))
						);
						break;
					}
					default:
					{
						setAuthorized(false);
						char buf[Wired::Socket::IPAddress::MAX_IP_STRING_LEN];
						socket->address().toString(buf);
						SEISCOMP_WARNING(
							"[%s:%d] %s",
							buf, socket->port(),
							X509_verify_cert_error_string(SSL_get_verify_result(socket->ssl()))
						);
					}
				}
			}
			else {
				char buf[Wired::Socket::IPAddress::MAX_IP_STRING_LEN];
				socket->address().toString(buf);
				SEISCOMP_WARNING(
					"[%s:%d] no authentication certificate provided",
					buf, socket->port()
				);
				setAuthorized(false);
			}
		}
};


}


SSLVerifyWebsocketEndpoint::SSLVerifyWebsocketEndpoint(Server *server,
                                                       Wired::SSLSocket *socket,
                                                       Wired::IPACL &allowAddresses)
: WebsocketEndpoint(server, socket, allowAddresses) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Wired::Session *SSLVerifyWebsocketEndpoint::createSession(Wired::Socket *socket) {
	return new SSLWebsocketSession(socket, _server);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
