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


#define SEISCOMP_COMPONENT log
#include <seiscomp/logging/log.h>
#include <seiscomp/logging/init.h>

#include <list>

using namespace std;


namespace Seiscomp {
namespace Logging {


Module::~Module()
{
}

void Module::init( int &argc, char **argv )
{
    (void)argc;
    (void)argv;
}



static std::list<Module *> moduleList;
static int *gArgc =0;
static char **gArgv = 0;

void init(int &argc, char **argv) {
	gArgc = &argc;
	gArgv = argv;

	list<Module*>::const_iterator it;
	for ( it = moduleList.begin(); it != moduleList.end(); ++it )
		(*it)->init( argc, argv );
}

Module *RegisterModule(Module *module) {
	moduleList.push_back( module );
	if ( gArgc )
		module->init(*gArgc, gArgv);

	return module;
}


}
}
