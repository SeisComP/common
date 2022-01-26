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


#ifndef GEMPA_BROKER_MESSAGEPROCESSOR_H__
#define GEMPA_BROKER_MESSAGEPROCESSOR_H__


#include <string>
#include <ostream>

#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/config/config.h>

#include <seiscomp/broker/processor.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


class Client;
class Message;


DEFINE_SMARTPOINTER(MessageProcessor);

/**
 * @brief The MessageProcessor class is used inside the broker to process
 *        messages in any way. The most important use case for such a
 *        processor is to store the message in the database if it suffices a
 *        certain format. Once could think of other use cases such as
 *        building statistics.
 */
class SC_BROKER_API MessageProcessor : public Processor {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		enum Constants {
			MaxAdditionalParams = 100
		};

		enum Mode {
			None        = 0x00,
			Messages    = 0x01,
			Connections = 0x02
		};

		using KeyValueCStrPair = std::pair<const char*,const char*>;
		using KeyCStrValues = KeyValueCStrPair *;

		using KeyValuePair = std::pair<std::string,std::string>;
		using KeyValues = std::vector<KeyValuePair>;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		MessageProcessor();
		virtual ~MessageProcessor();


	// ----------------------------------------------------------------------
	//  Public virtual interface
	// ----------------------------------------------------------------------
	public:
		virtual bool acceptConnection(Client *client,
		                              const KeyCStrValues inParams, int inParamCount,
		                              KeyValues &outParams) = 0;

		virtual void dropConnection(Client *client) = 0;

		virtual bool process(Message *msg) = 0;


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		int mode() const { return _mode; }

		/**
		 * @brief Returns whether the processor want to process messages.
		 * @return Flag
		 */
		bool isMessageProcessingEnabled() const { return _mode & Messages; }

		/**
		 * @brief Returns whether the processor want to process connections..
		 * @return Flag
		 */
		bool isConnectionProcessingEnabled() const { return _mode & Connections; }


	// ----------------------------------------------------------------------
	//  Protected methods
	// ----------------------------------------------------------------------
	protected:
		void setMode(int mode);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		int _mode;
};


DEFINE_INTERFACE_FACTORY(MessageProcessor);


}
}
}


#define REGISTER_BROKER_MESSAGE_PROCESSOR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Messaging::Broker::MessageProcessor, Class> __##Class##InterfaceFactory__(Service)


#endif
