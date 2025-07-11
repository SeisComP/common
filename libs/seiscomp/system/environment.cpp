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


#define SEISCOMP_COMPONENT Environment
#include <seiscomp/logging/log.h>

#include <string>
#include <stdlib.h>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <climits>
#include <cstdlib>

#if WIN32
#include <direct.h>
#include <shlobj.h>
#endif

#include <seiscomp/system/environment.h>
#include <seiscomp/utils/files.h>
#include <seiscomp/utils/replace.h>


namespace Seiscomp {


std::unique_ptr<Environment> Environment::_instance;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Environment::Environment() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Environment::~Environment() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Environment::init() {
	const char* installDir = getenv("SEISCOMP_ROOT");
	if ( !installDir ) {
#ifdef SEISCOMP_ROOT
		_installDir = SEISCOMP_ROOT;
		SEISCOMP_DEBUG("Setting predefined installdir: %s", _installDir);
#else
		_installDir = ".";
		SEISCOMP_DEBUG("Guessing installdir: %s", _installDir.c_str());
#endif
	}
	else {
		_installDir = installDir;
		SEISCOMP_DEBUG("Setting installdir from $SEISCOMP_ROOT: %s", _installDir);
	}


#ifdef SEISCOMP_SHARE_DIR
	_shareDir = _installDir + "/" SEISCOMP_SHARE_DIR;
#else
	_shareDir = _installDir + "/share";
#endif

#ifndef WIN32
	const char *homeDir = getenv("HOME");
	if ( !homeDir ) {
		SEISCOMP_WARNING("Could not read home directory!");
		_homeDir = ".";
	}
#else
	char homeDir[MAX_PATH];
	if ( SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, homeDir) != S_OK ) {
		SEISCOMP_WARNING("Could not get local application data path!");
		_homeDir = ".";
	}
#endif
	else
		_homeDir.assign(homeDir);


#ifdef SEISCOMP_CONFIG_DIR
	_globalConfigDir  = _installDir + "/" SEISCOMP_CONFIG_DIR;
#else
	_globalConfigDir  = _installDir + "/etc/defaults";
#endif

	_appConfigDir = _installDir + "/etc";

	const char* localConfigDir = getenv("SEISCOMP_LOCAL_CONFIG");
	if ( !localConfigDir ) {
#ifndef WIN32
		_localConfigDir  = _homeDir + "/.seiscomp";
#else
		_localConfigDir  = _homeDir + "/seiscomp";
#endif
	}
	else {
		_localConfigDir  = localConfigDir;
	}

	SEISCOMP_INFO("using local config dir: %s", _localConfigDir);

	_logDir           = _localConfigDir + "/log";
	_archiveFileName  = "_archive.log";

	if ( !createDir(_localConfigDir) ) {
		SEISCOMP_WARNING( "Could not create directory: %s", _localConfigDir);
	}

	if ( !createDir(_logDir) ) {
		SEISCOMP_ERROR("Could not create directory: %s", _logDir);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Environment* Environment::Instance() {
	if ( !_instance.get() ) {
		_instance = std::unique_ptr<Environment>(new Environment);
		if ( !_instance->init() )
			_instance.release();
	}

	return _instance.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Environment::logFile(const std::string& name) const {
	return logDir() + "/" + Util::removeExtension(Util::basename(name)) + ".log";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Environment::configFileLocation(const std::string& name, int stage) const {
	switch ( stage ) {
		case CS_DEFAULT_GLOBAL:
			return _globalConfigDir + "/global.cfg";
		case CS_DEFAULT_APP:
			return _globalConfigDir + "/" + Util::removeExtension(Util::basename(name)) + ".cfg";
		case CS_CONFIG_GLOBAL:
			return _appConfigDir + "/global.cfg";
		case CS_CONFIG_APP:
			return _appConfigDir + "/" + Util::removeExtension(Util::basename(name)) + ".cfg";
		case CS_USER_GLOBAL:
			return _localConfigDir + "/global.cfg";
		case CS_USER_APP:
			return _localConfigDir + "/" + Util::removeExtension(Util::basename(name)) + ".cfg";
		default:
			break;
	}

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define CHECK_RANGE(x, l, u) x >= l && x <= u
bool Environment::initConfig(Config::Config *config, const std::string& name,
                             int fromStage, int toStage, bool standalone) const {
	std::string globalConfig, appConfig;

	globalConfig = configFileLocation(name, CS_DEFAULT_GLOBAL);
	appConfig = configFileLocation(name, CS_DEFAULT_APP);

	if ( !standalone ) {
		if ( CHECK_RANGE(CS_DEFAULT_GLOBAL, fromStage, toStage) ) {
			if ( appConfig != globalConfig && Util::fileExists(globalConfig) )
				if ( !config->readConfig(globalConfig, CS_DEFAULT_GLOBAL) ) return false;
		}
	}

	if ( CHECK_RANGE(CS_DEFAULT_APP, fromStage, toStage) ) {
		if ( Util::fileExists(appConfig) )
			if ( !config->readConfig(appConfig, CS_DEFAULT_APP) ) return false;
	}

	globalConfig = configFileLocation(name, CS_CONFIG_GLOBAL);
	appConfig = configFileLocation(name, CS_CONFIG_APP);

	if ( !standalone ) {
		if ( CHECK_RANGE(CS_CONFIG_GLOBAL, fromStage, toStage) ) {
			if ( appConfig != globalConfig && Util::fileExists(globalConfig) )
				if ( !config->readConfig(globalConfig, CS_CONFIG_GLOBAL) ) return false;
		}
	}

	if ( CHECK_RANGE(CS_CONFIG_APP, fromStage, toStage) ) {
		if ( Util::fileExists(appConfig) )
			if ( !config->readConfig(appConfig, CS_CONFIG_APP) ) return false;
	}

	globalConfig = configFileLocation(name, CS_USER_GLOBAL);
	appConfig = configFileLocation(name, CS_USER_APP);

	if ( !standalone ) {
		if ( CHECK_RANGE(CS_USER_GLOBAL, fromStage, toStage) ) {
			if ( appConfig != globalConfig && Util::fileExists(globalConfig) )
				if ( !config->readConfig(globalConfig, CS_USER_GLOBAL) ) return false;
		}
	}

	if ( CHECK_RANGE(CS_USER_APP, fromStage, toStage) ) {
		if ( Util::fileExists(appConfig) )
			if ( !config->readConfig(appConfig, CS_USER_APP) ) return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Environment::createDir(const std::string& dir) const {
	bool success = true;
#if WIN32
	int ret = _mkdir( dir.c_str() );
#else
	int ret = mkdir( dir.c_str(), 0777 );
#endif
	if ( ret < 0 && errno != EEXIST )
	{
		SEISCOMP_ERROR( "Could not create directory: %s", dir.c_str() );
		success = false;
	}
	return success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Environment::configFileName(const std::string& programname) const {
	return _localConfigDir + "/" + Util::removeExtension(Util::basename(programname)) + ".cfg";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Environment::appConfigFileName(const std::string& programname) const {
	return _appConfigDir + "/" + Util::removeExtension(Util::basename(programname)) + ".cfg";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Environment::globalConfigFileName(const std::string& programname) const {
	return _globalConfigDir + "/" + Util::removeExtension(Util::basename(programname)) + ".cfg";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

struct PathResolver : public Util::VariableResolver {
	bool resolve(std::string& variable) const override {
		Environment *env = Environment::Instance();

		if ( variable == "LOGDIR" )
			variable = env->logDir();
		else if ( variable == "CONFIGDIR" )
			variable = env->configDir();
		else if ( variable == "DEFAULTCONFIGDIR" )
			variable = env->globalConfigDir();
		else if	( variable == "SYSTEMCONFIGDIR" )
			variable = env->appConfigDir();
		else if ( variable == "ROOTDIR" )
			variable = env->installDir();
		else if ( variable == "DATADIR" )
			variable = env->shareDir();
		else if ( variable == "KEYDIR" )
			variable = env->appConfigDir() + "/key";
		else if ( variable == "HOMEDIR" ) {
#ifndef WIN32
			const char *homeDir = getenv("HOME");
			if (!homeDir)
				return false;
#else
			char homeDir[MAX_PATH];
			if ( SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, homeDir) != S_OK )
				return false;
#endif
			else
				variable = homeDir;
		}
		else
			return false;

		return true;
	}
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Environment::absolutePath(const std::string& name) const {
	std::string tmpName = Util::replace(name, PathResolver());

	if ( tmpName.find("~/") == 0 )
		tmpName = homeDir() + tmpName.substr(1);

	bool trailingSlash = !tmpName.empty() && tmpName[tmpName.size()-1] == '/';

	char absolutePath[PATH_MAX];
	if ( realpath(tmpName.c_str(), absolutePath) )
		tmpName = absolutePath;

	if ( trailingSlash && !tmpName.empty() && tmpName[tmpName.size()-1] != '/' )
		tmpName += '/';

	return tmpName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Environment::resolvePath(const std::string& name) const {
	return Util::replace(name, PathResolver());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Seiscomp
