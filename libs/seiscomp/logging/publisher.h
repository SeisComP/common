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


#ifndef SC_LOGGING_PUBLISHER_H
#define SC_LOGGING_PUBLISHER_H


#include <seiscomp/logging/common.h>
#include <seiscomp/logging/node.h>


namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Channel;


/*! @class Seiscomp::Logging::Publisher <seiscomp/logging/publisher.h>
  @brief Publisher used by log macros.

  This derives from Node and interfaces to the static PublishLoc logging
  data allowing them to be enabled or disabled depending on subscriber
  status.

  An instance of this class is created for every error message location.
  Normally this class is not used directly.

  For example, this
  @code
     rDebug( "hello world" );
  @endcode

  is turned approximatly into this:
  @code
     static PublishLoc _rl = {
       pub: 0,
       component: "component",
       fileName: "myfile.cpp",
       functionName: "functionName()",
       lineNum: __LINE__,
       channel: 0
     };
     if(_rL.publish != 0)
       (*_rl.publish)( &_rL, _RLDebugChannel, "hello world" );
  @endcode

  The Publisher instance manages the contents of the static structure
  _rL.  When someone subscribes to it's message, then _rL.publish is set to
  point to the publishing function, and when there are no subscribers then
  it is set to 0.

  The code produced contains one if statement, and with optimization comes
  out to about 3 instructions on an x86 computer (not including the
  function call).  If there are no subscribers to this message then that
  is all the overhead, plus the memory usage for the structures
  involved and the initial registration when the statement is first
  encountered..

  @see Channel
  @author Valient Gough
 */
class SC_SYSTEM_CORE_API Publisher : public Node {
	public:
		Publisher();
		Publisher(PublishLoc *src);
		~Publisher() override;

	public:
		// metadata about the publisher and its publication
		PublishLoc *src;

	protected:
		void setEnabled(bool newState) override;

		Publisher(const Publisher &);
		Publisher & operator=(const Publisher &);
};


}
}


#endif
