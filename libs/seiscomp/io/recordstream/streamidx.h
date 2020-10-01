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


#ifndef SEISCOMP_IO_RECORDSTREAM_STREAMIDX_H
#define SEISCOMP_IO_RECORDSTREAM_STREAMIDX_H

#include <string>
#include <seiscomp/core/datetime.h>

namespace Seiscomp {
namespace RecordStream {

class SC_SYSTEM_CORE_API StreamIdx {
	public:
		StreamIdx();

		StreamIdx(const std::string& net, const std::string& sta,
		          const std::string& loc, const std::string& cha);

		StreamIdx(const std::string& net, const std::string& sta,
		          const std::string& loc, const std::string& cha,
		          const Seiscomp::Core::Time& stime,
		          const Seiscomp::Core::Time& etime);

		StreamIdx& operator=(const StreamIdx &other);

		bool operator<(const StreamIdx &other) const;

		bool operator!=(const StreamIdx &other) const;

		bool operator==(const StreamIdx &other) const;

		bool operator>=(const StreamIdx &other) const;

		bool operator>(const StreamIdx &other) const;

		bool operator<=(const StreamIdx &other) const;

		//! Returns the network code
		const std::string &network() const;

		//! Returns the station code
		const std::string &station() const;

		//! Returns the channel code
		const std::string &channel() const;

		//! Returns the location code
		const std::string &location() const;

		//! Returns the start time
		Core::Time startTime() const;

		//! Returns the end time
		Core::Time endTime() const;

		//! Returns a string: <sTime> <eTime> <network> <station> <channel> <location>
		//! <*Time> in format: %Y,%m,%d,%H,%M,%S
		std::string str(const Seiscomp::Core::Time& stime,
		                const Seiscomp::Core::Time& etime) const;


	private:
		const std::string _net;
		const std::string _sta;
		const std::string _loc;
		const std::string _cha;
		const Seiscomp::Core::Time _stime;
		const Seiscomp::Core::Time _etime;
};

} // namespace RecordStream
} // namespace Seiscomp

#endif
