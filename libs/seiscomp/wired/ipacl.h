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


#ifndef SEISCOMP_WIRED_IPACL_H
#define SEISCOMP_WIRED_IPACL_H


#include <vector>
#include <string>

#include <seiscomp/wired/devices/socket.h>


namespace Seiscomp {
namespace Wired {


/**
 * @brief The IPMask struct holds an IP4/6 address and an IP4/6 mask
 */
struct SC_SYSTEM_CORE_API IPMask {
	IPMask(uint32_t addr_ = 0, uint32_t mask_ = 0)
	: addr(addr_), mask(mask_) {}

	IPMask(Socket::IPAddress addr_, Socket::IPAddress mask_)
	: addr(addr_), mask(mask_) {}

	bool operator==(const IPMask &other) const {
		return (addr == other.addr && mask == other.mask);
	}

	bool operator!=(const IPMask &other) const {
		return !operator==(other);
	}

	Socket::IPAddress addr;
	Socket::IPAddress mask;
};


std::string toString(const IPMask &mask);
bool fromString(IPMask &mask, const std::string &str);


/**
 * @brief The IPACL class wraps the functionality of an IP access control list
 *
 * This class holds a list of IP address/mask pairs and checks if an IP matches
 * or not. It does not define whether it is a whitelist or blacklist.
 */
class SC_SYSTEM_CORE_API IPACL {
	public:
		typedef std::vector<IPMask> IPMasks;
		typedef IPMasks::const_iterator const_iterator;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Adds an IP mask in string representation, e.g. "192.168.0.0/24"
		 * @param ip The IP mask as string
		 * @return true on success, false otherwise
		 */
		bool add(const std::string &ip);

		/**
		 * @brief Checks if an IP address matches any of the items in the ACL.
		 * @param ip IP address in 32bit binary representation
		 * @return true if it matches, false otherwise. If the ACL list is
		 *         empty, it returns true as well
		 */
		bool check(const Socket::IPAddress &ip) const;

		/**
		 * @brief Checks if an IP address does not match any of the items in
		 *        the ACL
		 * @param ip IP address in 32bit binary representation
		 * @return true if it does not match, false otherwise. If the ACL list is
		 *         empty, it returns true as well
		 */
		bool not_check(const Socket::IPAddress &ip) const;

		//! Clears the ACL list
		void clear();

		//! Checks if ACL is empty
		bool empty() const;

		const_iterator begin() const;
		const_iterator end() const;

		/**
		 * @brief Merges two access lists
		 * @param other The other access list to be merged into this
		 * @return This instance
		 */
		IPACL &operator+=(const IPACL &other);

		/**
		 * @brief Removes items in this which are also in other
		 * @param other The other access list to be removed from this
		 * @return This instance
		 */
		IPACL &operator-=(const IPACL &other);

		/**
		 * @brief operator == Compares IPACL objects
		 * @param other The other IPACL object
		 * @return True, if both objects are equal
		 */
		bool operator==(const IPACL &other) const;

	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		IPMasks _masks;
};


std::string toString(const IPACL &acl);
bool fromString(IPACL &acl, const std::string &str);


inline IPACL::const_iterator IPACL::begin() const {
	return _masks.begin();
}

inline IPACL::const_iterator IPACL::end() const {
	return _masks.end();
}


}
}


#endif
