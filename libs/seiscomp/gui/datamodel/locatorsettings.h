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



#ifndef SEISCOMP_GUI_LOCATORSETTINGS_H
#define SEISCOMP_GUI_LOCATORSETTINGS_H

#include <QtGui>
#include <seiscomp/gui/datamodel/ui_locatorsettings.h>
#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API LocatorSettings : public QDialog {
	Q_OBJECT

	public:
		typedef QList< QPair<QString,QString> > ContentList;

	public:
		LocatorSettings(QWidget * parent = 0, Qt::WindowFlags f = 0);

	public:
		void addRow(const QString &name, const QString &value);
		void lastRowAdded();

		ContentList content() const;


	private:
		::Ui::LocatorSettings _ui;
};



}

}

#endif
