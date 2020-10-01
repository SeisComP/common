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



#ifndef SEISCOMP_GUI_MAGLISTVIEW_H
#define SEISCOMP_GUI_MAGLISTVIEW_H

#include <QtGui>
#include <seiscomp/gui/core/connectiondialog.h>
#include <seiscomp/gui/datamodel/ui_maglistview.h>
#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#include <seiscomp/datamodel/magnitude.h>
#endif
#include <seiscomp/gui/qt.h>

namespace Seiscomp {

namespace DataModel {

DEFINE_SMARTPOINTER(Event);
DEFINE_SMARTPOINTER(Origin);
DEFINE_SMARTPOINTER(Pick);
DEFINE_SMARTPOINTER(Station);
class DatabaseQuery;
class Notifier;

}

namespace Client {

DEFINE_SMARTPOINTER(Connection);

}

namespace Gui {


class SC_GUI_API MagListView : public QWidget {
	Q_OBJECT

	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef QMap<QString, DataModel::StationPtr> StationMap;


	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		MagListView(Seiscomp::DataModel::DatabaseQuery* reader,
		              bool withOrigins = true,
		              QWidget * parent = 0, Qt::WindowFlags f = 0);
		~MagListView();


	signals:
		void originAdded();
		void netMagAdded();

		void netMagSelected(Seiscomp::DataModel::Magnitude*,
		                    Seiscomp::DataModel::Origin* = nullptr,
                            Seiscomp::DataModel::Event* = nullptr);
		void originSelected(Seiscomp::DataModel::Origin*,
		                    Seiscomp::DataModel::Event* = nullptr);
		void eventSelected(Seiscomp::DataModel::Event*);

		void originUpdated(Seiscomp::DataModel::Origin*);


	public slots:
		void setAutoSelect(bool);
		void messageAvailable(Seiscomp::Core::Message*);
		void notifierAvailable(Seiscomp::DataModel::Notifier*);
		void expandEventItem(QTreeWidgetItem* eventItem, int col);
		void expandOriginItem(QTreeWidgetItem* originItem, int col);

	protected slots:
		void itemSelected(QTreeWidgetItem*,int);

		void readFromDatabase();
		void clear();
		//! HACK
		void onShowAll();


	private:
		void initTree();

		QTreeWidgetItem* addEvent(Seiscomp::DataModel::Event*);
		QTreeWidgetItem* addOrigin(Seiscomp::DataModel::Origin*, bool bold, QTreeWidgetItem* parent = nullptr);
		QTreeWidgetItem* addNetMag(Seiscomp::DataModel::Magnitude*, bool bold, QTreeWidgetItem* parent = nullptr);

		QTreeWidgetItem* findEvent(const std::string&);
		QTreeWidgetItem* findOrigin(const std::string&);
		QTreeWidgetItem* findNetMag(const std::string&);

		void readPicks(Seiscomp::DataModel::Origin*);


	private:
		::Ui::OriginListView _ui;
		QTreeWidgetItem* _unassociatedEventItem;
		QVector<DataModel::PickPtr> _associatedPicks;
		//StationMap _associatedStations;
		Seiscomp::DataModel::DatabaseQuery* _reader;
		bool _autoSelect;
		bool _withOrigins;
		bool _blockSelection;
		bool _readLock;

};


}
}

#endif
