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


#ifndef SEISCOMP_CLIENT_APPLICATION_H
#define SEISCOMP_CLIENT_APPLICATION_H


#include <seiscomp/system/application.h>

#include <seiscomp/core/message.h>

#include <seiscomp/client/queue.h>
#include <seiscomp/client/monitor.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/client.h>

#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/configmodule.h>

#include <seiscomp/math/coord.h>

#include <seiscomp/messaging/connection.h>

#include <seiscomp/utils/timer.h>
#include <seiscomp/utils/stringfirewall.h>

#include <set>
#include <thread>
#include <mutex>


#define SCCoreApp (Seiscomp::Client::Application::Instance())


namespace Seiscomp {

namespace Logging {
	class Output;
}

namespace Client {


MAKEENUM(
	ApplicationStatus,
	EVALUES(
		STARTED,
		FINISHED
	),
	ENAMES(
		"started",
		"finished"
	)
);


class SC_SYSTEM_CLIENT_API ApplicationStatusMessage : public Core::Message {
	DECLARE_SC_CLASS(ApplicationStatusMessage);
	DECLARE_SERIALIZATION;

	public:
		ApplicationStatusMessage();
		ApplicationStatusMessage(const std::string &module,
		                         ApplicationStatus status);

		ApplicationStatusMessage(const std::string &module,
		                         const std::string &username,
		                         ApplicationStatus status);


	public:
		virtual bool empty() const;

		const std::string &module() const;
		const std::string &username() const;
		ApplicationStatus status() const;


	private:
		std::string _module;
		std::string _username;
		ApplicationStatus _status;
};


struct SC_SYSTEM_CLIENT_API Notification {
	//! Declares the application internal notification types.
	//! Custom types can be used with negative values.
	enum Type {
		Object,
		Disconnect,
		Reconnect,
		Close,
		Timeout,
		AcquisitionFinished
	};

	Notification() : object(nullptr), type(Object) {}
	Notification(Core::BaseObject * o) : object(o), type(Object) {}
	Notification(int t) : object(nullptr), type(t) {}
	Notification(int t, Core::BaseObject * o) : object(o), type(t) {}

	Core::BaseObject *object;
	int type;
};


/**
 * \brief Application class to write commandline clients easily which are
 *        connected to the messaging and need database access.
 *
 * In addition to @ref System::Application it adds the method
 * @ref handleMessage which must be implemented to handle receives
 * messages. An additional abstraction layer is implemented which already
 * checks the message for notifier objects, extracts them and calls respective
 * callbacks:
 * * @ref addObject()
 * * @ref removeObject()
 * * @ref updateObject()
 */
class SC_SYSTEM_CLIENT_API Application : public System::Application {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef ObjectMonitor::Log ObjectLog;

		//! Initialization stages used when reporting errors
		enum ClientStage {
			MESSAGING = System::Application::ST_QUANTITY,
			DATABASE  = System::Application::ST_QUANTITY + 1,
			CST_QUANTITY
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Application(int argc, char **argv);
		~Application();


	// ----------------------------------------------------------------------
	//  Public functions
	// ----------------------------------------------------------------------
	public:
		//! Returns the configured agencyID
		const std::string &agencyID() const;

		//! Returns the configured author
		const std::string &author() const;

		/**
		 * Returns according to the configured white- and blacklist of
		 * agencyID's whether the passed agencyID is allowed or not
		 * @param agencyID The agencyID to check
		 * @return The boolean result
		 */
		bool isAgencyIDAllowed(const std::string &agencyID) const;

		/**
		 * Returns !isAgencyIDAllowed(agencyID)
		 * @param agencyID The agencyID to check
		 * @return !isAgencyIDAllowed(agencyID)
		 */
		bool isAgencyIDBlocked(const std::string &agencyID) const;

		/**
		 * Exit the application and set the returnCode.
		 * @param returnCode The value returned from exec()
		 */
		virtual void exit(int returnCode);

		//! Returns the application's messaging connection interface
		Client::Connection *connection() const;

		//! Returns the configured database type
		const std::string &databaseType() const;

		//! Returns the configured database connection parameters
		const std::string &databaseParameters() const;

		//! Returns the application's database interface
		IO::DatabaseInterface *database() const;

		//! Returns the application's database URI
		const std::string &databaseURI() const;

		//! Returns the application's database query interface
		DataModel::DatabaseQuery *query() const;

		//! Returns the configures recordstream URL to be used by
		//! RecordStream::Open()
		const std::string &recordStreamURL() const;

		//! Returns the list of configured points of interest
		const std::vector<Math::Geo::CityD> &cities() const;

		//! Returns the nearest city with respect to lat/lon and
		//! a given maximum distance and minimum population
		const Math::Geo::CityD *nearestCity(double lat, double lon,
		                                    double maxDist, double minPopulation,
		                                    double *dist, double *azi) const;

		//! Returns the config module object if available
		DataModel::ConfigModule *configModule() const;

		//! Returns the state of a station
		bool isStationEnabled(const std::string& networkCode,
		                      const std::string& stationCode);

		//! Returns the messaging-server
		const std::string &messagingURL() const;


		//! Returns the filename of the OpenSSL certificate or
		//! the certificate data Base64 encoded. The Base64 encoded
		//! data starts with the special DataTag.
		//! If no certificate is used the method returns an empty
		//! string.
		const std::string &messagingCertificate() const;

		//! Enables a timer that calls every n seconds the
		//! handleTimeout() methods
		//! A value of 0 seconds disables the timer
		void enableTimer(unsigned int seconds);

		//! Disables the timer
		void disableTimer();

		//! Sends a notification to the application. If used in derived
		//! classes to send custom notifications use negative notification
		//! types and reimplement dispatchNotification(...).
		void sendNotification(const Notification &);

		bool waitEvent();


	// ----------------------------------------------------------------------
	//  Initialization configuration methods
	//  These methods have to be called before the init() method.
	// ----------------------------------------------------------------------
	public:
		//! Sets the primary messaging group
		void setPrimaryMessagingGroup(const std::string&);

		//! Returns the set primary messaging group
		const std::string &primaryMessagingGroup() const;

		//! Sets the username used for the messaging connection
		void setMessagingUsername(const std::string&);

		/**
		 * Adds a group to subscribe to. This is only a default group.
		 * If another group or groups are given via commandline or config
		 * file this subscription will be overriden completely.
		 */
		void addMessagingSubscription(const std::string&);

		//! Initialize the database, default = true, true
		void setDatabaseEnabled(bool enable, bool tryToFetch);
		bool isDatabaseEnabled() const;

		//! Returns whether the inventory should be loaded from a
		//! file (false) or from the database (true)
		bool isInventoryDatabaseEnabled() const;

		//! Returns whether the config module should be loaded from a
		//! file (false) or from the database (true)
		bool isConfigDatabaseEnabled() const;

		//! Initialize the messaging, default = true
		void setMessagingEnabled(bool enable);
		bool isMessagingEnabled() const;

		/**
		 * @brief Toggles receiption of messaging membership messages. The
		 *        default is false.
		 * @param enable Flag
		 */
		void setMembershipMessagesEnabled(bool enable);
		bool areMembershipMessagesEnabled() const;

		//! Enables/disables sending of start/stop messages.
		//! If enabled, a start message (at startup) and a
		//! stop message (at shutdown) will be sent to the
		//! STATUS group. Default = false
		void setStartStopMessagesEnabled(bool enable);
		bool areStartStopMessagesEnabled() const;

		//! Enables/disables auto shutdown caused by
		//! the shutdown of a definable master module or
		//! master username. If both values are set the
		//! one coming first is used.
		void setAutoShutdownEnabled(bool enable);
		bool isAutoShutdownEnabled() const;

		//! Enables recordstream URL option, default = true
		void setRecordStreamEnabled(bool enable);
		bool isRecordStreamEnabled() const;

		//! Load the stations from the inventory at startup, default = false
		void setLoadStationsEnabled(bool enable);
		bool isLoadStationsEnabled() const;

		//! Load the complete inventory at startup, default = false
		void setLoadInventoryEnabled(bool enable);
		bool isLoadInventoryEnabled() const;

		//! Load the configmodule from the database at startup, default = false
		void setLoadConfigModuleEnabled(bool enable);
		bool isLoadConfigModuleEnabled() const;

		//! Load the cities.xml file, default = false
		void setLoadCitiesEnabled(bool enable);
		bool isLoadCitiesEnabled() const;

		//! Load the custom defined fep regions in ~/.seiscomp/fep or
		//! ~/seiscomp/trunk/share/fep, default = false
		void setLoadRegionsEnabled(bool enable);
		bool isLoadRegionsEnabled() const;

		//! Sets whether the received notifier are applied automatically
		//! or not, default: true

		/**
		 * Sets whether the received notifier are applied automatically
		 * or not, default: true
		 * When AutoApplyNotifier is enabled a received message will
		 * be handled in two passes:
		 *  1. pass: Apply all attached notifier
		 *  2. pass: Interpret all notifier
		 *
		 * So when using an object in an interprete callback it is
		 * garantueed that all child objects that also has been sent
		 * inside the message are attached to it.
		 */
		void setAutoApplyNotifierEnabled(bool enable);
		bool isAutoApplyNotifierEnabled() const;

		/**
		 * Sets whether the received notifier will be interpreted or not.
		 * Default: true
		 * When this option is enabled, the callback methods
		 *  addObject(), updateObject() and removeObject() will be
		 * called after a notifier has been received.
		 */
		void setInterpretNotifierEnabled(bool enable);
		bool isInterpretNotifierEnabled() const;

		/** Returns whether a custom publicID pattern has been configured
		    or not */
		bool hasCustomPublicIDPattern() const;

		/**
		 * Sets the number of retries if a connection fails.
		 * The default value is 0xFFFFFFFF and should be understood
		 * as "keep on trying".
		 */
		void setConnectionRetries(unsigned int);

		//! Sets the config module name to use when reading
		//! the database configuration. An empty module name
		//! means: read all available modules.
		//! The default module is "trunk".
		void setConfigModuleName(const std::string &module);
		const std::string &configModuleName() const;

		//! Sets the master module used when auto shutdown
		//! is activated.
		void setShutdownMasterModule(const std::string &module);

		//! Sets the master username used when auto shutdown
		//! is activated.
		void setShutdownMasterUsername(const std::string &username);


	// ----------------------------------------------------------------------
	//  Public methods
	//  These methods have to be called after the init() method.
	// ----------------------------------------------------------------------
	public:
		/**
		 * Adds a logger for an input object flow.
		 * This method must be called after Application::init().
		 * The returned pointer is managed by the Application and must not
		 * be deleted.
		 */
		ObjectLog *
		addInputObjectLog(const std::string &name,
		                  const std::string &channel = "");

		/**
		 * Adds a logger for an output object flow.
		 * This method must be called after Application::init().
		 * The returned pointer is managed by the Application and must not
		 * be deleted.
		 */
		ObjectLog *
		addOutputObjectLog(const std::string &name,
		                   const std::string &channel = "");

		/**
		 * Logs input/output object throughput.
		 * @param log Pointer returned by addInputObjectLog or addOutputObjectLog
		 * @param timestamp The timestamp to be logged
		 */
		void logObject(ObjectLog *log, const Core::Time &timestamp,
		               size_t count = 1);

		/**
		 * Reloads the application inventory from either an XML file or
		 * the database.
		 */
		bool reloadInventory();

		/**
		 * Reloads the application configuration (bindings) from either an
		 * XML file or the database.
		 */
		bool reloadBindings();

		/**
		 * @brief Injects a message from outside. The message will actually
		 *        take the same path as when it would have been received via
		 *        the messaging.
		 * @param msg The message. The ownership if left to the caller.
		 * @param pkt The optional network packet. The ownership is left to
		 *            the caller.
		 */
		void injectMessage(Core::Message *msg, Packet *pkt = nullptr);

		/**
		 * @brief Routes a notifier to either add/update or removeObject.
		 * @param notifier The notifier pointer which must not be nullptr
		 */
		void handleNotifier(DataModel::Notifier *notifier);


	// ----------------------------------------------------------------------
	//  Static public members
	// ----------------------------------------------------------------------
	public:
		//! Returns the pointer to the application's instance.
		static Application *Instance();


	// ----------------------------------------------------------------------
	//  Protected functions
	// ----------------------------------------------------------------------
	protected:
		virtual bool validateParameters() override;
		virtual bool handlePreFork() override;

		virtual bool init() override;

		/**
		 * Starts the mainloop until exit() or quit() is called.
		 * The default implementation waits for messages in blocking mode
		 * and calls handleMessage() whenever a new message arrives.
		 */
		virtual bool run() override;

		//! This method gets called when all messages has been read or
		//! the connection is invalid
		virtual void idle();

		//! Cleanup method called before exec() returns.
		virtual void done() override;

		//! Opens the configuration file and reads the state variables
		virtual bool initConfiguration() override;

		//! Initialized the database
		virtual bool initDatabase();

		//! Sets the database interface and creates a database query object
		void setDatabase(IO::DatabaseInterface* db);

		/**
		 * Reads the requested subscriptions from the configuration file
		 * and apply them to the messaging connection.
		 */
		virtual bool initSubscriptions();

		const std::set<std::string> &subscribedGroups() const;

		/**
		 * Called when the application received the AcquisitionFinished event.
		 * This is most likely send from the readRecords thread of the
		 * StreamApplication. The default implementation does nothing.
		 */
		virtual void handleEndAcquisition();


	// ----------------------------------------------------------------------
	//  Messaging handlers
	// ----------------------------------------------------------------------
	protected:
		virtual bool dispatch(Core::BaseObject*);

		//! Custom dispatch method for notifications with negative (< 0)
		//! types. The default implementation return false.
		virtual bool dispatchNotification(int type, Core::BaseObject*);

		/**
		 * Reads messages from the connection.
		 * @return true, if successfull, false if not. When returning false,
		 *         the mainloop will stop and the program is going to
		 *         terminate.
		 */
		bool readMessages();

		/**
		 * This method gets called when a previously started timer timeout's.
		 * The timer has to be started by enableTimer(timeout).
		 */
		virtual void handleTimeout();

		/**
		 * This method is called when close event is sent to the application.
		 * The default handler returns true and causes the event queue to
		 * shutdown and to exit the application.
		 * It false is returned the close event is ignored.
		 */
		virtual bool handleClose();

		/**
		 * This methods gets called when an auto shutdown has been
		 * initiated. The default implementation just quits.
		 */
		virtual void handleAutoShutdown();

		/**
		 * This methods gets called when an the log interval is reached
		 * and the application should prepare its logging information. This
		 * method can be used to sync logs.
		 * The default implementation does nothing.
		 */
		virtual void handleMonitorLog(const Core::Time &timestamp);

		/**
		 * This method gets called after the connection got lost.
		 */
		virtual void handleDisconnect();

		/**
		 * This method gets called after the connection got reestablished.
		 */
		virtual void handleReconnect();

		/**
		 * Handles receiption of a network packet which is a candidate
		 * for message decoding. Special service messages such as ENTER or
		 * LEAVE will not cause a message to be created. This method is always
		 * called *before* a message should be handled.
		 */
		virtual void handleNetworkMessage(const Client::Packet *msg);

		/**
		 * This method gets called whenever a new message arrives. Derived
		 * classes have to implement this method to receive messages.
		 * To enable autoapplying and notifier interpreting call this method
		 * inside the reimplemented version.
		 * @param msg The message. A smartpointer may be stored for
		 *            future use. The pointer must not be deleted!
		 */
		virtual void handleMessage(Core::Message *msg);

		//! Callback for interpret notifier
		virtual void addObject(const std::string &parentID, DataModel::Object*) {}

		//! Callback for interpret notifier
		virtual void removeObject(const std::string &parentID, DataModel::Object*) {}

		//! Callback for interpret notifier
		virtual void updateObject(const std::string &parentID, DataModel::Object*) {}


	// ----------------------------------------------------------------------
	//  Private functions
	// ----------------------------------------------------------------------
	private:
		bool initMessaging();

		bool loadConfig(const std::string &configDB);
		bool loadInventory(const std::string &inventoryDB);

		void startMessageThread();
		void runMessageThread();

		bool processEvent();

		void timeout();

		void monitorLog(const Core::Time &timestamp, std::ostream &os);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	protected:
		DataModel::DatabaseQueryPtr   _query;
		DataModel::ConfigModulePtr    _configModule;

		std::vector<Math::Geo::CityD> _cities;
		std::set<std::string>         _messagingSubscriptions;


	private:
		static Application           *_instance;


	protected:
		using StringVector = std::vector<std::string>;

		struct AppSettings : AbstractSettings {
			int                  objectLogTimeWindow{60};

			std::string          agencyID{"UNSET"};
			std::string          author{"@appname@@@@hostname@"};

			bool                 enableLoadRegions{false};
			std::string          customPublicIDPattern;
			std::string          configModuleName{"trunk"};

			bool                 enableFetchDatabase{true};
			bool                 enableLoadStations{false};
			bool                 enableLoadInventory{false};
			bool                 enableLoadConfigModule{false};
			bool                 enableAutoApplyNotifier{true};
			bool                 enableInterpretNotifier{true};

			unsigned int         retryCount{0xFFFFFFFF};

			Util::StringFirewall networkTypeFirewall;
			Util::StringFirewall stationTypeFirewall;

			struct Database {
				void accept(SettingsLinker &linker);

				bool        enable{true};
				bool        showDrivers{false};

				std::string type;
				std::string parameters;
				std::string URI;

				std::string inventoryDB;
				std::string configDB;
			}                    database;

			struct Inventory {
				void accept(SettingsLinker &linker);

				StringVector netTypeWhitelist;
				StringVector netTypeBlacklist;
				StringVector staTypeWhitelist;
				StringVector staTypeBlacklist;
			}                    inventory;

			// Messaging
			struct Messaging {
				void accept(SettingsLinker &linker);

				bool         enable{true};
				bool         membershipMessages{false};

				std::string  user;
				std::string  URL{"localhost/production"};
				std::string  primaryGroup{Protocol::LISTENER_GROUP};
				std::string  contentType;
				unsigned int timeout{3};
				std::string  certificate;

				StringVector subscriptions;

			}                    messaging;

			struct Client {
				void accept(SettingsLinker &linker);

				bool        startStopMessages{false};
				bool        autoShutdown{false};
				std::string shutdownMasterModule;
				std::string shutdownMasterUsername;
			}                    client;

			struct RecordStream {
				void accept(SettingsLinker &linker);

				bool        enable{false};
				bool        showDrivers{false};

				std::string URI;
				std::string file;
				std::string fileType;
			}                    recordstream;

			struct Processing {
				void accept(SettingsLinker &linker);

				StringVector         agencyWhitelist;
				StringVector         agencyBlacklist;
				Util::StringFirewall firewall;
			}                    processing;

			struct Cities {
				void accept(SettingsLinker &linker);

				bool        enable{false};
				std::string db;
			}                    cities;

			void accept(SettingsLinker &linker) override;
		};

		AppSettings                  _settings;

		ObjectMonitor               *_inputMonitor;
		ObjectMonitor               *_outputMonitor;

		ThreadedQueue<Notification>  _queue;
		std::thread                 *_messageThread;

		ConnectionPtr                _connection;
		IO::DatabaseInterfacePtr     _database;
		Util::Timer                  _userTimer;

		std::mutex                   _objectLogMutex;
};


inline bool Application::waitEvent() {
	return processEvent();
}


}
}


#endif
