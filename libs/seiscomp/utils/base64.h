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


#ifndef SEISCOMP_UTILS_BASE64_H
#define SEISCOMP_UTILS_BASE64_H

#include <seiscomp/core.h>
#include <string>

namespace Seiscomp {
namespace Util {

/**
 * Converts a datastream into a base64 encoded string.
 * @param target The target string
 * @param data The source data stream
 * @param data_size The source data streamsize (number of elements, not bytes!)
 */
SC_SYSTEM_CORE_API void encodeBase64(std::string &target, const char *data, size_t data_size);

/**
 * Converts a datastream into a base64 encoded string.
 * @param target The target string
 * @param data The source data string
 */
SC_SYSTEM_CORE_API void encodeBase64(std::string &target, const std::string &data);

/**
 * Decodes a base64 encoded string.
 * @param target The container for the decoded data stream
 * @param data The base64 encoded string
 * @param data_size The base64 encoded string size (number of elements, not bytes!)

 */
SC_SYSTEM_CORE_API void decodeBase64(std::string &target, const char *data, size_t data_size);

/**
 * Decodes a base64 encoded string.
 * @param target The container for the decoded data stream
 * @param data The base64 encoded string
 */
SC_SYSTEM_CORE_API void decodeBase64(std::string &target, const std::string &data);

}
}


#endif
