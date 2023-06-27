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


#ifndef SC_LOGGING_COMMON_H
#define SC_LOGGING_COMMON_H


#include <seiscomp/core.h>

#define LOG_CONCAT2(A,B) A##B
#define LOG_CONCAT(A,B) LOG_CONCAT2(A,B)
#define LOG_STR(X) #X


/*
    We use __printf__ attribute to allow gcc to inspect printf style arguments
    and give warnings if the rDebug(), rWarning(), etc macros are misused.

    We use __builtin_expect on GCC 2.96 and above to allow optimization of
    publication activation check.  We tell the compiler that the branch is
    unlikely to occur, allowing GCC to push unecessary code out of the main
    path.
*/
#ifdef __GNUC__

# define   likely(x)  __builtin_expect((x),1)
# define unlikely(x)  __builtin_expect((x),0)

#else

// Not using the gcc compiler, make the macros do nothing..  They are
// documented as the last instance of the macros..

/*!
*/
# define   likely(x)  (x)
/*! @def unlikely(x)
  Starting with GCC 2.96, we can tell the compiler that an if condition is
  likely or unlikely to occur, which allows the compiler to optimize for the
  normal case.
  @internal
*/
# define unlikely(x)  (x)

#endif

/*! @def SEISCOMP_COMPONENT
    @brief Specifies build-time component, eg -DSEISCOMP_COMPONENT="component-name"

    Define SEISCOMP_COMPONENT as the name of the component being built.
    For example, as a compile flag,  -DSEISCOMP_COMPONENT="component-name"

    If SEISCOMP_COMPONENT is not specified, then it will be defined as "[unknown]"
*/
#ifndef SEISCOMP_COMPONENT
#  warning SEISCOMP_COMPONENT not defined - setting to UNKNOWN
#define SEISCOMP_COMPONENT "[unknown]"
#endif // SEISCOMP_COMPONENT not defined

// Use somewhat unique names (doesn't matter if they aren't as they are used in
// a private context, so the compiler will make them unique if it must)
#define _SCLOGID LOG_CONCAT(_rL_, __LINE__)


#endif
