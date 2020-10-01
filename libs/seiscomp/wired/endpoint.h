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


#ifndef SEISCOMP_WIRED_ACCEPTOR_H
#define SEISCOMP_WIRED_ACCEPTOR_H


#include <seiscomp/wired/devices/socket.h>
#include <seiscomp/wired/session.h>
#include <seiscomp/wired/ipacl.h>


namespace Seiscomp {
namespace Wired {


class Server;

DEFINE_SMARTPOINTER(Endpoint);



class SC_SYSTEM_CORE_API Endpoint : public Session {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Endpoint constructor
		 * @param sock The socket the endpoint listens on.
		 */
		Endpoint(Socket *sock);


	// ----------------------------------------------------------------------
	//  Connection interface
	// ----------------------------------------------------------------------
	public:
		void update() override;

		Socket *socket() const { return static_cast<Socket*>(_device.get()); }
		Session *accept(Socket *socket);


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		virtual bool checkSocket(Socket *socket) const;
		virtual Session *createSession(Socket *socket);
};



class SC_SYSTEM_CORE_API AccessControlledEndpoint : public Endpoint {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief AccessControlledEndpoint constructor
		 * @param sock The socket to listen on
		 * @param allowedIPs The list of allowed IPs to connect
		 * @param deniedIPs The list of blocked IPs
		 */
		AccessControlledEndpoint(Socket *sock,
		                         const IPACL &allowedIPs,
		                         const IPACL &deniedIPs);


		/**
		 * @brief Compares the IPACL of the object with the given one
		 * @param allowedIPs The list of allowed IPs 
		 * @param deniedIPs The list of blocked IPs
		 * @return True, if the lists are equal
		 */
		bool compareIPACL(const IPACL &allowedIPs, const IPACL &deniedIPs) const;


	// ----------------------------------------------------------------------
	//  Protected Endpoint interface
	// ----------------------------------------------------------------------
	protected:
		bool checkSocket(Socket *socket) const override;


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		IPACL _allowedIPs;
		IPACL _deniedIPs;
};


}
}


#endif
