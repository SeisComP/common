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


#ifndef SEISCOMP_CONFIGURATION_GUI_PANEL_SYSTEM_H__
#define SEISCOMP_CONFIGURATION_GUI_PANEL_SYSTEM_H__

#include "../gui.h"

#include <QProcess>
#include <QWidget>

class QTextEdit;
class QTableWidget;

class SystemPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		SystemPanel(QWidget *parent = 0);

	public:
		void setModel(ConfigurationTreeItemModel *model);
		void activated();

	private slots:
		void onContextMenuRequested(const QPoint&);
		void modificationChanged(bool changed);
		void updateModuleState(bool logOutput = true);
		void start();
		void stop();
		void restart();
		void reload();
		void check();
		void enable();
		void disable();
		void updateConfig();
		void readStdout();
		void readStderr();

		void processFinished(int, QProcess::ExitStatus);

	private:
		void runSeiscomp(const QStringList &params);
		void showLog(const QString fileName, const QString &text);
		void logStdOut(const QByteArray &data);
		void logStdErr(const QByteArray &data);

	private:
		QTableWidget *_procTable;
		QLabel       *_procLabel;
		QTextEdit    *_logWindow;
		QProcess     *_process;
		QToolBar     *_cmdToolBar;
		QAction      *_enable;
		QAction      *_disable;
		QAction      *_updateConfig;
		StatusLabel  *_status;
		QLabel       *_helpLabel;
};


#endif
