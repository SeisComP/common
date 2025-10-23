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


#include "../gui.h"
#include "../icon.h"
#include "system.h"
#include <seiscomp/system/environment.h>

#include <QAction>
#include <QDialog>
#include <QDir>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolBar>


using namespace std;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


class LogDialog : public QDialog {
	public:
		LogDialog(QWidget *parent = NULL) : QDialog(parent) {
			_content = new QTextEdit(this);
			_content->setReadOnly(true);

			QPushButton *close = new QPushButton("&Close", this);
			connect(close, SIGNAL(clicked()), this, SLOT(accept()));

			QHBoxLayout *layout = new QHBoxLayout;
			layout->addStretch();
			layout->addWidget(close);

			QVBoxLayout *mainLayout = new QVBoxLayout;
			mainLayout->addWidget(_content);
			mainLayout->addLayout(layout);

			setLayout(mainLayout);
		}

		void setContent(const QString &text) {
			_content->setText(text);
		}

	private:
		QTextEdit          *_content;
};


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SystemPanel::SystemPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	QPalette pal;

	_process = NULL;

	_name = "System";
	_icon = QIcon(":/scconfig/icons/menu_scconfig_system.svg");
	setHeadline("System");
	setDescription("Module status and control");

	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(1);
	setLayout(l);

	_status = new StatusLabel;
	_status->hide();
	l->addWidget(_status);

	_cmdToolBar = new QToolBar;
	_cmdToolBar->setAutoFillBackground(true);
	_cmdToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	l->addWidget(_cmdToolBar);

	_helpLabel = new StatusLabel;
	_helpLabel->setWordWrap(true);
	_helpLabel->setInfoText(
	            tr("Commands ('Start', 'Stop', ...) only affect the selected "
	               "modules. Without selection, all modules are affected. "
	               "Press ESC for clearing all selections."
	));

	l->addWidget(_helpLabel);

	QAction *a = _cmdToolBar->addAction(tr("Refresh"));
	a->setToolTip("Refresh the shown state of the modules");
	a->setIcon(::icon("refresh"));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(updateModuleState()));

	_cmdToolBar->addSeparator();
	//_cmdToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	a = _cmdToolBar->addAction(tr("Start"));
	a->setToolTip("Start modules");
	a->setIcon(::icon("start_color"));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(start()));

	a = _cmdToolBar->addAction(tr("Stop"));
	a->setToolTip("Stop modules");
	a->setIcon(::icon("stop_color"));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(stop()));

	a = _cmdToolBar->addAction(tr("Restart"));
	a->setToolTip("Stop and start modules");
	a->setIcon(::icon("module_restart"));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(restart()));

	a = _cmdToolBar->addAction(tr("Check"));
	a->setToolTip("Restart modules which stopped unexpectedly");
	a->setIcon(::icon("module_check"));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(check()));

	a = _cmdToolBar->addAction(tr("Reload"));
	a->setToolTip("Reload the module configuration and apply during runtime of "
	              "the module. Supported only be specific modules.");
	a->setIcon(::icon("reload_config"));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(reload()));

	_cmdToolBar->addSeparator();

	_enable = _cmdToolBar->addAction(tr("Enable"));
	_enable->setToolTip("Enable selected modules for default startup");
	_enable->setIcon(::icon("module_enable"));
	connect(_enable, SIGNAL(triggered(bool)), this, SLOT(enable()));

	_disable = _cmdToolBar->addAction(tr("Disable"));
	_disable->setToolTip("Disable selected modules from default startup");
	_disable->setIcon(::icon("module_disable"));
	connect(_disable, SIGNAL(triggered(bool)), this, SLOT(disable()));

	_cmdToolBar->addSeparator();

	a = _cmdToolBar->addAction(tr("Update config"));
	a->setIcon(::icon("module_update_config"));
	a->setToolTip("Synchronize inventory, keys and bindings. Send inventory and "
	              "bindings configuration to messaging for writing to database. "
	              "Also generate configuration of standalone modules");
	connect(a, SIGNAL(triggered(bool)), this, SLOT(updateConfig()));

	QSplitter *splitter = new QSplitter(Qt::Horizontal);
	splitter->setHandleWidth(1);

	_procTable = new QTableWidget;
	_procTable->setAutoFillBackground(true);
	_procTable->setFrameShape(QFrame::NoFrame);
	_procTable->setColumnCount(3);
	_procTable->verticalHeader()->setVisible(false);
	_procTable->horizontalHeader()->setStretchLastSection(true);
	_procTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	_procTable->setHorizontalHeaderLabels(QStringList() << "Auto" << "Module" << "Status");
	_procTable->setAlternatingRowColors(true);
	_procTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	_procTable->setSortingEnabled(true);
	_procTable->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_procTable, SIGNAL(customContextMenuRequested(QPoint)), this,
	        SLOT(onContextMenuRequested(QPoint)));

	QSizePolicy sp = _procTable->sizePolicy();
	sp.setVerticalStretch(1);
	_procTable->setSizePolicy(sp);

	_logWindow = new QTextEdit;
	_logWindow->setAutoFillBackground(true);
	_logWindow->setReadOnly(true);
	_logWindow->setFrameShape(QFrame::NoFrame);
	/*
	QPalette pal = _logWindow->palette();
	pal.setColor(QPalette::Base, QColor(64,64,64));
	pal.setColor(QPalette::Text, Qt::white);
	_logWindow->setPalette(pal);
	*/

	QWidget *container = new QWidget;
	QVBoxLayout *cl = new QVBoxLayout;
	cl->setContentsMargins(0, 0, 0, 0);
	cl->setSpacing(0);
	container->setLayout(cl);
	_procLabel = new QLabel;
	_procLabel->setAutoFillBackground(true);
	_procLabel->setMargin(4);
	_procLabel->setText("Idle");
	cl->addWidget(_procLabel);
	cl->addWidget(_logWindow);

	splitter->addWidget(_procTable);
	splitter->addWidget(container);

	sp = splitter->sizePolicy();
	sp.setVerticalStretch(2);
	splitter->setSizePolicy(sp);
	l->addWidget(splitter);

	QAction *clearSelection = new QAction(this);
	clearSelection->setShortcut(QKeySequence("Escape"));
	connect(clearSelection, SIGNAL(triggered()), _procTable->selectionModel(), SLOT(clearSelection()));
	addAction(clearSelection);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::onContextMenuRequested(const QPoint &pos) {
	QMenu menu;

	// Get item at the given pos
	QTableWidgetItem *item = _procTable->itemAt(pos);
	if ( !item ) {
		return;
	}

	// Get module item
	if ( item->column() != 1 ) {
		item = _procTable->item(item->row(), 1);
	}

	// start log file
	QDir installDir(QString::fromStdString(Seiscomp::Environment::Instance()->installDir()));
	QString startFileName = installDir.filePath("var/log/" + item->text() + ".log");
	QFile startFile(startFileName);
	bool enabled1 = startFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QAction *action1 = menu.addAction("Show start log");
	action1->setEnabled(enabled1);

	// module log file
	QString moduleLogFileName = QDir::homePath() + "/.seiscomp/log/" + item->text() + ".log";
	QFile logFile(moduleLogFileName);
	bool enabled2 = logFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QAction *action2 = menu.addAction("Show module log");
	action2->setEnabled(enabled2);

	QAction *result = menu.exec(QCursor::pos());

	if ( result == action1 ) {
		// start log file
		showLog(startFileName, startFile.readAll());
	}
	else if ( result == action2 ) {
		// module log file
		showLog(moduleLogFileName, logFile.readAll());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::showLog(const QString fileName, const QString &text) {
	LogDialog dlg;
	dlg.setWindowTitle("Log file: " + fileName);
	if ( text.trimmed().isEmpty()) {
		dlg.setContent("File is empty.");
	}
	else {
		dlg.setContent(text);
	}
	dlg.exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::setModel(ConfigurationTreeItemModel *model) {
	if ( _model ) _model->disconnect(this);

	ConfiguratorPanel::setModel(model);

	connect(_model, SIGNAL(modificationChanged(bool)),
	        this, SLOT(modificationChanged(bool)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::activated() {
	modificationChanged(_model->isModified());
	updateModuleState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::modificationChanged(bool changed) {
	if ( changed ) {
		_status->setWarningText(
			tr("Module and/or binding configuration was changed but not saved. "
		       "Save configuration to apply before running commands.")
		);

		_status->show();
	}
	else
		_status->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::updateModuleState(bool logOutput) {
	setCursor(Qt::WaitCursor);

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	QProcess seiscomp;
	seiscomp.start(QString("%1%2")
	               .arg(env->installDir().c_str()).arg("/bin/seiscomp"),
	               QStringList() << "--csv" << "status");

	if ( !seiscomp.waitForStarted() ) {
		unsetCursor();
		return;
	}

	_procLabel->setText("Running");

	if ( !seiscomp.waitForFinished() ) {
		unsetCursor();
		return;
	}

	_procLabel->setText("Idle");

	for ( int i = 0; i < _procTable->rowCount(); ++i ) {
		QTableWidgetItem *item = _procTable->item(i, 1);
		item->setData(Qt::UserRole, 0);
	}

	_procTable->setSortingEnabled(false);

	QByteArray stdout = seiscomp.readAllStandardOutput();
	QList<QByteArray> output = stdout.split('\n');
	QList<QByteArray>::iterator it;
	for ( it = output.begin(); it != output.end(); ++it ) {
		if ( it->isEmpty() ) continue;
		QStringList toks = QString(it->constData()).split(';');
		if ( toks.count() < 4 ) continue;

		int row = -1;
		for ( int i = 0; i < _procTable->rowCount(); ++i ) {
			QTableWidgetItem *item = _procTable->item(i, 1);
			if ( item->text() == toks[0] ) {
				row = i;
				break;
			}
		}

		QTableWidgetItem *active;
		QTableWidgetItem *state;

		if ( row == -1 ) {
			QTableWidgetItem *name = new QTableWidgetItem(toks[0]);
			name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QFont f = name->font();
			f.setBold(true);
			name->setFont(f);

			state = new QTableWidgetItem;
			state->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			active = new QTableWidgetItem;
			active ->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			active->setFont(f);

			row = _procTable->rowCount();
			_procTable->insertRow(row);
			_procTable->setItem(row, 0, active);
			_procTable->setItem(row, 1, name);
			_procTable->setItem(row, 2, state);
		}
		else {
			active = _procTable->item(row, 0);
			state = _procTable->item(row, 2);
		}

		bool ok;
		int s = toks[1].toInt(&ok);
		if ( !ok ) s = -1;
		int f = toks[2].toInt(&ok);
		if ( !ok ) f = -1;
		int a = toks[3].toInt(&ok);
		if ( !ok ) a = -1;

		switch ( s ) {
			case -1:
				state->setBackground(Qt::gray);
				state->setText("undefined");
				break;
			case 0:
				state->setText("not running");
				if ( f == 1 ) {
					state->setForeground(Qt::white);
					state->setBackground(QColor(229, 34, 34));
				}
				else {
					state->setForeground(Qt::NoBrush);
					state->setBackground(Qt::NoBrush);
				}
				break;
			default:
				state->setForeground(Qt::white);
				state->setBackground(QColor(0, 166, 0));
				state->setText("running");
				break;
		}

		switch ( a ) {
			case 0:
				active->setText("Off");
				active->setForeground(QColor(229, 34, 34));
				break;
			case 1:
				active->setText("On");
				active->setForeground(QColor(0, 166, 0));
				break;
			default:
				active->setText("-");
				active->setForeground(Qt::NoBrush);
				break;
		}

		// Set updated flag
		state->setData(Qt::UserRole, 1);
	}

	for ( int i = 0; i < _procTable->rowCount(); ) {
		QTableWidgetItem *item = _procTable->item(i, 2);
		if ( item->data(Qt::UserRole).toInt() != 1 )
			_procTable->removeRow(i);
		else
			++i;
	}

	if ( logOutput ) {
		_logWindow->clear();
		logStdOut(stdout);
		logStdErr(seiscomp.readAllStandardError());
	}

	_procTable->setSortingEnabled(true);
	_procTable->sortByColumn(_procTable->horizontalHeader()->sortIndicatorSection(),
	                         _procTable->horizontalHeader()->sortIndicatorOrder());

	unsetCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::logStdOut(const QByteArray &data) {
	_logWindow->setTextColor(palette().color(QPalette::Text));
	_logWindow->insertPlainText(data.constData());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::logStdErr(const QByteArray &data) {
	_logWindow->setTextColor(QColor(128,92,0));
	_logWindow->insertPlainText(data.constData());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::start() {
	runSeiscomp(QStringList() << "start");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::stop() {
	runSeiscomp(QStringList() << "stop");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::restart() {
	runSeiscomp(QStringList() << "restart");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::reload() {
	runSeiscomp(QStringList() << "reload");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::check() {
	runSeiscomp(QStringList() << "check");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::enable() {
	runSeiscomp(QStringList() << "enable");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::disable() {
	runSeiscomp(QStringList() << "disable");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::updateConfig() {
	runSeiscomp(QStringList() << "update-config");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::runSeiscomp(const QStringList &params_) {
	if ( _process ) return;

	QStringList params = params_;
	QList<QTableWidgetItem*> items = _procTable->selectedItems();
	foreach ( QTableWidgetItem *item, items ) {
		if ( item->column() != 1 ) continue;
		params.append(item->text());
	}

	_logWindow->clear();
	_logWindow->setTextColor(palette().color(QPalette::Text));

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	_process = new QProcess;
	connect(_process, SIGNAL(readyReadStandardError()),
	        this, SLOT(readStderr()));
	connect(_process, SIGNAL(readyReadStandardOutput()),
	        this, SLOT(readStdout()));
	connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)),
	        this, SLOT(processFinished(int,QProcess::ExitStatus)));

	QString cmd = QString("%1%2")
	              .arg(env->installDir().c_str()).arg("/bin/seiscomp");
	_process->start(cmd, params, QIODevice::ReadWrite | QIODevice::Unbuffered);

	_logWindow->insertPlainText(QString("$ seiscomp %1\n").arg(params.join(" ")));

	if ( !_process->waitForStarted() )
		return;

	_procLabel->setText("Running");
	_cmdToolBar->setEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::readStdout() {
	logStdOut(_process->readAllStandardOutput());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::readStderr() {
	logStdErr(_process->readAllStandardError());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SystemPanel::processFinished(int code, QProcess::ExitStatus status) {
	delete _process;
	_process = NULL;
	_cmdToolBar->setEnabled(true);
	_procLabel->setText("Idle");

	updateModuleState(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
