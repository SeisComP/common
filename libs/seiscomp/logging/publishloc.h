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


#include <seiscomp/core.h>

#include <fmt/format.h>
#include <fmt/printf.h>

#include <cstdarg>


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
	PublishLoc(bool *enabled, const char *component,
	           const char *fileName, const char *functionName,
	           int lineNum, Channel *channel);
	~PublishLoc();

	bool *enabled;

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


void Publish(PublishLoc *, Channel *, const char *msg);
void Publish(PublishLoc *, Channel *, const std::string &msg);

template <typename S, typename... Args>
void PublishF(PublishLoc *, Channel *,
              const S &format, Args &&...args);

template <typename S, typename... Args>
void PublishP(PublishLoc *, Channel *,
              const S &format, Args &&...args);

void VPublish(PublishLoc *, Channel *,
              const char *format,
              va_list args);
void VPublish(PublishLoc *, Channel *,
              fmt::string_view format,
              fmt::format_args args);
void VPublish(PublishLoc *, Channel *,
              fmt::string_view format,
              fmt::printf_args args);


template <typename S, typename... Args>
inline void PublishF(PublishLoc *loc, Channel *channel,
                     const S &format, Args &&...args) {
	VPublish(loc, channel, format, fmt::make_format_args(args...));
}


template <typename S, typename... Args>
inline void PublishP(PublishLoc *loc, Channel *channel,
                     const S &format, Args &&...args) {
	VPublish(loc, channel, format, fmt::make_printf_args(args...));
}


}
}


#endif
