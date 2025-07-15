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
#include <QStatusBar>
#include <QtSvg/QSvgRenderer>

using namespace Seiscomp;
using namespace Seiscomp::Communication;
using namespace Seiscomp::DataModel;


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainWindow::MainWindow(QWidget * parent, Qt::WindowFlags flags)
 : QMainWindow(parent, flags)
{
	_connectionState = nullptr;
	_title = SCApp->name().c_str();
	_showFullscreen = false;

	setWindowTitle(_title);

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif

	_actionToggleFullScreen = new QAction(this);
	_actionToggleFullScreen->setObjectName(QString::fromUtf8("toggleFS"));
	_actionToggleFullScreen->setShortcut(QApplication::translate("MainWindow", "F11", 0));
	_actionToggleFullScreen->setText(QApplication::translate("MainWindow", "Toggle fullscreen", 0));

	_actionShowSettings = new QAction(this);
	_actionShowSettings->setObjectName(QString::fromUtf8("showSettings"));
	_actionShowSettings->setShortcut(QApplication::translate("MainWindow", "F2", 0));
	_actionShowSettings->setText(QApplication::translate("MainWindow", "Configure &connection...", 0));
	_actionShowSettings->setIcon(icon("wifi"));
	_actionShowSettings->setEnabled(SCApp->isMessagingEnabled() || SCApp->isDatabaseEnabled());

	connect(_actionToggleFullScreen, SIGNAL(triggered(bool)), this, SLOT(toggleFullScreen()));
	connect(_actionShowSettings, SIGNAL(triggered(bool)), SCApp, SLOT(showSettings()));	

	addAction(_actionToggleFullScreen);
	addAction(_actionShowSettings);

	QAction *inspectConfig = new QAction(this);
	inspectConfig->setObjectName(QString::fromUtf8("inspectConfig"));
	inspectConfig->setShortcut(QApplication::translate("MainWindow", "Alt+Ctrl+C", 0));
	inspectConfig->setText(QApplication::translate("MainWindow", "Inspect &configmodule...", 0));

	QAction *inspectInventory = new QAction(this);
	inspectInventory->setObjectName(QString::fromUtf8("inspectInventory"));
	inspectInventory->setShortcut(QApplication::translate("MainWindow", "Alt+Ctrl+I", 0));
	inspectInventory->setText(QApplication::translate("MainWindow", "Inspect &inventory...", 0));

	addAction(inspectConfig);
	addAction(inspectInventory);

	connect(inspectConfig, SIGNAL(triggered(bool)),
	        this, SLOT(inspectConfig()));

	connect(inspectInventory, SIGNAL(triggered(bool)),
	        this, SLOT(inspectInventory()));

	connect(SCApp, SIGNAL(connectionEstablished()),
	        this, SLOT(connectionEstablished()));

	connect(SCApp, SIGNAL(connectionLost()),
	        this, SLOT(connectionLost()));

	connect(SCApp, SIGNAL(showNotification(NotificationLevel, QString)),
	        this, SLOT(showNotification(NotificationLevel, QString)));

	setAcceptDrops(true);

	QSvgRenderer svg(QString(":/images/images/seiscomp-logo.svg"));
	long dim = Math::Filtering::next_power_of_2(fontMetrics().height() * 2);
	if ( dim < 64 ) dim = 64;
	QImage img(dim, dim, QImage::Format_ARGB32);
	QPainter paint(&img);
	svg.render(&paint);
	setWindowIcon(QIcon(QPixmap::fromImage(img)));
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
		PublicObject *po = PublicObject::Find(event->mimeData()->data("text/plain").toStdString());
		if ( po ) {
			Inspector *w = new Inspector(this, Qt::Tool);
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

	if ( _connectionState ) return;

	if ( SCApp ) {
		SCApp->settings().beginGroup(objectName());
		restoreState(SCApp->settings().value("state").toByteArray());
		restoreGeometry(SCApp->settings().value("geometry").toByteArray());
		SCApp->settings().endGroup();
	}

	if ( _showFullscreen != isFullScreen() )
		toggleFullScreen();

	if ( !statusBar() ) return;

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
void MainWindow::showNormal() {
	_showFullscreen = false;
	QMainWindow::showNormal();
	toggledFullScreen(false);
	fullScreenToggled(false);

	if ( menuBar() && !SCScheme.showMenu )
		menuBar()->setVisible(false);

	if ( statusBar() && !SCScheme.showStatusBar )
		statusBar()->setVisible(false);

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
	fullScreenToggled(true);

	if ( menuBar() && !SCScheme.showMenu )
		menuBar()->setVisible(false);

	if ( statusBar() && !SCScheme.showStatusBar )
		statusBar()->setVisible(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::toggleFullScreen() {
	if ( isFullScreen() )
		showNormal();
	else
		showFullScreen();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setTitle(QString t) {
	_title = t;

	if ( SCApp->connection() ) {
		QString title = _title + "@" + SCApp->messagingURL().c_str();
		if ( SCApp->isReadOnlyMessaging() )
			title += " (read-only)";
		setWindowTitle(title);
	}
	else
		setWindowTitle(_title);
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
void MainWindow::toggledFullScreen(bool) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::inspectConfig() {
	if ( !SCApp->configModule() ) return;

	Inspector *w = new Inspector(this, Qt::Tool);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setObject(Client::ConfigDB::Instance()->config());
	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::inspectInventory() {
	if ( !Client::Inventory::Instance()->inventory() ) return;

	Inspector *w = new Inspector(this, Qt::Tool);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setObject(Client::Inventory::Instance()->inventory());
	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showNotification(NotificationLevel level, QString message) {
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
}
}
