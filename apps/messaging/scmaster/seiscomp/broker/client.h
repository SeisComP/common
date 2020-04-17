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


#ifndef GEMPA_BROKER_CLIENT_H__
#define GEMPA_BROKER_CLIENT_H__


#include <seiscomp/wired/devices/socket.h>
#include <string>

#include <seiscomp/broker/message.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


class Message;
class Group;
class Queue;


/**
 * @brief The Client interface describes a client connected to a queue acting
 *        as message subscriber.
 *
 * The only thing that a client does is to store a name which is being assigned
 * by the queue and publish messages. The publish method is abstract and needs
 * to be implemented by derived classes such as communication protocols.
 */
class SC_BROKER_API Client {
	// ----------------------------------------------------------------------
	//  Public types and enumerations
	// ----------------------------------------------------------------------
	public:
		enum Constants {
			MaxLocalHeapSize = 128
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Client();

		//! D'tor
		virtual ~Client();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		const std::string &name() const { return _name; }

		/**
		 * @brief Returns the absolute memory pointer of an local heap
		 *        offset
		 * @param offset The offset in bytes
		 * @return The pointer of the memory block
		 */
		void *memory(int offset);
		const void *memory(int offset) const;

		/**
		 * @return The time in UTC when the client has connected.
		 */
		const Core::Time &created() const;


	// ----------------------------------------------------------------------
	//  Subscriber interface
	// ----------------------------------------------------------------------
	public:
		bool setMembershipInformationEnabled(bool enable);
		bool wantsMembershipInformation() const;

		/**
		 * @brief Sets whether to discard messages where receiver equals
		 *        the sender or not.
		 * @param enable The enable flag
		 * @return Success flag
		 */
		bool setDiscardSelf(bool enable);
		bool discardSelf() const;

		/**
		 * @brief Sets the number of messages required to send back an
		 *        acknoledgement.
		 * @param numberOfMessages The window size
		 */
		void setAcknowledgeWindow(SequenceNumber numberOfMessages);

		/**
		 * @brief Returns the IP address connected to the client socket.
		 *        If the underlying transport does not implement IP socket
		 *        communication, it can return 0.
		 * @return The IP address
		 */
		virtual Wired::Socket::IPAddress IPAddress() const = 0;

		/**
		 * @brief Publishes a message
		 *
		 * This method has to be implemented by all subclasses to encode it
		 * into their transport format and to send it.
		 *
		 * @param sender The sender of the message
		 * @param msg The message
		 * @return Number of bytes sent
		 */
		virtual size_t publish(Client *sender, Message *msg) = 0;

		/**
		 * @brief Notifies a client that a new member entered a group the client
		 *        is also member in.
		 * @param group The group the new member entered.
		 * @param newMember The client pointer to the new member.
		 * @param msg The message to be sent.
		 */
		virtual void enter(const Group *group, const Client *newMember, Message *msg) = 0;

		/**
		 * @brief Notifies a client that a member left a group the client
		 *        is also member in.
		 * @param group The group the member left.
		 * @param oldMember The client pointer to the member.
		 * @param msg The message to be sent.
		 */
		virtual void leave(const Group *group, const Client *oldMember, Message *msg) = 0;

		virtual void disconnected(const Client *disconnectedClient, Message *msg) = 0;

		//! Send acknowledgment to sender
		virtual void ack() = 0;

		//! Remove the clients resources
		virtual void dispose() = 0;


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		Queue          *_queue;
		Core::Time      _created;
		Core::Time      _lastSOHReceived;
		std::string     _name;
		bool            _wantsMembershipInformation;
		bool            _discardSelf;
		SequenceNumber  _sequenceNumber;
		SequenceNumber  _acknowledgeWindow;
		SequenceNumber  _acknowledgeCounter;
		Core::Time      _ackInitiated;
		int             _inactivityCounter; // The number of seconds
		                                    // of inactivity


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		// Local client heap to additional user data stored by e.g. plugins
		char            _heap[MaxLocalHeapSize];

	friend class Queue;
};


inline bool Client::wantsMembershipInformation() const {
	return _wantsMembershipInformation;
}

inline bool Client::discardSelf() const {
	return _discardSelf;
}

inline void *Client::memory(int offset) {
	return _heap + offset;
}

inline const void *Client::memory(int offset) const {
	return _heap + offset;
}

inline const Core::Time &Client::created() const {
	return _created;
}


}
}
}


#endif
