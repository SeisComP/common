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



#define SEISCOMP_COMPONENT Gui::MainWindow

#include "mainwindow.h"

#include <seiscomp/core/platform/platform.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/gui/core/inspector.h>
#include <seiscomp/gui/core/logmanager.h>
#include <seiscomp/gui/core/processmanager.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/io/database.h>
#include <seiscomp/math/filter.h>
#include <seiscomp/messaging/connection.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/client/configdb.h>
#include <seiscomp/client/inventory.h>

#ifdef MACOSX
#include <seiscomp/gui/core/osx.h>
#endif

#include <QImage>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QStatusBar>
#include <QtSvg/QSvgRenderer>

using namespace Seiscomp;
using namespace Seiscomp::Communication;
using namespace Seiscomp::DataModel;


namespace Seiscomp::Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainWindow::MainWindow(QWidget * parent, Qt::WindowFlags flags)
 : QMainWindow(parent, flags)
{
	_title = SCApp->name().c_str();

	setWindowTitle(_title);

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif

	_actionToggleFullScreen = new QAction(this);
	_actionToggleFullScreen->setObjectName(QString::fromUtf8("toggleFS"));
	_actionToggleFullScreen->setShortcut(QApplication::translate("MainWindow", "F11", nullptr));
	_actionToggleFullScreen->setText(QApplication::translate("MainWindow", "Toggle fullscreen", nullptr));

	_actionShowSettings = new QAction(this);
	_actionShowSettings->setObjectName(QString::fromUtf8("showSettings"));
	_actionShowSettings->setShortcut(QApplication::translate("MainWindow", "F2", nullptr));
	_actionShowSettings->setText(QApplication::translate("MainWindow", "Configure &connection...", nullptr));
	_actionShowSettings->setIcon(icon("wifi"));
	_actionShowSettings->setEnabled(SCApp->isMessagingEnabled() || SCApp->isDatabaseEnabled());

	connect(_actionToggleFullScreen, SIGNAL(triggered(bool)), this, SLOT(toggleFullScreen()));
	connect(_actionShowSettings, SIGNAL(triggered(bool)), SCApp, SLOT(showSettings()));

	addAction(_actionToggleFullScreen);
	addAction(_actionShowSettings);

	auto *inspectConfig = new QAction(this);
	inspectConfig->setObjectName(QString::fromUtf8("inspectConfig"));
	inspectConfig->setShortcut(QApplication::translate("MainWindow", "Alt+Ctrl+C", nullptr));
	inspectConfig->setText(QApplication::translate("MainWindow", "Inspect &configmodule...", nullptr));

	auto *inspectInventory = new QAction(this);
	inspectInventory->setObjectName(QString::fromUtf8("inspectInventory"));
	inspectInventory->setShortcut(QApplication::translate("MainWindow", "Alt+Ctrl+I", nullptr));
	inspectInventory->setText(QApplication::translate("MainWindow", "Inspect &inventory...", nullptr));

	addAction(inspectConfig);
	addAction(inspectInventory);

	connect(inspectConfig, &QAction::triggered,
	        this, &MainWindow::inspectConfig);

	connect(inspectInventory, &QAction::triggered,
	        this, &MainWindow::inspectInventory);

	connect(SCApp, &Application::connectionEstablished,
	        this, &MainWindow::connectionEstablished);

	connect(SCApp, &Application::connectionLost,
	        this, &MainWindow::connectionLost);

	connect(SCApp, &Application::showNotification,
	        this, &MainWindow::showNotification);

	setAcceptDrops(true);

	// create process state toolbar widget once process manager is created
	connect(SCApp, &Gui::Application::processManagerCreated, [this]() {
		if ( !_processState && statusBar() ) {
			_processState = new ProcessStateLabel(SCApp->processManager(), this);
			statusBar()->addPermanentWidget(_processState);
		}
	});
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainWindow::~MainWindow() {
	if ( SCApp ) {
		SCApp->settings().beginGroup(objectName());
		SCApp->settings().setValue("geometry", saveGeometry());
		SCApp->settings().setValue("state", saveState());
		SCApp->settings().endGroup();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
	if ( event->mimeData()->hasFormat("text/plain") ) {
		event->acceptProposedAction();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::dropEvent(QDropEvent *event) {
	if ( event->mimeData()->hasFormat("text/plain") ) {
		auto *po = PublicObject::Find(event->mimeData()->data("text/plain").toStdString());
		if ( po ) {
			auto *w = new Inspector(this, Qt::Tool);
			w->setAttribute(Qt::WA_DeleteOnClose);
			w->setObject(po);
			w->show();
			event->acceptProposedAction();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MainWindow::restoreGeometry(const QByteArray & geometry) {
	bool res = QMainWindow::restoreGeometry(geometry);
#ifdef MACOSX
	Mac::addFullscreen(this);
#endif
	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::paintEvent(QPaintEvent *e) {
	QMainWindow::paintEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showEvent(QShowEvent *e) {
	QMainWindow::showEvent(e);

	if ( !_logState && SCApp->logManager() ) {
		_logState = new LogStateLabel(SCApp->logManager(), statusBar());
		statusBar()->addPermanentWidget(_logState);
	}

	if ( _connectionState ) {
		return;
	}

	if ( SCApp ) {
		SCApp->settings().beginGroup(objectName());
		restoreState(SCApp->settings().value("state").toByteArray());
		restoreGeometry(SCApp->settings().value("geometry").toByteArray());
		SCApp->settings().endGroup();
	}

	if ( _showFullscreen != isFullScreen() ) {
		toggleFullScreen();
	}

	if ( !statusBar() ) {
		return;
	}

	_connectionState = new ConnectionStateLabel(statusBar());
	connect(_connectionState, &ConnectionStateLabel::customInfoWidgetRequested,
	        [](const QPoint &pos) { SCApp->showSettings(); } );

	statusBar()->addPermanentWidget(_connectionState);

	onChangedConnection();

	connect(SCApp, SIGNAL(changedConnection()),
	        this, SLOT(onChangedConnection()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::closeEvent(QCloseEvent *e) {
	if ( !statusBar() || _processState ) {
		auto *pm = SCApp->processManager();
		int runningCount = pm->runningCount();
		if ( runningCount > 0 ) {
			pm->showNormal();
			pm->activateWindow();

			QMessageBox mb;
			mb.setWindowTitle("Close aborted");
			if ( runningCount == 1 ) {
				mb.setText(QString("Found running process.").arg(runningCount));
				mb.setInformativeText("Do you want to terminate it?");
			}
			else {
				mb.setText(QString("Found %1 running processes.").arg(runningCount));
				mb.setInformativeText("Do you want to terminate them?");
			}
			mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			int ret = mb.exec();
			if ( ret == QMessageBox::Yes ) {
				auto procs = pm->processes();
				for ( auto *p : std::as_const(procs) ) {
					pm->terminate(p);
				}
			}

			e->ignore();
			return;
		}

		pm->close();
	}

	auto *lm = SCApp->logManager();
	if ( lm ) {
		lm->close();
	}

	QMainWindow::closeEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showNormal() {
	_showFullscreen = false;
	QMainWindow::showNormal();
	toggledFullScreen(false);
	emit fullScreenToggled(false);

	if ( menuBar() && !SCScheme.showMenu ) {
		menuBar()->setVisible(false);
	}

	if ( statusBar() && !SCScheme.showStatusBar ) {
		statusBar()->setVisible(false);
	}

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showFullScreen() {
	_showFullscreen = true;
	QMainWindow::showFullScreen();
	toggledFullScreen(true);
	emit fullScreenToggled(true);

	if ( menuBar() && !SCScheme.showMenu ) {
		menuBar()->setVisible(false);
	}

	if ( statusBar() && !SCScheme.showStatusBar ) {
		statusBar()->setVisible(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::toggleFullScreen() {
	if ( isFullScreen() ) {
		showNormal();
	}
	else {
		showFullScreen();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setTitle(QString t) {
	_title = std::move(t);

	if ( SCApp->connection() ) {
		QString title = _title + "@" + SCApp->messagingURL().c_str();
		if ( SCApp->isReadOnlyMessaging() ) {
			title += " (read-only)";
		}
		setWindowTitle(title);
	}
	else {
		setWindowTitle(_title);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString MainWindow::title() const {
	return _title;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::connectionEstablished() {
	onChangedConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::connectionLost() {
	onChangedConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::onChangedConnection() {
	if ( SCApp->connection() && SCApp->connection()->isConnected() ) {
		_connectionState->start(SCApp->messagingURL().c_str());
	}
	else {
		_connectionState->stop();
	}

	if ( SCApp->connection() ) {
		QString title = _title + "@" + SCApp->messagingURL().c_str();
		if ( SCApp->isReadOnlyMessaging() ) {
			title += " (read-only)";
		}
		setWindowTitle(title);
	}
	else {
		setWindowTitle(_title);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::toggledFullScreen(bool /*isFullScreen*/) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::inspectConfig() {
	if ( !SCApp->configModule() ) {
		return;
	}

	auto *w = new Inspector(this, Qt::Tool);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setObject(Client::ConfigDB::Instance()->config());
	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::inspectInventory() {
	if ( !Client::Inventory::Instance()->inventory() ) {
		return;
	}

	auto *w = new Inspector(this, Qt::Tool);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setObject(Client::Inventory::Instance()->inventory());
	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showNotification(const NotificationLevel &level,
                                  const QString &message) {
	switch ( level ) {
		default:
		case NL_INFO:
			SEISCOMP_INFO("%s", message.toStdString().c_str());
			break;
		case NL_WARNING:
			SEISCOMP_WARNING("%s", message.toStdString().c_str());
			break;
		case NL_CRITICAL:
		case NL_FAILURE:
			SEISCOMP_ERROR("%s", message.toStdString().c_str());
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Seiscomp::Gui
