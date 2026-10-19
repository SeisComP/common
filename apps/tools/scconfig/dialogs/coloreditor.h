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


#ifndef SCCONFIG_DIALOGS_COLOREDITOR_H
#define SCCONFIG_DIALOGS_COLOREDITOR_H


#include <QColor>
#include <QIcon>
#include <QString>


class QWidget;

namespace Seiscomp {
namespace System {
class Parameter;
}
}


bool parseColor(const QString &value, QColor &color);

QIcon colorSwatch(const QColor &color);

bool selectColor(QWidget *parent, const Seiscomp::System::Parameter *param,
                 const QString &currentValue, QString &value);


#endif
