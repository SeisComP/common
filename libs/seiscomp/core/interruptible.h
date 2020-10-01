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


#ifndef SEISCOMP_CORE_INTERRUPTIBLE_H
#define SEISCOMP_CORE_INTERRUPTIBLE_H


#include <string>
#include <list>


#include <seiscomp/core/baseobject.h>


namespace Seiscomp {
namespace Core {


class SC_SYSTEM_CORE_API OperationInterrupted : public GeneralException {
	public:
		OperationInterrupted(): GeneralException("operation interrupted") {}
		OperationInterrupted(const std::string& what): GeneralException(what) {}
};


DEFINE_SMARTPOINTER(InterruptibleObject);

/**
 * Classes that implement opterations, which may potentially take long time
 * and need to be interrupted by the user, should inherit from
 * InterruptibleObject.
 *
 * The inherited class is supposed to implement handleInterrupt(int sig),
 * which is being called when interrupt is requested.
 * This method is normally called from a signal handler, so it is not
 * allowed to throw any exceptions. Normally it
 * just sets a flag, and exception is thrown after returning from a signal
 * handler. For this purpose, the exception OperationInterrupted has been
 * defined. Note: according the POSIX standard, a flag that is set in a
 * signal handler should be of type 'volatile sig_atomic_t'.
 *
 * The main program should set up signal handler as follows:
 *
 * \code
 * void signalHandler(int signal) {
 *     Seiscomp::Core::InterruptibleObject::Interrupt(signal);
 * }
 *
 * int main(int argc, char **argv) {
 *     struct sigaction sa;
 *     sa.sa_handler = signalHandler;
 *     sa.sa_flags = 0;
 *     sigemptyset(&sa.sa_mask);
 *     sigaction(SIGINT, &sa, nullptr);
 *     sigaction(SIGTERM, &sa, nullptr);
 *
 *     // Optionally, disable SIGHUP, so it is not necessary
 *     // to start the process with nohup.
 *     sa.sa_handler = SIG_IGN;
 *     sigaction(SIGHUP, &sa, nullptr);
 *
 *     ...
 *
 *     return 0
 * }
 * \endcode
 */
class SC_SYSTEM_CORE_API InterruptibleObject : public BaseObject {
	public:
		InterruptibleObject();
		virtual ~InterruptibleObject();

		static void Interrupt(int sig);


	protected:
		virtual void handleInterrupt(int) = 0;


	private:
		static std::list<InterruptibleObject*>    _registered;
		std::list<InterruptibleObject*>::iterator _link;
};


} // namespace Core
} // namespace Seiscomp


#endif

