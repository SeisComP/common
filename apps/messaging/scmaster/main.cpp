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
#include <seiscomp/system/pluginregistry.h>
#include <seiscomp/system/application.h>
#include <seiscomp/wired/devices/socket.h>
#include <seiscomp/utils/timer.h>
#include <seiscomp/broker/messageprocessor.h>

#include "server.h"
#include "settings.h"
#include "timer.h"


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

	for ( size_t i = 0; i < global.queues.size(); ++i ) {
		Broker::Queue *q = _server->addQueue(global.queues[i].name,
		                                     global.queues[i].maxPayloadSize,
		                                     global.queues[i].acl);
		if ( q == NULL ) {
			SEISCOMP_ERROR("Failed to add queue: %s", global.queues[i].name.c_str());
			return false;
		}

		SEISCOMP_INFO("+ A %s", toString(global.queues[i].acl).c_str());
		SEISCOMP_INFO("+ Q %s", global.queues[i].name.c_str());

		if ( global.queues[i].groups.empty() )
			global.queues[i].groups = global.defaultGroups;

		for ( size_t g = 0; g < global.queues[i].groups.size(); ++g ) {
			Broker::Queue::Result r = q->addGroup(global.queues[i].groups[g]);
			if ( r != Broker::Queue::Success ) {
				SEISCOMP_ERROR("Failed to add group: %s/%s: %s",
				               global.queues[i].name.c_str(),
				               global.queues[i].groups[g].c_str(),
				               r.toString());
				return false;
			}
			else
				SEISCOMP_INFO("  + G %s", global.queues[i].groups[g].c_str());
		}

		for ( size_t p = 0; p < global.queues[i].messageProcessors.size(); ++p ) {
			string interface = global.queues[i].messageProcessors[p];
			Broker::MessageProcessorPtr proc = Broker::MessageProcessorFactory::Create(interface);
			if ( !proc ) {
				SEISCOMP_ERROR("Could not create message processor interface '%s'",
				               interface.c_str());
				return false;
			}

			if ( !proc->init(configuration(), "queues." + q->name() + ".processors.messages." + interface + ".") ) {
				SEISCOMP_ERROR("Failed to initialize message processor interface '%s'",
				               interface.c_str());
				return false;
			}

			SEISCOMP_INFO("  + MP %s", interface.c_str());
			q->add(proc.get());
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
		if ( ctx == NULL ) {
			SEISCOMP_ERROR("Failed to create SSL server context");
			return false;
		}

		Wired::SocketPtr socket = new Wired::SSLSocket(ctx);
		socket->setNoDelay(true);

		if ( socket->setReuseAddr(global.interface.ssl.socketPortReuse) != Wired::Socket::Success ) {
			SEISCOMP_ERROR("Unable to reuse port %d", global.interface.ssl.bind.port);
			return false;
		}

		Broker::WebsocketEndpoint *endpoint = new Broker::WebsocketEndpoint(_server.get(), socket.get(), global.interface.ssl.acl);
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
	Wired::TimerSession *timer = new Wired::TimerSession(boost::bind(&Master::handleWiredTimeout, this));
	if ( !timer->setInterval(10,0) ) {
		delete timer;
		SEISCOMP_ERROR("Failed to initialize timer");
		return false;
	}

	if ( !_server->addSession(timer) ) {
		delete timer;
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
