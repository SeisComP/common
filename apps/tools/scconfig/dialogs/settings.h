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

#ifndef SCCONFIG_DIALOGS_SETTINGS_H
#define SCCONFIG_DIALOGS_SETTINGS_H


#include <QDialog>


class QSpinBox;


/**
 * @brief Adjusts the configuration of scconfig itself.
 *
 * Accepting writes the parameters to @SYSTEMCONFIGDIR@/scconfig.cfg and
 * applies them to the views created afterwards. The dialog
 * stays open if the file cannot be written.
 */
class SettingsDialog : public QDialog {
	public:
		explicit SettingsDialog(QWidget *parent = nullptr);

	public:
		void accept() override;
		static QString expandLevelToolTip();

	private:
		QSpinBox *_level;
};


#endif
