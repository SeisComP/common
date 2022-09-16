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


#include <seiscomp/config/config.h>
#include <seiscomp/system/environment.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <map>


using namespace Seiscomp;
using namespace std;


struct Logger : Config::Logger {
	void log(Config::LogLevel level, const char *filename, int line, const char *msg) {
		switch ( level ) {
			case Config::ERROR:
				cerr << filename << ":" << line << ": error: ";
				break;
			case Config::WARNING:
				cerr << filename << ":" << line << ": warning: ";
				break;
			default:
				return;
		}

		cerr << msg << endl;
	}
};


string toupper(const string &s) {
	string tmp;
	string::const_iterator it;
	for ( it = s.begin(); it != s.end(); ++it )
		tmp += ::toupper(*it);
	return tmp;
}


int main(int argc, char **argv) {
	bool standalone = false;

	if ( argc < 2 || !strcmp(argv[1], "-h") ) {
		cout << "Usage:" << endl  << "  scchkcfg modulename [standalone]" << endl
		      << endl << "Check a SeisComP system configuration for "
		        "case-sensitivity issues of parameter names" << endl;
		cout << endl << "Examples:" << endl;
		cout << "Check configuration of scautopick including global" << endl
		     << "  scchkcfg scautopick" << endl;
		cout << endl;
		return EXIT_FAILURE;
	}

	cout << "Checking case-sensitivity issues of parameter names in configuration for: " << endl;
	cout << "  + " << argv[1] << endl;
	if ( argc > 2 ) {
		if ( !strcmp(argv[2], "standalone") ) {
			standalone = true;
		}
		else {
			cerr << "Unknown specifier '" << argv[2]
			     << "': expecting 'standalone' or none" << endl;
			return EXIT_FAILURE;
		}
	}
	else {
		cout << "  + global" << endl;
	}

	Config::Config cfg;
	Logger logger;
	cfg.setLogger(&logger);
	if ( !Environment::Instance()->initConfig(
	          &cfg, argv[1], Environment::CS_FIRST,
	          Environment::CS_LAST, standalone) ) {
		cerr << "Failed to read configuration" << endl;
		return EXIT_FAILURE;
	}

	cout << "Read configuration files OK:"  << endl;
	vector<string> toks;
	for ( auto fileIt = cfg.symbolTable()->includesBegin();
	     fileIt != cfg.symbolTable()->includesEnd(); ++fileIt ) {
		cout << "  + " << *fileIt << std::endl;
	}

	typedef vector<Config::Symbol*> SymbolList;
	typedef map<string, SymbolList> SymbolConflicts;
	SymbolConflicts conflicts;

	Config::SymbolTable *symtab = cfg.symbolTable();
	Config::SymbolTable::iterator it;

	// First pass, prepare uppercase symbols
	for ( it = symtab->begin(); it != symtab->end(); ++it ) {
		conflicts[toupper((*it)->name)].push_back(*it);
	}

	SymbolConflicts::iterator cit;
	int count = 0;

	for ( cit = conflicts.begin(); cit != conflicts.end(); ++cit ) {
		if ( cit->second.size() <= 1 ) {
			continue;
		}

		++count;
		cout << "Conflict #" << count << endl;

		SymbolList::iterator sit;
		for ( sit = cit->second.begin(); sit != cit->second.end(); ++sit ) {
			Config::Symbol *sym = *sit;
			cout << " " << sym->name << "    ";
			cout << sym->uri << ":" << sym->line << endl;
		}
	}

	if ( !count ) {
		cout << "No possible conflict detected" << endl;
		return EXIT_SUCCESS;
	}
	else {
		cout << count << " conflict" << (count > 1?"s":"") << " detected" << endl;
		return EXIT_FAILURE + 1;
	}
}
