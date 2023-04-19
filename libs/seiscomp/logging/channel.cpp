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
#include <mutex>
#include <string.h>

#include <seiscomp/logging/channel.h>

using namespace std;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char GlobalComponent[] = "/";
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Logging {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static Channel *gRootChannel =0;

// big lock around channel lookups..
static mutex gChannelLock;

// Use GetComponentChannel here because we want to reference the global
// versions, not the componentized versions..
// We'll use
SC_SYSTEM_CORE_API Channel *_SCDebugChannel = getGlobalChannel("debug", LL_DEBUG);
SC_SYSTEM_CORE_API Channel *_SCInfoChannel = getGlobalChannel("info", LL_INFO);
SC_SYSTEM_CORE_API Channel *_SCWarningChannel = getGlobalChannel("warning", LL_WARNING);
SC_SYSTEM_CORE_API Channel *_SCErrorChannel = getGlobalChannel("error", LL_ERROR);
SC_SYSTEM_CORE_API Channel *_SCNoticeChannel = getGlobalChannel("notice", LL_NOTICE);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel::Channel( const string &n, LogLevel level )
: Node()
, _name( n )
, _level( level ) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel::~Channel() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Channel::name() const {
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LogLevel Channel::logLevel() const {
	return _level;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::setLogLevel(LogLevel level) {
	_level = level;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel *Channel::getComponent(Channel *parent, const char *component) {
	ComponentMap::const_iterator it = components.find( component );

	if ( it == components.end() ) {
		Channel *ch = new Channel(_name, _level);
		components.insert(make_pair(string(component), ch));

		// connect to its parent
		if ( parent )
			parent->addPublisher(ch);

		// connect to globalized version
		addPublisher(ch);

		return ch;
	}
	else {
		return it->second;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel *getGlobalChannel(const char *path, LogLevel level) {
	return getComponentChannel(GlobalComponent, path, level);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Channel *getComponentChannel(const char *component, const char *path, LogLevel level) {
	// since we much with the globally visible channel tree, hold a lock..
	lock_guard<mutex> l(gChannelLock);

	string currentPath;

	if ( !gRootChannel )
		gRootChannel = new Channel("", level);

	Channel *current = gRootChannel;
	Channel *currentComponent = 0;
	if ( strcmp(component, GlobalComponent) != 0 )
		currentComponent = gRootChannel->getComponent(0, component);

	while( *path ) {
		// if log level is currently undefined but we now know what it is, then
		// define it..
		if ( (current->logLevel() == LL_UNDEFINED) && (level != LL_UNDEFINED) )
			current->setLogLevel(level);

		const char *next = strchr( path , '/' );
		size_t len = next ? next - path : strlen( path );

		if ( len > 1 ) {
			string pathEl( path, len );

			if ( !currentPath.empty() )
				currentPath += '/';
			currentPath += pathEl;

			Channel::ComponentMap::const_iterator it =
			current->subChannels.find( pathEl );

			if ( it != current->subChannels.end() ) {
				// found.  possibly creating sub-map
				current = it->second;
			}
			else {
				// create
				Channel *nm = new Channel( currentPath, level );
				current->subChannels.insert( make_pair(pathEl, nm) );

				current->addPublisher( nm );

				current = nm;
			}

			// track componentized version
			if ( currentComponent )
				currentComponent = current->getComponent(currentComponent,
				component);

			path += len;
		}
		else {
			// skip separator character
			++path;
		}
	}

	if ( currentComponent )
		return currentComponent;
	else
		return current;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Channel::publish(const Data &data) {
	set<Node*>::const_iterator it = data.seen.find(this);

	if ( it == data.seen.end() ) {
		const_cast<Data&>(data).seen.insert(this);
		Node::publish(data);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
