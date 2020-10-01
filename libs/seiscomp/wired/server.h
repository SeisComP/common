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


#ifndef SEISCOMP_WIRED_SERVER_H
#define SEISCOMP_WIRED_SERVER_H


#include <seiscomp/wired/reactor.h>
#include <seiscomp/wired/endpoint.h>


namespace Seiscomp {
namespace Wired {


DEFINE_SMARTPOINTER(Server);

class SC_SYSTEM_CORE_API Server : public Reactor {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Server();

		//! D'tor
		~Server() override;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void setCertificate(const std::string&);
		void setPrivateKey(const std::string&);

		//! Initializes the server and starts listening
		//! on all defined ports
		virtual bool init();

		//! Shutdown the server causing the run loop to terminate.
		virtual void shutdown() override;

		//! Clean up all sessions and sockets
		virtual void clear() override;

		//! Adds a session to a server
		virtual bool addSession(Session *session) override;
		virtual bool removeSession(Session *session) override;

		/**
		 * @brief Adds an endpoint session to the server. It will create a
		 *        corresponding socket (either unencrypted or SSL) and attach
		 *        it to the session.
		 * @param ip The bind address
		 * @param port The port to listen on
		 * @param useSSL Whether to use SSL or not
		 * @param endpoint The endpoint instance
		 * @return Status flag
		 */
		bool addEndpoint(Socket::IPAddress ip, Socket::port_t port, bool useSSL,
		                 Endpoint *endpoint);

		/**
		 * @brief Adds an endpoint session to the server. This function
		 *        requires a socket to be present for the endpoint. It will
		 *        set the socket to non-blocking and call bind on it with the
		 *        respective IP address and port.
		 * @param ip The bind address
		 * @param port The port to listen on
		 * @param endpoint The endpoint instance
		 * @return Status flag
		 */
		bool addEndpoint(Socket::IPAddress ip, Socket::port_t port, Endpoint *endpoint);

		/**
		 * @brief Adds an IPv6 endpoint bound to a port to the server.
		 * @param ip The bind IP mask
		 * @param port The port number to accept incoming connections.
		 * @param useSSL Flag if SSL should be used or not
		 * @param endpoint The implementation of the endpoint
		 * @return Success flag
		 */
		bool addEndpointV6(Socket::IPAddress ip, Socket::port_t port,
		                   bool useSSL, Endpoint *endpoint);

		bool removeEndpoint(Endpoint *endpoint);

		bool clearEndpoints();

		const SessionList &endpoints() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		virtual void endpointRemoved(Endpoint *endpoint);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		std::string  _certificate;
		std::string  _privateKey;
		SessionList  _endpoints;
};


}
}


#endif
