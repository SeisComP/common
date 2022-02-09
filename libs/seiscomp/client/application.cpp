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

#include <seiscomp/core/platform/platform.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/core/interruptible.h>
#include <seiscomp/core/system.h>

#include <seiscomp/logging/fd.h>
#include <seiscomp/logging/filerotator.h>
#ifndef WIN32
#include <seiscomp/logging/syslog.h>
#endif

#include <seiscomp/system/pluginregistry.h>

#include <seiscomp/math/geo.h>

#include <seiscomp/utils/files.h>
#include <seiscomp/utils/timer.h>
#include <seiscomp/utils/replace.h>

#include <seiscomp/seismology/regions.h>

#include <seiscomp/client/application.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/client/configdb.h>
#include <seiscomp/client/queue.ipp>

#include <seiscomp/datamodel/config.h>
#include <seiscomp/datamodel/configmodule.h>
#include <seiscomp/datamodel/configstation.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/version.h>

#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/recordstream.h>

#include <seiscomp/messaging/messages/database.h>

#include <seiscomp/utils/files.h>

#include <sstream>
#include <cerrno>
#include <functional>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#ifdef __SUNPRO_CC
#include <sys/stat.h>
#endif

#ifdef WIN32
#define snprintf _snprintf
#define popen _popen
#define pclose _pclose
#define STDERR_FILENO 2
#endif

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::IO;


namespace {


struct AppResolver : public Util::VariableResolver {
	AppResolver(const std::string& name)
	 : _name(name) {}

	bool resolve(std::string& variable) const {
		if ( Util::VariableResolver::resolve(variable) )
			return true;

		if ( variable == "appname" )
			variable = _name;
		else
			return false;

		return true;
	}

	const std::string &_name;
};


struct CityLessThan {
	bool operator()(const Math::Geo::CityD &x, const Math::Geo::CityD &y) {
		return x.population() < y.population();
	}
};

struct CityGreaterThan {
	bool operator()(const Math::Geo::CityD &x, const Math::Geo::CityD &y) {
		return x.population() > y.population();
	}
};

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
	if ( !params )
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

bool configDefault(const System::Application::Arguments &args,
                   const Config::Config &config, const string &query) {
	std::string param = "--" + query + "=";
	for ( size_t i = 1; i < args.size(); ++i ) {
		if ( !args[i].compare(0, param.size(), param) )
			return false;
	}

	Config::Symbol *symbol = config.symbolTable()->get(query);
	return symbol != nullptr && (
	       symbol->stage == Environment::CS_DEFAULT_GLOBAL ||
	       symbol->stage == Environment::CS_DEFAULT_APP);
}


} // private namespace


namespace Seiscomp {
namespace Client {


Application* Application::_instance = nullptr;


IMPLEMENT_SC_CLASS_DERIVED(ApplicationStatusMessage, Message, "app_stat_msg");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatusMessage::ApplicationStatusMessage() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatusMessage::ApplicationStatusMessage(const std::string &module,
                                                   ApplicationStatus status)
: Message(), _module(module), _status(status) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatusMessage::ApplicationStatusMessage(const std::string &module,
                                                   const std::string &username,
                                                   ApplicationStatus status)
: Message(), _module(module), _username(username), _status(status) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ApplicationStatusMessage::empty() const { return false; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &ApplicationStatusMessage::module() const { return _module; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &ApplicationStatusMessage::username() const { return _username; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatus ApplicationStatusMessage::status() const { return _status; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ApplicationStatusMessage::serialize(Archive& ar) {
	ar & TAGGED_MEMBER(module);
	ar & TAGGED_MEMBER(username);
	ar & TAGGED_MEMBER(status);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::AppSettings::Messaging::accept(SettingsLinker &linker) {
	linker

	& cfg(user, "username")
	& cfg(URL, "server")
	& cfg(primaryGroup, "primaryGroup")
	& cfg(subscriptions, "subscriptions")
	& cfg(contentType, "encoding")
	& cfg(timeout, "timeout")

	& cli(
		user, "Messaging", "user,u",
		"Client name used when connecting to the messaging.",
		true
	)
	& cli(
		URL, "Messaging", "host,H",
		"Messaging URL (host[:port][/queue]).", true
	)
	& cli(
		timeout, "Messaging", "timeout,t",
		"Connection timeout in seconds.", true
	)
	& cli(
		primaryGroup, "Messaging", "primary-group,g",
		"The primary message group of the client.", true
	)
	& cli(
		subscriptions, "Messaging", "subscribe-group,S",
		"A group to subscribe to. This option can be given more than once.",
		true
	)
	& cli(
		contentType, "Messaging", "content-type",
		"Sets the message content type (binary, json or xml).",
		true
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::AppSettings::Client::accept(SettingsLinker &linker) {
	linker

	& cfg(startStopMessages, "startStopMessage")
	& cfg(autoShutdown, "autoShutdown")
	& cfg(shutdownMasterModule, "shutdownMasterModule")
	& cfg(shutdownMasterUsername, "shutdownMasterUsername")

	& cli(
		startStopMessages, "Messaging", "start-stop-msg",
		"Sets sending of a start- and a stop message.", true
	)
	& cli(
		autoShutdown, "Generic", "auto-shutdown",
		"Enables automatic application shutdown triggered by a status message.",
		true
	)
	& cli(
		shutdownMasterModule, "Generic", "shutdown-master-username",
		"Triggers shutdown if the user name of the received messages match.",
		false
	)
	& cli(
		shutdownMasterUsername, "Generic", "shutdown-master-username",
		"Triggers shutdown if the user name of the received messages match.",
		false
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::AppSettings::Database::accept(SettingsLinker &linker) {
	linker
	& cfg(URI, "")
	& cfgAsPath(inventoryDB, "inventory")
	& cfgAsPath(configDB, "config")

	& cfg(type, "type")
	& cfg(parameters, "parameters")

	& cliSwitch(
		showDrivers, "Database", "db-driver-list",
		"List all supported database drivers."
	)
	& cli(
		URI, "Database", "database,d",
		"The database connection string, format: service://user:pwd@host/database."
	)
	& cli(
		inventoryDB, "Database", "inventory-db",
		"Load the inventory from the given database or file, format: [service://]location."
	)
	& cli(
		configDB, "Database", "config-db",
		"Load the configuration from the given database or file, format: [service://]location."
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::AppSettings::Inventory::accept(SettingsLinker &linker) {
	linker
	& cfg(netTypeWhitelist, "whitelist.nettype")
	& cfg(netTypeBlacklist, "blacklist.nettype")
	& cfg(staTypeWhitelist, "whitelist.statype")
	& cfg(staTypeBlacklist, "blacklist.statype");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::AppSettings::RecordStream::accept(SettingsLinker &linker) {
	linker
	& cliSwitch(
		showDrivers, "Records", "record-driver-list",
		"List all supported record stream drivers."
	)
	& cli(
		URI, "Records", "record-url,I",
		"The recordstream URL, format: [service://]location[#type]"
	)
	& cli(
		file, "Records", "record-file",
		"Specify a file as recordsource."
	)
	& cli(
		fileType, "Records", "record-type",
		"Specify a type for the records being read."
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::AppSettings::Processing::accept(SettingsLinker &linker) {
	linker
	& cfg(agencyBlacklist, "whitelist.agencies")
	& cfg(agencyWhitelist, "blacklist.agencies");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::AppSettings::Cities::accept(SettingsLinker &linker) {
	linker
	& cfg(enable, "loadCities")
	& cfg(db, "cityXML")
	& cli(
		db, "Cities", "city-xml",
		"Load the cities from the given XML file."
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::AppSettings::accept(SettingsLinker &linker) {
	linker
	& cfg(objectLogTimeWindow, "logging.objects.timeSpan")
	& cfg(agencyID, "agencyID")
	& cfg(author, "author")
	& cfg(enableLoadRegions, "loadRegions")
	& cfg(customPublicIDPattern, "publicIDPattern")
	& cfg(processing, "processing");

	if ( database.enable ) {
		linker
		& cfg(database, "database")
		& cfg(inventory, "inventory")
		& cfg(configModuleName, "configModule")
		& cli(
			configModuleName, "Database", "config-module",
			"The configmodule to use.",
			true
		);
	}

	if ( messaging.enable ) {
		linker & cfg(messaging, "connection") & cfg(client, "client");
	}

	if ( recordstream.enable ) {
		linker
		& cfg(recordstream.URI, "recordstream")
		& cfg(recordstream, "recordstream");
	}

	if ( cities.enable ) {
		linker
		& cfg(cities, "");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Application(int argc, char** argv)
: System::Application(argc, argv)
, _messageThread(nullptr) {
	_inputMonitor = _outputMonitor = nullptr;

	if ( _instance != this && _instance ) {
		SEISCOMP_WARNING("Another application object exists already. "
		                 "This usage is not intended. "
		                 "The Application::Instance() method will return "
		                 "the last created application.");
	}

	_instance = this;

	if ( _settings.messaging.user.empty() )
		_settings.messaging.user = _name;

	DataModel::Notifier::SetEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::~Application() {
	if ( _inputMonitor ) delete _inputMonitor;
	if ( _outputMonitor ) delete _outputMonitor;

	if ( _instance == this )
		_instance = nullptr;

	// Remove all queued notifiers
	DataModel::Notifier::Clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application *Application::Instance() {
	return _instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setDatabaseEnabled(bool enable, bool tryToFetch) {
	_settings.database.enable = enable;
	_settings.enableFetchDatabase = tryToFetch;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isDatabaseEnabled() const {
	return _settings.database.enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isInventoryDatabaseEnabled() const {
	return _settings.database.inventoryDB.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isConfigDatabaseEnabled() const {
	return _settings.database.configDB.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addMessagingSubscription(const std::string& group) {
	_settings.messaging.subscriptions.push_back(group);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setMessagingEnabled(bool enable) {
	_settings.messaging.enable = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isMessagingEnabled() const {
	return _settings.messaging.enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setMembershipMessagesEnabled(bool enable) {
	_settings.messaging.membershipMessages = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::areMembershipMessagesEnabled() const {
	return _settings.messaging.membershipMessages;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setStartStopMessagesEnabled(bool enable) {
	_settings.client.startStopMessages = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::areStartStopMessagesEnabled() const {
	return _settings.client.startStopMessages;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setAutoShutdownEnabled(bool enable) {
	_settings.client.autoShutdown = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isAutoShutdownEnabled() const {
	return _settings.client.autoShutdown;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setRecordStreamEnabled(bool enable) {
	_settings.recordstream.enable = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isRecordStreamEnabled() const {
	return _settings.recordstream.enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadStationsEnabled(bool enable) {
	_settings.enableLoadStations = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadStationsEnabled() const {
	return _settings.enableLoadStations;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadInventoryEnabled(bool enable) {
	_settings.enableLoadInventory = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadInventoryEnabled() const {
	return _settings.enableLoadInventory;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadConfigModuleEnabled(bool enable) {
	_settings.enableLoadConfigModule = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadConfigModuleEnabled() const {
	return _settings.enableLoadConfigModule;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadCitiesEnabled(bool enable) {
	_settings.cities.enable = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadCitiesEnabled() const {
	return _settings.cities.enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadRegionsEnabled(bool enable) {
	_settings.enableLoadRegions = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadRegionsEnabled() const {
	return _settings.enableLoadRegions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setAutoApplyNotifierEnabled(bool enable) {
	_settings.enableAutoApplyNotifier = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isAutoApplyNotifierEnabled() const {
	return _settings.enableAutoApplyNotifier;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setInterpretNotifierEnabled(bool enable) {
	_settings.enableInterpretNotifier = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isInterpretNotifierEnabled() const {
	return _settings.enableInterpretNotifier;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::hasCustomPublicIDPattern() const {
	return !_settings.customPublicIDPattern.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setConnectionRetries(unsigned int r) {
	_settings.retryCount = r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setConfigModuleName(const std::string &module) {
	_settings.configModuleName = module;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::configModuleName() const {
	return _settings.configModuleName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setShutdownMasterModule(const std::string &module) {
	_settings.client.shutdownMasterModule = module;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setShutdownMasterUsername(const std::string &name) {
	_settings.client.shutdownMasterUsername = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setPrimaryMessagingGroup(const std::string& group) {
	_settings.messaging.primaryGroup = group;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::primaryMessagingGroup() const {
	return _settings.messaging.primaryGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setMessagingUsername(const std::string& user) {
	_settings.messaging.user = user;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Connection *Application::connection() const {
	return _connection.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::databaseType() const {
	return _settings.database.type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::databaseParameters() const {
	return _settings.database.parameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IO::DatabaseInterface* Application::database() const {
	return _database.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::databaseURI() const {
	return _settings.database.URI;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::DatabaseQuery* Application::query() const {
	return _query.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string&  Application::recordStreamURL() const {
	return _settings.recordstream.URI;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::vector<Math::Geo::CityD>& Application::cities() const {
	return _cities;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Math::Geo::CityD *Application::nearestCity(double lat, double lon,
                                                 double maxDist, double minPopulation,
                                                 double *dist, double *azi) const {
	return Math::Geo::nearestCity(lat, lon, maxDist, minPopulation, _cities, dist, azi);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ConfigModule *Application::configModule() const {
	return _configModule.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isStationEnabled(const std::string& networkCode,
                                   const std::string& stationCode) {
	if ( _configModule ) {
		for ( size_t i = 0; i < _configModule->configStationCount(); ++i ) {
			DataModel::ConfigStation* cs = _configModule->configStation(i);
			if ( cs->networkCode() == networkCode &&
			     cs->stationCode() == stationCode )
				return cs->enabled();
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::messagingURL() const {
	return _settings.messaging.URL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::enableTimer(unsigned int seconds) {
	if ( !seconds ) {
		_userTimer.stop();
		return;
	}

	_userTimer.setTimeout(seconds);

	if ( _userTimer.isActive() ) return;

	_userTimer.setCallback(bind(&Application::timeout, this));
	_userTimer.start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::disableTimer() {
	_userTimer.disable();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handlePreFork() {
	_inputMonitor = new ObjectMonitor(_settings.objectLogTimeWindow);
	_outputMonitor = new ObjectMonitor(_settings.objectLogTimeWindow);

	_queue.resize(10);

	if ( _settings.database.showDrivers ) {
		DatabaseInterfaceFactory::ServiceNames* services = DatabaseInterfaceFactory::Services();
		if ( services ) {
			cout << "Supported database drivers: ";
			for ( DatabaseInterfaceFactory::ServiceNames::iterator it = services->begin();
			      it != services->end(); ++it ) {
				if ( it != services->begin() )
					cout << ", ";
				cout << *it;
			}
			cout << endl;
			delete services;
			return false;
		}
	}

	if ( _settings.recordstream.showDrivers ) {
		RecordStreamFactory::ServiceNames* services = RecordStreamFactory::Services();
		if ( services ) {
			cout << "Supported recordstream drivers: ";
			for ( RecordStreamFactory::ServiceNames::iterator it = services->begin();
			      it != services->end(); ++it ) {
				if ( it != services->begin() )
					cout << ", ";
				cout << *it;
			}
			cout << endl;
			delete services;
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::init() {
	bindSettings(&_settings);

	if ( !System::Application::init() )
		return false;

	if ( _settings.messaging.enable && !_connection ) {
		SEISCOMP_INFO("Connect to messaging");
		showMessage("Initialize messaging");
		if ( !initMessaging() ) {
			if ( !handleInitializationError(MESSAGING) )
			//if ( !_connection )
				return false;
		}
	}

	if ( _settings.database.enable && !_database ) {
		SEISCOMP_INFO("Connect to database");
		showMessage("Initialize database");
		if ( !initDatabase() ) {
			if ( !handleInitializationError(DATABASE) )
				return false;
		}

		if ( _query && _connection && _connection->schemaVersion() > _query->version() ) {
			stringstream ss;
			ss << "The schema v" << _query->version().toString() << " of the "
			      "database is older than the one the server is using (v" <<
			      _connection->schemaVersion().toString() << ") , not all "
			      "information will be stored in the database." << endl <<
			      "This should be fixed!";
			showWarning(ss.str().c_str());
			SEISCOMP_WARNING("%s", ss.str().c_str());
		}
	}

	if ( !reloadInventory() )
		return false;

	if ( _exitRequested )
		return false;

	if ( !reloadBindings() )
		return false;

	if ( _exitRequested )
		return false;

	if ( isLoadRegionsEnabled() ) {
		showMessage("Reading custom regions");
		Regions regions;
		regions.load();
	}

	if ( isLoadCitiesEnabled() ) {
		showMessage("Reading city data");

		IO::XMLArchive ar;
		bool foundCity;

		if ( _settings.cities.db.empty() ) {
			foundCity = ar.open((Environment::Instance()->configDir() + "/cities.xml").c_str());
			if ( !foundCity )
				foundCity = ar.open((Environment::Instance()->shareDir() + "/cities.xml").c_str());
		}
		else
			foundCity = ar.open(_settings.cities.db.c_str());

		if ( foundCity ) {
			ar >> NAMED_OBJECT("City", _cities);

			SEISCOMP_INFO("Found cities.xml and read %lu entries", (unsigned long)_cities.size());

			// Sort the cities descending
			std::sort(_cities.begin(), _cities.end(), CityGreaterThan());

			ar.close();
		}
	}

	showMessage("");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::timeout() {
	sendNotification(Notification::Timeout);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::sendNotification(const Notification &n) {
	_queue.push(n);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleEndAcquisition() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::reloadInventory() {
	if ( _settings.enableLoadInventory ) {
		if ( !_settings.database.inventoryDB.empty() ) {
			if ( !loadInventory(_settings.database.inventoryDB) ) return false;
		}
		else if ( _database ) {
			if ( _query ) {
				SEISCOMP_INFO("Loading complete inventory");
				showMessage("Loading inventory");
				Inventory::Instance()->load(_query.get());
				SEISCOMP_INFO("Finished loading complete inventory");
			}
			else {
				SEISCOMP_ERROR("No database query object");
				return false;
			}
		}

		int filtered = Inventory::Instance()->filter(&_settings.networkTypeFirewall,
		                                             &_settings.stationTypeFirewall);
		if ( filtered > 0 )
			SEISCOMP_INFO("Filtered %d stations by type", filtered);
	}
	else if ( _settings.enableLoadStations ) {
		if ( !_settings.database.inventoryDB.empty() ) {
			if ( !loadInventory(_settings.database.inventoryDB) ) return false;
		}
		else if ( _database ) {
			if ( _query ) {
				SEISCOMP_INFO("Loading inventory (stations only)");
				showMessage("Loading stations");
				Inventory::Instance()->loadStations(_query.get());
				SEISCOMP_INFO("Finished loading inventory (stations only)");
			}
			else {
				SEISCOMP_ERROR("No database query object");
				return false;
			}
		}

		int filtered = Inventory::Instance()->filter(&_settings.networkTypeFirewall,
		                                             &_settings.stationTypeFirewall);
		if ( filtered > 0 )
			SEISCOMP_INFO("Filtered %d stations by type", filtered);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::reloadBindings() {
	_configModule = nullptr;

	if ( _settings.enableLoadConfigModule ) {
		std::set<std::string> params;

		if ( !_settings.database.configDB.empty() ) {
			if ( !loadConfig(_settings.database.configDB) ) return false;
		}
		else if ( _database ) {
			if ( _query ) {
				SEISCOMP_INFO("Loading configuration module");
				showMessage("Reading station config");
				if ( !_settings.configModuleName.empty() )
					ConfigDB::Instance()->load(query(), _settings.configModuleName, Core::None, Core::None, Core::None, params);
				else
					ConfigDB::Instance()->load(query(), Core::None, Core::None, Core::None, Core::None, params);
				SEISCOMP_INFO("Finished loading configuration module");
			}
			else {
				SEISCOMP_ERROR("No database query object");
				return false;
			}
		}
		else {
			SEISCOMP_ERROR("Failed to load configuration module");
			return false;
		}

		DataModel::Config* config = ConfigDB::Instance()->config();
		if ( !config ) {
			return false;
		}

		for ( size_t i = 0; i < config->configModuleCount(); ++i ) {
			if ( config->configModule(i)->name() == _settings.configModuleName ) {
				_configModule = config->configModule(i);
				break;
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::run() {
	if ( _connection )
		startMessageThread();

	while ( !_exitRequested ) {
		if ( !processEvent() ) break;
		idle();
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::processEvent() {
	try {
		Notification evt = _queue.pop();
		BaseObjectPtr obj = evt.object;
		switch ( evt.type ) {
			case Notification::Object:
				if ( !obj ) {
					SEISCOMP_ERROR("Got nullptr object");
					return true;
				}

				//SEISCOMP_DEBUG("Received object: %s, refCount: %d", obj->className(), obj->referenceCount());
				if ( !dispatch(obj.get()) ) {
					SEISCOMP_WARNING("Could not dispatch objects");
				}
				break;

			case Notification::Reconnect:
				handleReconnect();
				break;

			case Notification::Disconnect:
				// Reset sync request because server is not going to respond
				// to our initial sync message anymore
				handleDisconnect();
				break;

			case Notification::Timeout:
				handleTimeout();
				break;

			case Notification::Close:
				if ( handleClose() ) {
					SEISCOMP_INFO("Close event received, returning");
					return false;
				}
				else
					SEISCOMP_INFO("Close event received but ignored");
				break;

			case Notification::AcquisitionFinished:
				handleEndAcquisition();
				break;

			default:
				if ( !dispatchNotification(evt.type, obj.get()) )
					SEISCOMP_WARNING("Wrong eventtype in queue: %d", evt.type);
				break;
		}
	}
	catch ( QueueClosedException& ex ) {
		SEISCOMP_INFO("%s, returning", ex.what());
		return false;
	}
	catch ( GeneralException& ex ) {
		SEISCOMP_INFO("Exception: %s, returning", ex.what());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::dispatch(Core::BaseObject* obj) {
	//SEISCOMP_DEBUG("dispatch %s (refCount = %d)", obj->className(), obj->referenceCount());
	Message *msg = Message::Cast(obj);
	if ( msg ) {
		if ( _settings.client.autoShutdown ) {
			// Filter application status messages
			ApplicationStatusMessage *as = ApplicationStatusMessage::Cast(msg);
			if ( as ) {
				SEISCOMP_DEBUG("Received application status: module=%s, username=%s: %s",
				               as->module().c_str(), as->username().c_str(),
				               as->status().toString());
				if ( as->status() == FINISHED ) {
					if ( !_settings.client.shutdownMasterModule.empty()
					  && as->module() == _settings.client.shutdownMasterModule ) {
						SEISCOMP_INFO("Initiate self shutdown because of module %s shutdown",
						              as->module().c_str());
						handleAutoShutdown();
					}
					else if ( !_settings.client.shutdownMasterUsername.empty()
					       && as->username() == _settings.client.shutdownMasterUsername ) {
						SEISCOMP_INFO("Initiate self shutdown because of user %s shutdown",
						              as->username().c_str());
						handleAutoShutdown();
					}
				}
			}
		}

		handleMessage(msg);
		return true;
	}
	else {
		Packet *pkt = Packet::Cast(obj);
		if ( pkt ) {
			handleNetworkMessage(pkt);
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::dispatchNotification(int, Core::BaseObject*) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::idle() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {
	if ( _connection && _connection->isConnected() ) {
		if ( _settings.client.startStopMessages ) {
			ApplicationStatusMessage stat(name(), _settings.messaging.user, FINISHED);
			_connection->send(Protocol::STATUS_GROUP, &stat);
		}
		_connection->disconnect();
	}

	_queue.close();

	if ( _userTimer.isActive() ) {
		SEISCOMP_INFO("Disable timer");
		disableTimer();
	}

	if ( _messageThread ) {
		SEISCOMP_INFO("Waiting for message thread");
		_messageThread->join();
		delete _messageThread;
		_messageThread = nullptr;
		SEISCOMP_INFO("Message thread finished");
	}

	_connection = nullptr;
	_query = nullptr;
	_database = nullptr;

	Inventory::Reset();
	ConfigDB::Reset();

	System::Application::done();

	SEISCOMP_DEBUG("Leaving ::done");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::validateParameters() {
	if ( !System::Application::validateParameters() ) {
		return false;
	}

	_settings.messaging.user = Util::replace(_settings.messaging.user, AppResolver(_name));

	//if ( commandline().hasOption("subscription-list") )
	//	split(_messagingSubscriptionRequests, _messagingStrSubscriptions.c_str(), ",");

	if ( !_settings.database.URI.empty() ) {
		if ( !_settings.database.type.empty() || !_settings.database.parameters.empty() ) {
			if ( !configDefault(_arguments, _configuration, "database.type") ||
			     !configDefault(_arguments, _configuration, "database.parameters") ) {
				SEISCOMP_ERROR("You are using the deprecated parameter "
				               "'database.type' or 'database.parameters' along "
				               "with the new 'database' parameter which takes "
				               "precedence. Please remove the old parameters.");
			}
		}
	}
	else {
		if ( !_settings.database.type.empty() && !_settings.database.parameters.empty() ) {
			_settings.database.URI = _settings.database.type + "://" + _settings.database.parameters;
			if ( !configDefault(_arguments, _configuration, "database.type") ||
			     !configDefault(_arguments, _configuration, "database.parameters") ) {
				SEISCOMP_ERROR("DEPRECATION WARNING: You are using the "
				               "parameter 'database.type' and "
				               "'database.parameters' which will be removed in "
				               "the next major release. Please remove the old "
				               "parameters and set the new 'database' "
				               "parameter to '%s' instead.",
				               _settings.database.URI.c_str());
			}
		}
	}

	const char* tmp = strstr(_settings.database.URI.c_str(), "://");
	if ( tmp ) {
		std::copy(_settings.database.URI.c_str(), tmp, std::back_inserter(_settings.database.type));
		_settings.database.parameters = tmp + 3;
	}


	if ( !_settings.recordstream.URI.empty() ) {
		if ( !configDefault(_arguments, _configuration, "recordstream.service") ||
		     !configDefault(_arguments, _configuration, "recordstream.source") ) {
			SEISCOMP_ERROR("You are using the deprecated parameter "
			               "'recordstream.service' or "
			               "'recordstream.source' along with the new "
			               "'recordstream' parameter which takes "
			               "precedence. Please remove the old parameters.");
		}
	}
	else {
		try {
			_settings.recordstream.URI = configGetString("recordstream.service") + "://" +
			                configGetString("recordstream.source");
			if ( !configDefault(_arguments, _configuration, "recordstream.service") ||
			     !configDefault(_arguments, _configuration, "recordstream.source") ) {
				SEISCOMP_ERROR("DEPRECATION WARNING: You are using the "
				               "parameter 'recordstream.service' or "
				               "'recordstream.source' which will be removed in "
				               "the next major release. Please remove the old "
				               "parameters and set the new 'recordstream' "
				               "parameter to '%s' instead.",
				               _settings.recordstream.URI.c_str());
			}
		}
		catch (...) {}
	}


	_settings.agencyID = Util::replace(_settings.agencyID, AppResolver(_name));
	_settings.author = Util::replace(_settings.author, AppResolver(_name));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initConfiguration() {
	if ( !System::Application::initConfiguration() )
		return false;

	copy(
		_settings.processing.agencyBlacklist.begin(),
		_settings.processing.agencyBlacklist.end(),
		inserter(_settings.processing.firewall.deny, _settings.processing.firewall.deny.end())
	);

	copy(
		_settings.processing.agencyWhitelist.begin(),
		_settings.processing.agencyWhitelist.end(),
		inserter(_settings.processing.firewall.allow, _settings.processing.firewall.allow.end())
	);

	copy(
		_settings.inventory.netTypeBlacklist.begin(),
		_settings.inventory.netTypeBlacklist.end(),
		inserter(_settings.networkTypeFirewall.deny, _settings.networkTypeFirewall.deny.end())
	);

	copy(
		_settings.inventory.netTypeWhitelist.begin(),
		_settings.inventory.netTypeWhitelist.end(),
		inserter(_settings.networkTypeFirewall.allow, _settings.networkTypeFirewall.allow.end())
	);

	copy(
		_settings.inventory.staTypeBlacklist.begin(),
		_settings.inventory.staTypeBlacklist.end(),
		inserter(_settings.stationTypeFirewall.deny, _settings.stationTypeFirewall.deny.end())
	);

	copy(
		_settings.inventory.staTypeWhitelist.begin(),
		_settings.inventory.staTypeWhitelist.end(),
		inserter(_settings.stationTypeFirewall.allow, _settings.stationTypeFirewall.allow.end())
	);

	if ( !_settings.customPublicIDPattern.empty() ) {
		DataModel::PublicObject::SetIdPattern(_settings.customPublicIDPattern);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initMessaging() {
	Result status = OK;

	_messagingSubscriptions.clear();
	for ( size_t i = 0; i < _settings.messaging.subscriptions.size(); ++i )
		_messagingSubscriptions.insert(_settings.messaging.subscriptions[i]);

	int retryCount = _settings.retryCount;

	while ( !_exitRequested && !_connection ) {
		SEISCOMP_DEBUG("Trying to connect to %s@%s with primary group = %s",
		               _settings.messaging.user.c_str(),
		               _settings.messaging.URL.c_str(),
		               _settings.messaging.primaryGroup.c_str());
		_connection = new Connection;
		status = _connection->setSource(_settings.messaging.URL);
		if ( status == OK ) {
			_connection->setMembershipInfo(_settings.messaging.membershipMessages);
			status = _connection->connect(
				_settings.messaging.user,
				_settings.messaging.primaryGroup,
				_settings.messaging.timeout * 1000
			);

			if ( status == OK )
				break;
		}

		if ( status == GroupDoesNotExist ) {
			SEISCOMP_ERROR("Connection error: primary messaging group '%s' does not exist",
			               _settings.messaging.primaryGroup.c_str());
		}
		else {
			SEISCOMP_ERROR("Connection error: %s", _connection->lastErrorMessage().c_str());
		}

		_connection = nullptr;
		if ( status != NetworkError )
			break;

		if ( retryCount )
			--retryCount;

		if ( !retryCount )
			break;

		SEISCOMP_WARNING("Connection error: %s -> trying again after 2 secs", status.toString());
		Core::sleep(2);
	}

	if ( !_connection ) {
		SEISCOMP_ERROR("Could not connect to message system");
		return false;
	}

	// Register monitor logging callback
	_connection->setInfoCallback(bind(&Application::monitorLog, this, placeholders::_1, placeholders::_2));

	if ( !_baseSettings.logging.toStdout )
		SEISCOMP_NOTICE("Connection to %s established", _settings.messaging.URL.c_str());

	Version localSchemaVersion = Version(DataModel::Version::Major, DataModel::Version::Minor);
	if ( _connection->schemaVersion() > localSchemaVersion ) {
		stringstream ss;
		ss << "Local schema v" << localSchemaVersion.toString() << " is "
		      "older than the one the server supports (v" <<
		      _connection->schemaVersion().toString() << ") , incoming messages "
		      "will not be readable but sending will work.";
		showWarning(ss.str().c_str());
		SEISCOMP_WARNING("%s", ss.str().c_str());
	}
	else if ( _connection->schemaVersion() < localSchemaVersion ) {
		stringstream ss;
		ss << "Local schema v" << localSchemaVersion.toString() << " is "
		      "more recent than the one the server supports (v" <<
		      _connection->schemaVersion().toString() << ") , not all "
		      "information can be handled by the server and will be ignored.";
		showWarning(ss.str().c_str());
		SEISCOMP_WARNING("%s", ss.str().c_str());
	}

	if ( _settings.messaging.contentType == "binary" )
		_connection->setContentType(Protocol::Binary);
	else if ( _settings.messaging.contentType == "json" )
		_connection->setContentType(Protocol::JSON);
	else if ( _settings.messaging.contentType == "xml" )
		_connection->setContentType(Protocol::XML);
	else if ( !_settings.messaging.contentType.empty() ) {
		SEISCOMP_ERROR("Invalid message content type: %s",
		               _settings.messaging.contentType.c_str());
		return false;
	}

	if ( _settings.client.startStopMessages ) {
		SEISCOMP_DEBUG("Send START message to group %s",
		               Protocol::STATUS_GROUP.c_str());
		ApplicationStatusMessage stat(name(), _settings.messaging.user, STARTED);
		_connection->send(Protocol::STATUS_GROUP, &stat);
	}

	return initSubscriptions();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initSubscriptions() {
	bool requestAllGroups = false;

	for ( set<string>::iterator it = _messagingSubscriptions.begin();
	      it != _messagingSubscriptions.end(); ++it ) {
		if ( (*it) == "*" || (*it) == "...") {
			requestAllGroups = true;
			break;
		}
	}

	if ( requestAllGroups ) {
		for ( auto &&group : _connection->protocol()->groups() ) {
			if ( _connection->subscribe(group) != OK ) {
				SEISCOMP_ERROR("Could not subscribe to group '%s'",
				               group.c_str());
				return false;
			}
		}
	}
	else {
		for ( auto &&group : _messagingSubscriptions ) {
			if ( _connection->subscribe(group) != OK ) {
				SEISCOMP_ERROR("Could not subscribe to group '%s'", group.c_str());
				return false;
			}
		}
	}

	/*
	for ( int i = 0; i < _connection->groupCount(); ++i ) {
		if ( requestAllGroups ) {
			if ( !_connection->subscribe(_connection->group(i)) ) {
				SEISCOMP_ERROR("Could not subscribe to group '%s'", _connection->group(i));
				return false;
			}
		}
		else {
			if ( _messagingSubscriptions.find(_connection->group(i)) != _messagingSubscriptions.end() ) {
				if ( !_connection->subscribe(_connection->group(i)) ) {
					SEISCOMP_ERROR("Could not subscribe to group '%s'", _connection->group(i));
					return false;
				}
				else {
					SEISCOMP_DEBUG("Subscribe to group '%s'", _connection->group(i));
				}
			}
		}
	}
	*/

	if ( _settings.client.autoShutdown )
		_connection->subscribe(Protocol::STATUS_GROUP);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::runMessageThread() {
	SEISCOMP_INFO("Starting message thread");
	while ( readMessages() ) {}
	SEISCOMP_INFO("Leaving message thread");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::startMessageThread() {
	_messageThread = new thread(bind(&Application::runMessageThread, this));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setDatabase(IO::DatabaseInterface* db) {
	_database = db;
	if ( !_query )
		_query = new DataModel::DatabaseQuery(_database.get());
	else
		_query->setDriver(_database.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initDatabase() {
	setDatabase(nullptr);

	if ( !_settings.database.URI.empty() ) {
		SEISCOMP_INFO("Read database service parameters from configfile");
		SEISCOMP_INFO("Trying to connect to %s", _settings.database.URI.c_str());

		IO::DatabaseInterfacePtr db = IO::DatabaseInterface::Open(_settings.database.URI.c_str());
		if ( db ) {
			SEISCOMP_INFO("Connected successfully");
			setDatabase(db.get());
			if ( _query->hasError() ) {
				SEISCOMP_ERROR("%s", _query->errorMsg().c_str());
				setDatabase(nullptr);
				return false;
			}
			else
				return true;
		}
		else {
			if ( _settings.enableFetchDatabase ) {
				SEISCOMP_WARNING("Database connection to %s failed, trying to fetch the service message",
				                 _settings.database.URI.c_str());
			}
			else {
				SEISCOMP_WARNING("Database connection to %s failed", _settings.database.URI.c_str());
				return false;
			}
		}
	}

	// Try to fetch here
	if ( !_connection ) {
		SEISCOMP_ERROR("Fetching database parameters failed, no messaging connection");
		return false;
	}

	Util::StopWatch fetchTimeout;

	// Wait for 5 seconds for a valid database provide message
	_connection->setTimeout(5000);

	MessagePtr msg;
	// TODO: Add optional timeout parameter to recv
	while ( (msg = _connection->recv()) ) {
		DatabaseProvideMessage *dbrmsg = DatabaseProvideMessage::Cast(msg);
		if ( dbrmsg ) {
			std::string dbType = dbrmsg->service();
			std::string dbParameters = dbrmsg->parameters();
			_settings.database.URI = dbType + "://" + dbParameters;

			SEISCOMP_INFO("Received database service parameters");
			SEISCOMP_INFO("Trying to connect to %s database", dbrmsg->service());
			IO::DatabaseInterfacePtr db = dbrmsg->database();
			if ( db ) {
				setDatabase(db.get());
				SEISCOMP_INFO("Connected successfully");
				_connection->setTimeout(0);
				if ( _query->hasError() ) {
					SEISCOMP_ERROR("%s", _query->errorMsg().c_str());
					setDatabase(nullptr);
					return false;
				}
				else
					return true;
			}
			else
				SEISCOMP_WARNING("Database connection to %s://%s failed",
				                 dbrmsg->service(), dbrmsg->parameters());
			continue;
		}

		if ( fetchTimeout.elapsed() >= TimeSpan(5.0) ) break;
	}

	// Reset timeout
	_connection->setTimeout(0);

	SEISCOMP_ERROR("Timeout while waiting for database provide message");

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::set<std::string>& Application::subscribedGroups() const {
	return _messagingSubscriptions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ObjectMonitor::Log *
Application::addInputObjectLog(const std::string &name, const std::string &channel) {
	if ( !_inputMonitor ) return nullptr;
	return _inputMonitor->add(name, channel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ObjectMonitor::Log *
Application::addOutputObjectLog(const std::string &name, const std::string &channel) {
	if ( !_outputMonitor ) return nullptr;
	return _outputMonitor->add(name, channel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::loadConfig(const std::string &configDB) {
	SEISCOMP_INFO("Loading configuration module %s", configDB.c_str());
	showMessage("Reading station config from");

	if ( configDB.find("://") == string::npos ) {
		try { ConfigDB::Instance()->load(configDB.c_str()); }
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return false;
		}
	}
	else if ( configDB.find("file://") == 0 ) {
		try { ConfigDB::Instance()->load(configDB.substr(7).c_str()); }
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return false;
		}
	}
	else {
		SEISCOMP_INFO("Trying to connect to %s", configDB.c_str());
		IO::DatabaseInterfacePtr db = IO::DatabaseInterface::Open(configDB.c_str());
		if ( db ) {
			SEISCOMP_INFO("Connected successfully");
			DataModel::DatabaseQueryPtr query = new DataModel::DatabaseQuery(db.get());
			ConfigDB::Instance()->load(query.get());
		}
		else {
			SEISCOMP_WARNING("Database connection to %s failed", configDB.c_str());
			return false;
		}
	}

	SEISCOMP_INFO("Finished loading configuration module");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::loadInventory(const std::string &inventoryDB) {
	SEISCOMP_INFO("Loading complete inventory from %s", inventoryDB.c_str());
	showMessage("Loading inventory");
	if ( inventoryDB.find("://") == string::npos ) {
		try { Inventory::Instance()->load(inventoryDB.c_str()); }
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return false;
		}
	}
	else if ( inventoryDB.find("file://") == 0 ) {
		try { Inventory::Instance()->load(inventoryDB.substr(7).c_str()); }
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return false;
		}
	}
	else {
		SEISCOMP_INFO("Trying to connect to %s", inventoryDB.c_str());
		IO::DatabaseInterfacePtr db = IO::DatabaseInterface::Open(inventoryDB.c_str());
		if ( db ) {
			SEISCOMP_INFO("Connected successfully");
			DataModel::DatabaseQueryPtr query = new DataModel::DatabaseQuery(db.get());
			Inventory::Instance()->load(query.get());
		}
		else {
			SEISCOMP_WARNING("Database connection to %s failed", inventoryDB.c_str());
			return false;
		}
	}

	SEISCOMP_INFO("Finished loading complete inventory");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::logObject(ObjectMonitor::Log *log, const Core::Time &timestamp,
                            size_t count) {
	lock_guard<mutex> l(_objectLogMutex);
	log->push(timestamp, count);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::readMessages() {
	if ( !_connection ) {
		return true;
	}

	// We store a plain C-pointer here because SmartPointers are not
	// thread-safe. So the message has to be deleted manually when enqueueing
	// fails. Otherwise the referenceCount will be decremented when leaving
	// the method which can result in race conditions and much more important
	// in a segfault.
	Packet *pkt = nullptr;
	Result result;
	Message *msg = _connection->recv(&pkt, &result);
	if ( pkt ) {
		if ( msg ) {
			if ( !_queue.push(pkt) ) {
				delete pkt;
				delete msg;
				return false;
			}

			if ( _queue.push(msg) ) {
				return true;
			}

			delete msg;
			return false;
		}
		else {
			if ( !_queue.push(pkt) ) {
				delete pkt;
				return false;
			}

			return true;
		}
	}
	else if ( !_exitRequested ) {
		// Did not receive a packet: probably connection loss
		if ( msg ) delete msg;

		if ( !result ) {
			if ( result == SystemError ) {
				SEISCOMP_ERROR("Message read error: %d: %s: %s (%d)",
				               result.toInt(), result.toString(),
				               strerror(errno), errno);
			}
			else {
				SEISCOMP_ERROR("Message read error: %d: %s",
				               result.toInt(), result.toString());
			}
		}

		// We should never step into this case
		if ( _connection->isConnected() ) {
			SEISCOMP_ERROR("Internal error: still connected");
			return true;
		}

		SEISCOMP_WARNING("Connection lost, trying to reconnect");
		if ( !_queue.push(Notification::Disconnect) ) {
			return false;
		}

		bool first = true;
		while ( !_exitRequested ) {
			_connection->reconnect();
			if ( _connection->isConnected() ) {
				SEISCOMP_INFO("Reconnected successfully");
				if ( _database ) {
					while ( !_database->isConnected() ) {
						SEISCOMP_WARNING("Connection lost to database %s, trying to reconnect",
						                 _settings.database.URI.c_str());
						if ( _database->connect(_settings.database.URI.c_str()) ) {
							SEISCOMP_INFO("Reconnected successfully to %s",
							              _settings.database.URI.c_str());
						}
						else {
							Core::sleep(2);
						}
					}
				}
				_queue.push(Notification::Reconnect);
				break;
			}
			else {
				if ( first ) {
					first = false;
					SEISCOMP_INFO("Reconnecting failed, trying again every 2 seconds");
				}
				Core::sleep(2);
			}
		}

		if ( _exitRequested ) {
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::monitorLog(const Core::Time &now, std::ostream &os) {
	handleMonitorLog(now);

	// Append app name
	os << "&app=" << name();

	lock_guard<mutex> l(_objectLogMutex);

	ObjectMonitor::const_iterator it;

	_inputMonitor->update(now);
	_outputMonitor->update(now);

	bool first = true;

	for ( it = _inputMonitor->begin(); it != _inputMonitor->end(); ++it ) {
		if ( first ) {
			os << "&";
			first = false;
		}

		os << "in(";
		if ( !it->name.empty() )
			os << "name:" << it->name << ",";
		if ( !it->channel.empty() )
			os << "chan:" << it->channel << ",";

		os << "cnt:" << it->count << ",";
		os << "avg:" << ((float)it->count / (float)it->test->timeSpan()) << ",";
		os << "tw:" << it->test->timeSpan();

		if ( it->test->last() )
			os << ",last:" << it->test->last().iso();
		os << /*"utime:" << now.iso() <<*/ ")&";
	}

	for ( it = _outputMonitor->begin(); it != _outputMonitor->end(); ++it ) {
		if ( first ) {
			os << "&";
			first = false;
		}

		os << "out(";
		if ( !it->name.empty() )
			os << "name:" << it->name << ",";
		if ( !it->channel.empty() )
			os << "chan:" << it->channel << ",";

		os << "cnt:" << it->count << ",";
		os << "avg:" << ((float)it->count / (float)it->test->timeSpan()) << ",";
		os << "tw:" << it->test->timeSpan();

		if ( it->test->last() )
			os << ",last:" << it->test->last().iso();
		os << /*"utime:" << now.iso() <<*/ ")&";
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleTimeout() {
	std::cerr << "Unhandled Application::Timeout" << std::endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handleClose() {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleAutoShutdown() {
	SEISCOMP_DEBUG("Handling auto shutdown: quit");
	quit();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleMonitorLog(const Core::Time &timestamp) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleReconnect() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleDisconnect() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleMessage(Core::Message* msg) {
	DataModel::NotifierMessage* nm;

	if ( _settings.enableAutoApplyNotifier || _settings.enableInterpretNotifier )
		nm = DataModel::NotifierMessage::Cast(msg);

	if ( _settings.enableAutoApplyNotifier ) {
		if ( !nm ) {
			for ( MessageIterator it = msg->iter(); *it; ++it ) {
				DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
				if ( n ) n->apply();
			}
		}
		else {
			for ( DataModel::NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it )
				(*it)->apply();
		}
	}

	if ( _settings.enableInterpretNotifier ) {
		if ( !nm ) {
			for ( MessageIterator it = msg->iter(); *it; ++it ) {
				DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
				if ( n ) handleNotifier(n);
			}
		}
		else {
			for ( DataModel::NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it )
				handleNotifier(it->get());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleNetworkMessage(const Packet *msg) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::injectMessage(Core::Message *msg, Packet *pkt) {
	if ( pkt )
		handleNetworkMessage(pkt);
	if ( msg )
		handleMessage(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleNotifier(DataModel::Notifier* n) {
	switch ( n->operation() ) {
		case DataModel::OP_ADD:
			addObject(n->parentID(), n->object());
			break;
		case DataModel::OP_REMOVE:
			removeObject(n->parentID(), n->object());
			break;
		case DataModel::OP_UPDATE:
			updateObject(n->parentID(), n->object());
			break;
		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exit(int returnCode) {
	System::Application::exit(returnCode);
	_queue.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Application::agencyID() const {
	return _settings.agencyID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Application::author() const {
	return _settings.author;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isAgencyIDAllowed(const std::string &agencyID) const {
	return _settings.processing.firewall.isAllowed(agencyID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isAgencyIDBlocked(const std::string &agencyID) const {
	return !isAgencyIDAllowed(agencyID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
