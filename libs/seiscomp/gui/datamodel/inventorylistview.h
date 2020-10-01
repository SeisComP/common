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



#ifndef SEISCOMP_GUI_INVENTORYLISTVIEW_H
#define SEISCOMP_GUI_INVENTORYLISTVIEW_H

#include <QTreeWidget>
#include <QStack>
#include <QMap>
#ifndef Q_MOC_RUN
#include <seiscomp/datamodel/object.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {

namespace DataModel {

class Network;
class Station;
class Stream;
class Component;

}


namespace Gui {

class SC_GUI_API InventoryListView : public QTreeWidget,
                                               public DataModel::Visitor {
	Q_OBJECT


	public:
		InventoryListView(QWidget *parent, Qt::WindowFlags f = 0);

		bool selectStream(const QString& streamID, bool);
		void selectStreams(const QString&, bool);

		QStringList selectedStreams() const;
		void expandNetworks();

	protected:
		bool visit(DataModel::PublicObject*);
		void visit(DataModel::Object*);
		void finished();


	signals:
		void streamChanged(QString, bool);
	
	public slots:
		void clearSelection();
		void setNonSelectedHidden(bool);

	private slots:
		void onItemChanged(QTreeWidgetItem *item, int column);
		void onItemPressed(QTreeWidgetItem *item, int column);


	private:
		void updateChildSelection(QTreeWidgetItem *item);
		void updateParentSelection(QTreeWidgetItem *item);

		void setNonSelectedHidden(QTreeWidgetItem*, bool);

		template <typename T>
		QTreeWidgetItem* add(T*);

		template <typename T>
		QTreeWidgetItem* add(QTreeWidgetItem* parent, T*);

		template <typename T>
		QTreeWidgetItem* create(T* object);

		QTreeWidgetItem* createDefaultItem();
		QTreeWidgetItem* insert(DataModel::Object*);

		void setRow(QTreeWidgetItem*,
		            const QString& first,
		            const QString& second = "",
		            const QString& third = "");

		void notifyAboutStateChange(const QString& streamID, bool state, bool hightPriority = false);

	private:
		QStack<QTreeWidgetItem*> _itemStack;

		typedef QMap<QChar, QTreeWidgetItem*> ComponentMap;
		typedef QMap<QString, ComponentMap> StreamMap;
		typedef QMap<QString, StreamMap> StationMap;
		typedef QMap<QString, StationMap> NetworkMap;
		typedef QMap<QString, QTreeWidgetItem*> StreamItems;

		StreamItems _streamItems;

		QTreeWidgetItem* _highestChangedItem;
};


}
}


#endif


