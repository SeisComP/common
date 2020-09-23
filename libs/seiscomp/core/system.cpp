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


#define SEISCOMP_COMPONENT System
#include <seiscomp/logging/log.h>
#include <seiscomp/core/system.h>
#include <seiscomp/core/platform/platform.h>
#include <stdlib.h>

#ifdef MACOSX
	#include <sys/param.h>
	#define HOST_NAME_MAX MAXHOSTNAMELEN
#endif


#ifndef WIN32
#include <unistd.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif



#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

namespace Seiscomp {
namespace Core {

std::string getHostname() {
#ifdef WIN32
	static bool initialized = false;
	if ( !initialized ) {
		WSADATA wsaData;
		int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
		if (wsaerr != 0) {
			SEISCOMP_ERROR("WSAStartup failed with error: %d", wsaerr);
		}
		initialized = true;
	}
#endif

	char hostname[HOST_NAME_MAX];
	if ( gethostname(hostname, HOST_NAME_MAX) != 0 ) {
		const char* name = nullptr;
		name = getenv("HOSTNAME");
		return (name) ? name : "";
	}
	return hostname;
}




void sleep(unsigned long seconds) {
#ifndef WIN32
	::sleep(seconds);
#else
	Sleep(seconds*1000);
#endif
}




void msleep(unsigned long milliseconds) {
#ifndef WIN32
	::usleep(milliseconds * 1000);
#else
	Sleep(milliseconds);
#endif
}




unsigned int pid() {
#if defined(_MSC_VER)
	return GetCurrentProcessId();
#else
	return (unsigned int)getpid();
#endif
}




bool system(const std::string& command) {
	return std::system(command.c_str()) == -1 ? false : true;
}


} // namespace Core
} // namespace Seiscomp

