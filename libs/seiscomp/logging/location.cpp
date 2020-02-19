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
#include <seiscomp/logging/location.h>

#include <mutex>
#include <map>

using namespace std;

namespace Seiscomp {
namespace Logging {

struct FileNodeMap : public map<string, FileNode*> {
	~FileNodeMap() {
		for ( iterator it = begin(); it != end(); ++it )
			delete it->second;
	}
};

static FileNodeMap gFileMap;

/*
locks for global maps
*/
static mutex gMapLock;

/*! @class Seiscomp::Logging::FileNode <seiscomp/logging/location.h>
  @brief Provides filename based logging nodes.

  This allows subscribing to messages only from particular files.
  For example,
  @code

  int main()
  {
      // display some messages to stderr
      StdioNode std( STDERR_FILENO );

      // subscribe to everything from this file
      std.subscribeTo( FileNode::Lookup( __FILE__ ) );

      // and everything from "important.cpp"
      std.subscribeTo( FileNode::Lookup( "important.cpp" ));
  }
  @endcode

  Note that file names are not required to be unique across the entire
  program.  Different components may contain the same filename, which is
  why there is a second Lookup function which also takes the component
  name.

  @see Channel
  @author Valient Gough
*/


FileNode::FileNode(const char *_cn, const char *_fileName)
    : Node()
    , componentName( _cn )
    , fileName( _fileName )
{
}

FileNode::FileNode(const char *_fileName)
    : Node()
    , fileName( _fileName )
{
}

FileNode::~FileNode()
{
}

FileNode *
FileNode::Lookup( const char *fileName )
{
	lock_guard<std::mutex> l(gMapLock);

	// no component specified, so look for componentless filename node
	FileNodeMap::const_iterator it = gFileMap.find( fileName );

	if ( it != gFileMap.end() ) {
		return it->second;
	}
	else {
		// create the componentless filename node.  We can't create a fully
		// componentized version because we don't have a component name..
		FileNode *node = new FileNode(fileName);
		gFileMap.insert( make_pair(string(fileName), node));

		return node;
    }
}

FileNode *
FileNode::Lookup(const char *componentName, const char *fileName)
{
    // do this first before we take out the lock
	FileNode *partial = Lookup( fileName );

	lock_guard<std::mutex> l(gMapLock);

    // fullName is "[componentName]::[fileName]"
	string fullName = componentName;
	fullName += "::";
	fullName += fileName;

	FileNodeMap::const_iterator it = gFileMap.find( fullName );

	if ( it != gFileMap.end() ) {
		return it->second;
	}
	else {
		FileNode *node = new FileNode( componentName, fileName );
		gFileMap.insert( make_pair( fullName, node ));

		// partial node never publishes, but it can forward publications from
		// the fully specified nodes..
		partial->addPublisher( node );

		return node;
	}
}

}
}
