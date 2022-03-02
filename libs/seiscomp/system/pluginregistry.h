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


#ifndef SEISCOMP_SYSTEM_PLUGINREGISTRY_H
#define SEISCOMP_SYSTEM_PLUGINREGISTRY_H

#include <list>
#include <string>

#include <seiscomp/core/plugin.h>
#include <seiscomp/core.h>


namespace Seiscomp {

namespace Config {
	class Config;
}

namespace System {


class SC_SYSTEM_CORE_API PluginRegistry {
	// ----------------------------------------------------------------------
	// Public types
	// ----------------------------------------------------------------------
	public:
		struct PluginEntry {
			PluginEntry(void *h, Core::PluginPtr p, const std::string &fn = "")
			 : handle(h), plugin(p), filename(fn) {}
			void *handle;
			Core::PluginPtr plugin;
			std::string filename;
		};

		class SC_SYSTEM_CORE_API iterator : public std::list<PluginEntry>::const_iterator {
			private:
				typedef std::list<PluginEntry>::const_iterator base;

			public:
				iterator();
				iterator(const base&);

			public:
				const Core::Plugin* operator*() const;
				Core::Plugin* value_type(const iterator&);
		};


	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
	private:
		PluginRegistry();

	public:
		~PluginRegistry();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the global instance
		static PluginRegistry *Instance();
		static void Reset();

		//! Adds a plugin to be loaded
		void addPluginName(const std::string &name);

		//! Adds a plugin search path
		void addPluginPath(const std::string &path);

		//! Adds a plugin package search path pointing to
		//! shareDir + "/plugins/" + package
		void addPackagePath(const std::string &package);

		/**
		 * Loads all plugins in the defined search paths
		 * added with addPluginName
		 * @return The number of loaded plugins or -1 in case of an error
		 */
		int loadPlugins();

		/**
		 * Loads all plugins in the defined search paths
		 * configured by the config object. All names added
		 * with addPluginName will be replaced by the configured
		 * plugin names in "core.plugins" if there are any.
		 * Otherwise the default plugin list will be extended by
		 * "plugins".
		 * @return The number of loaded plugins or -1 in case of an error
		 */
		int loadConfiguredPlugins(const Config::Config *config);


		//! Unloads all plugins
		void freePlugins();

		//! Returns the number of registered plugins
		int pluginCount() const;

		//! Returns the start iterator over all registered plugins
		iterator begin() const;
		//! Returns the end iterator over all registered plugins
		iterator end() const;


	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
	private:
		std::string find(const std::string &name) const;
		PluginEntry open(const std::string &file) const;
		bool findLibrary(void *handle) const;


	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
	private:
		using PluginList = std::list<PluginEntry>;
		using PathList = std::vector<std::string>;
		using NameList = std::vector<std::string>;

		static PluginRegistry *_instance;

		PluginList             _plugins;
		PathList               _paths;
		NameList               _pluginNames;
};


}
}


#endif
