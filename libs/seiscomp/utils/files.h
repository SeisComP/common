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

#ifndef SEISCOMP_UTILS_FILES
#define SEISCOMP_UTILS_FILES

#include <string>
#include <streambuf>
#include <iostream>
#include <seiscomp/core.h>

namespace Seiscomp {
namespace Util {

/**
 * Removes the path portion from a given path:
 * /dir/file.cpp -> file.cpp
 * @return basename */
SC_SYSTEM_CORE_API std::string basename(const std::string& name);

//! Checks if a file is exists
SC_SYSTEM_CORE_API bool fileExists(const std::string& file);

//! Checks if a path (directory) exists
SC_SYSTEM_CORE_API bool pathExists(const std::string& path);

//! Creates a new directory inclusive all unavailable
//! parent directories
SC_SYSTEM_CORE_API bool createPath(const std::string& path);

/** Removes the extension from the filename
 * test.xyz -> test
 * @return name without extension */
SC_SYSTEM_CORE_API std::string removeExtension(const std::string& name);

/**
 * Converts a string or char array to a streambuf object.
 * @return The streambuf object. The caller is responsible to delete the object
 */
SC_SYSTEM_CORE_API std::streambuf *bytesToStreambuf(char *data, size_t n);
SC_SYSTEM_CORE_API std::streambuf *stringToStreambuf(const std::string &str);

/**
 * Tries to open a file and returns a corresponding output stream.
 * The special name '-' refers to stdout.
 * @return The ostream object. The caller is responsible to delete the object
 */
SC_SYSTEM_CORE_API std::ostream *file2ostream(const char *fn);

/**
 * Tries to open a file and returns a corresponding input stream.
 * The special name '-' refers to stdin.
 * @return The istream object. The caller is responsible to delete the object
 */
SC_SYSTEM_CORE_API std::istream *file2istream(const char *fn);


} // namespace Util
} // namespace Seiscomp

#endif
