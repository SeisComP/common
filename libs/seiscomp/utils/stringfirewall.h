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


#ifndef SEISCOMP_UTILS_STRINGFIREWALL_H
#define SEISCOMP_UTILS_STRINGFIREWALL_H


#include <seiscomp/core/baseobject.h>

#include <string>
#include <set>
#include <map>


namespace Seiscomp {
namespace Util {


DEFINE_SMARTPOINTER(StringFirewall);


/**
 * @brief The StringFirewall class implements a "firewall" for strings.
 *
 * It manages a blacklist and a whitelist of strings. Whether a string is
 * accepted is checked with the following algorithm:
 *   - Accept if both lists are empty
 *   - If either list is empty the partial result (accepted1 or accepted2) is true
 *   - If the input string matches any item in the whitelist, accepted1 is true,
 *     false otherwise
 *   - If the input string matches any item in the blacklist, accepted2 is false,
 *     true otherwise
 *   - The result is accepted1 && accepted2
 */
class SC_SYSTEM_CORE_API StringFirewall : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public typedefs and variables
	// ----------------------------------------------------------------------
	public:
		typedef std::set<std::string> StringSet;

		StringSet allow; //!< The whitelist
		StringSet deny;  //!< The blacklist


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief isAllowed evaluates if a string may pass the firewall.
		 * @param s The input string
		 * @return true if it may pass, false otherwise
		 */
		virtual bool isAllowed(const std::string &s) const;

		/**
		 * @brief isDenied evaluates if a string is blocked by the firewall.
		 *
		 * It holds that isDenied(s) == !isAllowed(s).
		 *
		 * @param s The input string
		 * @return true if it is blocked, false otherwise
		 */
		bool isDenied(const std::string &s) const;
};



/**
 * @brief The WildcardStringFirewall class extends the StringFirewall by
 *        allowing wildcard ('*' or '?') matches for strings.
 *
 * Each call to either isAllowed or isDenied is cached by default to reduce
 * evaluation time for repeating queries. If queries do not repeat or are
 * more or less random then caching should be disabled.
 */
class SC_SYSTEM_CORE_API WildcardStringFirewall : public StringFirewall {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		WildcardStringFirewall();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * Clears the internal evaluation cache. This should be called when
		 * either #allow or #deny have changed.
		 */
		void clearCache() const;

		/**
		 * @brief Enables query caching. The default is true.
		 * @param enable Whether to cache queries.
		 */
		void setCachingEnabled(bool enable);

		/**
		 * @brief isAllowed evaluates if a string may pass the firewall.
		 * @param s The input string
		 * @return true if it may pass, false otherwise
		 */
		virtual bool isAllowed(const std::string &s) const;


	// ----------------------------------------------------------------------
	//  Private members and typedefs
	// ----------------------------------------------------------------------
	private:
		typedef std::map<std::string, bool> StringPassMap;
		mutable StringPassMap _cache;
		bool                  _enableCaching;
};


}
}


#endif
