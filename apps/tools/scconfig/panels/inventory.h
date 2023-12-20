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


#ifndef SEISCOMP_CONFIGURATION_GUI_PANEL_INVENTORY_H__
#define SEISCOMP_CONFIGURATION_GUI_PANEL_INVENTORY_H__

#include "../gui.h"

#include <QDialog>
#include <QProcess>
#include <QTextEdit>


class QListView;
class QTreeView;
class QFileSystemModel;
class QSortFilterProxyModel;
class QLineEdit;
class QPushButton;
class QComboBox;

class LogDialog;
class StatusPanel;


class ProcessWidget : public QDialog {
	Q_OBJECT

	public:
		ProcessWidget(QWidget *parent = 0);
		~ProcessWidget();

		int start(const QString &cmd, const QStringList &params);
		void done(int r);


	private slots:
		void started();
		void error(QProcess::ProcessError error);

		void readStderr();
		void readStdout();
		void processFinished(int, QProcess::ExitStatus);


	private:
		QProcess    *_process;
		LogDialog   *_logWindow;
		QPushButton *_btnOK;
		QPushButton *_btnStop;
		StatusPanel *_status;
		int          _exitCode;
};


class ImportDialog : public QDialog {
	Q_OBJECT

	public:
		ImportDialog(const QStringList &formats, QWidget *parent = 0);

		QString format() const;
		QString source() const;

	private slots:
		void openFileDialog();

	private:
		QComboBox *_formats;
		QLineEdit *_source;
};


class InventoryPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		InventoryPanel(QWidget *parent = 0);


	private:
		int runProc(const QString &cmd, const QStringList &params);
		int runSCProc(const QString &cmd, const QStringList &params);
		int runSC(const QStringList &params);


	private slots:
		void headerSectionClicked(int);

		void switchToIconView();
		void switchToListView();
		void switchToDetailedView();
		void renameFile();
		void deleteFiles();
		void inspectFile();

		void import();
		void testInventory();
		void testInventoryFile();
		void testSync();
		void sync();
		void syncKeys();


	private:
		QListView             *_folderView;
		QTreeView             *_folderTree;
		QFileSystemModel      *_folderModel;
		QItemSelectionModel   *_selectionModel;
};


#endif
