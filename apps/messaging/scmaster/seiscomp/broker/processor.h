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


#ifndef GEMPA_BROKER_PROCESSOR_H__
#define GEMPA_BROKER_PROCESSOR_H__


#include <seiscomp/core/baseobject.h>
#include <seiscomp/config/config.h>

#include <seiscomp/broker/api.h>


namespace Seiscomp {
namespace Messaging {
namespace Broker {


class Queue;


DEFINE_SMARTPOINTER(Processor);

class SC_BROKER_API Processor : public Core::BaseObject {
	public:
		Processor();
		virtual ~Processor();


	public:
		/**
		 * @brief Initiales the configuration of a processor from a config object
		 *        and a given parameter name prefix.
		 * @param conf The configuration file object
		 * @param configPrefix The prefix that must be preprended to all
		 *                     parameters.
		 * @return Success flag.
		 */
		virtual bool init(const Config::Config &conf, const std::string &configPrefix) = 0;

		/**
		 * @brief When a processor has been added to a queue, this method will be
		 *        called. The default implementation does nothing. This method
		 *        can be used to e.g. allocate additional client memory of
		 *        the local client heap.
		 * @param queue The queue the processor was attached to.
		 */
		virtual void attach(Queue *queue);

		/**
		 * @brief Shuts down the processor.
		 * @return Success flag.
		 */
		virtual bool close() = 0;

		/**
		 * @brief Add information to a state of health message
		 * @param timestamp The timestamp of the information
		 * @param os The output stream to write to
		 */
		virtual void getInfo(const Core::Time &timestamp, std::ostream &os) = 0;


	private:
		Queue *_queue;

	friend class Queue;
};


}
}
}


#endif
