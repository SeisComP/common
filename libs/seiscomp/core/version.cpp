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


#include <seiscomp/core/version.h>
#include <seiscomp/core/strings.h>
#include <iostream>

#include <seiscomp/core/build_version.h>
#include <seiscomp/version.h>


#ifdef WITH_GIT_REVISION
extern SC_SYSTEM_CORE_API const char* git_revision() {
	return SC_GIT_REVISION;
}
#endif
#ifdef WITH_BUILD_INFOS
extern SC_SYSTEM_CORE_API const char* build_system() {
	return SC_BUILD_SYSTEM;
}
extern SC_SYSTEM_CORE_API const char* compiler_version() {
	return SC_COMPILER_VERSION;
}
extern SC_SYSTEM_CORE_API const char* os_version() {
	return SC_OS_VERSION;
}
#endif

#define TO_STR_(X) #X
#define VERSION_TOKEN_STR(X) TO_STR_(X)

namespace Seiscomp {
namespace Core {


FrameworkVersion CurrentVersion;


FrameworkVersion::FrameworkVersion()
: _version(SEISCOMP_VERSION)
, _api(SC_API_VERSION)
, _release(SEISCOMP_RELEASE_BRANCH) {}


std::string FrameworkVersion::toString() const {
	if ( _release.empty() )
		return _version.toString();
	else
		return _version.toString() + " " + _release;
}


std::string FrameworkVersion::systemInfo() const {
	std::string s;
#ifndef WIN32
#ifdef WITH_GIT_REVISION
	s += std::string("GIT HEAD: ") + git_revision();
#endif
#ifdef WITH_BUILD_INFOS
	#ifdef WITH_GIT_REVISION
	s += "\n";
	#endif
	s += std::string("Compiler: ") + compiler_version() + "\n";
	s += std::string("Build system: ") + build_system() + "\n";
	s += std::string("OS: ") + os_version();
#endif
#endif
	return s;
}


const Version &FrameworkVersion::version() const {
	return _version;
}


const Version &FrameworkVersion::api() const {
	return _api;
}


std::string Version::toString() const {
	return Core::toString(int(majorTag())) + "." + Core::toString(int(minorTag())) + "." + Core::toString(int(patchTag()));
}


bool Version::fromString(const std::string &str) {
	size_t pos = str.find('.'), pos2;
	if ( pos == std::string::npos ) return false;

	int maj, min, patch = 0;
	if ( !Core::fromString(maj, str.substr(0, pos)) )
		return false;

	pos2 = str.find('.', pos + 1);
	if ( pos2 == std::string::npos ) {
		if ( !Core::fromString(min, str.substr(pos + 1)) )
			return false;
	}
	else {
		if ( !Core::fromString(min, str.substr(pos + 1, pos2 - pos - 1)) )
			return false;

		if ( !Core::fromString(patch, str.substr(pos2 + 1)) )
			return false;
	}

	if ( (maj & 0xFFFF0000) || (min & 0xFFFFFF00) || (patch & 0xFFFFFF00) )
		return false;

	packed = pack(maj, min, patch);
	return true;
}


}
}
