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


#ifndef SEISCOMP_SYSTEM_COMMANDLINE_H
#define SEISCOMP_SYSTEM_COMMANDLINE_H


#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/shared_ptr.hpp>
#include <seiscomp/core/exceptions.h>
#include <seiscomp/core.h>

#include <map>
#include <functional>


namespace Seiscomp {
namespace System {


class SC_SYSTEM_CORE_API CommandLine {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		CommandLine();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void addGroup(const char*);

		void addOption(const char* group, const char* option,
		               const char* description);

		template <typename T>
		void addOption(const char* group, const char* option,
		               const char* description, T* storage,
		               bool storageAsDefault = true);

		template <typename T>
		void addOption(const char* group, const char* option,
		               const char* description, std::vector<T>* storage);

		template <typename T, typename DT>
		void addOption(const char* group, const char* option,
		               const char* description, T* storage,
		               const DT& defaultValue);

		template <typename T>
		void addCustomOption(const char* group, const char* option,
		                     const char* description, T* customValidator);

		bool parse(int argc, char** argv);
		bool parse(int argc, char** argv, std::function<bool(const std::string &)> unknownArgumentFilter);

		void printOptions() const;

		/**
		 * Returns whether a command line option is set or not.
		 * This does not apply to mapped parameters from the
		 * configuration file.
		 */
		bool hasOption(const std::string& option) const;

		template <typename T>
		T option(const std::string& option) const;

		template <typename T, int LEN>
		T option(const char (&option)[LEN]) const;

		std::vector<std::string> unrecognizedOptions() const;


	// ----------------------------------------------------------------------
	//  Protected functions
	// ----------------------------------------------------------------------
	protected:
		typedef boost::program_options::options_description program_options;
		typedef boost::program_options::options_description_easy_init options_description_easy_init;
		typedef boost::program_options::options_description options_description;
		typedef boost::program_options::variables_map variables_map;

		options_description* findGroup(const char* group,
		                               const char* option = nullptr) const;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		typedef boost::shared_ptr<options_description> program_options_ptr;
		typedef std::vector<program_options_ptr> program_options_list;
		typedef std::map<std::string, program_options_ptr> program_options_map;

		program_options_ptr _options;
		program_options_list _groups;
		program_options_map _groupsMap;
		variables_map _variableMap;

		std::vector<std::string> _unrecognizedOptions;
};


#include <seiscomp/system/commandline.ipp>

}
}

#endif
