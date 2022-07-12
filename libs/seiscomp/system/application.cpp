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


#define SEISCOMP_COMPONENT Application

#include <seiscomp/core/strings.h>
#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/system.h>
#include <seiscomp/core/platform/platform.h>

#include <seiscomp/datamodel/version.h>

#include <seiscomp/logging/fd.h>
#include <seiscomp/logging/filerotator.h>
#ifndef WIN32
#include <seiscomp/logging/syslog.h>
#endif

#include <seiscomp/utils/certstore.h>
#include <seiscomp/utils/files.h>
#include <seiscomp/utils/replace.h>

#include <seiscomp/system/application.h>
#include <seiscomp/system/pluginregistry.h>
#include <seiscomp/system/hostinfo.h>

#include <sstream>

#include <cerrno>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <boost/bind.hpp>

#ifdef WIN32
#define snprintf _snprintf
#define popen _popen
#define pclose _pclose
#define STDERR_FILENO 2
#endif

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::System;


namespace {

class FlagCounter: public boost::program_options::untyped_value {
	public:
		FlagCounter(unsigned int* count);
		void xparse(boost::any&, const vector<string>&) const;

	private:
		unsigned int *_count;
};

FlagCounter::FlagCounter(unsigned int* count)
	: boost::program_options::untyped_value(true), _count(count) {
}

void FlagCounter::xparse(boost::any&, const vector<string>&) const {
	++(*_count);
}


/*
void printTraces() {
    #ifndef MACOSX
	void *array[20];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 20);
	strings = backtrace_symbols(array, size);

	SEISCOMP_ERROR(" Obtained %zd stack frames:", size);
	SEISCOMP_ERROR(" --------------------------");

	for (i = 0; i < size; i++)
		SEISCOMP_ERROR(" #%.2d %s", i, strings[i]);

	free(strings);
    #endif
}

int runGDB() {
	char tmp[]="/tmp/sc3-crash-XXXXXX";
	int fd;
	bool full_bt = false;

	fd = mkstemp(tmp);
	if( fd == -1 )
		return -1;
	else {
		char gdb_cmd[]="bt\nquit";
		char gdb_cmd_full[]="bt full\nquit";
		char cmd[128];
		FILE *fp;

		if( full_bt )
			write(fd, gdb_cmd_full, strlen(gdb_cmd_full));
		else
			write(fd, gdb_cmd, strlen(gdb_cmd));
		close(fd);

		snprintf(cmd, sizeof(cmd), "gdb -nw -n -batch -x \"%s\" --pid=%d",
		         tmp, getpid());
		cmd[sizeof(cmd)-1]=0;

		fflush(nullptr);
		fp = popen(cmd, "r");
		if( !fp )
			return -1;
		else {
			char buff[4096];
			size_t len;

			while(fgets(buff, sizeof(buff), fp)) {
				len = strlen(buff);
				if( buff[len-1] == '\n')
					buff[len-1]=0;

				SEISCOMP_ERROR(" %s", buff);
			}
			pclose(fp);
		}
	}

	return 0;
}
*/

void crashHandler() {
	/*
	if ( runGDB() == -1 )
		printTraces();
	*/

	Application *app = Application::Instance();
	if ( !app ) return;

	const string &crash_handler = app->crashHandler();
	if ( crash_handler.empty() ) return;

	FILE *fp;/* = fopen(crash_handler, "rb");
	if ( fp == nullptr ) {
		SEISCOMP_ERROR("crash handler executable '%s' not found", crash_handler);
		return;
	}

	fclose(fp);
	*/

	char buff[4096];
	size_t len;

	snprintf(buff, sizeof(buff), "%s %s %d", crash_handler.c_str(), app->path(), HostInfo().pid());
	SEISCOMP_INFO("Running command: %s", buff);

	fp = popen(buff, "r");
	if ( ! fp ) return;

	while ( fgets(buff, sizeof(buff), fp) ) {
		len = strlen(buff);
		if( buff[len-1] == '\n')
			buff[len-1] = 0;

		SEISCOMP_INFO(" %s", buff);
	}

	pclose(fp);
}

void signalHandler(int signal) {
	// Prevent running the crashHandler again when the crashHandler crashes
	static bool signalCatched = false;

	// Logging disabled due to internal mutex locking that can lock the whole
	// application when interrupting a normal log operation and logging here again

	switch ( signal ) {
		case SIGABRT:
			//SEISCOMP_ERROR("ABORT");
			//SEISCOMP_ERROR("BACKTRACE:");
			//crashHandler();
			exit(-1);

		case SIGSEGV:
			//SEISCOMP_ERROR("SEGFAULT");
			if ( !signalCatched ) {
				signalCatched = true;
				//SEISCOMP_ERROR("BACKTRACE:");
				crashHandler();
			}
			exit(-1);

		default:
			InterruptibleObject::Interrupt(signal);
	}
}

void registerSignalHandler(bool term, bool crash) {
	static volatile bool termRegistered = false;
	static volatile bool crashRegistered = false;

	SEISCOMP_DEBUG("Registering signal handler");

	if ( crash && crashRegistered )
		return;

	if ( crash ) {
#ifndef WIN32
		struct sigaction sa;
		sa.sa_handler = signalHandler;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGSEGV, &sa, nullptr);
		sigaction(SIGABRT, &sa, nullptr);

		sa.sa_handler = SIG_IGN;
		sigaction(SIGHUP, &sa, nullptr);
		sigaction(SIGPIPE, &sa, nullptr);
#else
		signal(SIGSEGV, signalHandler);
		signal(SIGABRT, signalHandler);
		signal(SIGPIPE, SIG_IGN);
#endif

		crashRegistered = true;
	}

	if ( term && termRegistered )
		return;

	if ( term ) {
#ifndef WIN32
		struct sigaction sa;
		sa.sa_handler = signalHandler;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGTERM, &sa, nullptr);
		sigaction(SIGINT, &sa, nullptr);

		sa.sa_handler = SIG_IGN;
		sigaction(SIGHUP, &sa, nullptr);
		sigaction(SIGPIPE, &sa, nullptr);
#else
		signal(SIGTERM, signalHandler);
		signal(SIGINT, signalHandler);
		signal(SIGPIPE, SIG_IGN);
#endif

		termRegistered = true;
	}
}


struct ParamRef {
	ParamRef() : param(nullptr) {}
	ParamRef(System::SchemaParameter *param, const string &reference)
	  : param(param), reference(reference) {}
	System::SchemaParameter* param;
	string                   reference;
};

typedef map<string, ParamRef> ParamMap;
void mapSchemaParameters(ParamMap &map, System::SchemaParameters *params,
                         const string &reference, const string &path = "") {
	if ( params == nullptr )
		return;

	string p = path.empty() ? "" : path + ".";
	for ( size_t i = 0; i < params->parameterCount(); ++i ) {
		System::SchemaParameter *param = params->parameter(i);
		map[p + param->name] = ParamRef(param, reference);
	}

	for ( size_t i = 0; i < params->groupCount(); ++i ) {
		System::SchemaGroup *group = params->group(i);
		mapSchemaParameters(map, group, reference, p + group->name);
	}
}

// comparison of strings separated into parts by '.' character
bool compare_string_toks(const string &a, const string &b) {
	vector<string> ta, tb;
	Core::split(ta, a.c_str(), ".", false);
	Core::split(tb, b.c_str(), ".", false);

	if ( ta.size() == 1 && tb.size() > 1 )
		return true;
	if ( ta.size() > 1 && tb.size() == 1 )
		return false;

	vector<string>::const_iterator a_it = ta.begin(), b_it = tb.begin();
	for ( ; a_it != ta.end() && b_it != tb.end(); ++a_it, ++b_it ) {
		int cmp = Core::compareNoCase(*a_it, *b_it);
		if ( cmp < 0 )
			return true;
		if ( cmp > 0 )
			return false;
	}

	return ta.size() <= tb.size();
}

// pad string to spefic length
inline string pad(const string &s, size_t len, char c = ' ') {
	return s.length() >= len ? s : (s + string(len-s.length(), c));
}

} // unnamed namespace


namespace Seiscomp {
namespace System {


Application *Application::_instance = nullptr;
bool Application::_handleTermination = true;
bool Application::_handleCrash = false;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::BaseSettings::BaseSettings()
: enableDaemon(true)
, certificateStoreDirectory("@ROOTDIR@/var/lib/certs")
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::BaseSettings::accept(SettingsLinker &linker) {
	linker
	& cfg(crashHandler, "scripts.crashHandler")
	& cfg(logging, "logging")
	& cliAsPath(
		alternativeConfigFile,
		"Generic", "config-file",
		"Use alternative configuration file"
	)
	& cli(
		plugins,
		"Generic", "plugins",
		"Load given plugins"
	)
	& cli(
		lockfile,
		"Verbose", "lockfile,l",
		"Path to lock file"
	)
	& cfg(certificateStoreDirectory, "certStore");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Application(int argc, char** argv) {
	_version = CurrentVersion.toString();

	if ( _instance != this && _instance != nullptr ) {
		SEISCOMP_WARNING("Another application object exists already. "
		                 "This usage is not intended. "
		                 "The Application::Instance() method will return "
		                 "the last created application.");
	}

	_instance = this;

	_commandline = boost::shared_ptr<System::CommandLine>(new System::CommandLine);

	_logger = nullptr;

	_argc = argc;
	_argv = new char*[size_t(argc)];

	_arguments.clear();
	for ( int i = 0; i < argc; ++i ) {
		_arguments.push_back(argv[i]);
		_argv[i] = new char[strlen(argv[i]) + 1];
		strcpy(_argv[i], argv[i]);
	}

	if ( argc > 0 )
		_name = argv[0];
	else
		_name = "";

	size_t pos = _name.rfind('/');
	if ( pos != string::npos )
		_name.erase(0, pos+1);

	pos = _name.rfind('\\');
	if ( pos != string::npos )
		_name.erase(0, pos+1);

	pos = _name.rfind('.');
	if ( pos != string::npos )
		_name.erase(pos);

	registerSignalHandler(_handleTermination, _handleCrash);

	_baseSettings.enableDaemon = true;

	_baseSettings.logging.verbosity = 2;
	_baseSettings.logging.context = false;
	_baseSettings.logging.component = -1; // -1=unset, 0=off, 1=on
	_baseSettings.logging.toStdout = false;
	_baseSettings.logging.UTC = false;

	_returnCode = 0;
	_exitRequested = false;

	bindSettings(&_baseSettings);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::~Application() {
	PluginRegistry::Reset();

	closeLogging();

	if ( _instance == this )
		_instance = nullptr;

	for ( int i = 0; i < _argc; ++i )
		delete[] _argv[i];

	delete[] _argv;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application* Application::Instance() {
	return _instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::HandleSignals(bool termination, bool crash) {
	_handleTermination = termination;
	_handleCrash = crash;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#ifdef WITH_SVN_REVISION
extern SC_SYSTEM_CLIENT_API const char* svn_revision();
#endif
#ifdef WITH_BUILD_INFOS
//extern const char* last_build();
extern SC_SYSTEM_CORE_API const char* build_system();
extern SC_SYSTEM_CORE_API const char* compiler_version();
#endif
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::printVersion() {
	const char *appVersion = version();
	if ( appVersion == nullptr ) {
		cout << name() << endl;
		cout << "Framework: " << frameworkVersion() << endl;
	}
	else {
		cout << name() << ": " << appVersion << endl;
		cout << "Framework: " << frameworkVersion() << endl;
	}

	cout << "API version: "
	     << SC_API_VERSION_MAJOR(SC_API_VERSION) << "."
	     << SC_API_VERSION_MINOR(SC_API_VERSION) << "."
	     << SC_API_VERSION_PATCH(SC_API_VERSION) << endl;

	cout << "Data schema version: "
	     << toString(Seiscomp::DataModel::Version().Major) << "."
	     << toString(Seiscomp::DataModel::Version().Minor) << endl;

	cout << CurrentVersion.systemInfo() << endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::printConfigVariables() {
	list<string> varList;
	Config::Variables::const_iterator cv_it = _configuration.getVariables().begin();
	for ( ; cv_it != _configuration.getVariables().end(); ++cv_it ) {
		varList.push_back(pad(cv_it->first, 50) + " " + cv_it->second);
	}

	varList.sort(compare_string_toks);
	cout << "available configuration variables:" << endl;
	list<string>::const_iterator it = varList.begin();
	for ( ; it != varList.end(); ++it ) {
		cout << "  " << *it << endl;
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::schemaValidationNames(vector<string> &modules,
                                        vector<string> &plugins) const {

	// process global and application secific modules
	modules.push_back("global");
	modules.push_back(name());

	// process all loaded plugins
	for ( PluginRegistry::iterator it = PluginRegistry::Instance()->begin();
		it != PluginRegistry::Instance()->end(); ++it ) {
		plugins.push_back(Util::removeExtension(Util::basename(it->filename)));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::validateSchemaParameters() {
	System::SchemaDefinitions defs;
	string path = Environment::Instance()->appConfigDir() + "/descriptions";
	if ( !defs.load(path.c_str()) ) {
		cerr << "Could not load schema definitions from directory " << path << endl;
		return false;
	}

	vector<string> moduleNames;
	vector<string> pluginNames;
	schemaValidationNames(moduleNames, pluginNames);

	ParamMap paramMap;

	// map module parameter path to SchemaParameter instance
	for ( vector<string>::const_iterator mn_it = moduleNames.begin();
	      mn_it != moduleNames.end(); ++mn_it ) {
		System::SchemaModule *module = defs.module(*mn_it);
		cerr << "Schema module '" << *mn_it << "' ";
		if ( module == nullptr ) {
			cerr << "not found" << endl;
		}
		else {
			cerr << "loaded" << endl;
			mapSchemaParameters(paramMap, module->parameters.get(),
			                    "module " + module->name);
		}

		System::SchemaDefinitions::PluginList plugins = defs.pluginsForModule(*mn_it);
		System::SchemaDefinitions::PluginList::const_iterator p_it = plugins.begin();
		for ( ; p_it != plugins.end(); ++p_it ) {
			vector<string>::iterator pn_it = find(pluginNames.begin(),
			                                      pluginNames.end(), (*p_it)->name);
			if ( pn_it != pluginNames.end() ) {
				mapSchemaParameters(paramMap, (*p_it)->parameters.get(),
				                    "plugin " + (*p_it)->name);
				cerr << "Schema plugin '" << *pn_it << "' loaded" << endl;
				pluginNames.erase(pn_it);
			}
		}
	}

	if ( paramMap.empty() ) {
		cerr << "No schema parameters found" << endl;
		return false;
	}

	map<string, string> typeMappings;
	typeMappings["color"] = "string";
	typeMappings["host-with-port"] = "string";
	typeMappings["gradient"] = "list:string";
	typeMappings["path"] = "string";

	list<string> missing;
	list<string> invalidType;
	list<string> emptyDesc;

	Config::Variables::const_iterator cv_it = _configuration.getVariables().begin();
	for ( ; cv_it != _configuration.getVariables().end(); ++cv_it ) {
		ParamMap::iterator pm_it = paramMap.find(cv_it->first);
		if ( pm_it == paramMap.end() ) {
			missing.push_back(cv_it->first);
			continue;
		}

		if ( cv_it->second != pm_it->second.param->type ) {
			map<string, string>::const_iterator tm_it =
			        typeMappings.find(pm_it->second.param->type);
			if ( tm_it == typeMappings.end() || tm_it->second != cv_it->second ) {
				string line = pad(cv_it->first, 40);
				line += " expected: " + cv_it->second + ", ";
				if ( pm_it->second.param->type.empty() )
					line += "undefined in schema";
				else
					line += "found in schema: " + pm_it->second.param->type;
				invalidType.push_back(line + " [" +
				                      pm_it->second.reference + "]");
			}
		}

		if ( pm_it->second.param->description.empty() ) {
			emptyDesc.push_back(pad(cv_it->first, 40) + " [" +
			                    pm_it->second.reference + "]");
		}

		paramMap.erase(pm_it);
	}

	if ( !missing.empty() ) {
		missing.sort(compare_string_toks);
		cout << endl
		     << "parameters missing in schema:" << endl;
		list<string>::const_iterator it = missing.begin();
		for ( ; it != missing.end(); ++it ) {
			cout << "  " << *it << endl;
		}
	}

	if ( !invalidType.empty() ) {
		invalidType.sort(compare_string_toks);
		cout << endl
		     << "parameters of invalid type:" << endl;
		list<string>::const_iterator it = invalidType.begin();
		for ( ; it != invalidType.end(); ++it ) {
			cout << "  " << *it << endl;
		}
	}

	if ( !emptyDesc.empty() ) {
		emptyDesc.sort(compare_string_toks);
		cout << endl
		     << "parameters without schema description:" << endl;
		list<string>::const_iterator it = emptyDesc.begin();
		for ( ; it != emptyDesc.end(); ++it ) {
			cout << "  " << *it << endl;
		}
	}

	if ( !paramMap.empty() ) {
		list<string> paramList;
		ParamMap::const_iterator pm_it = paramMap.begin();
		for ( ; pm_it != paramMap.end(); ++pm_it ) {
			string line = pad(pm_it->first, 40);
			paramList.push_back(line + " [" + pm_it->second.reference + "]");
		}
		paramList.sort(compare_string_toks);
		cout << endl
		     << "parameters found in schema but not read by application:" << endl;
		list<string>::const_iterator it = paramList.begin();
		for ( ; it != paramList.end(); ++it ) {
			cout << "  " << *it << endl;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Application::name() const {
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addPluginPackagePath(const string &package) {
	PluginRegistry::Instance()->addPackagePath(package);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Application::Arguments& Application::arguments() const {
	return _arguments;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Config::Config &Application::configuration() const {
	return _configuration;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* Application::path() const {
	return _argv[0];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setDaemonEnabled(bool enable) {
	_baseSettings.enableDaemon = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoggingContext(bool e) {
	_baseSettings.logging.context = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoggingComponent(bool e) {
	_baseSettings.logging.component = e ? 1 : 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoggingToStdErr(bool e) {
	_baseSettings.logging.toStdout = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addLoggingComponentSubscription(const string &c) {
	_baseSettings.logging.components.push_back(c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isExitRequested() const {
	return _exitRequested;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::printUsage() const {
	commandline().printOptions();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::closeLogging() {
	if ( _logger ) {
		delete _logger;
		_logger = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Application::crashHandler() const {
	return _baseSettings.crashHandler;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::bindSettings(AbstractSettings *settings) {
	_settings.push_back(settings);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Application::argumentStr(const string &query) const {
	string param = "--" + query;
	for ( size_t i = 1; i < _arguments.size(); ++i ) {
		if ( !_arguments[i].compare(0, param.size(), param) ) {
			string value = _arguments[i].substr(param.size());
			if ( !value.empty() && value[0] == '=' ) {
				value.erase(0, 1);
				return value;
			}
		}
	}

	throw Seiscomp::Config::OptionNotFoundException(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Application::configGetInt(const string &query) const {
	try {
		return atoi(argumentStr(query).c_str());
	}
	catch ( ... ) {}

	return _configuration.getInt(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Application::configGetDouble(const string &query) const {
	try {
		return atof(argumentStr(query).c_str());
	}
	catch ( ... ) {}

	return _configuration.getDouble(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::configGetBool(const string &query) const {
	try {
		return argumentStr(query) == "true";
	}
	catch ( ... ) {}

	return _configuration.getBool(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Application::configGetString(const string &query) const {
	try {
		return argumentStr(query);
	}
	catch ( ... ) {}

	return _configuration.getString(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Application::configGetPath(const string &query) const {
	try {
		return Environment::Instance()->absolutePath(argumentStr(query));
	}
	catch ( ... ) {}

	return Environment::Instance()->absolutePath(_configuration.getString(query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<int> Application::configGetInts(const string &query) const {
	return _configuration.getInts(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<double> Application::configGetDoubles(const string &query) const {
	return _configuration.getDoubles(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<bool> Application::configGetBools(const string &query) const {
	return _configuration.getBools(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<string> Application::configGetStrings(const string &query) const {
	try {
		string param = argumentStr(query);
		vector<string> tmp;
		Core::split(tmp, param.c_str(), ",");
		for ( size_t i = 0; i < tmp.size(); ++i )
			Core::trim(tmp[i]);
		return tmp;
	}
	catch ( ... ) {}

	return _configuration.getStrings(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetBool(const string &query, bool v) {
	_configuration.setBool(query, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetInt(const string &query, int v) {
	_configuration.setInt(query, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetDouble(const string &query, double v) {
	_configuration.setDouble(query, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetString(const string &query, const string &v) {
	_configuration.setString(query, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetBools(const string &query,
                                 const vector<bool> &values) {
	_configuration.setBools(query, values);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetInts(const string &query,
                                const vector<int> &values) {
	_configuration.setInts(query, values);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetDoubles(const string &query,
                                   const vector<double> &values) {
	_configuration.setDoubles(query, values);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetStrings(const string &query,
                                   const vector<string> &values) {
	_configuration.setStrings(query, values);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configUnset(const string &query) {
	_configuration.remove(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::saveConfiguration() {
	return _configuration.writeConfig(Environment::Instance()->configFileName(name()),
	                                  true, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Application::operator()() {
	return exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Application::exec() {
	_exitRequested = false;
	_returnCode = 1;

	// Query current locale
	string current_locale = setlocale(LC_ALL, nullptr);

	if ( init() ) {
		_returnCode = 0;

		// If run return false and the returnCode still indicates no error
		// it is set to 1 to indicate wrong behaviour.
		if ( !run() && _returnCode == 0 )
			_returnCode = 1;
	}

	_exitRequested = true;

	done();

	if ( !_baseSettings.logging.toStdout ) SEISCOMP_NOTICE("Shutdown");

	// Restore locale
	setlocale(LC_ALL, current_locale.c_str());

	return _returnCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initPlugins() {
	PluginRegistry::Instance()->addPackagePath(name());

	if ( !_baseSettings.plugins.empty() ) {
		vector<string> tokens;
		Core::split(tokens, _baseSettings.plugins.c_str(), ",");
		vector<string>::iterator it = tokens.begin();
		for ( ; it != tokens.end(); ++it )
			PluginRegistry::Instance()->addPluginName(Core::trim(*it));

		if ( PluginRegistry::Instance()->loadPlugins() < 0 ) {
			SEISCOMP_ERROR("Failed to load all requested plugins, bailing out");
			return false;
		}
	}
	else {
		if ( PluginRegistry::Instance()->loadConfiguredPlugins(&_configuration) < 0 ) {
			SEISCOMP_ERROR("Failed to load all requested plugins, bailing out");
			return false;
		}
	}

	if ( PluginRegistry::Instance()->pluginCount() ) {
		string pluginList;
		pluginList = "\nPlugins:\n"
					  "--------\n";
		int idx = 1;
		for ( PluginRegistry::iterator it = PluginRegistry::Instance()->begin();
			it != PluginRegistry::Instance()->end(); ++it ) {
			pluginList += " [" + toString(idx) + "]\n";
			pluginList += "  description: " + (*it)->description().description + "\n";
			pluginList += "       author: " + (*it)->description().author + "\n";
			pluginList += "      version: " + toString((*it)->description().version.major)
			                                + "." + toString((*it)->description().version.minor)
			                                + "." + toString((*it)->description().version.revision)
			                                + "\n";
			pluginList += "          API: " + toString(SC_API_VERSION_MAJOR((*it)->description().apiVersion))
			                                + "." + toString(SC_API_VERSION_MINOR((*it)->description().apiVersion))
			                                + "." + toString(SC_API_VERSION_PATCH((*it)->description().apiVersion))
			                                + "\n";
			++idx;
		}

		SEISCOMP_INFO("%s", pluginList.c_str());
	}
	else
		SEISCOMP_INFO("No plugins loaded");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handleCommandLineOptions() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handlePreFork() {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::init() {
	setlocale(LC_ALL, "C");

	Logging::enableConsoleLogging(Logging::getGlobalChannel("error"));

	showMessage("Reading configuration");

	// First commandline parsing stage
	initCommandLine();

	if ( !parseCommandLine() ) {
		printUsage();
		exit(-1);
		return false;
	}

	// Enable tracking of configuration variables output or validation of those
	// is requested on commandline
	_configuration.trackVariables(commandline().hasOption("print-config-vars") ||
	                              commandline().hasOption("validate-schema-params"));

	if ( !initConfiguration() ) {
		exit(-1);
		return false;
	}

	_commandline.reset();
	_commandline = boost::shared_ptr<System::CommandLine>(new System::CommandLine);
	initCommandLine();

	if ( !parseCommandLine() ) {
		printUsage();
		exit(-1);
		return false;
	}

	if ( commandline().hasOption("help") ) {
		printUsage();
		exit(0);
		return false;
	}

	if ( commandline().hasOption("version") ) {
		printVersion();
		exit(0);
		return false;
	}

	if ( handleCommandLineOptions() ) {
		exit(-1);
		return false;
	}

	/*
	if ( !_settings.parse(commandline()) ) {
		cerr << "Try --help for help" << endl;
		exit(-1);
		return false;
	}
	*/

	_settingsLinker.reset();
	_settingsLinker.proc().get(&commandline());
	for ( list<AbstractSettings*>::iterator it = _settings.begin();
	      it != _settings.end(); ++it )
		(*it)->accept(_settingsLinker);
	if ( !_settingsLinker ) {
		cerr << _settingsLinker.lastError() << endl;
		cerr << "Try --help for help" << endl;
		exit(1);
		return false;
	}

	if ( !validateParameters() ) {
		cerr << "Try --help for help" << endl;
		exit(2);
		return false;
	}

	showMessage("Initialize logging");
	if ( !initLogging() ) {
		if ( !handleInitializationError(LOGGING) )
			return false;
	}

	showMessage("Loading plugins");
	if ( !initPlugins() ) {
		if ( !handleInitializationError(PLUGINS) )
			return false;
	}

	if ( commandline().hasOption("print-config-vars") ) {
		printConfigVariables();
		exit(0);
		return false;
	}

	if ( commandline().hasOption("validate-schema-params") ) {
		validateSchemaParameters();
		exit(0);
		return false;
	}

	if ( commandline().hasOption("dump-settings") ) {
		_settingsLinker.reset();
		_settingsLinker.proc().dump(cout);
		for ( list<AbstractSettings*>::iterator it = _settings.begin();
		      it != _settings.end(); ++it )
			(*it)->accept(_settingsLinker);
		exit(0);
		return false;
	}

	if ( !handlePreFork() ) {
		exit(3);
		return false;
	}

	if ( commandline().hasOption("daemon") ) {
		if ( !forkProcess() ) {
			cerr << "FATAL: Process forking failed" << endl;
			exit(4);
			return false;
		}
	}

	if ( _baseSettings.lockfile.length() > 0 ) {
		int r = acquireLockfile(_baseSettings.lockfile);
		if ( r < 0 ) {
			// Error should have been reported by acquireLockfile()
			exit(5);
			return false;
		}
		else if ( r == 0 ) {
			SEISCOMP_ERROR("Already running");
			exit(6);
			return false;
		}
	}

	if ( !_baseSettings.certificateStoreDirectory.empty() ) {
		string absolutePath = Environment::Instance()->absolutePath(_baseSettings.certificateStoreDirectory);
		if ( Util::pathExists(absolutePath) ) {
			if ( !Util::CertificateStore::global().init(absolutePath) )
				return false;
		}
	}

	if ( _exitRequested )
		return false;

	showMessage("");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handleInitializationError(int) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createBaseCommandLineDescription() {
	commandline().addGroup("Generic");
	//commandline().addOption("Generic", "name,N", "set application name used for configuration file lookup", &_name);
	commandline().addOption("Generic", "help,h", "Produce help message.");
	commandline().addOption("Generic", "version,V", "Show version information.");
	commandline().addOption("Generic", "print-config-vars", "Print all available configuration variables and exit.");
	commandline().addOption("Generic", "validate-schema-params", "Validates the applications description xml and exit.");
	if ( !_settings.empty() )
		commandline().addOption("Generic", "dump-settings", "Dump the bound settings and exit.");
	//commandline().addOption("Generic", "crash-handler", "path to crash handler script", &_crashHandler);

	if ( _baseSettings.enableDaemon )
		commandline().addOption("Generic", "daemon,D", "Run as daemon.");

	commandline().addGroup("Verbose");
	commandline().addCustomOption("Verbose", "v,v", "Increase verbosity level (may be repeated, eg. -vv).", new FlagCounter(&_baseSettings.logging.verbosity));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::initCommandLine() {
	_settingsLinker.reset();
	_settingsLinker.proc().bind(&commandline());
	for ( list<AbstractSettings*>::iterator it = _settings.begin();
	      it != _settings.end(); ++it )
		(*it)->accept(_settingsLinker);

	createBaseCommandLineDescription();
	createCommandLineDescription();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *Application::version() {
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createCommandLineDescription() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::validateParameters() {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::parseCommandLine() {
	return commandline().parse(_argc, _argv, [](const string &arg) {
		return !arg.compare(0, 2, "--") && arg.find("=", 2) != string::npos;
	});
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initConfiguration() {
//	if ( !_configuration.readConfig(Environment::Instance()->configFileName()) ) {
//		std::cerr << "An error occured while reading configuration file: "
//		          << Environment::Instance()->configFileName()
//		          << std::endl;
//		return false;
//	}

	if ( _baseSettings.alternativeConfigFile.empty() ) {
		if ( !Environment::Instance()->initConfig(&_configuration, name()) ) {
			SEISCOMP_ERROR("Configuration file errors found, abort");
			return false;
		}
	}
	else {
		_baseSettings.alternativeConfigFile = Environment::Instance()->absolutePath(_baseSettings.alternativeConfigFile);
		if ( !Util::fileExists(_baseSettings.alternativeConfigFile) ) {
			SEISCOMP_ERROR("Could not find alternative configuration file %s, abort", _baseSettings.alternativeConfigFile.c_str());
			return false;
		}
		if ( !_configuration.readConfig(_baseSettings.alternativeConfigFile) ) {
			SEISCOMP_ERROR("Error found in alternative configuration file %s, abort", _baseSettings.alternativeConfigFile.c_str());
			return false;
		}
	}

	_settingsLinker.reset();

	_settingsLinker.proc().get(this);
	for ( list<AbstractSettings*>::iterator it = _settings.begin();
	      it != _settings.end(); ++it )
		(*it)->accept(_settingsLinker);
	if ( !_settingsLinker ) {
		cerr << _settingsLinker.lastError() << endl;
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initLogging() {
	Logging::disableConsoleLogging();

	if ( _baseSettings.logging.quiet )
		return true;

	bool enableLogging = _baseSettings.logging.verbosity > 0;

	if ( _baseSettings.logging.trace || _baseSettings.logging.debug ) {
		enableLogging = true;
		_baseSettings.logging.verbosity = 4;
		_baseSettings.logging.toStdout = true;
		if ( _baseSettings.logging.trace ) {
			_baseSettings.logging.context = true;
			_baseSettings.logging.component = 1;
		}
	}

	if ( enableLogging ) {
		//cerr << "using loglevel " << _verbosity << endl;
#ifndef WIN32
		if ( _baseSettings.logging.syslog ) {
			Logging::SyslogOutput* syslogOutput = new Logging::SyslogOutput();
			const char *facility = nullptr;
			string tmp_facility;

			try {
				tmp_facility = configGetString("logging.syslog.facility");
				facility = tmp_facility.c_str();
			}
			catch ( ... ) {}

			if ( syslogOutput->open(_name.c_str(), facility) ) {
				cerr << "using syslog: " << _name << ", "
				     << (facility?facility:"default") << "(code="
				     << syslogOutput->facility() << ")" << endl;
				_logger = syslogOutput;
			}
			else {
				cerr << "failed to open syslog: " << _name << endl;
				delete syslogOutput;
				syslogOutput = nullptr;
				return false;
			}
		}
		else
#endif
		if ( !_baseSettings.logging.toStdout ) {
			string logFile = _baseSettings.logging.alternativeLogFile;
			if ( logFile.empty() )
				logFile = Environment::Instance()->logFile(_name.c_str());

			Logging::FileOutput* logger;
			if ( _baseSettings.logging.file.rotator.enable )
				logger = new Logging::FileRotatorOutput(
					_baseSettings.logging.file.rotator.timeSpan,
					_baseSettings.logging.file.rotator.archiveSize,
					_baseSettings.logging.file.rotator.maxFileSize
				);
			else
				logger = new Logging::FileOutput();

			if ( logger->open(logFile.c_str()) ) {
				//cerr << "using logfile: " << logFile << endl;
				_logger = logger;
			}
			else {
				cerr << "failed to open logfile: " << logFile << endl;
				delete logger;
				logger = nullptr;
			}
		}
		else
			_logger = new Logging::FdOutput(STDERR_FILENO);

		if ( _logger ) {
			_logger->setUTCEnabled(_baseSettings.logging.UTC);
			_logger->logComponent(_baseSettings.logging.component < 0 ? !_baseSettings.logging.toStdout : _baseSettings.logging.component);
			_logger->logContext(_baseSettings.logging.context);
			if ( !_baseSettings.logging.components.empty() ) {
				for ( ComponentList::iterator it = _baseSettings.logging.components.begin();
				      it != _baseSettings.logging.components.end(); ++it ) {
					_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "notice"));
					switch ( _baseSettings.logging.verbosity ) {
						default:
						case 4:
							_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "debug"));
						case 3:
							_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "info"));
						case 2:
							_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "warning"));
						case 1:
							_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "error"));
					}
				}
			}
			else {
				_logger->subscribe(Logging::getGlobalChannel("notice"));
				switch ( _baseSettings.logging.verbosity ) {
					default:
					case 4:
						_logger->subscribe(Logging::getGlobalChannel("debug"));
					case 3:
						_logger->subscribe(Logging::getGlobalChannel("info"));
					case 2:
						_logger->subscribe(Logging::getGlobalChannel("warning"));
					case 1:
						_logger->subscribe(Logging::getGlobalChannel("error"));
				}
			}
		}
		else
			return false;
	}

	if ( !_baseSettings.logging.toStdout ) {
		const char *appVersion = version();
		SEISCOMP_NOTICE("Starting %s %s", name().c_str(), appVersion?appVersion:"");
		SEISCOMP_NOTICE("  Framework   : %s", frameworkVersion());
		SEISCOMP_NOTICE("  API Version : %d.%d.%d",
		                SC_API_VERSION_MAJOR(SC_API_VERSION),
		                SC_API_VERSION_MINOR(SC_API_VERSION),
		                SC_API_VERSION_PATCH(SC_API_VERSION));
		SEISCOMP_NOTICE("  Version     : %s", CurrentVersion.systemInfo().c_str());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::forkProcess() {
#ifndef WIN32
	pid_t pid;

	// Become a session leader to lose controlling TTY.
	if ( (pid = fork()) < 0 ) {
		SEISCOMP_ERROR("can't fork: %s", strerror(errno));
		return false;
	}
	else if ( pid != 0 ) // parent
		::exit(0);

	if ( setsid() < 0 ) {
		SEISCOMP_ERROR("setsid: %s", strerror(errno));
		return false;
	}

	// Ensure future opens won't allocate controlling TTYs. The user Nominal Animal
	// gave a good explaination about the usage of the SIGHUP signal in combination
	// with daemon processes in his post on stack overlow:
	//
	// https://stackoverflow.com/questions/53672944/sighup-signal-handling-to-deamonize-a-command-in-unix-system-programming
	//
	// "Yes, the parent process forks a child process, and that child does setsid()
	// so that it will be the process group leader (and the only process) in the new
	// process group, and have no controlling terminal. That last part is the key.
	//
	// (If there was a reason why the child process should run in the same process
	// group as the parent process, one could use int fd = open("/dev/tty", O_RDWR);
	// if (fd != -1) ioctl(fd, TIOCNOTTY); to detach from the controlling terminal.
	// setsid() is easier, and it is usually preferable to have the child run in a
	// new process group anyway, as it and its children can be sent a signal without
	// affecting any other processes.)
	// Now, whenever a process that has no controlling terminal opens a terminal
	// device (a tty or a pseudo-tty), that device will become its controlling terminal
	// (unless the O_NOCTTY flag was used when opening the device).
	// Whenever the controlling terminal is disconnected, a SIGHUP signal is delivered
	// to each process having that terminal as their controlling terminal.
	// (That SIG_UP thing is just a typo. Signal names do not have an underscore,
	// only the special handlers SIG_DFL, SIG_IGN, and SIG_ERR do.)

	// If the daemon process opens a terminal device for any reason -- for example,
	// because a library wants to print an error message to a console, and opens /dev/tty1
	// or similar to do so --, the daemon will inadvertently acquire a controlling terminal.
	// Other than interposing open(), fopen(), opendir(), and so on, to ensure their
	// underlying open() flags will include O_NOCTTY, there is not much a daemon can do to
	// ensure it will not inadvertently acquire a controlling terminal. Instead, the easier
	// option is to just assume that it might, and simply ensure that that does not cause
	// too much trouble. To avoid the most typical issue, dying from SIGHUP when the controlling
	// terminal is disconnected, the daemon process can simply ignore the delivery of the
	// SIGHUP signal."

	// In short, it is a belt-and-suspenders approach. The setsid() detaches the process
	// from the controlling terminal; and SIGHUP is ignored in case the daemon inadvertently
	// acquires a controlling terminal by opening a tty device without using the O_NOCTTY flag."

	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if ( sigaction(SIGHUP, &sa, nullptr) < 0 ) {
		SEISCOMP_ERROR("can't ignore SIGHUP: %s", strerror(errno));
		return false;
	}

	if ( (pid = fork()) < 0 ) {
		SEISCOMP_ERROR("can't fork: %s", strerror(errno));
		return false;
	}
	else if ( pid != 0 ) // parent
		::exit(0);

	// Attach file descriptors 0, 1, and 2 to /dev/null.
	close(0);
	close(1);
	close(2);
	int fd0 = open("/dev/null", O_RDWR);
	int fd1 = dup(0);
	int fd2 = dup(0);

	if ( fd0 != 0 || fd1 != 1 || fd2 != 2 ) {
		SEISCOMP_ERROR("forkProcess: unexpected file descriptors %d %d %d", fd0, fd1, fd2);
		return false;
	}

	return true;
#else
	return false;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Application::acquireLockfile(const string &lockfile) {
#ifndef WIN32
	int fd = open(lockfile.c_str(), O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if( fd < 0 ) {
		SEISCOMP_ERROR("could not open %s: %s", lockfile.c_str(), strerror(errno));
		return -1;
	}
	else if ( fd <= 2 ) {
		SEISCOMP_ERROR("acquireLockfile: unexpected file descriptor %d", fd);
		return -1;
	}

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	if ( fcntl(fd, F_SETLK, &lock ) < 0 ) {
		close(fd);
		if(errno == EACCES || errno == EAGAIN) return 0; // already locked

		SEISCOMP_ERROR("could not lock %s: %s\n", lockfile.c_str(), strerror(errno));
		return -1;
	}

	if ( ftruncate(fd, 0) < 0 ) {
		SEISCOMP_ERROR("ftruncate: %s", strerror(errno));
		return -1;
	}

	char buf[30];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%d", getpid());
	size_t pid_len = strlen(buf);

	if ( static_cast<size_t>(write(fd, buf, pid_len)) != pid_len ) {
		SEISCOMP_ERROR("could not write pid file at %s: %s\n", lockfile.c_str(), strerror(errno));
		return -1;
	}

	int val;
	if ( (val = fcntl(fd, F_GETFD,0)) < 0 ) {
		SEISCOMP_ERROR("fcntl: %s", strerror(errno));
		return -1;
	}

	val |= FD_CLOEXEC;
	if ( fcntl(fd, F_SETFD, val) < 0 ) {
		SEISCOMP_ERROR("fcntl: %s", strerror(errno));
		return -1;
	}

	// locking successful
	return fd;
#else
	return -1;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showMessage(const char*) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showWarning(const char*) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exit(int returnCode) {
	_returnCode = returnCode;
	_exitRequested = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::quit() {
	this->exit(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleInterrupt(int) {
	this->exit(_returnCode);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *Application::frameworkVersion() const {
	return _version.c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Detail {

template <>
bool getConfig(const Application *app, const string &symbol, bool) {
	return app->configGetBool(symbol);
}

template <>
int getConfig(const Application *app, const string &symbol, bool) {
	return app->configGetInt(symbol);
}

template <>
unsigned int getConfig(const Application *app, const string &symbol, bool) {
	return static_cast<unsigned int>(app->configGetInt(symbol));
}

template <>
double getConfig(const Application *app, const string &symbol, bool) {
	return app->configGetDouble(symbol);
}

template <>
vector<bool> getConfig(const Application *app, const string &symbol, bool) {
	return app->configGetBools(symbol);
}

template <>
vector<int> getConfig(const Application *app, const string &symbol, bool) {
	return app->configGetInts(symbol);
}

template <>
vector<double> getConfig(const Application *app, const string &symbol, bool) {
	return app->configGetDoubles(symbol);
}

template <>
string getConfig(const Application *app, const string &symbol, bool asPath) {
	if ( asPath )
		return Environment::Instance()->absolutePath(app->configGetString(symbol));
	else
		return app->configGetString(symbol);
}

template <>
vector<string> getConfig(const Application *app, const string &symbol, bool asPath) {
	if ( !asPath )
		return app->configGetStrings(symbol);

	vector<string> items = app->configGetStrings(symbol);
	for ( size_t i = 0; i < items.size(); ++i )
		items[i] = Environment::Instance()->absolutePath(items[i]);

	return items;
}

string join(const string &prefix, const char *relativeName) {
	string symbolName = prefix;
	if ( !symbolName.empty() && *relativeName ) {
		symbolName += '.';
	}
	symbolName += relativeName;
	return symbolName;
}

}
}
}
