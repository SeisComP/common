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



#ifndef SEISCOMP_GUI_MAINWINDOW_H
#define SEISCOMP_GUI_MAINWINDOW_H

#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#endif
#include <seiscomp/gui/core/connectionstatelabel.h>

#include <QMainWindow>

namespace Seiscomp {

namespace Core {

DEFINE_SMARTPOINTER(Message);

}

namespace Client {

DEFINE_SMARTPOINTER(Connection);
DEFINE_SMARTPOINTER(Packet);

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


class SC_GUI_API MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow(QWidget * parent = 0, Qt::WindowFlags = 0);
		~MainWindow();


	public:
		bool restoreGeometry(const QByteArray & geometry);


	protected:
		void paintEvent(QPaintEvent *e);
		void showEvent(QShowEvent*);
		void dropEvent(QDropEvent *);
		void dragEnterEvent(QDragEnterEvent *);

		virtual void toggledFullScreen(bool);

	signals:
		void fullScreenToggled(bool);

	public slots:
		void showNormal();
		void showFullScreen();

		void setTitle(QString);
		QString title() const;


	private slots:
		void onChangedConnection();
		void toggleFullScreen();

		void connectionEstablished();
		void connectionLost();

		void inspectConfig();
		void inspectInventory();


	protected:
		QAction* _actionToggleFullScreen;
		QAction* _actionShowSettings;


	private:
		QMenuBar *_menuBar;
		QWidget *_menuWidget;
		ConnectionStateLabel *_connectionState;
		QString _title;
		bool _showFullscreen;
};


}
}


#endif

