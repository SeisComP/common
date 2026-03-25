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


#define SEISCOMP_COMPONENT ExternalLocator

#include <seiscomp/logging/log.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/system/environment.h>

#include "external.h"

#include <sstream>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>


using namespace std;


namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ADD_SC_PLUGIN("Locator wrapper for external scripts", "gempa GmbH <seiscomp-devel@gempa.de>", 0, 1, 0)
REGISTER_LOCATOR(ExternalLocator, "External");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExternalLocator::ExternalLocator() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ExternalLocator::init(const Config::Config &config) {
	try {
		_profiles = config.getStrings("ExternalLocator.profiles");
	}
	catch ( ... ) {}

	for ( string &p : _profiles ) {
		size_t pos = p.find(':');
		if ( pos != string::npos ) {
			_scripts[p.substr(0, pos)] = Environment::Instance()->absolutePath(p.substr(pos+1));
			p.erase(pos, string::npos);
		}
		else {
			_scripts[p] = p;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExternalLocator::IDList ExternalLocator::profiles() const {
	return _profiles;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ExternalLocator::setProfile(const string &name) {
	_currentScript = _scripts.find(name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ExternalLocator::capabilities() const {
	return InitialLocation | DistanceCutOff | FixedDepth | IgnoreInitialLocation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Origin *ExternalLocator::locate(PickList &pickList) {
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Origin *ExternalLocator::locate(PickList &pickList,
                                           double initLat, double initLon,
                                           double initDepth,
                                           const Seiscomp::Core::Time &initTime) {
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Origin *ExternalLocator::relocate(const DataModel::Origin *origin) {
	if ( _currentScript == _scripts.end() ) {
		throw LocatorException("No profile selected");
	}

	int inPipe[2], outPipe[2];

	// Create in and out pipes
	if ( pipe(inPipe) ) {
		SEISCOMP_ERROR("Could not create stdin pipe: %s (%i)",
		               strerror(errno), errno);
		throw LocatorException("system error: stdin");
	}

	if ( pipe(outPipe) ) {
		SEISCOMP_ERROR("Could not create stdout pipe: %s (%i)",
		               strerror(errno), errno);
		close(inPipe[0]); close(inPipe[1]);
		throw LocatorException("system error: stdout");
	}

	// Call script
	pid_t pid = fork();
	if ( pid < 0 ) {
		SEISCOMP_ERROR("Could not fork sub process: %s (%i)",
		               strerror(errno), errno);
		close(inPipe[0]); close(inPipe[1]);
		close(outPipe[0]); close(outPipe[1]);
		throw LocatorException("system error: fork");
	}

	if ( pid == 0 ) {
		{
			int oldFD = inPipe[0];
			if ( dup2(oldFD, STDIN_FILENO) < 0 ) {
				_exit(200); // ERROR_CHILD_STDIN
			}
			// close redundant file descriptors
			close(inPipe[0]); close(inPipe[1]);
		}

		{
			int oldFD = outPipe[1];
			if ( dup2(oldFD, STDOUT_FILENO) < 0 ) {
				_exit(201); // ERROR_CHILD_STDOUT
			}
			// close redundant file descriptors
			close(outPipe[0]); close(outPipe[1]);
		}

		vector<string> args;
		vector<char*> cargs;
		Core::split(args, _currentScript->second.c_str(), " ");

		if ( _enableDistanceCutOff ) {
			args.push_back("--max-dist=" + Core::toString(_distanceCutOff));
		}
		if ( _ignoreInitialLocation ) {
			args.push_back("--ignore-initial-location");
		}
		if ( _usingFixedDepth ) {
			args.push_back("--fixed-depth=" + Core::toString(_fixedDepth));
		}

		for ( string &s : args ) cargs.push_back(s.data());
		cargs.push_back(nullptr);

		execvp(cargs[0], cargs.data());
		_exit(202); // ERROR_CHILD_EXEC
	}

	// Close child channels
	close(inPipe[0]);
	close(outPipe[1]);

	IO::XMLArchive ar;
	DataModel::Origin *tmp;

	{
		// TODO: Append picks and wrap it in EventParameters
		bool wasEnabled = DataModel::PublicObject::IsRegistrationEnabled();
		DataModel::PublicObject::SetRegistrationEnabled(false);

		DataModel::EventParametersPtr ep = new DataModel::EventParameters();

		DataModel::OriginPtr copyOrg = new DataModel::Origin(*origin);

		// Copy arrivals and picks
		for ( size_t i = 0; i < origin->arrivalCount(); ++i ){
			DataModel::Arrival *arrival = origin->arrival(i);
			DataModel::Pick *pick = getPick(arrival);

			if ( !pick ) {
				throw PickNotFoundException("pick '" + arrival->pickID() + "' not found");
			}

			copyOrg->add(new DataModel::Arrival(*arrival));
			ep->add(new DataModel::Pick(*pick));
		}

		// Copy comments
		for ( size_t i = 0; i < origin->commentCount(); ++i ){
			copyOrg->add(new DataModel::Comment(*origin->comment(i)));
		}

		ep->add(copyOrg.get());

		ostringstream oss;
		ar.create(oss.rdbuf());
		tmp = const_cast<DataModel::Origin*>(origin);
		ar << ep;
		ar.close();
		tmp = nullptr;
		string content = oss.str();
		write(inPipe[1], content.c_str(), content.size());
		DataModel::PublicObject::SetRegistrationEnabled(wasEnabled);
	}

	// Done with writing
	close(inPipe[1]);

	// Wait for script
	int status = 0;
	pid_t res = waitpid(pid, &status, WUNTRACED | WCONTINUED);
	// pid does not exist or is no child process
	if ( res <= 0 ) {
		SEISCOMP_ERROR("waitpid: %s (%i)", strerror(errno), errno);
		throw LocatorException("system error: exec");
	}

	if ( WIFEXITED(status) ) {
		int exit = WEXITSTATUS(status);
		if ( exit == EXIT_SUCCESS ) {
			string content;
			char buf[512];
			int r;
			while ( (r = read(outPipe[0], buf, 512)) > 0 ) {
				content += string(buf, r);
			}

			// Close parent channel
			close(outPipe[0]);

			istringstream iss(content);
			ar.open(iss.rdbuf());
			ar >> tmp;
			ar.close();

			if ( !tmp )
				throw LocatorException("no origin in result document");

			if ( tmp->publicID() == origin->publicID() ) {
				DataModel::PublicObject::GenerateId(tmp);
			}

			return tmp;
		}
	}

	// Close parent channel
	close(outPipe[0]);

	throw LocatorException("external script exited with error");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
