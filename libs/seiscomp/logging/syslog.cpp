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
#define SYSLOG_NAMES

#ifndef WIN32

#include <seiscomp/logging/syslog.h>
#include <syslog.h>

#include <cstring>
#include <iostream>

#ifndef SYSLOG_FACILITY
#  define SYSLOG_FACILITY LOG_LOCAL0
#endif


namespace Seiscomp {
namespace Logging {


SyslogOutput::SyslogOutput()
	: _openFlag(false), _facility(SYSLOG_FACILITY) {
}

SyslogOutput::SyslogOutput(const char* ident, const char *facility)
    : _openFlag(false), _facility(SYSLOG_FACILITY) {
	SyslogOutput::open(ident, facility);
}

SyslogOutput::~SyslogOutput() {
	SyslogOutput::close();
}

bool SyslogOutput::open(const char* ident, const char *facility) {
	const CODE *names = facilitynames;
	_facility = SYSLOG_FACILITY;

	if ( facility ) {
		_facility = -1;

		for ( ; names->c_name; ++names ) {
			if ( strcmp(names->c_name, facility) == 0 ) {
				_facility = names->c_val;
				break;
			}
		}

		if ( _facility == -1 ) {
			std::cerr << "Invalid syslog facility: " << facility << std::endl;
			return false;
		}
	}

	openlog(ident, 0, _facility);
	_openFlag = true;
	return true;
}

void SyslogOutput::close() {
	if ( _openFlag )
		closelog();
	
	_openFlag = false;
}

bool SyslogOutput::isOpen() const {
	return _openFlag;
}

void SyslogOutput::log(const char* channelName,
                       LogLevel level,
                       const char* msg,
                       time_t time) {
	
	int priority = LOG_ALERT;
	switch ( level ) {
		case LL_CRITICAL:
			priority = LOG_CRIT;
			break;
		case LL_ERROR:
			priority = LOG_ERR;
			break;
		case LL_WARNING:
			priority = LOG_WARNING;
			break;
		case LL_NOTICE:
			priority = LOG_NOTICE;
			break;
		case LL_INFO:
			priority = LOG_INFO;
			break;
		case LL_DEBUG:
		case LL_UNDEFINED:
			priority = LOG_DEBUG;
			break;
		default:
			break;
	}

	syslog(priority, "[%s/%s] %s", channelName, component(), msg);
}



}
}

#endif
