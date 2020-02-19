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
#include <seiscomp/logging/log.h>
#include <seiscomp/logging/channel.h>
#include <seiscomp/logging/publisher.h>
#include <seiscomp/logging/fd.h>

#include <unistd.h>
#include <mutex>


namespace Seiscomp {
namespace Logging {

#ifdef WIN32
#define STDERR_FILENO 2
#endif

void RegisterVA(PublishLoc *loc, Channel *channel, const char *format, va_list args ) {
	static std::mutex registrationLock;
	std::lock_guard<std::mutex> lock(registrationLock);

	loc->channel = channel;

	Publisher *pub = new Publisher(loc);

	loc->pub = pub;
	loc->publish = Publisher::Publish;
	loc->publishVA = Publisher::PublishVA;

	if ( pub->enabled() ) {
		loc->enable();

		// pass through to the publication function since it is active at
		// birth.
		Publisher::PublishVA(loc, channel, format, args);
	}
	else
		loc->disable();
}


void Register(PublishLoc *loc, Channel *channel, const char *format, ... ) {
	static std::mutex registrationLock;
	std::lock_guard<std::mutex> lock(registrationLock);

	loc->channel = channel;

	Publisher *pub = new Publisher(loc);

	loc->pub = pub;
	loc->publish = Publisher::Publish;
	loc->publishVA = Publisher::PublishVA;

	if ( pub->enabled() ) {
		loc->enable();

		// pass through to the publication function since it is active at
		// birth.
		va_list args;
		va_start (args, format);
		Publisher::PublishVA(loc, channel, format, args);
		va_end( args );
	}
	else
		loc->disable();
}


PublishLoc::~PublishLoc() {
	disable();
	if ( pub != NULL ) {
		delete pub;
		pub = NULL;
	}
}


static FdOutput __consoleLogger(STDERR_FILENO);


Channel* getAll() {
	return getGlobalChannel("*");
}


Channel* getComponentAll(const char* component) {
	return getComponentChannel(component, "*");
}


Channel* getComponentDebugs(const char* component) {
	return getComponentChannel(component, "debug");
}


Channel* getComponentInfos(const char* component) {
	return getComponentChannel(component, "info");
}


Channel* getComponentWarnings(const char* component) {
	return getComponentChannel(component, "warning");
}


Channel* getComponentErrors(const char* component) {
	return getComponentChannel(component, "error");
}


Channel* getComponentNotices(const char* component) {
	return getComponentChannel(component, "notice");
}


Output* consoleOutput() {
	return &__consoleLogger;
}


void enableConsoleLogging(Channel* channel) {
	__consoleLogger.subscribe(channel);
}


void disableConsoleLogging() {
	__consoleLogger.clear();
}


#define DO_LOG(CHANNEL) \
	SEISCOMP_##CHANNEL("%s", msg)

#define DO_CHANNEL(CHANNEL) \
	SEISCOMP_LOG(CHANNEL, "%s", msg)


void debug(const char* msg) {
	DO_LOG(DEBUG);
}


void info(const char* msg) {
	DO_LOG(INFO);
}


void warning(const char* msg) {
	DO_LOG(WARNING);
}


void error(const char* msg) {
	DO_LOG(ERROR);
}


void notice(const char* msg) {
	DO_LOG(NOTICE);
}


void log(Channel* ch, const char* msg) {
	DO_CHANNEL(ch);
}


}
}
