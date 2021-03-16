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


#ifndef SC_GUI_INSPECTOR_H
#define SC_GUI_INSPECTOR_H


#include <QTreeWidget>
#include <QTableWidget>
#include <QTimer>
#include <QStack>

#include <seiscomp/gui/core/ui_inspector.h>
#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#endif
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API Inspector : public QWidget {

	Q_OBJECT

	public:
		Inspector(QWidget * parent = 0, Qt::WindowFlags f = 0);
		~Inspector();


	public:
		void setObject(Core::BaseObject *obj);
		void selectObject(Core::BaseObject *obj);


	private:
		void addObject(QTreeWidgetItem *parent);
		void addProperty(const std::string &name, const std::string &type,
		                 const std::string &value, bool isIndex = false,
		                 bool isOptional = false, bool isReference = false);
		void selectObject(QTreeWidgetItem *parent, Core::BaseObject *obj);
		bool filterTree(QTreeWidgetItem *parent, const std::string &type,
		                const std::string &attr, const std::string &value,
		                QTreeWidgetItem **firstMatch,
		                bool parentMatch = false);


	private slots:
		void selectionChanged();
		void filterChanged(QString);
		void applyFilter();
		void linkClicked(QString);
		void back();


	private:
		typedef QPair<Core::BaseObject*, Core::BaseObject*> State;

		Core::BaseObject *_object;
		Core::BaseObject *_currentSelection;
		Ui::Inspector     _ui;
		QStack<State>     _history;
		QTimer            _filterTimer;
};


}
}


#endif
