/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
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


#ifndef GEMPA_BROKER_GROUP_H__
#define GEMPA_BROKER_GROUP_H__


#include <seiscomp/core/baseobject.h>
#include <string>

#include <seiscomp/broker/hashset.h>
#include <seiscomp/broker/statistics.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


class Client;
class Queue;


DEFINE_SMARTPOINTER(Group);

/**
 * @brief The Group class implements a particular group (or channel/topic) of
 *        a queue.
 *
 * Each group can have members. A member is a client. This class is nothing
 * else than a manager of members in an efficient way. It implements a very
 * fast hashset (KHash) of its members which makes member tests very fast and
 * does not make it necessary to manage additional complicated lookup
 * structures.
 */
class SC_BROKER_API Group : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef KHashSet<Client*> Members;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		explicit Group(const char *name);

		//! D'tor
		~Group();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the name of the group.
		const std::string &name() const;

		//! Returns the number of members.
		size_t memberCount() const;

		/**
		 * @brief Adds a member to the group if it is not yet.
		 * @param client The pointer identifying a unique client
		 * @return true on success, false otherwise e.g. duplicates
		 */
		bool addMember(Client *client);

		/**
		 * @brief Removes a member from the group.
		 * @param client The pointer identifying a unique client
		 * @return true on success, false otherwise e.g. does not exist
		 */
		bool removeMember(Client *client);

		//! Returns if a client is a member of not.
		bool hasMember(const Client *client) const;

		//! Removes all members.
		void clearMembers() { _members.clear(); }

		const Members &members() const;


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		std::string _name;
		Members     _members;
		mutable Tx  _txMessages;
		mutable Tx  _txBytes;
		mutable Tx  _txPayload;


	friend class Queue;
};


inline const std::string &Group::name() const {
	return _name;
}

inline const Group::Members &Group::members() const {
	return _members;
}


}
}
}


#endif
