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

#ifndef SEISCOMP_UTILS_MISC
#define SEISCOMP_UTILS_MISC

#include <string>
#include <seiscomp/core.h>
#include <iostream>


namespace Seiscomp {
namespace Util {


SC_SYSTEM_CORE_API char getShortPhaseName(const std::string &phase);

template <class T, class A>
T join(const A &begin, const A &end, const T &glue);

template <typename T>
void toHex(std::string &out, T v);


extern char HEXCHARS[];


}
}


#include <seiscomp/utils/misc.ipp>


#endif
