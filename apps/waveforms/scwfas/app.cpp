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


#define SEISCOMP_COMPONENT WFAS
#include <seiscomp/logging/log.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/utils/files.h>

#include <boost/bind.hpp>

#include "app.h"
#include "settings.h"


using namespace Seiscomp;
using namespace std;


namespace Seiscomp {
namespace Applications {
namespace Wfas {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Application(int argc, char** argv)
: Client::Application(argc, argv) {
	setMessagingEnabled(false);
	setDatabaseEnabled(false, false);
	bindSettings(&global);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::init() {
	if ( !Client::Application::init() )
		return false;

	SEISCOMP_DEBUG("Filebase: %s", global.filebase.c_str());
	global.dcid = agencyID();

	_server.setTriggerMode(Wired::DeviceGroup::LevelTriggered);

	Wired::IPACL globalAllow, globalDeny;

	if ( global.arclink.port > 0 )
		_server.addEndpoint(Wired::Socket::IPAddress(), global.arclink.port, false,
		                    new ArclinkListener(globalAllow, globalDeny));

	if ( global.fdsnws.port > 0 )
		_server.addEndpoint(Wired::Socket::IPAddress(), global.fdsnws.port, false,
		                    new FDSNWSListener(globalAllow, globalDeny));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::run() {
	if ( _arclinkServerPort > 0 )
		SEISCOMP_INFO("Starting Arclink server on port %d", _arclinkServerPort);

	if ( _fdsnwsServerPort > 0 )
		SEISCOMP_INFO("Starting FDSNWS server on port %d", _fdsnwsServerPort);

	if ( !_server.init() )
		return false;

	return _server.run();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {
	Client::Application::done();

	SEISCOMP_INFO("Shutdown server");
	_server.shutdown();
	_server.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exit(int returnCode) {
	Client::Application::exit(returnCode);
	_server.shutdown();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}
