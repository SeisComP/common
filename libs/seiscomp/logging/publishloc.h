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



#ifndef SC_LOGGING_PUBLISHLOC_H
#define SC_LOGGING_PUBLISHLOC_H

#include <seiscomp/logging/common.h>
#include <stdarg.h>


namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Channel;
class SC_SYSTEM_CORE_API Node;


struct SC_SYSTEM_CORE_API PublishLoc {
/*! @struct PublishLoc
    @brief Internal RLog structure - static struct for each log() statement

    @internal
    Structure created for each log location to keep track of logging state
    and associated data.

    Only static members are initialized at build time, which is why
    RLogChannel is passed as argument.  Otherwise entire structure will have
    to be initialized at run-time which adds extra code and a guard variable
    for the struct.
 */
	~PublishLoc();

	bool *enabled;

	// If the compiler supports printf attribute specification on function
	// pointers, we'll use it here so that the compiler knows to check for
	// proper printf formatting.  If it doesn't support it, then we'll
	// force the check by inserting a bogus inline function..
	//! function to call when we reach the log statement.
	void (*publish)(PublishLoc *, Channel *, const char *format, ...)
	    PRINTF_FP(3,4);

	void (*publishVA)(PublishLoc *, Channel *, const char *format, va_list args);

	Node *pub;
	const char *component;
	const char *fileName;
	const char *functionName;
	int lineNum;
	Channel *channel;

	inline void enable() { *enabled = true; }
	inline void disable() { *enabled = false; }
	inline bool isEnabled() { return *enabled; }
};


SC_SYSTEM_CORE_API void Register(PublishLoc *loc, Channel *, const char *format, ... ) PRINTF(3,4);
SC_SYSTEM_CORE_API void RegisterVA(PublishLoc *loc, Channel *, const char *format, va_list args );


/* @def LOG_NO_COPY
   @brief Disables class copy constructor and operator =.

   This macro declares a private copy constructor and assignment operator
   which prevents automatic generation of these operation by the compiler.

   Attention, it switches access to private, so use it only at the end of the
   class declaration.
 */
#define LOG_NO_COPY(CNAME) \
	private: \
		CNAME(const CNAME&); \
		CNAME & operator = (const CNAME &)


}
}

#endif
