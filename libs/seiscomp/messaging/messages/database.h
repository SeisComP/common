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


#ifndef SEISCOMP_CLIENT_DATABASEMESSAGE_H
#define SEISCOMP_CLIENT_DATABASEMESSAGE_H


#include <seiscomp/messaging/messages/service.h>
#include <seiscomp/io/database.h>


namespace Seiscomp {

// Forward declarations
namespace IO {

class DatabaseInterface;

}

namespace Client {


DEFINE_SMARTPOINTER(DatabaseRequestMessage);

/**
 * \brief Message for requesting a database service
 * This message type requests a databaseservice.
 * The servicename is optional. If the servicename is not
 * given the serviceprovider can choose between different
 * services it offers of this type.
 * So if the provider offers a mysql database and a postgresql
 * database it can select the published service by its own.
 * If the servicename is set the provider has to publish
 * the service matching this name if it is able to do so.
 */
class SC_SYSTEM_CLIENT_API DatabaseRequestMessage : public ServiceRequestMessage {
	DECLARE_SC_CLASS(DatabaseRequestMessage)

	public:
		//! Constructor
		DatabaseRequestMessage();

		/**
		 * Constructor
		 * @param service The requested service name.
		 *                The name can be set nullptr to let the
		 *                service request handler decide which
		 *                interface it will return.
		 */
		DatabaseRequestMessage(const char *service);
};


DEFINE_SMARTPOINTER(DatabaseProvideMessage);

/**
 * \brief Message for providing a database service
 * When receiving this message a corresponding databaseinterface
 * can be created which one connects to using the provided
 * parameters.
 * \code
 * DatabaseProvideMessagePtr msg = DatabaseProvideMessage_Cast(con->read());
 * Seiscomp::IO::DatabaseInterfacePtr db = msg->interface();
 * if ( db != nullptr ) {
 *   // do fancy things with the interface
 * }
 * \endcode
 */
class SC_SYSTEM_CLIENT_API DatabaseProvideMessage : public ServiceProvideMessage {
	DECLARE_SC_CLASS(DatabaseProvideMessage)

	public:
		//! Constructor
		DatabaseProvideMessage();

		/**
		 * Constructor
		 * @param service The provided service name.
		 * @param params The connection parameters.
		 */
		DatabaseProvideMessage(const char *service,
		                       const char *params);

	public:
		/**
		 * Returns a database interface for the provided service
		 * which is already connected to the database.
		 * @return The connected database interface. nullptr, if the
		 *         databaseinterface cannot be created or if the
		 *         connection fails.
		 *         NOTE: The caller is reponsible for deleting the
		 *         returned object.
		 */
		IO::DatabaseInterface *database() const;
};


}
}


#endif
