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


#ifndef SEISCOMP_SYSTEM_HOSTINFO_H
#define SEISCOMP_SYSTEM_HOSTINFO_H


#include <seiscomp/core.h>
#include <string>
#include <stdint.h>


namespace Seiscomp {
namespace System {


/**
 * @brief The HostInfo class returns information about the host system of
 *        the current process.
 */
class SC_SYSTEM_CORE_API HostInfo {
	// ------------------------------------------------------------------
	// X'truction
	// ------------------------------------------------------------------
	public:
		HostInfo();
		~HostInfo();


	// ------------------------------------------------------------------
	// Public query interface
	// ------------------------------------------------------------------
	public:
		int pid() const;
		std::string name() const;
		std::string login() const;
		std::string programName() const;

		/**
		 * @brief Returns the amount of available memory on this host.
		 * @return Total memory in kb
		 */
		int64_t totalMemory() const;

		/**
		 * @brief Returns the currently used memory of this process.
		 * @return Used memory in kb
		 */
		int getCurrentMemoryUsage() const;

		/**
		 * @brief Computes the current CPU usage with respect to the
		 *        last call. The very first call to this method will not
		 *        succeed and return an invalid value (-1).
		 * @param forceReset Forces resetting of the internal state. If set
		 *                   to true then an invalid value will be returned
		 *                   and the current time is set as the next reference
		 *                   time.
		 * @return The relative CPU usage. 1 means 100% of one core. If
		 *         the returned value is negative then it is invalid.
		 */
		double getCurrentCpuUsage(bool forceReset = false);


	// ------------------------------------------------------------------
	// Private members
	// ------------------------------------------------------------------
	private:
		class Impl;
		Impl *_impl;
};


}
}


#endif
