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


#ifndef SEISCOMP_GUI_APPLICATION_H
#define SEISCOMP_GUI_APPLICATION_H

#include <QApplication>
#include <QString>
#include <QRectF>

#ifndef Q_MOC_RUN
#include <seiscomp/client/application.h>
#endif

#include <seiscomp/gui/core/maps.h>
#include <seiscomp/gui/core/scheme.h>
#include <seiscomp/gui/core/messagethread.h>
#ifndef Q_MOC_RUN
#include <seiscomp/gui/core/messages.h>
#endif

#include <seiscomp/gui/core/ui_showplugins.h>


#define SCApp (Seiscomp::Gui::Application::Instance())
#define SCScheme (SCApp->scheme())

class QAbstractItemView;
class QHeaderView;
class QSplashScreen;

namespace Seiscomp {

namespace Core {

DEFINE_SMARTPOINTER(Message);

}

namespace Communication {

DEFINE_SMARTPOINTER(Connection);

}

namespace IO {

DEFINE_SMARTPOINTER(DatabaseInterface);

}

namespace Logging {

class FileOutput;

}

namespace DataModel {

DEFINE_SMARTPOINTER(DatabaseQuery);
DEFINE_SMARTPOINTER(Network);
DEFINE_SMARTPOINTER(Station);
DEFINE_SMARTPOINTER(Notifier);
DEFINE_SMARTPOINTER(Object);

}


namespace Gui {


class ConnectionDialog;


struct MessageGroups {
	std::string pick;
	std::string amplitude;
	std::string magnitude;
	std::string location;
	std::string focalMechanism;
	std::string event;
};


class SC_GUI_API Application : public QObject, public Client::Application {
	Q_OBJECT

	public:
		//! Application flags
		enum Flags {
			//! Show splash screen on startup
			SHOW_SPLASH            = 0x001,
			//! The application wants a database connection
			WANT_DATABASE          = 0x002,
			//! The application wants a messaging connection
			WANT_MESSAGING         = 0x004,
			//! The connection dialog should be opened when
			//! either one of the connections that has been
			//! requested with 'WANT_[SERVICE]' failed to create
			OPEN_CONNECTION_DIALOG = 0x008,
			//! If WANT_DATABASE is enabled and no custom settings
			//! are provided in the configuration file it tries to
			//! fetch the database connection from the messaging
			FETCH_DATABASE         = 0x010,
			//! Should received notifier messages be applied or not
			AUTO_APPLY_NOTIFIER    = 0x020,
			//! Should received notifier messages be interpreted or not
			//! When this flag is not set the signals 'addObject',
			//! 'removeObject' and 'updateObject' are not fired
			INTERPRETE_NOTIFIER    = 0x040,
			LOAD_STATIONS          = 0x080,
			LOAD_INVENTORY         = 0x100,
			LOAD_CONFIGMODULE      = 0x200,
			DEFAULT                = SHOW_SPLASH |
			                         WANT_DATABASE |
			                         WANT_MESSAGING |
			                         OPEN_CONNECTION_DIALOG |
			                         FETCH_DATABASE |
			                         AUTO_APPLY_NOTIFIER |
			                         INTERPRETE_NOTIFIER
		};

		enum Type {
			//! Console application
			Tty,
			//! GUI client application
			GuiClient
		};

	public:
		Application(int& argc, char **argv, int flags = DEFAULT, Type type = GuiClient);
		virtual ~Application();


	public:
		//! Returns the pointer to the application's instance.
		static Application *Instance();

		//! Checks if the installed Qt version is at least the
		//! one passed in 'ver'
		static bool minQtVersion(const char *ver);

		//! Creates a CSV table into a string.
		static QString createCSV(const QAbstractItemView* view,
		                         const QHeaderView *header = nullptr);

		//! Copies all selected items of specified item view to clipboard as CSV
		static void copyToClipboard(const QAbstractItemView* view,
		                            const QHeaderView *header = nullptr);

		Type type() const;

		Scheme &scheme();

		QSettings &settings();
		const QSettings &settings() const;

		bool startFullScreen() const;
		bool nonInteractive() const;

		bool isReadOnlyMessaging() const { return _readOnlyMessaging; }

		const MapsDesc &mapsDesc() const;
		const MessageGroups &messageGroups() const;

		Core::TimeSpan maxEventAge() const;

		QColor configGetColor(const std::string& query, const QColor& base) const;
		Gradient configGetColorGradient(const std::string& query, const Gradient& base) const;
		QFont configGetFont(const std::string& query, const QFont& base) const;
		QPen configGetPen(const std::string& query, const QPen& base) const;
		QBrush configGetBrush(const std::string& query, const QBrush& base) const;

		void configSetColorGradient(const std::string& query, const Gradient &gradient);

		void setFilterCommandsEnabled(bool);

		const std::string &commandTarget() const;

		void sendCommand(Command command, const std::string& parameter);
		void sendCommand(Command command, const std::string& parameter, Core::BaseObject*);

		//! Sets the mainwidget which is used as hint to close the
		//! splashscreen when the widget is shown
		void setMainWidget(QWidget*);

		void showMessage(const char*) override;
		void showWarning(const char*) override;

		bool notify(QObject *receiver, QEvent *e);

		bool sendMessage(Seiscomp::Core::Message* msg);
		bool sendMessage(const char* group, Seiscomp::Core::Message* msg);

		//! This method allows to emit notifier locally. They are not being sent over
		//! the messaging but interpreted and signalled to other local components.
		void emitNotifier(Seiscomp::DataModel::Notifier* n);

		QFont font() const;
		void setFont(const QFont &font);

		QPalette palette() const;
		void setPalette(const QPalette &pal);


	protected:
		bool init() override;
		bool run() override;
		void done() override;

		void exit(int returnCode) override;

		bool initConfiguration() override;
		bool initSubscriptions() override;

		void schemaValidationNames(std::vector<std::string> &modules,
		                           std::vector<std::string> &plugins) const override;

		bool validateParameters() override;

		bool handleInitializationError(int) override;
		void handleInterrupt(int) throw() override;

		virtual QString splashImagePath() const;


	signals:
		void changedConnection();
		void changedDatabase();

		void connectionEstablished();
		void connectionLost();

		void showNotification(NotificationLevel level, QString message);

		void messageSkipped(Seiscomp::Client::Packet*);
		void messageAvailable(Seiscomp::Core::Message*, Seiscomp::Client::Packet*);

		void notifierAvailable(Seiscomp::DataModel::Notifier*);

		void addObject(const QString &parentID, Seiscomp::DataModel::Object*);
		void removeObject(const QString &parentID, Seiscomp::DataModel::Object*);
		void updateObject(const QString &parentID, Seiscomp::DataModel::Object*);


	public slots:
		void showSettings();
		void quit();


	private slots:
		void createConnection(QString host, QString user, QString group,
		                      int TimeOut, QString peerCertificate);
		void destroyConnection();
		void databaseChanged();

		void messagesAvailable();

		void onConnectionEstablished();
		void onConnectionLost();
		void connectionError(int code);

		void objectDestroyed(QObject*);
		void closedLastWindow();

		void showAbout();
		void showHelpIndex();
		void showAppHelp();
		void showPlugins();

		void timerSOH();
		void handleSignalNotification();


	private:
		void startMessageThread();
		void closeMessageThread();
		void createSettingsDialog();
		ConnectionDialog *cdlg();


	private:
		static Application *_instance;


	protected:
		struct _GUI_Core_Settings : System::Application::AbstractSettings {
			bool        fullScreen{false};
			bool        interactive{true};
			std::string guiGroup{"GUI"};
			std::string commandTargetClient;

			struct _MapsDesc : MapsDesc {
				_MapsDesc();

				QString format;

				void accept(SettingsLinker &linker);
			}           mapsDesc;

			void accept(SettingsLinker &linker);

		}                   _settings;

		QApplication       *_app;
		Type                _type;
		Scheme             *_scheme;
		mutable QSettings  *_qSettings;
		QTimer              _timerSOH;

		bool                _readOnlyMessaging;
		Core::TimeSpan      _eventTimeAgo;
		MessageGroups       _messageGroups;

		QWidget            *_mainWidget;
		QSplashScreen      *_splash;
		ConnectionDialog   *_dlgConnection;
		bool                _settingsOpened;
		bool                _filterCommands;

		MessageThread      *_thread;
		int                 _flags;

		QSocketNotifier    *_signalNotifier;
		int                 _signalSocketFd[2];
};


template <typename T>
class Kicker : public Application {
	public:
		Kicker(int& argc, char **argv, int flags = DEFAULT)
			: Application(argc, argv, flags) {}


	protected:
		virtual void setupUi(T*) {}
		virtual bool initUi(T*) { return true; }

		virtual bool run() {
			showMessage("Setup user interface");

			T* w = new T;
			if ( !initUi(w) ) {
				showMessage("Kicker initialization failed, abort");
				delete w;
				return false;
			}

			setupUi(w);
			setMainWidget(w);

			if ( startFullScreen() )
				w->showFullScreen();
			else
				w->showNormal();

			return Application::run();
		}
};


}
}


#endif
