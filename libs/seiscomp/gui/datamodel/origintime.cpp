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



#include "origintime.h"
#include <iostream>
#include <seiscomp/gui/core/application.h>


namespace Seiscomp {
namespace Gui {


OriginTimeDialog::OriginTimeDialog(double lon, double lat,
                                   Seiscomp::Core::Time time,
                                   QWidget * parent, Qt::WindowFlags f)
 : QDialog(parent, f) {

	_ui.setupUi(this);

	_ui.label->setFont(SCScheme.fonts.normal);
	_ui.label_2->setFont(SCScheme.fonts.normal);
	_ui.labelLatitude->setFont(SCScheme.fonts.highlight);
	_ui.labelLongitude->setFont(SCScheme.fonts.highlight);

	_ui.labelLatitude->setText(QString("%1 %2").arg(lat, 0, 'f', 2).arg(QChar(0x00b0)));
	_ui.labelLongitude->setText(QString("%1 %2").arg(lon, 0, 'f', 2).arg(QChar(0x00b0)));

	int y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;
	time.get(&y, &M, &d, &h, &m, &s);
	_ui.timeEdit->setTime(QTime(h, m, s));
	_ui.dateEdit->setDate(QDate(y, M, d));
}


Seiscomp::Core::Time OriginTimeDialog::time() const {
	QDateTime dt(_ui.dateEdit->date(), _ui.timeEdit->time());
	Seiscomp::Core::Time t;

	t.set(_ui.dateEdit->date().year(),
	      _ui.dateEdit->date().month(),
	      _ui.dateEdit->date().day(),
	      _ui.timeEdit->time().hour(),
	      _ui.timeEdit->time().minute(),
	      _ui.timeEdit->time().second(),
	      0);

	return t;
}


}
}
