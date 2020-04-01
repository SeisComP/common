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


#ifndef SEISCOMP_APPS_SCWSAS_SETTINGS_H__
#define SEISCOMP_APPS_SCWSAS_SETTINGS_H__


#include <seiscomp/system/application.h>
#include <seiscomp/system/environment.h>


namespace Seiscomp {
namespace Applications {
namespace Wfas {


// Define default configuration
struct Settings : System::Application::AbstractSettings {
	Settings() {
		filebase = Environment::Instance()->installDir() + "/var/lib/archive";
	}

	struct Arclink {
		Arclink() {
			port = -1;
		}

		int         port;

		void accept(System::Application::SettingsLinker &linker) {
			linker
			& cfg(port, "port")
			& cli(port,
			      "Server", "arclink-port",
			      "Port to listen to for data requests with Arclink protocol",
			      true);
		}
	} arclink;

	struct FDSNWS {
		FDSNWS() {
			port = 8080;
			baseUrl = "http://localhost:8080/fdsnws";
			maxTimeWindow = 0;
		}

		int         port;
		std::string baseUrl;
		int         maxTimeWindow;

		void accept(System::Application::SettingsLinker &linker) {
			linker
			& cfg(port, "port")
			& cli(port, "Server", "fdsnws-port",
			      "Port to listen to for data requests with FDSNWS protocol",
			      true)
			& cfg(baseUrl, "baseUrl")
			& cli(baseUrl, "Server", "fdsnws-baseurl",
			      "The base URL for the FDSNWS service",
			      true)
			& cfg(maxTimeWindow, "maxTimeWindow");
		}
	} fdsnws;

	std::string sdsBackend;
	std::string dcid;
	std::string filebase;

	virtual void accept(System::Application::SettingsLinker &linker) {
		linker
		& cfg(arclink, "arclink")
		& cfg(fdsnws, "fdsnws")
		& cfg(sdsBackend, "handlerSDS")
		& cfgAsPath(filebase, "filebase");
	}
};


extern Settings global;


}
}
}


#endif
