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


#include <seiscomp/gui/core/compat.h>
#include <seiscomp/core/version.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/utils/files.h>

#include <QFile>
#include <QScreen>

#include "about.h"


AboutWidget::AboutWidget(QWidget* parent, Qt::WindowFlags f)
 : QWidget(parent, f) {
	_ui.setupUi(this);
	setWindowFlags(Qt::Tool| Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_DeleteOnClose);
	activateWindow();

	QFile licenseFile(Seiscomp::Environment::Instance()->absolutePath("@DATADIR@/doc/seiscomp/LICENSE.html").c_str());
	licenseFile.open(QFile::ReadOnly);

	QString licenseText(licenseFile.readAll());

	_ui.textLicense->setHtml(licenseText);
	_ui.labelVersion->setText(Seiscomp::Core::CurrentVersion.toString().data());

	int minWidth = QT_FM_WIDTH(fontMetrics(), 'M')*65;
	int minHeight = fontMetrics().height()*45;
	int w = width(), h = height();
	if ( w < minWidth ) w = minWidth;
	if ( h < minHeight ) h = minHeight;

	if ( w > QApplication::primaryScreen()->size().width() ) {
		w = QApplication::primaryScreen()->size().width();
	}
	if ( h > QApplication::primaryScreen()->size().height() ) {
		h = QApplication::primaryScreen()->size().height();
	}

	resize(w, h);
}
