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



#ifndef SEISCOMP_GUI_CONNECTIONDIALOG_H
#define SEISCOMP_GUI_CONNECTIONDIALOG_H

#include <seiscomp/gui/core/ui_connectiondialog.h>
#include <seiscomp/gui/qt.h>
#ifndef Q_MOC_RUN
#include <seiscomp/io/database.h>
#include <seiscomp/messaging/connection.h>
#endif

#include <QDialog>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API ConnectionDialog : public QDialog {
	Q_OBJECT

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		ConnectionDialog(Seiscomp::Client::ConnectionPtr*,
		                 Seiscomp::IO::DatabaseInterfacePtr*,
		                 QWidget* parent = 0, Qt::WindowFlags f = 0);
		~ConnectionDialog();


	public:
		void setUsername(const QString& username);

		void setClientParameters(const QString& server,
		                         const QString& username,
		                         const QString& primaryGroup,
		                         const QStringList& groups,
		                         int timeout);

		bool setMessagingEnabled(bool);

		bool setDatabaseParameters(const QString &uri);
		bool setDatabaseParameters(const QString &type, const QString &connection);

		bool setDefaultDatabaseParameters(const QString &uri);
		bool setDefaultDatabaseParameters(const QString &type, const QString &connection);

		std::string databaseURI() const;

		bool connectToMessaging();
		bool connectToDatabase();

		bool hasConnectionChanged() const;
		bool hasDatabaseChanged() const;


	// ------------------------------------------------------------------
	//  Public signals
	// ------------------------------------------------------------------
	signals:
		void aboutToConnect(QString host, QString user, QString group, int timeout);
		void aboutToDisconnect();

		void databaseChanged();


	// ------------------------------------------------------------------
	//  Public slots
	// ------------------------------------------------------------------
	public slots:
		void onConnectionError(int code);
		int exec();


	// ------------------------------------------------------------------
	//  Protected slots
	// ------------------------------------------------------------------
	protected slots:
		void onConnect();
		void onSwitchToReported();
		void onDatabaseConnect();
		void onItemChanged(QListWidgetItem *item);
		void onSelectAll();
		void onDeselectAll();


	// ------------------------------------------------------------------
	//  Members
	// ------------------------------------------------------------------
	private:
		::Ui::ConnectionDialog _ui;
		Seiscomp::Client::ConnectionPtr* _connection;
		Seiscomp::IO::DatabaseInterfacePtr* _db;
		QStringList _requestedGroups;

		bool _requestAllGroups;
		bool _messagingEnabled;

		bool _changedDatabase;
		bool _changedConnection;

		QString _reportedDbType;
		QString _reportedDbParameters;
};


}
}

#endif
