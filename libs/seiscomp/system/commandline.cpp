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


#define SEISCOMP_COMPONENT CommandLine

#include <seiscomp/system/commandline.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/logging/log.h>

#include <boost/program_options/parsers.hpp>
#include <iostream>
#include <fstream>


using namespace std;
using namespace boost::program_options;


namespace Seiscomp {
namespace System {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CommandLine::CommandLine() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CommandLine::options_description*
CommandLine::findGroup(const char* group, const char* option) const {
	if ( group == nullptr ) return nullptr;

	program_options_map::const_iterator it = _groupsMap.find(group);
	if ( it == _groupsMap.end() ) {
		if ( option )
			SEISCOMP_WARNING("Commandline group '%s' not found -> parameter '%s' ignored",
			                 group, option);
		return nullptr;
	}

	return (*it).second.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CommandLine::addGroup(const char* name) {
	if ( _groupsMap.find(name) != _groupsMap.end() ) return;
	program_options_ptr po = program_options_ptr(new options_description(name));
	_groupsMap[name] = po;
	_groups.push_back(po);
	//_commandLineOptions.add(po);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CommandLine::addOption(const char* group, const char* option,
                            const char* description) {
	options_description* o = findGroup(group, option);

	if ( o )
		(options_description_easy_init(o))(option, description);
		//o->add_options()(option, description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CommandLine::parse(int argc, char** argv) {
	if ( !argc ) return true;

	_options = program_options_ptr(new options_description());

	for ( program_options_list::const_iterator it = _groups.begin();
	      it != _groups.end(); ++it )
		_options->add(*(*it));

	try {
		parsed_options parsed = command_line_parser(argc, argv).options(*_options).allow_unregistered().run();
		store(parsed, _variableMap, false);

		// NOTE: To enable a configuration file for commandline options uncomment the following lines
		//       of code
		/*
		ifstream ifs((Environment::Instance()->configDir() + "/seiscomp.cfg").c_str());
		if (ifs) {
			cerr << "Using " << Environment::Instance()->configDir() << "/seiscomp.cfg" << endl;
			store(parse_config_file(ifs, *_options), _variableMap);
		}
		*/

		notify(_variableMap);

		_unrecognizedOptions = collect_unrecognized(parsed.options, include_positional);
	}
	catch ( exception& e ) {
		cout << "Error: " << e.what() << endl;
		cout << *_options << endl;
		return false;
	}

	notify(_variableMap);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CommandLine::parse(int argc, char** argv,
                        std::function<bool(const std::string &)> unknownArgumentFilter) {
	if ( !CommandLine::parse(argc, argv) ) {
		return false;
	}

	for ( size_t i = 0; i < _unrecognizedOptions.size(); ) {
		const auto &arg = _unrecognizedOptions[i];
		if ( arg.compare(0, 1, "-") == 0 ) {
			if ( unknownArgumentFilter(arg) ) {
				_unrecognizedOptions.erase(_unrecognizedOptions.begin() + i);
			}
			else {
				cout << "Error: unknown option '" << arg << "'" << endl;
				return false;
			}
		}
		else {
			++i;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool CommandLine::hasOption(const string& option) const {
	map<string, boost::program_options::variable_value>::
	const_iterator it = _variableMap.find(option);
	if ( it == _variableMap.end() ) return false;
	return !it->second.defaulted();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<string> CommandLine::unrecognizedOptions() const {
	return _unrecognizedOptions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void CommandLine::printOptions() const {
	if ( _options )
		cout << *_options << endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
