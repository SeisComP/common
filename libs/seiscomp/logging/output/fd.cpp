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


#define SEISCOMP_COMPONENT log


#include <seiscomp/logging/output/fd.h>
#include <seiscomp/core/interfacefactory.ipp>
#include <seiscomp/core/strings.h>

#include <cstdio>
#include <sstream>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp::Logging {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#ifdef _WIN32
#  include <io.h>
#  define write(fd, buf, n) _write((fd), (buf), static_cast<unsigned>(n))
#else
#  include <unistd.h>
#  define USE_COLOURS
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

const char kNormalColor[] = "\033[0m";
const char kRedColor[]    = "\033[31m";
const char kGreenColor[]  = "\033[32m";
const char kYellowColor[] = "\033[33m";

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FdOutput::FdOutput(int fdOut) : _fdOut(fdOut) {
#ifdef USE_COLOURS
    _colorize = isatty(fdOut);
#else
    _colorize = false;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FdOutput::setup(const Util::Url &url) {
	if ( !url.path().empty() ) {
		return false;
	}

	return Core::fromString(_fdOut, url.host());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FdOutput::log(const char* channelName,
                   LogLevel level,
                   const char* msg,
                   time_t time) {
	char timeStamp[32];

	tm currentTime;

	currentTime = _useUTC ? *gmtime(&time) : *localtime(&time);

	const char *color = nullptr;

	if ( _colorize ) {
		sprintf(timeStamp, "%s%02i:%02i:%02i%s ",
			kGreenColor,
			currentTime.tm_hour,
			currentTime.tm_min,
			currentTime.tm_sec,
			kNormalColor);

		switch(level) {
			case LL_CRITICAL:
			case LL_ERROR:
				color = kRedColor;
				break;
			case LL_WARNING:
				color = kYellowColor;
				break;
			case LL_NOTICE:
			case LL_INFO:
			case LL_DEBUG:
			default:
				break;
		}
	}
	else {
		sprintf(timeStamp, "%02i:%02i:%02i ",
			currentTime.tm_hour,
			currentTime.tm_min,
			currentTime.tm_sec);
	}

	std::ostringstream ss;

	ss << timeStamp;
	ss << '[' << channelName;
	if ( likely(_logComponent) )
		ss << "/" << component();
	ss << "] ";
	if ( unlikely(_logContext) )
		ss << "(" << fileName() << ':' << lineNum() << ") ";

#ifndef _WIN32
	// THREAD ID
	/*
	char tid[16] = "";
	snprintf(tid,15,"%lu",pthread_self());
	ss << "[tid:" << tid << "] ";
	*/
#endif

	if( color )
		ss << color;

	ss << msg;

	if ( color )
		ss << kNormalColor;

	ss << std::endl;

	std::string out = ss.str();
	ssize_t len = write(_fdOut, out.c_str(), out.length());
	if ( len == -1 ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


using namespace Seiscomp::Logging;

REGISTER_LOGGING_OUTPUT_INTERFACE(FdOutput, "fd");


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
