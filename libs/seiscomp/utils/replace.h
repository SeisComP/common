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


#ifndef SEISCOMP_UTILS_REPLACE_H
#define SEISCOMP_UTILS_REPLACE_H

#include <string>
#include <seiscomp/core.h>

namespace Seiscomp {
namespace Util {


/**
 * Functor structur to resolve a variable and return
 * the content of it.
 */
struct SC_SYSTEM_CORE_API VariableResolver {
	/**
	 * Resolves a variable. The default implementation replaces:
     *  - hostname: The name of the host (uname -n)
     *  - user: The name of the user (echo $USER)
     *  - more to come...
     * When inheriting this class, call this method in the
     * new implementation to enable the build-in variables if this
     * behaviour in intended.
	 * @param variable The variable name that has to be resolved. The result
	 *                 will be written into it as well (inout).
	 * @return Whether the variable could be resolved or not
	 */
	virtual bool resolve(std::string& variable) const;
	virtual ~VariableResolver() {};
};


/**
 * Replaces variables of a string by their content.
 * Variables have to be enclosed by '@'. Two '@' will be
 * replaced by one '@'.
 * Replacing the variable "host" results when feeding the
 * input string "hostname: @host@".
 * This function used the VariableResolver class and replaces
 * beside the default variables:
 *  - [empty] => '@'
 * @param input The string containing variables to be replaced
 * @return The replaced input string
 */
SC_SYSTEM_CORE_API
std::string replace(const std::string& input);


/**
 * See: string replace(string)
 * @param input The string containing variables to be replaced
 * @param resolver A resolver functor that resolves the variable name
 *                 to its content.
 * @return The replaced input string
 */
SC_SYSTEM_CORE_API
std::string replace(const std::string& input,
                    const VariableResolver& resolver);

SC_SYSTEM_CORE_API
std::string replace(const char* input,
                    const VariableResolver& resolver);

SC_SYSTEM_CORE_API
std::string replace(const std::string &input,
                    const VariableResolver &resolver,
                    const std::string &prefix, const std::string &postfix,
                    const std::string &emptyValue);

}
}

#endif
