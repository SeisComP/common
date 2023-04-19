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
#include <seiscomp/logging/publisher.h>

#include <seiscomp/logging/channel.h>
#include <seiscomp/logging/log.h>

#include <cstring>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
using namespace Seiscomp::Logging;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Publisher::Publisher() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Publisher::Publisher(PublishLoc *loc) : Node(), src( loc ) {
	// link to channel for channel based subscriptions
	// Lookup the componentized version
	Node *channelNode = getComponentChannel( src->component,
	   src->channel->name().c_str(), src->channel->logLevel() );
	channelNode->addPublisher( this );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Publisher::~Publisher() {
	clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Publisher::setEnabled(bool active) {
	if ( src ) {
		if ( active )
			src->enable();
		else
			src->disable();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Publisher::Publish(PublishLoc *loc, Channel *, const char *msg) {
	Data data;

	data.publisher = loc;
	data.time = time(0);
	data.msg = msg;

	loc->pub->publish(data);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Publisher::Publish(PublishLoc *loc, Channel *, const std::string &msg) {
	Data data;

	data.publisher = loc;
	data.time = time(0);
	data.msg = msg.c_str();

	loc->pub->publish(data);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Publisher::VPublish(PublishLoc *loc, Channel *,
                         const char *format, va_list ap) {
	Data data;

	data.publisher = loc;
	data.time = time(0);
	data.msg = 0;

	char msgBuf[64];
	char *buf = msgBuf;
	int bufSize = sizeof(msgBuf);

	// loop until we have allocated enough space for the message
	for ( int numTries = 10; numTries; --numTries ) {
		va_list args;

		// va_copy() is defined in C99, __va_copy() in earlier definitions, and
		// automake says to fall back on memcpy if neither exist...
		//
		// FIXME: memcpy doesn't work for compilers which use array type for
		//        va_list such as Watcom
#if defined( va_copy )
		va_copy( args, ap );
#elif defined( __va_copy )
		__va_copy( args, ap );
#else
		memcpy( &args, &ap, sizeof(va_list) );
#endif

		int ncpy = vsnprintf( buf , bufSize, format, args );
		va_end( args );

		// if it worked, then return the buffer
		if ( ncpy > -1 && ncpy < bufSize ) {
			data.msg = buf;
			break;
		}
		else {
			// newer implementations of vsnprintf return # bytes needed..
			if ( ncpy > 0 )
				bufSize = ncpy + 1;
			else
				bufSize *= 2; // try twice as much space as before

			if ( buf != msgBuf ) delete[] buf;

			buf = new char[bufSize];
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Publisher::VPublish(PublishLoc *loc, Channel *,
                         fmt::string_view format, fmt::format_args args) {
	auto line = fmt::vformat(format, args);

	Data data;

	data.publisher = loc;
	data.time = time(0);
	data.msg = line.c_str();

	loc->pub->publish(data);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Publisher::VPublish(PublishLoc *loc, Channel *,
                         fmt::string_view format, fmt::printf_args args) {
	auto line = fmt::vsprintf(format, args);

	Data data;

	data.publisher = loc;
	data.time = time(0);
	data.msg = line.c_str();

	loc->pub->publish(data);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

