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


#ifndef SCCONFIG_DIALOGS_IMPORTSTATIONSFDSNWS_H
#define SCCONFIG_DIALOGS_IMPORTSTATIONSFDSNWS_H


#include "ui_importfdsnws.h"
#include "ui_importfdsnws_sidebar.h"


class ImportFDSNWSDialog : public QDialog {
	Q_OBJECT

	public:
		explicit ImportFDSNWSDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());


	public:
		virtual void accept();


	private:
		void clearTable();


	private slots:
		void fetch();


	private:
		Ui::ImportFDSNWS        _ui;
		Ui::ImportFDSNWSSidebar _uiSideBar;
		std::string             _query;
};


#endif
