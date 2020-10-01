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


#ifndef SEISCOMP_CORE_PLUGIN_H
#define SEISCOMP_CORE_PLUGIN_H

#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/version.h>

namespace Seiscomp {
namespace Core {


DEFINE_SMARTPOINTER(Plugin);

/**
 * The plugin class defines a general class to load code from
 * outside at runtime.
 * The macro IMPLEMENT_SC_PLUGIN(MyPlugin) has to be called
 * inside the plugin implementation to make it compatible.
 * It is also possible to declare the entry function without
 * the macro. One has to implement the function as follows:
 * \code
 * extern "C" Seiscomp::Core::Plugin* createSCPlugin();
 * \endcode
 *
 * One has to derive MyPlugin from Seiscomp::Core::Plugin and
 * set the description entries in the constructor.
 *
 * There is also a convenience macro ADD_SC_PLUGIN that does
 * not require any custom class declaration.
 * \code
 * SC_ADD_PLUGIN("My description", "I am the author", 0, 0, 1)
 * \endcode
 * This macro must only be called once inside a plugin library.
 */

class SC_SYSTEM_CORE_API Plugin : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(Plugin);

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef Plugin* (*CreateFunc)();

		struct Description {
			struct Version {
				int major;
				int minor;
				int revision;
			};

			std::string description;
			std::string author;
			Version     version;
			int         apiVersion;
		};


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		Plugin();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		const Description &description() const;


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		Description _description;
};


}
}


#ifdef WIN32
#define IMPLEMENT_SC_PLUGIN(CLASSNAME) \
  extern "C" __declspec(dllexport) Seiscomp::Core::Plugin* createSCPlugin() { return new CLASSNAME; }
#else
#define IMPLEMENT_SC_PLUGIN(CLASSNAME) \
  extern "C" Seiscomp::Core::Plugin* createSCPlugin() { return new CLASSNAME; }
#endif

#define ADD_SC_PLUGIN(DESC, AUTHOR, VMAJ, VMIN, VREV) \
  namespace { \
    class __PluginStub__ : public Seiscomp::Core::Plugin { \
      public: \
        __PluginStub__() { \
          _description.description = DESC; \
          _description.author = AUTHOR; \
          _description.version.major = VMAJ; \
          _description.version.minor = VMIN; \
          _description.version.revision = VREV; \
          _description.apiVersion = SC_API_VERSION; \
        } \
    }; \
  } \
  IMPLEMENT_SC_PLUGIN(__PluginStub__)

#endif
