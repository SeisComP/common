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


#ifndef SEISCOMP_SYSTEM_APPLICATION_H
#define SEISCOMP_SYSTEM_APPLICATION_H

#include <boost/shared_ptr.hpp>

#include <seiscomp/core/exceptions.h>
#include <seiscomp/core/interruptible.h>
#include <seiscomp/config/config.h>
#include <seiscomp/system/commandline.h>
#include <seiscomp/system/settings.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/system/schema.h>


namespace Seiscomp {

namespace Logging {
	class Output;
}

namespace System {


class Application;


namespace Detail {


template <typename T>
T getConfig(const Application *app, const std::string &symbol, bool asPath);

std::string join(const std::string &prefix, const char *relativeName);


}


/**
 * \brief Application class to write commandline applications easily using
 * \brief configuration files and commandline options.
 *
 * The class Application works as follows:
 *
 * \code
 * exec()
 *   init()
 *   run()
 *   done()
 * \endcode
 *
 * All of the above methods are virtual. A derived class can reimplement
 * each method to fit its needs.
 * The Application class does all the administrative work:
 * - Reading from configuration files
 * - Handling commandline parameters
 * - Process forking when creating daemons
 * - Signal handling
 *
 * ### Detailed worflow
 *
 * In the simple call sequence above the most complex implementation is
 * hidden inside \ref init(). It deals with handling the
 * commandline, reading configuration files, setup logging and other various
 * tasks. Many of the steps can be intercepted and re-implemented if necessary.
 * Always good practice is to call the implemented method of the base class
 * first and check the result, e.g. when re-implementing \ref init().
 * @code
 * bool MyApplication::init() {
 *   if ( !Application::init() ) {
 *     SEISCOMP_ERROR("Error on initializing the application");
 *     return false;
 *   }
 *
 *   // Do custom initializations
 *   return true;
 * }
 * @endcode
 *
 * If any of the functions called from within @ref init() returns false then
 * initialization is cancelled and @ref init() returns false.
 *
 * * init() (virtual)
 *   * createCommandLineDescription() (virtual)
 *   * initConfiguration() (virtual)
 *   * printUsage() (virtual)
 *   * printVersion() (virtual)
 *   * handleCommandLineOptions() (virtual)
 *   * validateParameters() (virtual)
 *   * handleInitializationError() (virtual)
 *   * printConfigVariables() (virtual)
 *   * validateSchemaParameters() (virtual)
 *   * handlePreFork() (virtual)
 *   * forkProcess() (virtual)
 * * run() (virtual)
 *   * Is pure virtual and needs to be implemented.
 * * done() (virtual)
 *   * Sets internal exist flag and causes isExitRequested() to return
 *     false.
 *
 * ### Settings
 *
 * An important part of an application are settings. The term settings refers
 * to a collection of several options. Let's illusttrate it with an example.
 * An application introduces two parameters, param1 and param2 whereas the
 * first one is an integer and the second a string. The imperative approach
 * to read the two parameters is:
 *
 * @code
 * void MyApplication::createCommandLineDescription() {
 *   Application::createCommandLineDescription();
 *
 *   commandline().addGroup("My params");
 *   commandline().addOption("My params", "param1", "Sets param1", &_param1);
 *   commandline().addOption("My params", "param2", "Sets param2", &_param2);
 * }
 *
 * bool MyApplication::initConfiguration() {
 *   if ( !Application::initConfiguration() )
 *     return false;
 *
 *   try { _param1 = configGetInt("param1"); }
 *   catch ( ... ) {}
 *
 *   try { _param2 = configGetString("param2"); }
 *   catch ( ... ) {}
 * }
 * @endcode
 *
 * This procedure has to be repeated over and over again for each new
 * parameter. When dealing with arrays of settings then one has to repeat
 * even more boilerplate code.
 *
 * The declarative approach is more efficient and easy to use. An example
 * to illustrate the concept:
 *
 * @code
 * class MyApplication : public Application {
 *   public:
 *     MyApplication(int argc, char **argv)
 *     : Application(argc, argv) {
 *       // Bind the below settings structure to the application. Commandline
 *       // and configuration files are handled transparently.
 *       bindSettings(&_settings);
 *     }
 *
 *   private:
 *     struct Settings : AbstractSettings {
 *       int param1;
 *       string param2;
 *
 *       virtual void accept(SettingsLinker &linker) {
 *         linker & cfg(param1, "param1") & cli("My params", "param1", "Sets param1")
 *                & cfg(param2, "param2") & cli("My params", "param2", "Sets param2");
 *       }
 *     } _settings;
 * };
 * @endcode
 *
 * Here a settings structure is declared (which can be nested and also
 * contain arrays of other structures) which is bound to the application.
 * The only requirement for the root settings type is that it inherits from
 * \ref AbstractSettings. All other composite types below must implement
 * an accept method which takes \ref SettingsLinker as the only argument passed
 * via a reference.
 *
 * @code
 * void accept(SettingsLinker &linker);
 * @endcode
 */
class SC_SYSTEM_CORE_API Application : public Core::InterruptibleObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef std::vector<std::string> Arguments;

		enum Stage {
			COMMANDLINE,
			CONFIGURATION,
			LOGGING,
			PLUGINS,
			ST_QUANTITY
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Application(int argc, char** argv);
		~Application();


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		int operator()();


	// ----------------------------------------------------------------------
	//  Public functions
	// ----------------------------------------------------------------------
	public:
		//! Returns a list of commandline parameters
		const Arguments &arguments() const;

		//! Returns the commandline interface to add groups and options
		CommandLine &commandline();
		const CommandLine &commandline() const;

		//! Returns the local configuration object
		const Config::Config &configuration() const;

		//! Returns the path of the application (arg[0])
		const char* path() const;

		/**
		 * Returns the name of the application used for section lookup
		 * in the configuration repository.
		 * @return The name of the application
		 */
		const std::string& name() const;

		/**
		 * Adds a pacakge search path to the pluginregistry. This call
		 * is equal to
		 * \code
		 * Seiscomp::System::PluginRegistry::Instance()->addPackagePath(package);
		 * \endcode
		 * @param path
		 */
		void addPluginPackagePath(const std::string &package);

		//! Returns the version string
		const char *frameworkVersion() const;

		/**
		 * Enters the mainloop and waits until exit() is called
		 * or a appropriate signal has been fired (e.g. SIGTERM).
		 * @return The value that was set with to exit()
		 */
		int exec();

		/**
		 * Exit the application and set the returnCode.
		 * @param returnCode The value returned from exec()
		 */
		virtual void exit(int returnCode);

		/**
		 * Exit the application and set the returnCode to 0.
		 * This call is equivalent to exit(0).
		 */
		void quit();

		/**
		 * Returns whether exit has been requested or not.
		 * This query is important within own run loops to
		 * check for an abort criteria.
		 */
		bool isExitRequested() const;

		//! Prints the program usage. The default implementation
		//! prints the commandline options only
		virtual void printUsage() const;

		//! Returns the path to the crashhandler
		const std::string&  crashHandler() const;


	// ----------------------------------------------------------------------
	//  Initialization configuration functions
	//  This functions have to be called before the init() method
	// ----------------------------------------------------------------------
	public:
		//! Enables the daemon mode to be selectable via commandline
		void setDaemonEnabled(bool enable);

		//! Enables/disables logging of context (source file + line number)
		void setLoggingContext(bool);

		//! Enables/disables logging of component
		void setLoggingComponent(bool);

		//! Enables/disables logging to stderr
		void setLoggingToStdErr(bool);

		//! Adds a certain component to the logging output
		void addLoggingComponentSubscription(const std::string&);

		//! Closes the logging backend
		void closeLogging();

		//! Returns the applications version. The default implementation
		//! returns nullptr and uses the global framework version instead.
		virtual const char *version();


	// ----------------------------------------------------------------------
	//  Static public members
	// ----------------------------------------------------------------------
	public:
		//! Returns the pointer to the application's instance.
		static Application *Instance();

		/**
		 * Enabled/disables signal handling.
		 * It is enabled by default.
		 * NOTE: Call this method BEFORE construction when disabling signal
		 *       handling.
		 * @param termination enables/disables SIGTERM, SIGINT
		 * @param crash enables/disables SIGSEGV, SIGABRT
		 */
		static void HandleSignals(bool termination, bool crash);


	// ----------------------------------------------------------------------
	//  Protected functions
	// ----------------------------------------------------------------------
	protected:
		//! Only reimplement this in derived classes that are supposed to
		//! be inherited again. This is not meant for user code. Use
		//! \ref createCommandLineDescription() instead.
		//! If this method is being implemented, the base implementation
		//! must be called.
		virtual void createBaseCommandLineDescription();

		//! Reimplement this method to add additional commandline groups
		//! and/or options
		virtual void createCommandLineDescription();

		//! This method is called right after the commandline has been parsed
		//! and before parameters are validated. This is useful to run
		//! commands such as --version or custom queries.
		//! Return true if an option has been handled and the application
		//! should quit. The default implementation just returns false,
		virtual bool handleCommandLineOptions();

		//! This method is called just before the process would fork if
		//! daemon mode is requested. Even without daemon mode it will
		//! be called. All plugins have been loaded already and e.g. an
		//! additional commandline check can be performed which would
		//! require a particular plugin to be loaded.
		//! Return false if the application should be terminated. The default
		//! implementation just returns true.
		virtual bool handlePreFork();

		//! This method can be used to verify custom configuration or
		//! commandline parameters
		virtual bool validateParameters();

		//! Initialization method.
		virtual bool init();

		/**
		 * Starts the mainloop until exit() or quit() is called.
		 * The default implementation waits for messages in blocking mode
		 * and calls handleMessage() whenever a new message arrives.
		 */
		virtual bool run() = 0;

		//! Cleanup method called before exec() returns.
		virtual void done();

		/**
		 * Forks the process.
		 * @return The result of forking
		 */
		virtual bool forkProcess();

		//! Opens the configuration file and reads the state variables
		virtual bool initConfiguration();

		//! Loads plugins
		virtual bool initPlugins();

		/**
		 * Prints the version information to stdout
		 */
		virtual void printVersion();

		/**
		 * Prints all available configuration variables
		 */
		virtual void printConfigVariables();

		/**
		 * Returns lists of schema modules and plugins to validate in
		 * method validateSchemaParameters
		 */
		virtual void schemaValidationNames(std::vector<std::string> &modules,
		                                   std::vector<std::string> &plugins) const;

		/**
		 * Validates configuration variables use by application against
		 * description xml file
		*/
		virtual bool validateSchemaParameters();

		//! Handles the interrupt request from outside
		virtual void handleInterrupt(int) override;

		/**
		 * Derived class can implement this method to react on
		 * errors while initialization. The default implementation
		 * does nothing.
		 * @param stage The stage where the error occured
		 */
		virtual bool handleInitializationError(int stage);


	// ----------------------------------------------------------------------
	//  Verbosity handlers
	// ----------------------------------------------------------------------
	protected:
		//! Callback method to display a message regarding the current
		//! initialization state
		virtual void showMessage(const char*);

		//! Callback method to display a warning regarding the current
		//! initialization state
		virtual void showWarning(const char*);


	// ----------------------------------------------------------------------
	//  Configuration query functions
	// ----------------------------------------------------------------------
	public:
		/**
		 * Read a single value from the application's configuration.
		 * All configuration query methods throw exceptions when
		 * the query could not be resolved or the requested format
		 * did not match.
		 * This documentation applies to all configGet* functions.
		 * @param query The query
		 * @return The requested value
		 */
		bool configGetBool(const std::string& query) const;
		int configGetInt(const std::string& query) const;
		double configGetDouble(const std::string& query) const;
		std::string configGetString(const std::string& query) const;

		/**
		 * @brief Method that resolves a string variable and produces a
		 *        canonicalized absolute pathname.
		 * @param query The query
		 * @return The path
		 */
		std::string configGetPath(const std::string& query) const;

		std::vector<bool> configGetBools(const std::string& query) const;
		std::vector<int> configGetInts(const std::string& query) const;
		std::vector<double> configGetDoubles(const std::string& query) const;
		std::vector<std::string> configGetStrings(const std::string& query) const;

		/**
		 * Write a singel value to the local section of the clients
		 * configuration file.
		 */
		void configSetBool(const std::string& query, bool v);
		void configSetInt(const std::string& query, int v);
		void configSetDouble(const std::string& query, double v);
		void configSetString(const std::string& query, const std::string &v);

		void configSetBools(const std::string& query, const std::vector<bool>&);
		void configSetInts(const std::string& query, const std::vector<int>&);
		void configSetDoubles(const std::string& query, const std::vector<double>&);
		void configSetStrings(const std::string& query, const std::vector<std::string>&);

		void configUnset(const std::string& query);

		bool saveConfiguration();


		template <typename T>
		struct OptionBinding {
			enum Flags {
				NoFlags         = 0x00,
				IsKey           = 0x01,
				InterpretAsPath = 0x02,
				CLIPrintDefault = 0x04,
				CLIIsSwitch     = 0x08
			};

			OptionBinding(T &value,
			              int flags,
			              const char *configFileRelativeSymbol,
			              const char *cliGroup = nullptr,
			              const char *cliAbsoluteSymbol = nullptr,
			              const char *cliDesc = nullptr)
			: value(value)
			, flags(flags)
			, configFileRelativeSymbol(configFileRelativeSymbol)
			, cliGroup(cliGroup)
			, cliAbsoluteSymbol(cliAbsoluteSymbol)
			, cliDesc(cliDesc) {}

			bool isKey() { return flags & IsKey; }
			bool printDefault() { return flags & CLIPrintDefault; }
			bool isSwitch() { return flags & CLIIsSwitch; }

			T            &value;
			int           flags;
			const char   *configFileRelativeSymbol;
			const char   *cliGroup;
			const char   *cliAbsoluteSymbol;
			const char   *cliDesc;
		};

		template <typename T>
		struct OptionBinding< std::vector<T> > {
			enum Flags {
				NoFlags         = 0x00,
				IsKey           = 0x01,
				InterpretAsPath = 0x02,
				CLIPrintDefault = 0x04,
				CLIIsSwitch     = 0x08
			};

			typedef std::function<void(T &instance)> InitFunction;

			OptionBinding(std::vector<T> &value,
			              int flags,
			              const char *configFileRelativeSymbol,
			              const char *cliGroup = nullptr,
			              const char *cliAbsoluteSymbol = nullptr,
			              const char *cliDesc = nullptr,
			              InitFunction ctor = nullptr)
			: value(value)
			, ctor(ctor)
			, flags(flags)
			, configFileRelativeSymbol(configFileRelativeSymbol)
			, cliGroup(cliGroup)
			, cliAbsoluteSymbol(cliAbsoluteSymbol)
			, cliDesc(cliDesc) {}

			bool isKey() { return flags & IsKey; }
			bool printDefault() { return flags & CLIPrintDefault; }
			bool isSwitch() { return flags & CLIIsSwitch; }

			std::vector<T> &value;
			InitFunction    ctor;
			int             flags;
			const char     *configFileRelativeSymbol;
			const char     *cliGroup;
			const char     *cliAbsoluteSymbol;
			const char     *cliDesc;
		};


		class OptionLinker {
			public:
				OptionLinker() : _stage(None) {}


			public:
				void bind(CommandLine *cli) {
					setStage(BindCli);
					_external.cli = cli;
				}

				void get(const CommandLine *cli) {
					setStage(GetCli);
					_external.constCli = cli;
				}

				void get(const Application *app) {
					setStage(GetCfg);
					_external.constApp = app;
				}

				void dump(std::ostream &os) {
					setStage(Print);
					_external.os = &os;
				}


			public:
				// A single non-array option
				template <typename T, typename V>
				void visitSingle(V &visitor, OptionBinding<T> &visitedItem) {
					switch ( _stage ) {
						case None:
							break;
						case BindCli:
							if ( visitedItem.cliAbsoluteSymbol ) {
								if ( visitedItem.cliGroup )
									_external.cli->addGroup(visitedItem.cliGroup);
								CliLinkHelper<T, IsNativelySupported<T>::value>::process(*this, visitedItem);
							}
							break;
						case GetCli:
							if ( !visitedItem.cliAbsoluteSymbol )
								return;
							if ( !CliGetHelper<T, IsNativelySupported<T>::value>::process(*this, visitedItem) )
								visitor.setError(std::string("Invalid commandline value for ") + visitedItem.cliAbsoluteSymbol);
							break;
						case GetCfg:
							if ( !visitedItem.isKey() && !visitedItem.configFileRelativeSymbol )
								return;
							if ( !CfgLinkHelper<T, IsNativelySupported<T>::value>::process(*this, visitedItem, visitor.configPrefix) )
								visitor.setError("Invalid configuration value for " + Detail::join(visitor.configPrefix, visitedItem.configFileRelativeSymbol));
							break;
						case PutCfg:
							break;
						case Print:
							if ( visitedItem.configFileRelativeSymbol )
								*_external.os << Detail::join(visitor.configPrefix, visitedItem.configFileRelativeSymbol);
							else if ( visitedItem.cliAbsoluteSymbol )
								*_external.os << "--" << visitedItem.cliAbsoluteSymbol;
							else if ( visitedItem.isKey() )
								*_external.os << "*KEY*";
							else
								return;
							*_external.os << ": ";
							PrintHelper<T, IsNativelySupported<T>::value>::process(*_external.os, visitedItem.value);
							*_external.os << std::endl;
							break;
					}
				}

				// A single array option
				template <typename T, typename V>
				void visitSingle(V &visitor, OptionBinding< std::vector<T> > &visitedItem) {
					switch ( _stage ) {
						case None:
							break;
						case BindCli:
							if ( visitedItem.cliAbsoluteSymbol ) {
								if ( visitedItem.cliGroup )
									_external.cli->addGroup(visitedItem.cliGroup);
								CliLinkHelper<std::vector<T>, IsNativelySupported<T>::value>::process(*this, visitedItem);
							}
							break;
						case GetCli:
							if ( !visitedItem.cliAbsoluteSymbol )
								return;
							if ( !CliGetHelper<std::vector<T>, IsNativelySupported<T>::value>::process(*this, visitedItem) )
								visitor.setError(std::string("Invalid commandline value for ") + visitedItem.cliAbsoluteSymbol);
							break;
						case GetCfg:
							if ( !visitedItem.configFileRelativeSymbol )
								return;
							if ( !CfgLinkHelper<std::vector<T>, IsNativelySupported<T>::value>::process(*this, visitedItem, visitor.configPrefix) )
								visitor.setError("Invalid configuration value for " + Detail::join(visitor.configPrefix, visitedItem.configFileRelativeSymbol));
							break;
						case PutCfg:
							break;
						case Print:
							if ( visitedItem.configFileRelativeSymbol ) {
								if ( *visitedItem.configFileRelativeSymbol ) {
									*_external.os << Detail::join(visitor.configPrefix, visitedItem.configFileRelativeSymbol);
								}
								else {
									*_external.os << visitor.configPrefix;
								}
							}
							else if ( visitedItem.cliAbsoluteSymbol )
								*_external.os << "--" << visitedItem.cliAbsoluteSymbol;
							else if ( visitedItem.isKey() )
								*_external.os << "*KEY*";
							else
								return;
							*_external.os << ": ";
							PrintHelper<std::vector<T>, IsNativelySupported<T>::value>::process(*_external.os, visitedItem.value);
							*_external.os << std::endl;
							break;
					}
				}

				// An array option consisting of composites
				template <typename T, typename V>
				void visitMultiple(V &visitor, OptionBinding< std::vector<T> > &visitedItem) {
					switch ( _stage ) {
						case None:
							break;
						case BindCli:
						case GetCli:
							visitor.push(visitedItem);
							for ( size_t i = 0; i < visitedItem.value.size(); ++i )
								visitedItem.value[i].accept(visitor);
							visitor.pop();
							break;
						case GetCfg:
							try {
								std::vector<std::string> items;
								items = Detail::getConfig< std::vector<std::string> >(
									_external.constApp,
									Detail::join(
										visitor.configPrefix,
										visitedItem.configFileRelativeSymbol
									),
									false
								);
								std::string oldKey = _key;
								visitor.push(visitedItem);
								for ( size_t i = 0; i < items.size(); ++i ) {
									T value;
									if ( visitedItem.ctor ) {
										visitedItem.ctor(value);
									}

									OptionBinding<T> item(value, false, items[i].c_str());
									std::string oldKey = _key;
									visitor.push(item);
									_key = items[i];
									item.value.accept(visitor);
									if ( visitor.success() )
										visitedItem.value.push_back(value);
									visitor.pop();
									_key = oldKey;
								}
								visitor.pop();
								_key = oldKey;
							}
							catch ( ... ) {}
							break;
						case PutCfg:
							break;
						case Print:
							if ( visitedItem.configFileRelativeSymbol ) {
								*_external.os << Detail::join(visitor.configPrefix, visitedItem.configFileRelativeSymbol);
								*_external.os << ": ";
								if ( visitedItem.value.empty() )
									*_external.os << "[]" << std::endl;
								else {
									*_external.os << "[" << visitedItem.value.size() << "]" << std::endl;
									std::string oldKey = _key;
									visitor.push(visitedItem);
									for ( size_t i = 0; i < visitedItem.value.size(); ++i ) {
										*_external.os << "{" << std::endl;
										visitedItem.value[i].accept(visitor);
										*_external.os << "}" << std::endl;
									}
									visitor.pop();
									_key = oldKey;
								}
							}
							break;
					}
				}


			// Helpers
			private:
				template <typename T>
				T key() const {
					return Generic::Detail::MustMatch<std::string,T>::get(_key);
				}


				template <typename T>
				struct IsNativelySupported {
					enum {
						value = Generic::Detail::IsClassType<T>::value  ?
						(
							boost::is_same<std::string,T>::value ?
							1
							:
							0
						)
						:
						1
					};
				};


				template <typename T, int IS_SUPPORTED>
				struct CliLinkHelper {};

				template <typename T>
				struct CliLinkHelper<T,0> {
					template <typename P>
					static void process(P &proc, OptionBinding<T> &visitedItem) {
						using namespace Seiscomp::Core;
						proc._proxyValueStore[&visitedItem.value] = toString(visitedItem.value);
						proc._external.cli->addOption(
							visitedItem.cliGroup?visitedItem.cliGroup:"Generic",
							visitedItem.cliAbsoluteSymbol,
							visitedItem.cliDesc,
							&proc._proxyValueStore[&visitedItem.value],
							visitedItem.printDefault()
						);
					}
				};

				template <typename T>
				struct CliLinkHelper<T,1> {
					template <typename P>
					static void process(P &proc, OptionBinding<T> &visitedItem) {
						if ( visitedItem.isSwitch() ) {
							proc._external.cli->addOption(
								visitedItem.cliGroup?visitedItem.cliGroup:"Generic",
								visitedItem.cliAbsoluteSymbol,
								visitedItem.cliDesc
							);
						}
						else {
							proc._external.cli->addOption(
								visitedItem.cliGroup?visitedItem.cliGroup:"Generic",
								visitedItem.cliAbsoluteSymbol,
								visitedItem.cliDesc,
								&visitedItem.value,
								visitedItem.printDefault()
							);
						}
					}
				};

				template <typename T>
				struct CliLinkHelper<std::vector<T>,1> {
					template <typename P>
					static void process(P &proc, OptionBinding< std::vector<T> > &visitedItem) {
						proc._external.cli->addOption(
							visitedItem.cliGroup?visitedItem.cliGroup:"Generic",
							visitedItem.cliAbsoluteSymbol,
							visitedItem.cliDesc,
							&visitedItem.value
						);
					}
				};


				template <typename T, int IS_SUPPORTED>
				struct CliGetHelper {};

				template <typename T>
				struct CliGetHelper<T,0> {
					template <typename P>
					static bool process(P &proc, OptionBinding<T> &visitedItem) {
						using namespace Seiscomp::Core;
						try {
							bool hasOption = false;
							const char *s = strchr(visitedItem.cliAbsoluteSymbol, ',');
							if ( s ) {
								size_t len = s - visitedItem.cliAbsoluteSymbol;
								s = visitedItem.cliAbsoluteSymbol;
								Core::trim(s, len);
								hasOption = proc._external.constCli->hasOption(std::string(s, len));
							}
							else
								hasOption = proc._external.constCli->hasOption(visitedItem.cliAbsoluteSymbol);

							if ( hasOption )
								return fromString(visitedItem.value, proc._proxyValueStore[&visitedItem.value]);
							else
								return true;
						}
						catch ( std::exception & ) {
							return true;
						}
					}
				};

				template <typename T>
				struct CliGetHelper<T,1> {
					template <typename P>
					static bool process(P &proc, OptionBinding<T> &visitedItem) {
						if ( visitedItem.cliAbsoluteSymbol && visitedItem.isSwitch() ) {
							const char *s = strchr(visitedItem.cliAbsoluteSymbol, ',');
							if ( s ) {
								size_t len = static_cast<size_t>(s - visitedItem.cliAbsoluteSymbol);
								s = visitedItem.cliAbsoluteSymbol;
								Core::trim(s, len);
								if ( proc._external.constCli->hasOption(std::string(s, len)) ) {
									visitedItem.value = true;
								}
							}
							else if ( proc._external.constCli->hasOption(visitedItem.cliAbsoluteSymbol) ) {
								visitedItem.value = true;
							}
						}

						return true;
					}
				};

				template <typename T>
				struct CliGetHelper<std::vector<T>,1> {
					template <typename P>
					static bool process(P &, OptionBinding< std::vector<T> > &) {
						return true;
					}
				};


				template <typename T, int IS_SUPPORTED>
				struct CfgLinkHelper {};

				template <typename T>
				struct CfgLinkHelper<T,0> {
					template <typename P>
					static bool process(P &proc,
					                    OptionBinding<T> &visitedItem,
					                    const std::string &prefix) {
						try {
							std::string tmp;
							if ( visitedItem.isKey() )
								tmp = proc.template key<std::string>();
							else
								tmp = Detail::getConfig<std::string>(
									proc._external.constApp,
									Detail::join(prefix, visitedItem.configFileRelativeSymbol),
									visitedItem.flags & OptionBinding<T>::InterpretAsPath
								);
							return fromString(visitedItem.value, tmp);
						}
						catch ( ... ) {}

						return true;
					}
				};

				template <typename T>
				struct CfgLinkHelper<std::vector<T>,0> {
					template <typename P>
					static bool process(P &proc,
					                    OptionBinding< std::vector<T> > &visitedItem,
					                    const std::string &prefix) {
						try {
							std::vector<std::string> tmp;
							if ( visitedItem.isKey() )
								tmp = proc.template key<std::vector<std::string> >();
							else
								tmp = Detail::getConfig<std::vector<std::string> >(
									proc._external.constApp,
									Detail::join(prefix, visitedItem.configFileRelativeSymbol),
									visitedItem.flags & OptionBinding< std::vector<T> >::InterpretAsPath
								);
							visitedItem.value.resize(tmp.size());
							for ( size_t i = 0; i < tmp.size(); ++i ) {
								if ( !fromString(visitedItem.value[i], tmp[i]) )
									return false;
							}
						}
						catch ( ... ) {}

						return true;
					}
				};

				template <typename T>
				struct CfgLinkHelper<T,1> {
					template <typename P>
					static bool process(P &proc,
					                    OptionBinding<T> &visitedItem,
					                    const std::string &prefix) {
						if ( visitedItem.isKey() )
							visitedItem.value = proc.template key<T>();
						else {
							try {
								visitedItem.value = Detail::getConfig<T>(
									proc._external.constApp,
									Detail::join(prefix, visitedItem.configFileRelativeSymbol),
									visitedItem.flags & OptionBinding<T>::InterpretAsPath
								);
							}
							catch ( ... ) {}
						}
						return true;
					}
				};

				template <typename T, int IS_SUPPORTED>
				struct PrintHelper {};

				template <typename T>
				struct PrintHelper<T,0> {
					static void process(std::ostream &os, const T &value) {
						os << toString(value);
					}
				};

				template <typename T>
				struct PrintHelper<std::vector<T>,0> {
					static void process(std::ostream &os, const std::vector<T> &value) {
						if ( value.empty() ) {
							os << "[]";
							return;
						}

						for ( size_t i = 0; i < value.size(); ++i ) {
							if ( i ) os << ", ";
							PrintHelper<T,0>::process(os, value[i]);
						}
					}
				};

				template <typename T>
				struct PrintHelper<T,1> {
					static void process(std::ostream &os, const T &value) {
						os << value;
					}
				};

				template <typename T>
				struct PrintHelper<std::vector<T>,1> {
					static void process(std::ostream &os, const std::vector<T> &value) {
						if ( value.empty() )
							os << "[]";
						else
							os << Core::toString(value);
					}
				};


			private:
				enum Stage {
					None,
					BindCli,
					GetCli,
					GetCfg,
					PutCfg,
					Print
				};

				void setStage(Stage s) {
					_stage = s;
				}

				Stage                        _stage;
				std::string                  _key; //!< The current array item key value
				std::map<void*, std::string> _proxyValueStore;

				// Output structures depending on the stage
				union {
					std::ostream      *os;
					CommandLine       *cli;
					const CommandLine *constCli;
					Application       *app;
					const Application *constApp;
				}             _external;
		};


		typedef Generic::SettingsVisitor<
			OptionBinding, OptionLinker
		> SettingsLinker;


		class AbstractSettings {
			public:
				virtual ~AbstractSettings() {}

			public:
				virtual void accept(SettingsLinker &linker) = 0;

				template <typename T>
				static OptionBinding<T> key(T &boundValue) {
					return OptionBinding<T>(boundValue, OptionBinding<T>::IsKey, nullptr);
				}

				template <typename T>
				static OptionBinding<T> cfg(T &boundValue, const char *name) {
					return OptionBinding<T>(boundValue, 0, name);
				}

				template <typename T>
				static OptionBinding< std::vector<T> > cfg(std::vector<T> &boundValue, const char *name, typename OptionBinding< std::vector<T> >::InitFunction ctor = nullptr) {
					return OptionBinding< std::vector<T> >(boundValue, 0, name, nullptr, nullptr, nullptr, ctor);
				}

				template <typename T>
				static OptionBinding<T> cfgAsPath(T &boundValue, const char *name) {
					return OptionBinding<T>(boundValue, OptionBinding<T>::InterpretAsPath, name);
				}

				template <typename T>
				static OptionBinding<T> cli(T &boundValue, const char *group, const char *option,
				                            const char *desc, bool default_ = false, bool switch_ = false) {
					int flags = 0;
					if ( default_ ) flags |= OptionBinding<T>::CLIPrintDefault;
					if ( switch_ ) flags |= OptionBinding<T>::CLIIsSwitch;
					return OptionBinding<T>(boundValue, flags, nullptr, group, option, desc);
				}

				template <typename T>
				static OptionBinding<T> cliAsPath(T &boundValue, const char *group, const char *option,
				                                  const char *desc, bool default_ = false, bool switch_ = false) {
					int flags = OptionBinding<T>::InterpretAsPath;
					if ( default_ ) flags |= OptionBinding<T>::CLIPrintDefault;
					if ( switch_ ) flags |= OptionBinding<T>::CLIIsSwitch;
					return OptionBinding<T>(boundValue, flags, nullptr, group, option, desc);
				}

				static OptionBinding<bool> cliSwitch(bool &boundValue, const char *group, const char *option,
				                                     const char *desc) {
					return OptionBinding<bool>(boundValue, OptionBinding<bool>::CLIIsSwitch, nullptr, group, option, desc);
				}
		};


		void bindSettings(AbstractSettings *settings);


	// ----------------------------------------------------------------------
	//  Private functions
	// ----------------------------------------------------------------------
	private:
		std::string argumentStr(const std::string &query) const;

		bool parseCommandLine();
		int acquireLockfile(const std::string &lockfile);

		void initCommandLine();
		bool initLogging();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	protected:
		static Application *_instance;

		typedef std::vector<std::string> ComponentList;

		SettingsLinker                 _settingsLinker;

		static bool                    _handleCrash;
		static bool                    _handleTermination;

		int                            _argc;
		char                         **_argv;

		std::string                    _name;

		Arguments                      _arguments;
		boost::shared_ptr<CommandLine> _commandline;

		Logging::Output               *_logger;

		// Initialization configuration
		Config::Config                 _configuration;

		int                            _returnCode;
		bool                           _exitRequested;

		std::string                    _version;

		std::list<AbstractSettings*>   _settings;


		struct BaseSettings : AbstractSettings {
			BaseSettings();

			std::string alternativeConfigFile;
			bool        enableDaemon;
			std::string crashHandler;
			std::string lockfile;
			std::string plugins;
			std::string certificateStoreDirectory;

			struct Logging {
				unsigned int  verbosity{2};
				bool          quiet{false};
				bool          trace{false};
				bool          debug{false};
#ifndef WIN32
				bool          syslog{false};
#endif
				bool          context{false};
				int           component{-1};
				bool          toStdout{false};
				bool          UTC{false};
				std::string   alternativeLogFile;
				ComponentList components;

				struct File {
					struct Rotator {
						bool enable{true};
						int timeSpan{60 * 60 * 24}; /* one day*/
						int archiveSize{7}; /* one week archive */
						int maxFileSize{100 * 1024 * 1024}; /* max 100MB per logfile */

						void accept(SettingsLinker &linker) {
							linker
							& cfg(timeSpan, "timeSpan")
							& cfg(archiveSize, "archiveSize")
							& cfg(maxFileSize, "maxFileSize");
						}
					} rotator;

					void accept(SettingsLinker &linker) {
						linker
						& cfg(rotator.enable, "rotator")
						& cfg(rotator, "rotator");
					}
				} file;

				void accept(SettingsLinker &linker) {
					linker
					& cfg(verbosity, "level")
					& cfg(context, "context")
					& cfg(component, "component")
					& cfg(components, "components")
					& cfg(syslog, "syslog")
					& cfg(toStdout, "stderr")
					& cfg(UTC, "utc")
					& cfg(file, "file")

					& cli(
						quiet,
						"Verbose", "quiet,q",
						"Quiet mode: no logging output"
					)
					& cli(
						verbosity,
						"Verbose", "verbosity",
						"Verbosity level [0..4]",
						false)
					& cli(
						context,
						"Verbose", "print-context",
						"Print source file and line number",
						false
					)
					& cli(
						component,
						"Verbose", "print-component",
						"Print the log component (default: file:1, stdout:0)",
						false
					)
					& cli(
						components,
						"Verbose", "component",
						"Limits the logging to a certain component. "
						"This option can be given more than once"
					)
					& cli(
						toStdout,
						"Verbose", "console",
						"Send log output to stdout"
					)
					& cli(
						debug,
						"Verbose", "debug",
						"debug mode: --verbosity=4 --console=1",
						false, true
					)
					& cli(
						trace,
						"Verbose", "trace",
						"trace mode: --verbosity=4 --console=1 --print-component=1 --print-context=1",
						false, true
					)
					& cli(
						alternativeLogFile,
						"Verbose", "log-file",
						"Use alternative log file"
					)
					& cli(
						UTC,
						"Verbose", "log-utc",
						"Use UTC instead of local timezone"
					)
#ifndef WIN32
					& cli(
						syslog,
						"Verbose", "syslog,s",
						"Use syslog",
						false, true
					)
#endif
					;
				}
			} logging;

			virtual void accept(SettingsLinker &linker);
		} _baseSettings;
};


inline CommandLine &Application::commandline() {
	return *_commandline;
}

inline const CommandLine &Application::commandline() const {
	return *_commandline;
}


}
}


#endif
