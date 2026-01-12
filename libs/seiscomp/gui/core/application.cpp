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

#include <seiscomp/system/hostinfo.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/connectiondialog.h>
#include <seiscomp/gui/core/aboutwidget.h>
#include <seiscomp/gui/core/processmanager.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/messaging/connection.h>
#include <seiscomp/messaging/messages/database.h>
#include <seiscomp/system/pluginregistry.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/utils/files.h>
#include <seiscomp/utils/misc.h>

#include <QHeaderView>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplashScreen>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

#include <set>
#include <iostream>
#include <string>
#include <algorithm>
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#endif


using namespace std;
using namespace Seiscomp::DataModel;


bool fromString(QString &qstring, const std::string &stdstring) {
	qstring = stdstring.c_str();
	return true;
}

std::string toString(const QString &qstring) {
	return qstring.toStdString();
}


namespace {


// Don't catch signal on windows since that path hasn't tested.
#ifdef WIN32
struct Initializer {
	Initializer() {
		Seiscomp::Client::Application::HandleSignals(false, false);
	}
};

Initializer __init;
#endif


class ShowPlugins : public QDialog {
	public:
		ShowPlugins(QWidget *parent = nullptr) : QDialog(parent) {
			_ui.setupUi(this);
			_ui.labelHeadline->setFont(SCScheme.fonts.heading3);
			_ui.labelHeadline->setText(QString("Plugins for %1").arg(SCApp->name().c_str()));

			QString content;

			Seiscomp::System::PluginRegistry::iterator it;
			for ( it = Seiscomp::System::PluginRegistry::Instance()->begin();
			      it != Seiscomp::System::PluginRegistry::Instance()->end(); ++it ) {
				QFileInfo info(it->filename.c_str());

				content += QString("<p><b>%1</b><br/>"
				                   "<i>%2</i><br/>"
				                   "File: <u>%7</u><br/>"
				                   "Author: %6<br/>"
				                   "Version: %3.%4.%5</p>")
				           .arg(info.baseName())
				           .arg((*it)->description().description.c_str())
				           .arg((*it)->description().version.major)
				           .arg((*it)->description().version.minor)
				           .arg((*it)->description().version.revision)
				           .arg((*it)->description().author.c_str())
				           .arg(info.absoluteFilePath());
			}

			_ui.content->setHtml(content);
		}

	private:
		Ui::ShowPlugins _ui;
};


void drawText(QPainter &p, const QPoint &hotspot, int align, const QString &s) {
	QRect r(hotspot, hotspot);

	if ( align & Qt::AlignLeft ) {
		r.setRight(p.window().right());
	}
	else if ( align & Qt::AlignRight ) { {
		r.setLeft(p.window().left());
	}
	}
	else if ( align & Qt::AlignHCenter ) {
		r.setLeft(hotspot.x()-p.window().width());
		r.setRight(hotspot.x()+p.window().width());
	}

	if ( align & Qt::AlignTop ) {
		r.setBottom(p.window().bottom());
	}
	else if ( align & Qt::AlignBottom ) {
		r.setTop(p.window().top());
	}
	else if ( align & Qt::AlignVCenter ) {
		r.setTop(hotspot.y()-p.window().height());
		r.setBottom(hotspot.y()+p.window().height());
	}

	p.drawText(r, align, s);
}


class SplashScreen : public QSplashScreen {
	public:
		SplashScreen(const QPixmap & pixmap = QPixmap())
		: QSplashScreen(pixmap) {}

		void drawContents(QPainter *painter) {
			painter->setPen(SCScheme.colors.splash.message);
			painter->drawText(
				SCScheme.splash.message.pos.x(),
				SCScheme.splash.message.pos.y() + painter->fontMetrics().descent() - painter->fontMetrics().height(),
				180, painter->fontMetrics().height(),
				SCScheme.splash.message.align,
				message()
			);
		}
};


}


namespace Seiscomp {
namespace Gui {


Application* Application::_instance = nullptr;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::_GUI_Core_Settings::accept(SettingsLinker &linker) {
	linker
	& cli(
		styleSheet,
		"User interface",
		"stylesheet",
		"Apply the stylesheet (.qss) to the application"
	)
	& cliSwitch(
		fullScreen,
		"User interface",
		"full-screen,F",
		"Starts the application in fullscreen"
	)
	& cliInverseSwitch(
		interactive,
		"User interface",
		"non-interactive,N",
		"Use non interactive presentation mode"
	)
	& cfg(fullScreen, "mode.fullscreen")
	& cfg(interactive, "mode.interactive")
	& cfg(mapsDesc, "map")
	& cfg(commandTargetClient, "commands.target");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::_GUI_Core_Settings::_MapsDesc::_MapsDesc() {
	location = "@DATADIR@/maps/world%s.png";
	isMercatorProjected = false;
	cacheSize = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::_GUI_Core_Settings::_MapsDesc::accept(SettingsLinker &linker) {
	linker
	& cfg(type, "type")
	& cfg(format, "format")
	& cfg(location, "location")
	& cfg(cacheSize, "cacheSize");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Application(int& argc, char **argv, int flags, Type type)
: QObject(), Client::Application(argc, argv)
, _qSettings(nullptr)
, _readOnlyMessaging(false)
, _mainWidget(nullptr)
, _splash(nullptr)
, _dlgConnection(nullptr)
, _settingsOpened(false)
, _flags(flags) {
	bindSettings(&_settings);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	// This is especially important for displays with a display pixel ratio
	// greater than 1, e.g. 4k displays. Otherwise QIcon pixmaps will be scaled
	// up to the native display resolution which looks blurry at best.
	// In Qt6 this setting is default.
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	_type = type;
	if ( type == Tty ) {
		_flags &= ~SHOW_SPLASH;
		setenv("QT_QPA_PLATFORM", "offscreen", 1);
		_app = new QApplication(argc, argv);
	}
	else {
		_app = new QApplication(argc, argv);
	}

#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
	_app->setDesktopFileName(("de.gempa.seiscomp." + name()).data());
#endif

	setDaemonEnabled(false);

	if ( _instance != this && _instance != nullptr ) {
		SEISCOMP_WARNING("Another GUI application object exists already. "
		                 "This usage is not intended. "
		                 "The Application::Instance() method will return "
		                 "the last created application.");
	}

	_instance = this;

	// Sceme must be created after the instance has been initialized
	// because it uses SCApp pointer
	_scheme = new Scheme();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

	// Disable group separator (thousand separator) in output and input. E.g.,
	// QLineEdits using a QDoubleValidator will no longer accept a comma.
	auto appLocale = QLocale::c();
	appLocale.setNumberOptions(QLocale::OmitGroupSeparator |
	                           QLocale::RejectGroupSeparator);
	QLocale::setDefault(appLocale);

	_thread = nullptr;
	_filterCommands = true;

	// argc and argv may be modified by QApplication. It removes the
	// commandline options it recognizes so we can go on without an
	// "unknown command" error when parsing the commandline with our own
	// class Client::CommandLine

	setDaemonEnabled(false);
	setRecordStreamEnabled(true);

	setDatabaseEnabled(_flags & WANT_DATABASE, _flags & FETCH_DATABASE);
	setMessagingEnabled(_flags & WANT_MESSAGING);

	setAutoApplyNotifierEnabled(_flags & AUTO_APPLY_NOTIFIER);
	setInterpretNotifierEnabled(_flags & INTERPRETE_NOTIFIER);

	/*
	setAutoApplyNotifierEnabled(false);
	setInterpretNotifierEnabled(false);
	*/

	setLoadInventoryEnabled(_flags & LOAD_INVENTORY);
	setLoadStationsEnabled(_flags & LOAD_STATIONS);
	setLoadConfigModuleEnabled(_flags & LOAD_CONFIGMODULE);
	setLoadCitiesEnabled(true);

	setConnectionRetries(0);

#ifndef WIN32
	if ( ::socketpair(AF_UNIX, SOCK_STREAM, 0, _signalSocketFd) ) {
		qFatal("Couldn't create HUP socketpair");
	}

	_signalNotifier = new QSocketNotifier(_signalSocketFd[1], QSocketNotifier::Read, this);
	connect(_signalNotifier, SIGNAL(activated(int)), this, SLOT(handleSignalNotification()));
#else
	_signalNotifier = nullptr;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::~Application() {
	if ( _dlgConnection ) delete _dlgConnection;
	if ( _qSettings ) delete _qSettings;
	if ( _scheme ) delete _scheme;
	if ( _app ) delete _app;
#ifndef WIN32
	close(_signalSocketFd[0]);
	close(_signalSocketFd[1]);
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application* Application::Instance() {
	return _instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::minQtVersion(const char *ver) {
	QString s = QString::fromLatin1(ver);
	QString sq = qVersion();
	return ((sq.section('.',0,0).toInt()<<16)+(sq.section('.',1,1).toInt()<<8)+sq.section('.',2,2).toInt()>=
	       (s.section('.',0,0).toInt()<<16)+(s.section('.',1,1).toInt()<<8)+s.section('.',2,2).toInt());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Type Application::type() const {
	return _type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString Application::createCSV(const QAbstractItemView* view,
                               const QHeaderView *header) {
	QAbstractItemModel *model = view->model();
	QModelIndexList items = view->selectionModel()->selectedRows();
	QString csv;
	int columns = model->columnCount();

	if ( items.empty() ) {
		return csv;
	}

	// Add header
	int c = 0;
	for ( int i = 0; i < columns; ++i ) {
		if ( header && header->isSectionHidden(i) ) {
			continue;
		}

		csv += c++ == 0 ? "# " : ";";
		csv += model->headerData(i, Qt::Horizontal).toString();
	}

	for ( auto it = items.constBegin(); it != items.constEnd(); ++it ) {
		csv += '\n';

		c = 0;
		for ( int i = 0; i < columns; ++i ) {
			if ( header && header->isSectionHidden(i) ) {
				continue;
			}

			if ( c++ > 0 ) {
				csv += ';';
			}

			csv += model->data(it->sibling(it->row(), i)).toString();
		}
	}

	return csv;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::copyToClipboard(const QAbstractItemView *view,
                                  const QHeaderView *header) {
	QClipboard *cb = QApplication::clipboard();
	if ( cb ) {
		cb->setText(createCSV(view, header));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setMainWidget(QWidget* w) {
	_mainWidget = w;

	QMainWindow *mw = dynamic_cast<QMainWindow*>(w);
	if ( mw ) {
		QMenu *helpMenu = mw->menuBar()->findChild<QMenu*>("menuHelp");
		if ( helpMenu == nullptr ) {
			helpMenu = new QMenu(mw->menuBar());
			helpMenu->setObjectName("menuHelp");
			helpMenu->setTitle("&Help");
			mw->menuBar()->addAction(helpMenu->menuAction());
		}

		QAction *a = helpMenu->addAction("&About SeisComP");
		connect(a, SIGNAL(triggered()), this, SLOT(showAbout()));

		a = helpMenu->addAction("&Documentation index");
		a->setShortcut(QKeySequence("F1"));
		connect(a, SIGNAL(triggered()), this, SLOT(showHelpIndex()));

		a = helpMenu->addAction(QString("Documentation for %1").arg(name().c_str()));
		a->setShortcut(QKeySequence("Shift+F1"));
		connect(a, SIGNAL(triggered()), this, SLOT(showAppHelp()));

		a = helpMenu->addAction("&Loaded Plugins");
		connect(a, SIGNAL(triggered()), this, SLOT(showPlugins()));
	}

	if ( _splash )
		_splash->finish(w);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QWidget *Application::mainWidget() {
	return _mainWidget;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showAbout() {
	AboutWidget *w = new AboutWidget(nullptr);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setWindowModality(Qt::ApplicationModal);

	if ( _mainWidget ) {
		QPoint p = _mainWidget->geometry().center();
		QRect g = w->geometry();
		g.moveCenter(p);
		w->setGeometry(g);
	}

	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showHelpIndex() {
	QString indexFile = QString("%1/doc/seiscomp/html/index.html")
	                    .arg(Environment::Instance()->shareDir().c_str());

	if ( !QFile::exists(indexFile) ) {
		QMessageBox::information(nullptr, "Help index",
		                         tr("The help package has not been found (not installed?)."));
		return;
	}

	QDesktopServices::openUrl(QString("file://%1").arg(indexFile));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showAppHelp() {
	QString indexFile = QString("%1/doc/seiscomp/html/apps/%2.html")
	                    .arg(Environment::Instance()->shareDir().c_str())
	                    .arg(name().c_str());

	if ( !QFile::exists(indexFile) ) {
		QMessageBox::information(nullptr, QString("%1 help").arg(name().c_str()),
		                         tr("Help for %1 is not available.").arg(name().c_str()));
		return;
	}

	QDesktopServices::openUrl(QString("file://%1").arg(indexFile));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showPlugins() {
	ShowPlugins dlg;
	dlg.exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::timerSOH() {
	stateOfHealth();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleSignalNotification() {
	int signal;
	_signalNotifier->setEnabled(false);
	ssize_t bytesRead = ::read(_signalSocketFd[1], &signal, sizeof(signal));
	if ( bytesRead != sizeof(signal) )
		qWarning() << "Failed to read int from pipe";
	QApplication::quit();
	_signalNotifier->setEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme& Application::scheme() {
	return *_scheme;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QSettings &Application::settings() {
	if ( !_qSettings ) {
		_qSettings = new QSettings;
	}
	return *_qSettings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QSettings &Application::settings() const {
	if ( !_qSettings ) {
		_qSettings = new QSettings;
	}
	return *_qSettings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::startFullScreen() const {
	return _settings.fullScreen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::nonInteractive() const {
	return !_settings.interactive;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MapsDesc &Application::mapsDesc() const {
	return _settings.mapsDesc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MessageGroups &Application::messageGroups() const {
	return _messageGroups;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::TimeSpan Application::maxEventAge() const {
	return _eventTimeAgo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor Application::configGetColor(const std::string& query,
                                   const QColor& base) const {
	try {
		std::string col = configGetString(query);
		return readColor(query, col, base);
	}
	catch ( ... ) {}

	return base;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Gradient Application::configGetColorGradient(const std::string& query,
                                             const Gradient& base) const {
	decltype(configGetStrings(query)) colors;
	try {
		colors = configGetStrings(query);
	}
	catch ( ... ) {
		return base;
	}

	Gradient grad;
	for ( size_t i = 0; i < colors.size(); ++i ) {
		QColor color;
		qreal value;

		std::vector<std::string> toks;
		size_t size = Core::split(toks, colors[i].c_str(), ":");
		if ( (size < 2) || (size > 3) ) {
			SEISCOMP_ERROR("Wrong format of color entry %lu in '%s'",
			               i, query.c_str());
			return base;
		}

		if ( !Core::fromString(value, toks[0]) ) {
			SEISCOMP_ERROR("Wrong value format of color entry %lu in '%s'",
			               i, query.c_str());
			return base;
		}

		bool ok;
		color = readColor("", toks[1], color, &ok);
		if ( !ok ) {
			return base;
		}

		QString text;
		if ( size == 3 ) {
			text = QString::fromStdString(toks[2]);
		}

		grad.setColorAt(value, color, text);
	}

	return grad;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QFont Application::configGetFont(const std::string& query, const QFont& base) const {
	QFont f(base);

	try {
		f.setFamily(configGetString(query + ".family").c_str());
	}
	catch ( ... ) {}

	try {
		f.setPointSize(configGetInt(query + ".size"));
	}
	catch ( ... ) {}

	try {
		f.setBold(configGetBool(query + ".bold"));
	}
	catch ( ... ) {}

	try {
		f.setItalic(configGetBool(query + ".italic"));
	}
	catch ( ... ) {}

	try {
		f.setUnderline(configGetBool(query + ".underline"));
	}
	catch ( ... ) {}

	try {
		f.setOverline(configGetBool(query + ".overline"));
	}
	catch ( ... ) {}

	return f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPen Application::configGetPen(const std::string& query, const QPen& base) const {
	QPen p(base);

	// Color
	try {
		const std::string& colQuery = query + ".color";
		p.setColor(readColor(colQuery, configGetString(colQuery), base.color()));
	}
	catch ( ... ) {}

	// Style
	try {
		const std::string& styleQuery = query + ".style";
		p.setStyle(readPenStyle(styleQuery, configGetString(styleQuery), base.style()));
	}
	catch ( ... ) {}

	// Width
	try {
		p.setWidth(configGetDouble(query + ".width"));
	}
	catch ( ... ) {}

	return p;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QBrush Application::configGetBrush(const std::string& query, const QBrush& base) const {
	QBrush b(base);

	// Color
	try {
		const std::string& colQuery = query + ".color";
		b.setColor(readColor(colQuery, configGetString(colQuery), base.color()));
	}
	catch ( ... ) {}

	// Style
	try {
		const std::string& styleQuery = query + ".style";
		b.setStyle(readBrushStyle(styleQuery, configGetString(styleQuery), base.style()));
	}
	catch ( ... ) {}

	return b;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetColorGradient(const std::string& query, const Gradient &gradient) {
	std::vector<std::string> colors;
	Gradient::const_iterator it;
	for ( it = gradient.begin(); it != gradient.end(); ++it ) {
		string c = Core::toString(it.key());
		c += ":";

		Util::toHex(c, (unsigned char)it.value().first.red());
		Util::toHex(c, (unsigned char)it.value().first.green());
		Util::toHex(c, (unsigned char)it.value().first.blue());
		if ( it.value().first.alpha() != 255 )
			Util::toHex(c, (unsigned char)it.value().first.alpha());

		if ( !it.value().second.isEmpty() ) {
			c += ":";
			c += it.value().second.toStdString();
		}
		colors.push_back(c);
	}

	_configuration.setStrings(query, colors);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initConfiguration() {
	if ( !Client::Application::initConfiguration() ) {
		return false;
	}

	QPalette pal;
	_scheme->colors.background = pal.color(QPalette::Window);
	_scheme->fetch();

	pal.setColor(QPalette::Window, _scheme->colors.background);

	if ( _type == GuiClient) {
		dynamic_cast<QApplication*>(_app)->setPalette(pal);
	}

	_settings.mapsDesc.location = Environment::Instance()->absolutePath(
		_settings.mapsDesc.location.toStdString()
	).c_str();

	_eventTimeAgo = 0.0;
	bool setTimeAgo = false;
	try {
		_eventTimeAgo += double(configGetInt("events.timeAgo.days")*24*60*60);
		setTimeAgo = true;
	}
	catch (...) {}
	try {
		_eventTimeAgo += double(configGetInt("events.timeAgo.hours")*60*60);
		setTimeAgo = true;
	}
	catch (...) {}
	try {
		_eventTimeAgo += double(configGetInt("events.timeAgo.minutes")*60);
		setTimeAgo = true;
	}
	catch (...) {}
	try {
		_eventTimeAgo += double(configGetInt("events.timeAgo.seconds"));
		setTimeAgo = true;
	}
	catch (...) {}

	// Default is: display events from 1 day ago until 'now'
	if ( !setTimeAgo ) {
		_eventTimeAgo = double(24*60*60);
	}

	_app->setOrganizationName("gempa");
	_app->setApplicationName(name().c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::validateParameters() {
	if ( !Client::Application::validateParameters() ) {
		return false;
	}

	if ( _settings.mapsDesc.format == "mercator" ) {
		_settings.mapsDesc.isMercatorProjected = true;
	}
	else if ( _settings.mapsDesc.format == "rectangular" ) {
		_settings.mapsDesc.isMercatorProjected = false;
	}
	else if ( !_settings.mapsDesc.format.isEmpty() ) {
		cerr << "Unknown map format: " << qPrintable(_settings.mapsDesc.format) << endl;
		return false;
	}

	// There is nothing to validate. It is just the best place to show up
	// the splash screen before the time consuming initialization starts
	// and after the possible early exit because of the "--help" flag.
	if ( _flags & SHOW_SPLASH ) {
		QPixmap pmSplash;

		if ( splashImagePath().isEmpty() ) {
			if ( _app->devicePixelRatio() >= 1.5 ) {
				pmSplash = QPixmap(":/sc/assets/splash-default@x2.png");
				pmSplash.setDevicePixelRatio(2);
			}
			else {
				pmSplash = QPixmap(":/sc/assets/splash-default.png");
			}
		}
		else {
			pmSplash = QPixmap(splashImagePath());
		}

		QRect bbox;
		QPainter p(&pmSplash);

		QFont f;
		f.setFamilies({ "Noto Sans", "sans" });

		f.setPointSize(14);
		f.setBold(true);
		p.setFont(f);
		p.setPen(QColor(25, 25, 25));
		p.drawText(35, 103 + f.pointSize(), Seiscomp::Core::CurrentVersion.version().toString().data());

		f.setBold(false);
		p.setFont(f);
		p.drawText(35, 125 + f.pointSize(), Core::CurrentVersion.release().data());
		p.drawText(35, 191 + f.pointSize(), name().data());

		f.setPointSize(28);
		f.setBold(true);
		p.setFont(f);
		p.setPen(QColor(255, 255, 255));
		p.drawText(
			0, 203, 695, p.window().height() - 203,
			Qt::AlignRight | Qt::AlignTop,
			QString(" / %1").arg(Core::CurrentVersion.version().majorTag()),
			&bbox
		);

		f.setBold(false);
		p.setFont(f);
		p.setPen(QColor(128, 194, 209));
		p.drawText(0, 203, 695 - bbox.width(), p.window().height() - 203, Qt::AlignRight | Qt::AlignTop, Core::CurrentVersion.release().data());

		// Reset LC_ALL locale to "C" since it is overwritten during
		// first usage of QPixmap
		setlocale(LC_ALL, "C");

		_splash = new SplashScreen(pmSplash);
		f.setPointSize(10);
		f.setBold(false);
		_splash->setFont(f);

		_splash->setAttribute(Qt::WA_DeleteOnClose);
		connect(_splash, SIGNAL(destroyed(QObject*)),
		        this, SLOT(objectDestroyed(QObject*)));

		if ( _mainWidget ) {
			_splash->finish(_mainWidget);
		}

		_splash->show();
		_splash->showMessage(QString());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initSubscriptions() {
	if ( _type == Tty )
		return Client::Application::initSubscriptions();
	else
		return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::schemaValidationNames(std::vector<std::string> &modules,
                                        std::vector<std::string> &plugins) const {
	Client::Application::schemaValidationNames(modules, plugins);
	plugins.push_back("GUI");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::init() {
	if ( !Client::Application::init() ) {
		if ( _type == GuiClient ) {
			QMessageBox::critical(nullptr, tr("Initialization error"),
			                      tr("There were errors during initialization, "
			                         "please check the logs"));
		}
		return false;
	}

	if ( !_settings.styleSheet.empty() ) {
		_app->setStyleSheet(("file:///" + _settings.styleSheet).data());
	}

	// Check author read-only
	try {
		vector<string> blacklistedAuthors = configGetStrings("blacklist.authors");
		if ( find(blacklistedAuthors.begin(), blacklistedAuthors.end(), author()) != blacklistedAuthors.end() )
			_readOnlyMessaging = true;
	}
	catch ( ... ) {}

	try {
		vector<string> blacklistedUsers = configGetStrings("blacklist.users");
		SEISCOMP_DEBUG("Check if user %s is blacklisted", System::HostInfo().login().c_str());
		if ( find(blacklistedUsers.begin(), blacklistedUsers.end(), System::HostInfo().login()) != blacklistedUsers.end() ) {
			SEISCOMP_DEBUG("User %s is blacklisted, setup read-only connection", System::HostInfo().login().c_str());
			_readOnlyMessaging = true;
		}
	}
	catch ( ... ) {}

	_messageGroups.pick = "PICK";
	_messageGroups.amplitude = "AMPLITUDE";
	_messageGroups.magnitude = "MAGNITUDE";
	_messageGroups.location = "LOCATION";
	_messageGroups.focalMechanism = "FOCMECH";
	_messageGroups.event = "EVENT";

	try { _messageGroups.pick = configGetString("groups.pick"); }
	catch ( ... ) {}
	try { _messageGroups.amplitude = configGetString("groups.amplitude"); }
	catch ( ... ) {}
	try { _messageGroups.magnitude = configGetString("groups.magnitude"); }
	catch ( ... ) {}
	try { _messageGroups.location = configGetString("groups.location"); }
	catch ( ... ) {}
	try { _messageGroups.focalMechanism = configGetString("groups.focalMechanism"); }
	catch ( ... ) {}
	try { _messageGroups.event = configGetString("groups.event"); }
	catch ( ... ) {}

	if ( Client::Application::_settings.soh.interval > 0 ) {
		_timerSOH.setInterval(Client::Application::_settings.soh.interval * 1000);
	}

	if ( _type == GuiClient ) {
		_app->setWindowIcon(icon("seiscomp-logo"));
	}

	if ( isMessagingEnabled() && (_type != Tty) ) {
		if ( !cdlg()->hasConnectionChanged() ) {
			const set<string>& subscriptions = subscribedGroups();
			QStringList groups;
			for ( set<string>::const_iterator it = subscriptions.begin();
			      it != subscriptions.end(); ++it )
				groups << (*it).c_str();

			cdlg()->setClientParameters(Client::Application::_settings.messaging.URL.c_str(),
			                            Client::Application::_settings.messaging.user.c_str(),
			                            Client::Application::_settings.messaging.primaryGroup.c_str(),
			                            groups, Client::Application::_settings.messaging.timeout,
			                            Client::Application::_settings.messaging.certificate.c_str());
		}
	}

	if ( isDatabaseEnabled() && (_type != Tty) ) {
		cdlg()->setDefaultDatabaseParameters(databaseURI().c_str());

		if ( !cdlg()->hasDatabaseChanged() ) {
			cdlg()->setDatabaseParameters(databaseURI().c_str());
		}

		cdlg()->connectToDatabase();
	}

	if ( !_settingsOpened && isMessagingEnabled() && (_type != Tty) ) {
		cdlg()->connectToMessaging();
	}

	/*
	if ( _flags & OPEN_CONNECTION_DIALOG ) {
		bool ok = true;

		if ( (_flags & WANT_MESSAGING) && !messaging )
			ok = false;

		if ( (_flags & WANT_DATABASE) && !database )
			ok = false;

		if ( !ok )
			showSettings();

		result = true;
	}
	*/

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString Application::splashImagePath() const {
	return {};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConnectionDialog *Application::cdlg() {
	createSettingsDialog();
	return _dlgConnection;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createSettingsDialog() {
	if ( _dlgConnection ) {
		return;
	}

	if ( _type == Tty ) {
		return;
	}

	_dlgConnection = new ConnectionDialog(&_connection, &_database);
	_dlgConnection->setMessagingEnabled(isMessagingEnabled());

	connect(_dlgConnection, &ConnectionDialog::aboutToConnect,
	        this, &Application::createConnection);

	connect(_dlgConnection, &ConnectionDialog::aboutToDisconnect,
	        this, &Application::destroyConnection);

	connect(_dlgConnection, &ConnectionDialog::databaseChanged,
	        this, &Application::databaseChanged);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handleInitializationError(int stage) {
	if ( (_type == Tty) || (stage != MESSAGING && stage != DATABASE) ) {
		if ( stage == PLUGINS ) {
			std::cerr << "Failed to load plugins: check the log for more details" << std::endl;
			this->exit(1);
		}
		else if ( stage == LOGGING ) {
			std::cerr << "Failed to initialize logging" << std::endl;
			this->exit(1);
		}

		return false;
	}

	if ( (_flags & OPEN_CONNECTION_DIALOG) && !_settingsOpened ) {
		const set<string>& subscriptions = subscribedGroups();
		QStringList groups;
		for ( set<string>::const_iterator it = subscriptions.begin();
		      it != subscriptions.end(); ++it )
			groups << (*it).c_str();

		cdlg()->setClientParameters(Client::Application::_settings.messaging.URL.c_str(),
		                            Client::Application::_settings.messaging.user.c_str(),
		                            Client::Application::_settings.messaging.primaryGroup.c_str(),
		                            groups, Client::Application::_settings.messaging.timeout,
		                            Client::Application::_settings.messaging.certificate.c_str());

		cdlg()->setDatabaseParameters(databaseURI().c_str());

		cdlg()->connectToMessaging();
		cdlg()->connectToDatabase();

		_settingsOpened = true;

		if ( isMessagingEnabled() || isDatabaseEnabled() ) {
			if ( _thread ) _thread->setReconnectOnErrorEnabled(false);

			int res = cdlg()->exec();
			if ( res != QDialog::Accepted ) {
				Client::Application::quit();
				return false;
			}

			if ( _thread ) _thread->setReconnectOnErrorEnabled(true);
		}

		if ( cdlg()->hasDatabaseChanged() )
			emit changedDatabase();

		setDatabase(database());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleInterrupt(int signal) throw() {
	ssize_t bytesWritten = ::write(_signalSocketFd[0], &signal, sizeof(signal));
	if ( bytesWritten != sizeof(signal) )
		qWarning() << "Failed to write int to pipe";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::run() {
	if ( _connection && _connection->isConnected() ) {
		startMessageThread();
	}

	connect(_app, SIGNAL(lastWindowClosed()), this, SLOT(closedLastWindow()));
	connect(&_timerSOH, SIGNAL(timeout()), this, SLOT(timerSOH()));
	_sohLastUpdate = Core::Time::Now();
	_timerSOH.start();

	Client::Application::exit(QApplication::exec());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {
	if ( _thread ) destroyConnection();
	if ( _mainWidget ) {
		QWidget *mainWidget = _mainWidget;
		_mainWidget = nullptr;
		delete mainWidget;
	}

	Client::Application::done();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showMessage(const char* msg) {
	if ( _splash ) {
		_splash->showMessage(msg);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showWarning(const char* msg) {
	if ( _type != Tty ) {
		QMessageBox::warning(nullptr, "Warning", msg);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::notify(QObject *receiver, QEvent *e) {
	try {
		return _app->notify(receiver, e);
	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("An exception occurred while calling an event handler: %s", e.what());
		::exit(-1);
	}
	catch ( ... ) {
		SEISCOMP_ERROR("An unknown exception occurred while calling an event handler");
		::exit(-1);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::sendMessage(Seiscomp::Core::Message* msg) {
	return sendMessage(nullptr, msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::sendMessage(const char* group, Seiscomp::Core::Message* msg) {
	bool result = false;

	if ( _readOnlyMessaging ) {
		QMessageBox::critical(dynamic_cast<QApplication*>(_app)->activeWindow(),
		                      tr("Read-only connection"),
		                      tr("This is a read-only session. No message has been sent."));
		return false;
	}

	if ( connection() )
		result =
			group
			?
			connection()->send(group, msg)
			:
			connection()->send(msg)
		;


	if ( result ) return true;

	QMessageBox msgBox(dynamic_cast<QApplication*>(_app)->activeWindow());
	QPushButton *settingsButton = msgBox.addButton(tr("Setup connection"), QMessageBox::ActionRole);
	QPushButton *retryButton = msgBox.addButton(tr("Retry"), QMessageBox::ActionRole);
	QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);

	msgBox.setWindowTitle("Error");
	msgBox.setText("Sending the message failed!\nAre you connected?");
	msgBox.setIcon(QMessageBox::Critical);

	while ( !result ) {
		msgBox.exec();

		if ( msgBox.clickedButton() == retryButton ) {
			if ( connection() )
				result = (
					group
					?
					connection()->send(group, msg)
					:
					connection()->send(msg)
				) == Client::OK;
		}
		else if ( msgBox.clickedButton() == settingsButton ) {
			showSettings();
		}
		else if (msgBox.clickedButton() == abortButton) {
			break;
		}
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showSettings() {
	if ( !isMessagingEnabled() && !isDatabaseEnabled() ) return;

	if ( _thread ) _thread->setReconnectOnErrorEnabled(false);

	cdlg()->exec();

	if ( cdlg()->hasDatabaseChanged() ) {
		/*
		if ( query() )
			query()->setDriver(_database.get());
		*/
		emit changedDatabase();
	}

	if ( _thread ) {
		_thread->setReconnectOnErrorEnabled(true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::quit() {
	if ( _app ) {
		_app->quit();
	}
	else {
		Client::Application::quit();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createConnection(QString host, QString user,
                                   QString group, int timeout,
                                   QString peerCertificate) {
	Client::Result status = Client::OK;

	SEISCOMP_DEBUG("createConnection(%s, %s, %s, %d)",
	               host.toUtf8().constData(),
	               user.toUtf8().constData(),
	               group.toUtf8().constData(),
	               timeout);

	_connection = new Client::Connection;
	status = _connection->setSource(host.toStdString());
	if ( status == Client::OK ) {
		_connection->setCertificate(Client::Application::_settings.messaging.certificate);
		_connection->setMembershipInfo(Client::Application::_settings.messaging.membershipMessages);
		if ( !peerCertificate.isEmpty() ) {
			_connection->setCertificate(peerCertificate.toStdString());
		}

		status = _connection->connect(user.toStdString(), group.toStdString(),
		                              timeout);
	}

	if ( status != Client::OK ) {
		QMessageBox::warning(nullptr, "ConnectionError",
		                     QString("Could not establish connection for:\n"
		                     "  Host: %1\n"
		                     "  User: %2\n"
		                     "  Group: %3\n"
		                     "  Timeout: %4\n"
		                     "\n"
		                     "  ERROR: (%5) %6")
		                     .arg(host).arg(user).arg(group)
		                     .arg(timeout).arg(status.toInt())
		                     .arg(_connection->lastErrorMessage().c_str()));
	}
	else {
		if ( Client::Application::_settings.messaging.contentType == "binary" ) {
			_connection->setContentType(Client::Protocol::Binary);
		}
		else if ( Client::Application::_settings.messaging.contentType == "json" ) {
			_connection->setContentType(Client::Protocol::JSON);
		}
		else if ( Client::Application::_settings.messaging.contentType == "xml" ) {
			_connection->setContentType(Client::Protocol::XML);
		}
		else if ( !Client::Application::_settings.messaging.contentType.empty() ) {
			SEISCOMP_ERROR("Invalid message content type: %s",
			               Client::Application::_settings.messaging.contentType.c_str());
		}
	}

	Client::Application::_settings.messaging.user = user.toStdString();
	Client::Application::_settings.messaging.URL = host.toStdString();
	Client::Application::_settings.messaging.primaryGroup = group.toStdString();
	Client::Application::_settings.messaging.timeout = timeout;
	Client::Application::_settings.messaging.certificate = peerCertificate.toStdString();

	startMessageThread();
	if ( _thread ) {
		_thread->setReconnectOnErrorEnabled(false);
	}

	emit changedConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::destroyConnection() {
	if ( _thread ) {
		_thread->setReconnectOnErrorEnabled(false);
	}

	if ( _connection ) {
		_connection->disconnect();
	}

	closeMessageThread();

	ConnectionDialog *dlg = cdlg();
	if ( dlg ) {
		dlg->setDefaultDatabaseParameters("","");
	}

	_connection = nullptr;
	emit changedConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::databaseChanged() {
	if ( query() ) {
		query()->setDriver(_database.get());
		Client::Application::_settings.database.URI = cdlg()->databaseURI();
		if ( query()->hasError() ) {
			if ( _database ) {
				_database->disconnect();
			}
			QMessageBox::critical(nullptr, "Database Error",
			                      query()->errorMsg().c_str());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::startMessageThread() {
	if ( _thread ) {
		if ( _thread->connection() != _connection )
			closeMessageThread();
		else
			return;
	}

	if ( _connection ) {
		_thread = new MessageThread(_connection.get());
		//connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater()));
		connect(_thread, SIGNAL(messagesAvailable()), this, SLOT(messagesAvailable()));
		connect(_thread, SIGNAL(connectionError(int)), this, SLOT(connectionError(int)));
		connect(_thread, SIGNAL(connectionEstablished()), this, SLOT(onConnectionEstablished()));
		connect(_thread, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
		connect(_thread, SIGNAL(connectionEstablished()), this, SIGNAL(connectionEstablished()));
		connect(_thread, SIGNAL(connectionLost()), this, SIGNAL(connectionLost()));
		_thread->setReconnectOnErrorEnabled(true);
		_thread->start();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::closeMessageThread() {
	if ( _thread ) {
		_thread->setReconnectOnErrorEnabled(false);
		_thread->wait();
		delete _thread;
		_thread = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::messagesAvailable() {
	if ( !_connection ) return;

	Seiscomp::Core::MessagePtr msg;
	Client::PacketPtr pkt;
	while ( _connection->inboxSize() > 0 ) {
		msg = _connection->recv(pkt);
		if ( !msg ) {
			// end of traffic?
			if ( !pkt )
				break;

			emit messageSkipped(pkt.get());
			continue;
		}

		if ( isDatabaseEnabled() ) {
			Client::DatabaseProvideMessage *dbmsg = Client::DatabaseProvideMessage::Cast(msg);
			if ( dbmsg && cdlg() ) {
				cdlg()->setDefaultDatabaseParameters(dbmsg->service(), dbmsg->parameters());
				if ( database() == nullptr ) {
					cdlg()->setDatabaseParameters(dbmsg->service(), dbmsg->parameters());
					cdlg()->connectToDatabase();
					if ( cdlg()->hasDatabaseChanged() ) {
						Client::Application::_settings.database.URI = cdlg()->databaseURI();
						setDatabase(database());
					}
				}
			}
		}

		CommandMessage *cmd = CommandMessage::Cast(msg);
		if ( cmd && _filterCommands ) {
			QRegularExpression re(cmd->client().c_str());
			if ( re.match(Client::Application::_settings.messaging.user.c_str()).hasMatch() ) {
				if ( cmd->command() == CM_SHOW_NOTIFICATION ) {
					if ( !cmd->parameter().empty() ) {
						NotificationLevel nl = NL_UNDEFINED;
						QString message;

						size_t p = cmd->parameter().find(' ');
						if ( p != string::npos ) {
							if ( cmd->parameter()[0] == '[' &&
							     cmd->parameter()[p-1] == ']' ) {
								nl.fromString(cmd->parameter().substr(1, p-2));
								message = cmd->parameter().substr(p+1).c_str();
							}
						}
						else
							message = cmd->parameter().c_str();

						emit showNotification(nl, message);
					}
				}

				emit messageAvailable(cmd, pkt.get());
			}
			else {
				SEISCOMP_DEBUG("Ignoring command message for client: %s, user is: %s",
				               cmd->client().c_str(),
				               Client::Application::_settings.messaging.user.c_str());
			}

			continue;
		}

		emit messageAvailable(msg.get(), pkt.get());

		NotifierMessage* nm = NotifierMessage::Cast(msg);

		if ( isAutoApplyNotifierEnabled() ) {
			if ( !nm ) {
				for ( Core::MessageIterator it = msg->iter(); *it; ++it ) {
					DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
					if ( n ) {
						// SEISCOMP_DEBUG("Non persistent notifier for '%s'", n->parentID().c_str());
						n->apply();
					}
				}
			}
			else {
				for ( NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it ) {
					// SEISCOMP_DEBUG("Notifier for '%s'", (*it)->parentID().c_str());
					(*it)->apply();
				}
			}
		}

		if ( !nm ) {
			for ( Core::MessageIterator it = msg->iter(); *it; ++it ) {
				DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
				if ( n ) {
					emitNotifier(n);
				}
			}
		}
		else {
			for ( NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it )
				emitNotifier(it->get());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::emitNotifier(Notifier* n) {
	emit notifierAvailable(n);
	if ( isInterpretNotifierEnabled() ) {
		switch ( n->operation() ) {
			case OP_ADD:
				emit addObject(n->parentID().c_str(), n->object());
				break;
			case OP_REMOVE:
				emit removeObject(n->parentID().c_str(), n->object());
				break;
			case OP_UPDATE:
				emit updateObject(n->parentID().c_str(), n->object());
				break;
			default:
				break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::onConnectionEstablished() {
	if ( _type != Tty)
		cdlg()->connectToMessaging();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::onConnectionLost() {
	if ( _type != Tty)
		cdlg()->onConnectionError(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::connectionError(int code) {
	if ( _type == Tty) return;

	if ( _connection && !_connection->isConnected() ) {
		SEISCOMP_ERROR("Connection went away...");
		closeMessageThread();
		cdlg()->onConnectionError(code);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::objectDestroyed(QObject* o) {
	if ( o == _splash )
		_splash = nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::closedLastWindow() {
	if ( _app ) {
		_app->quit();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exit(int returnCode) {
	if ( _thread )
		_thread->setReconnectOnErrorEnabled(false);

	if ( _app ) {
		_app->exit(returnCode);
	}

	Client::Application::exit(returnCode);

	closeMessageThread();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setFilterCommandsEnabled(bool e) {
	_filterCommands = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::commandTarget() const {
	return _settings.commandTargetClient;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::sendCommand(Command command, const std::string &parameter) {
	sendCommand(command, parameter, nullptr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::sendCommand(Command command, const std::string &parameter, Core::BaseObject *obj) {
	if ( commandTarget().empty() ) {
		QMessageBox::critical(nullptr,
		            "Commands",
		            "Variable <commands.target> is not set. To disable sending commands "
		            "to all connected clients, set a proper target. You can use "
		            "regular expressions to specify a group of clients (HINT: all = \".*$\").");
		return;
	}

	CommandMessagePtr cmsg = new CommandMessage(commandTarget(), command);
	cmsg->setParameter(parameter);
	cmsg->setObject(obj);

	sendMessage(_settings.guiGroup.c_str(), cmsg.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QFont Application::font() const {
	return _app->font();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setFont(const QFont &font) {
	_app->setFont(font);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPalette Application::palette() const {
	return _app->palette();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setPalette(const QPalette &pal) {
	_app->setPalette(pal);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ProcessManager *Application::processManager() {
	if ( !_processManager ) {
		_processManager = new ProcessManager(_mainWidget);
		emit processManagerCreated();
	}

	return _processManager;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
