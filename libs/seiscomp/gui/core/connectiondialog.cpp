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



#define SEISCOMP_COMPONENT GUI::ConnectionDialog

#include "connectiondialog.h"
#include <seiscomp/logging/log.h>
#include <seiscomp/io/database.h>
#include <seiscomp/messaging/messages/database.h>
#include <seiscomp/utils/timer.h>

#include <QMessageBox>

using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::IO;


namespace Seiscomp {
namespace Gui {


ConnectionDialog::ConnectionDialog(ConnectionPtr* con, DatabaseInterfacePtr* db, QWidget * parent , Qt::WindowFlags f)
 : QDialog(parent, f), _connection(con), _db(db), _requestAllGroups(false) {
	_ui.setupUi(this);

	_changedConnection = false;
	_changedDatabase = false;
	_messagingEnabled = true;

	_ui.comboDbType->addItem("[unsupported]");
	DatabaseInterfaceFactory::ServiceNames* services = DatabaseInterfaceFactory::Services();
	if ( services ) {
		for ( DatabaseInterfaceFactory::ServiceNames::iterator it = services->begin();
		      it != services->end(); ++it ) {
			_ui.comboDbType->addItem((*it).c_str());
		}
		delete services;
	}

	if ( _connection == NULL ) {
		_ui.groupMessaging->setEnabled(false);
		_ui.groupSubscriptions->setEnabled(false);
	}

	if ( _db == NULL ) _ui.groupDatabase->setEnabled(false);

	connect(_ui.btnConnect, SIGNAL(clicked(bool)), this, SLOT(onConnect()));
	connect(_ui.btnDbSwitchToReported, SIGNAL(clicked(bool)), this, SLOT(onSwitchToReported()));
	connect(_ui.btnDbConnect, SIGNAL(clicked(bool)), this, SLOT(onDatabaseConnect()));
	connect(_ui.listSubscriptions, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onItemChanged(QListWidgetItem*)));
	connect(_ui.btnSelectAll, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(_ui.btnDeselectAll, SIGNAL(clicked()), this, SLOT(onDeselectAll()));
}


ConnectionDialog::~ConnectionDialog() {
}


void ConnectionDialog::setUsername(const QString& username) {
	_ui.editUser->setText(username);
}


void ConnectionDialog::setClientParameters(const QString& server,
                                           const QString& username,
                                           const QString& primaryGroup,
                                           const QStringList& groups,
                                           int timeout) {
	_requestedGroups = groups;
	_requestAllGroups = false;

	for ( QStringList::iterator it = _requestedGroups.begin();
	      it != _requestedGroups.end(); ++it ) {
		if ( (*it) == "*" ) {
			_requestAllGroups = true;
			break;
		}
	}

	setUsername(username);

	_ui.editServer->setText(server);
	_ui.editPrimaryGroup->setText(primaryGroup);
	_ui.timeoutSpinBox->setValue(timeout);
}


bool ConnectionDialog::setMessagingEnabled(bool e) {
	_messagingEnabled = e;
	_ui.groupMessaging->setEnabled(e);
	_ui.groupMessaging->setVisible(e);
	_ui.groupSubscriptions->setEnabled(e);
	_ui.groupSubscriptions->setVisible(e);
	resize(width(),1);
	updateGeometry();
	return true;
}


bool ConnectionDialog::setDatabaseParameters(const QString& uri) {
	QStringList tmp = uri.split("://");
	QString type, connection;
	type = tmp.size() > 1?tmp[0]:"mysql";
	connection = tmp.size() > 1?tmp[1]:tmp[0];

	return setDatabaseParameters(type, connection);
}


bool ConnectionDialog::setDatabaseParameters(const QString& type, const QString& connection) {
	int selIndex = _ui.comboDbType->findText(type);
	_ui.editDbConnection->setText(connection);
	if ( selIndex != -1 ) {
		_ui.comboDbType->setCurrentIndex(selIndex);
		return true;
	}
	else {
		_ui.comboDbType->setCurrentIndex(0);
		return false;
	}
}


bool ConnectionDialog::setDefaultDatabaseParameters(const QString &uri) {
	QStringList tmp = uri.split("://");
	QString type, connection;
	type = tmp.size() > 1?tmp[0]:"mysql";
	connection = tmp.size() > 1?tmp[1]:tmp[0];

	return setDefaultDatabaseParameters(type, connection);
}


bool ConnectionDialog::setDefaultDatabaseParameters(const QString &type, const QString &connection) {
	_reportedDbType = type;
	_reportedDbParameters = connection;

	if ( !type.isEmpty() || !connection.isEmpty() )
		_ui.labelDbReported->setText(type + "://" + connection);
	else
		_ui.labelDbReported->setText(QString());
	return true;
}


std::string ConnectionDialog::databaseURI() const {
	return (_ui.comboDbType->currentText() + "://" + _ui.editDbConnection->text()).toStdString();
}


void ConnectionDialog::onConnectionError(int /*code*/) {
	if ( !*_connection || !(*_connection)->isConnected() ) {
		_ui.btnConnect->setText("Connect");
		_ui.editUser->setEnabled(true);
		_ui.editServer->setEnabled(true);
		_ui.editPrimaryGroup->setEnabled(true);
		_ui.timeoutSpinBox->setEnabled(true);
		
		_ui.groupSubscriptions->setEnabled(false);
		_ui.listSubscriptions->clear();

		_ui.btnDbSwitchToReported->setEnabled(false);
	}
}


void ConnectionDialog::onConnect() {
	if ( *_connection && (*_connection)->isConnected() ) {
		_ui.btnConnect->setText("Connect");
		_ui.editUser->setEnabled(true);
		_ui.editServer->setEnabled(true);
		_ui.editPrimaryGroup->setEnabled(true);
		_ui.timeoutSpinBox->setEnabled(true);
		
		_ui.groupSubscriptions->setEnabled(false);
		_ui.listSubscriptions->clear();

		_ui.btnDbSwitchToReported->setEnabled(false);

		setDefaultDatabaseParameters("","");

		emit aboutToDisconnect();
		_changedConnection = true;

		return;
	}

	QString user = _ui.editUser->text();
	QString host = _ui.editServer->text();

	if ( host.isEmpty() ) {
		QMessageBox::information(this, tr(""),
		                         tr("Please enter a servername."),
		                         QMessageBox::Ok);
		return;
	}

	if ( !connectToMessaging() ) {
		/*
		QMessageBox::information(this, tr(""),
		                         tr("The connection has not been established."),
		                         QMessageBox::Ok);
		*/
		return;
	}
}


void ConnectionDialog::onSwitchToReported() {
	QCursor c = cursor();
	setCursor(Qt::WaitCursor);

	if ( _reportedDbType.isEmpty() || _reportedDbParameters.isEmpty() ) {
		setCursor(c);
		QMessageBox::critical(this, "Error", "Insufficent database parameters");
		return;
	}

	if ( _ui.comboDbType->currentText() == _reportedDbType &&
	     _ui.editDbConnection->text() == _reportedDbParameters ) {
		setCursor(c);
		QMessageBox::information(this, "Not modified", "Database parameters are already in use");
		return;
	}

	setDatabaseParameters(_reportedDbType, _reportedDbParameters);

	// Disconnect from database
	if ( *_db && (*_db)->isConnected() ) {
		(*_db)->disconnect();
		(*_db) = NULL;
		_ui.btnDbConnect->setText("Connect");
		_ui.comboDbType->setEnabled(true);
		_ui.editDbConnection->setEnabled(true);
	}

	if ( connectToDatabase() )
		_ui.btnDbConnect->setText("Disconnect");

	setCursor(c);
}


int ConnectionDialog::exec() {
	_changedConnection = false;
	_changedDatabase = false;
	return QDialog::exec();
}


bool ConnectionDialog::connectToMessaging() {
	if ( !_messagingEnabled || !_connection ) return false;

	SEISCOMP_DEBUG("Setting up connection state in settings dialog");

	QString user = _ui.editUser->text();
	QString host = _ui.editServer->text();
	int timeout  = _ui.timeoutSpinBox->value();

	Client::Connection *_lastConnection = _connection->get();

	if ( !*_connection || !(*_connection)->isConnected() ) {
		SEISCOMP_DEBUG("Request a connection in settings dialog");
		emit aboutToConnect(host, user, _ui.editPrimaryGroup->text(), timeout*1000);
	}
	//(*_connection) = Connection::Create(("4803@" + host).toAscii(), user.toAscii(), _ui.editPrimaryGroup->text().toAscii());

	if ( !*_connection || !(*_connection)->isConnected() )
		return false;

	_changedConnection = _lastConnection != _connection->get();

	_ui.btnDbSwitchToReported->setEnabled(true);

	_ui.groupSubscriptions->setEnabled(true);

	_ui.listSubscriptions->blockSignals(true);

	if ( _requestAllGroups )
		_requestedGroups.clear();

	for ( auto &&group : (*_connection)->protocol()->groups() ) {
		QListWidgetItem* item = new QListWidgetItem(group.c_str(), _ui.listSubscriptions);
		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		if ( _requestAllGroups ) {
			item->setCheckState(Qt::Checked);
			SEISCOMP_DEBUG("Joining group: %s in settings dialog", group.c_str());
			(*_connection)->subscribe(group);
			_requestedGroups.append(group.c_str());
		}
		else {
			if ( _requestedGroups.contains(group.c_str()) ) {
				item->setCheckState(Qt::Checked);
				SEISCOMP_DEBUG("Joining group: %s in settings dialog", group.c_str());
				(*_connection)->subscribe(group);
			}
			else
				item->setCheckState(Qt::Unchecked);
		}
	}


	_ui.listSubscriptions->blockSignals(false);

	_ui.btnConnect->setText("Disconnect");
	_ui.editUser->setEnabled(false);
	_ui.editServer->setEnabled(false);
	_ui.editPrimaryGroup->setEnabled(false);
	_ui.timeoutSpinBox->setEnabled(false);
	
	return true;
}


bool ConnectionDialog::connectToDatabase() {
	if ( !_db ) return false;

	_changedDatabase = false;

	if ( !*_db || !(*_db)->isConnected() ) {
		if ( !_ui.editDbConnection->text().isEmpty() ) {
			Client::DatabaseProvideMessage tmp(_ui.comboDbType->currentText().toStdString().c_str(),
			                                   _ui.editDbConnection->text().toStdString().c_str());
			*_db = tmp.database();
		}
		else
			*_db = NULL;

		_changedDatabase = true;
		emit databaseChanged();
	}

	if ( *_db && (*_db)->isConnected() ) {
		_ui.comboDbType->setEnabled(false);
		_ui.editDbConnection->setEnabled(false);
		_ui.btnDbConnect->setText("Disconnect");
		return true;
	}

	return false;
}


void ConnectionDialog::onDatabaseConnect() {
	if ( *_db && (*_db)->isConnected() ) {
		(*_db)->disconnect();
		(*_db) = NULL;
		_ui.btnDbConnect->setText("Connect");
		_ui.comboDbType->setEnabled(true);
		_ui.editDbConnection->setEnabled(true);
		return;
	}

	if ( !connectToDatabase() )
		QMessageBox::critical(this, "Error", "Connection failed");
}


bool ConnectionDialog::hasConnectionChanged() const {
	return _changedConnection;
}


bool ConnectionDialog::hasDatabaseChanged() const {
	return _changedDatabase;
}


void ConnectionDialog::onItemChanged(QListWidgetItem *item) {
	_requestAllGroups = false;

	if ( item->checkState() == Qt::Checked ) {
		(*_connection)->subscribe((const char*)item->text().toLatin1());
		_requestedGroups.append(item->text());
	}
	else {
		(*_connection)->unsubscribe((const char*)item->text().toLatin1());
		_requestedGroups.removeAll(item->text());
	}
}


void ConnectionDialog::onSelectAll() {
	_requestAllGroups = true;

	for ( int i = 0; i < _ui.listSubscriptions->count(); ++i ) {
		if ( _ui.listSubscriptions->item(i)->checkState() == Qt::Unchecked ) {
			_ui.listSubscriptions->item(i)->setCheckState(Qt::Checked);
		}
	}
}

void ConnectionDialog::onDeselectAll() {
	_requestAllGroups = true;

	for ( int i = 0; i < _ui.listSubscriptions->count(); ++i ) {
		if ( _ui.listSubscriptions->item(i)->checkState() == Qt::Checked ) {
			_ui.listSubscriptions->item(i)->setCheckState(Qt::Unchecked);
		}
	}
}


}
}
