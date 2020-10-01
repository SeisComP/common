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


#ifndef SC_LOGGING_INIT_H
#define SC_LOGGING_INIT_H


namespace Seiscomp {
namespace Logging {

/*! @class Module <seiscomp/logging/init.h>
	@brief Allows registration of external modules to log.

	Currently this only allows for initialization callbacks.  When init()
	is called, init() is called on all modules which have been registered.

	@author Valient Gough
*/
class SC_SYSTEM_CORE_API Module {
	public:
		virtual ~Module();

		/*! Called by init() to give the modules the command-line arguments */
		virtual void init( int &argv, char **argc );

		/*! Must be implemented to return the name of the module. */
		virtual const char *moduleName() const =0;
};

/*! @relates Module
	Registers the module - which will have init() called when ::init is
	called.
	Returns the module so that it can be used easily as a static initializer.
	@code
	class MyModule : public Seiscomp::Logging::Module
	{
	public:
		virtual const char *moduleName() const {return "MyModule";}
	};
	static Module * testModule = Seiscomp::Logging::RegisterModule( new MyModule() );
	@endcode
*/
SC_SYSTEM_CORE_API Module *RegisterModule(Module * module);

}
}

#endif
