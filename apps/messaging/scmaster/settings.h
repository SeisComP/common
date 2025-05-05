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


#ifndef SEISCOMP_SCMASTER_SETTINGS_H__
#define SEISCOMP_SCMASTER_SETTINGS_H__


#include <seiscomp/system/application.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/wired/ipacl.h>
#include <seiscomp/wired/server.h>

#include <functional>


// Maximum 1 megabyte of message size
#define DEFAULT_MAX_WS_PAYLOAD_SIZE 1*1024*1024


// Define default configuration
struct Settings : Seiscomp::System::Application::AbstractSettings {
	std::vector<std::string> defaultGroups;

	struct Queue {
		Queue() : maxPayloadSize(DEFAULT_MAX_WS_PAYLOAD_SIZE) {}
		std::string              name;
		std::vector<std::string> groups;
		Seiscomp::Wired::IPACL   acl; // Default 0.0.0.0/0
		std::vector<std::string> plugins;
		unsigned int             maxPayloadSize;
		std::vector<std::string> messageProcessors;

		struct DB {
			std::string driver;
			std::string parameters;

			void accept(Seiscomp::System::Application::SettingsLinker &linker) {
				linker & cfg(driver, "driver") & cfg(parameters, "read");
			}
		} dbstore;

		void accept(Seiscomp::System::Application::SettingsLinker &linker) {
			linker
			& key(name)
			& cfg(groups, "groups")
			& cfg(acl, "acl")
			& cfg(plugins, "plugins")
			& cfg(maxPayloadSize, "maxPayloadSize")
			& cfg(messageProcessors, "processors.messages")
			& cfg(dbstore, "processors.messages.dbstore");
		}
	};

	std::vector<Queue> queues;

	struct Interface {
		Interface()
		: bind(Seiscomp::Wired::Socket::IPAddress(0,0,0,0), 18180)
		, socketPortReuse(true) {}

		Seiscomp::Wired::BindAddress bind;
		Seiscomp::Wired::IPACL       acl; // Default empty
		bool                         socketPortReuse;

		struct SSL {
			SSL() {}

			Seiscomp::Wired::BindAddress bind;
			Seiscomp::Wired::IPACL       acl;
			bool                         socketPortReuse{true};
			std::string                  key;
			std::string                  certificate;
			bool                         verifyPeer{false};

			void accept(Seiscomp::System::Application::SettingsLinker &linker) {
				linker
				& cfg(bind, "bind")
				& cli(bind, "Wired", "sbind", "The encrypted bind address in format [ip:]port")
				& cfg(acl, "acl")
				& cfg(socketPortReuse, "socketPortReuse")
				& cfgAsPath(key, "key")
				& cfgAsPath(certificate, "certificate")
				& cfg(verifyPeer, "verifyPeer");
			}
		} ssl;

		void accept(Seiscomp::System::Application::SettingsLinker &linker) {
			linker
			& cfg(bind, "bind")
			& cli(bind, "Wired", "bind", "The non encrypted bind address in format [ip:]port")
			& cfg(acl, "acl")
			& cfg(socketPortReuse, "socketPortReuse")
			& cfg(ssl, "ssl");
		}
	} interface;

	struct HTTP {
		HTTP()
		: filebase(Seiscomp::Environment::Instance()->absolutePath("@DATADIR@/scmaster/http/"))
		, staticPath("/"), brokerPath("/") {}

		std::string filebase;
		std::string staticPath;
		std::string brokerPath;

		void accept(Seiscomp::System::Application::SettingsLinker &linker) {
			linker
			& cfgAsPath(filebase, "filebase")
			& cfg(staticPath, "staticPath")
			& cfg(brokerPath, "brokerPath");
		}
	} http;

	virtual void accept(Seiscomp::System::Application::SettingsLinker &linker) {
		linker
		& cfg(defaultGroups, "defaultGroups")
		& cfg(queues, "queues", [this](Queue &queue) { queue.groups = defaultGroups; })
		& cfg(interface, "interface")
		& cfgAsPath(http, "http");
	}
};


extern Settings global;


#endif
