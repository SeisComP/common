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
#include <seiscomp/core/strings.h>
#include <seiscomp/system/pluginregistry.h>
#include <seiscomp/system/application.h>
#include <seiscomp/wired/devices/socket.h>
#include <seiscomp/broker/messageprocessor.h>

#include <openssl/err.h>
#include <openssl/x509.h>

#include "server.h"
#include "settings.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::System;
using namespace Seiscomp::Messaging;


/**
 * @brief The Master application class
 */
class Master : public Application {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Master(int argc, char** argv);


	// ----------------------------------------------------------------------
	//  Application main interface
	// ----------------------------------------------------------------------
	public:
		virtual bool init();
		virtual bool run();
		virtual void done();


	// ----------------------------------------------------------------------
	//  Application events
	// ----------------------------------------------------------------------
	protected:
		virtual bool validateParameters();
		virtual void handleInterrupt(int);


	private:
		void handleWiredTimeout();


	private:
		Broker::ServerPtr _server;
};


Master::Master(int argc, char** argv) : Application(argc, argv) {
	// Map configuration to settings
	bindSettings(&global);
}


bool Master::init() {
	if ( !Application::init() )
		return false;

	_server = new Broker::Server;

	for ( auto &queue : global.queues ) {
		auto q_item = _server->addQueue(queue.name,
		                                queue.maxPayloadSize);
		if ( !q_item ) {
			SEISCOMP_ERROR("Failed to add queue: %s", queue.name.c_str());
			return false;
		}

		q_item->acl = queue.acl;
		q_item->dbURL = queue.dbstore.driver + "://" + queue.dbstore.parameters;

		SEISCOMP_INFO("+ A %s", toString(queue.acl).c_str());
		SEISCOMP_INFO("+ Q %s", queue.name.c_str());

		auto q = q_item->queue;

		if ( queue.groups.empty() ) {
			queue.groups = global.defaultGroups;
		}

		for ( size_t g = 0; g < queue.groups.size(); ++g ) {
			Broker::Queue::Result r = q->addGroup(queue.groups[g]);
			if ( r != Broker::Queue::Success ) {
				SEISCOMP_ERROR("Failed to add group: %s/%s: %s",
				               queue.name.c_str(),
				               queue.groups[g].c_str(),
				               r.toString());
				return false;
			}
			else {
				SEISCOMP_INFO("  + G %s", queue.groups[g].c_str());
			}
		}

		for ( size_t p = 0; p < queue.messageProcessors.size(); ++p ) {
			string interface = queue.messageProcessors[p];
			Broker::MessageProcessorPtr proc = Broker::MessageProcessorFactory::Create(interface);
			if ( !proc ) {
				SEISCOMP_ERROR("Could not create message processor interface '%s'",
				               interface.c_str());
				return false;
			}

			SEISCOMP_INFO("  + MP %s", interface.c_str());
			if ( !proc->init(configuration(), "queues." + q->name() + ".processors.messages." + interface + ".") ) {
				SEISCOMP_ERROR("Failed to initialize message processor interface '%s'",
				               interface.c_str());
				return false;
			}

			if ( !q->add(proc.get()) ) {
				SEISCOMP_ERROR("Attaching message processor '%s' failed",
				               interface.c_str());
				return false;
			}
		}
	}

	if ( _server->numberOfQueues() == 0 ) {
		SEISCOMP_ERROR("No queues configured, exiting");
		return false;
	}

	return true;
}


bool Master::run() {
	if ( global.interface.bind.port > 0 ) {
		Wired::SocketPtr socket = new Wired::Socket;
		socket->setNoDelay(true);

		if ( socket->setReuseAddr(global.interface.socketPortReuse) != Wired::Socket::Success ) {
			SEISCOMP_ERROR("Unable to reuse port %d", global.interface.bind.port);
			return false;
		}

		Broker::WebsocketEndpoint *endpoint = new Broker::WebsocketEndpoint(_server.get(), socket.get(), global.interface.acl);
		if ( !_server->addEndpoint(global.interface.bind.address, global.interface.bind.port, endpoint) ) {
			delete endpoint;
			SEISCOMP_ERROR("Failed to bind to port %d", global.interface.bind.port);
			return false;
		}

		SEISCOMP_INFO("Bound unencrypted to %s:%d",
		              toString(global.interface.bind.address).c_str(),
		              global.interface.bind.port);
	}

	if ( (global.interface.ssl.bind.port > 0)
	  && !global.interface.ssl.key.empty()
	  && !global.interface.ssl.certificate.empty() ) {
		SSL_CTX *ctx = Wired::SSLSocket::createServerContext(global.interface.ssl.certificate.c_str(), global.interface.ssl.key.c_str());
		if ( !ctx ) {
			SEISCOMP_ERROR("Failed to create SSL server context");
			return false;
		}

		if ( global.interface.ssl.verifyPeer ) {
			// Allow self-signed certificates
			if ( !SSL_CTX_load_verify_locations(ctx, global.interface.ssl.certificate.c_str(), nullptr) ) {
				SEISCOMP_ERROR("Loading verification certificate failed");
				ERR_print_errors_cb([](const char *str, size_t len, void *) -> int {
					Core::trimBack(str, len);
					SEISCOMP_ERROR("%.*s", static_cast<int>(len), str);
					return 0;
				}, nullptr);
				SSL_CTX_free(ctx);
				return false;
			}

			SSL_CTX_set_verify(
				ctx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,
				[](int, X509_STORE_CTX *) -> int {
					return 1;
				}
			);
		}

		Wired::SSLSocketPtr socket = new Wired::SSLSocket(ctx);
		socket->setNoDelay(true);

		if ( socket->setReuseAddr(global.interface.ssl.socketPortReuse) != Wired::Socket::Success ) {
			SEISCOMP_ERROR("Unable to reuse port %d", global.interface.ssl.bind.port);
			return false;
		}

		Broker::WebsocketEndpoint *endpoint = nullptr;

		if ( global.interface.ssl.verifyPeer ) {
			endpoint = new Broker::SSLVerifyWebsocketEndpoint(_server.get(), socket.get(), global.interface.ssl.acl);
		}
		else {
			endpoint = new Broker::WebsocketEndpoint(_server.get(), socket.get(), global.interface.ssl.acl);
		}

		if ( !_server->addEndpoint(global.interface.ssl.bind.address, global.interface.ssl.bind.port, endpoint) ) {
			delete endpoint;
			SEISCOMP_ERROR("Failed to bind to port %d", global.interface.ssl.bind.port);
			return false;
		}

		SEISCOMP_INFO("Bound encrypted to %s:%d",
		              toString(global.interface.ssl.bind.address).c_str(),
		              global.interface.ssl.bind.port);
	}

	if ( !_server->init() ) {
		SEISCOMP_ERROR("Initialization failed");
		return false;
	}

	// Create statistics timer
	if ( !_server->setTimer(10, 0, std::bind(&Master::handleWiredTimeout, this)) ) {
		SEISCOMP_ERROR("Failed to add timer");
		return false;
	}

	return _server->run();
}


void Master::done() {
	if ( _server ) {
		_server->shutdown();
		_server = NULL;
	}

	Application::done();
}


bool Master::validateParameters() {
	if ( !Application::validateParameters() )
		return false;

	set<string> additionalPlugins;
	for ( size_t i = 0; i < global.queues.size(); ++i ) {
		for ( size_t j = 0; j < global.queues[i].plugins.size(); ++j ) {
			additionalPlugins.insert(global.queues[i].plugins[j]);
		}
	}

	if ( !additionalPlugins.empty() ) {
		// Patch the configuration object and inject our additional plugins.
		vector<string> configuredPlugins;
		try { configuredPlugins = _configuration.getStrings("plugins"); }
		catch ( ... ) {}
		configuredPlugins.insert(configuredPlugins.end(), additionalPlugins.begin(), additionalPlugins.end());
		_configuration.setStrings("plugins", configuredPlugins);
	}

	return true;
}


void Master::handleInterrupt(int) {
	_server->stop();
}


void Master::handleWiredTimeout() {
	_server->createStatisticsSnapshot();
}


int main(int argc, char **argv) {
	return Master(argc, argv)();
}
