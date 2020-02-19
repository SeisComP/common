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



#define SEISCOMP_COMPONENT Gui

#include <seiscomp/gui/datamodel/locatorsettings.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LocatorSettings::LocatorSettings(QWidget * parent, Qt::WindowFlags flags)
: QDialog(parent, flags) {
	_ui.setupUi(this);

	_ui.tableParameters->setColumnCount(2);
	_ui.tableParameters->setHorizontalHeaderLabels(
		QStringList() << "Parameter" << "Value"
	);
	
	_ui.tableParameters->horizontalHeader()->setStretchLastSection(true);
	_ui.tableParameters->verticalHeader()->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LocatorSettings::addRow(const QString &name, const QString &value) {
	QTableWidgetItem *nameItem = new QTableWidgetItem;
	QTableWidgetItem *valueItem = new QTableWidgetItem;
	nameItem->setText(name);
	nameItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
	valueItem->setText(value);
	valueItem->setFlags(valueItem->flags() | Qt::ItemIsEditable);

	int row = _ui.tableParameters->rowCount();
	_ui.tableParameters->insertRow(row);
	_ui.tableParameters->setItem(row, 0, nameItem);
	_ui.tableParameters->setItem(row, 1, valueItem);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LocatorSettings::lastRowAdded() {
	_ui.tableParameters->resizeColumnToContents(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LocatorSettings::ContentList LocatorSettings::content() const {
	ContentList list;

	for ( int i = 0; i < _ui.tableParameters->rowCount(); ++i ) {
		list.append(
			QPair<QString,QString>(
				_ui.tableParameters->item(i,0)->text(),
				_ui.tableParameters->item(i,1)->text()
			)
		);
	}

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
