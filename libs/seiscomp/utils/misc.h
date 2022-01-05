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

template <typename R, class... Types>
/**
 * @brief A wrapper around a function that returns a results and throws
 *        an exception in case of an error.
 *
 * This wrapper instead returns true if no exception has been thrown and false
 * otherwise. The result is passed as reference as 3rd parameter in \p ret.
 *
 * @code
 * int getSomeAttribute(const Object *obj) {
 *     if ( !obj->attribute() ) {
 *         throw runtime_error("Attribute is not set");
 *     }
 *     return obj->attribute();
 * }
 * @endcode
 *
 * To use this function it should be wrapped in catch-except
 * and the return value must be stored somewhere, which might
 * be inconvenient.
 *
 * @code
 * cout << "Attribute is ";
 *
 * int attribute;
 *
 * if ( catchBool(getSomeAttribute, attribute, obj) ) {
 *     cout << attribute
 * }
 * else {
 *     cout << "not set";
 * }
 *
 * cout << endl;
 * @endcode
 *
 * Which version is preferred is up to the user. With this wrapper both
 * versions are possible.
 *
 * @param[in] func The function to be wrapped.
 * @param[out] ret The return value of the function is stored here.
 * @param[in] args
 * @return True if no exception was thrown the the input function,
 *         false otherwise.
 */
bool catchBool(R func(Types...), R &ret, Types... args);

extern char HEXCHARS[];


}
}


#include <seiscomp/utils/misc.ipp>


#endif
