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


#ifndef SEISCOMP_MESSAGING_MESSAGES_DATABASE_H
#define SEISCOMP_MESSAGING_MESSAGES_DATABASE_H


#include <seiscomp/core/message.h>
#include <seiscomp/client.h>


namespace Seiscomp {
namespace Client {


DEFINE_SMARTPOINTER(ServiceRequestMessage);

/**
 * \brief Message for requesting a service
 * This class is the base class for all kinds of service
 * requests being sent over the network.
 * It holds an optional servicename. If the servicename
 * is not set the request handler can choose a service
 * matching the servicetype defined by the classname.
 * A message of this type cannot not be sent. One has to
 * derive a class to define the type of service (database,
 * fileaccess, ...).
 */
class SC_SYSTEM_CLIENT_API ServiceRequestMessage : public Core::Message {
	DECLARE_SC_CLASS(ServiceRequestMessage)
	DECLARE_SERIALIZATION;

	protected:
		//! Constructor
		ServiceRequestMessage();

		/**
		 * C'tor
		 * @param service The requested service name.
		 *                The name can be set nullptr to let the
		 *                service request handler decide which
		 *                interface it will return.
		 */
		ServiceRequestMessage(const char *service);

		//! Implemented interface from Message
		bool empty() const override;

	public:
		/**
		 * @return The requested service name
		 */
		const char* service() const;

	private:
		std::string _serviceName;
};


DEFINE_SMARTPOINTER(ServiceProvideMessage);

/**
 * \brief Message for providing a service
 * This class is the base class for all kinds of service
 * providers.
 * It holds a servicename and the connection parameters.
 */
class SC_SYSTEM_CLIENT_API ServiceProvideMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ServiceProvideMessage)
	DECLARE_SERIALIZATION;

	protected:
		/**
		 * Constructor
		 * @param service The provided service name.
		 * @param params The connection parameters.
		 */
		ServiceProvideMessage(const char *service,
		                      const char *params);

	public:
		/**
		 * @return The provided service name.
		 */
		const char *service() const;

		/**
		 * @return The connection parameters.
		 */
		const char *parameters() const;

		//! Implemented interface from Message
		virtual bool empty() const;

	private:
		std::string _serviceName;
		std::string _parameters;
};


}
}


#endif
