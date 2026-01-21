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

#define SEISCOMP_COMPONENT Gui::ProcessManager

#include <seiscomp/core/datetime.h>
#include <seiscomp/core/optional.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/gui/core/processmanager.h>
#include <seiscomp/gui/core/utils.h>

#include <seiscomp/logging/log.h>


#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QTextEdit>

#include <cerrno>

#include <csignal>
#include <unistd.h>

namespace Seiscomp::Gui {

namespace {

QString runtime(const Core::TimeSpan &ts) {

	int sec = ts.seconds();
	int days = sec / 86400;
	int hours = (sec - days * 86400) / 3600;
	int minutes = (sec - days * 86400 - hours * 3600) / 60;
	int seconds = sec - days * 86400 - hours * 3600 - 60 * minutes;

	if ( days > 0 ) {
		return QString("%1d %2h").arg(days).arg(hours);
	}

	if ( hours > 0 ) {
		return QString("%1h %2m").arg(hours).arg(minutes);
	}

	if ( minutes > 0 ) {
		return QString("%1m %2s").arg(minutes).arg(seconds);
	}

	auto secf = static_cast<qreal>(seconds);
	secf += static_cast<qreal>(ts.microseconds()) / 1000000.0;
	return QString::number(secf, 'f', 2);
}


} // ns anonymous
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct ProcessManager::Item {
	QString name;
	QString description;
	QIcon icon;
	QVariant userData;
	Core::Time created;
	OPT(Core::Time) started;
	OPT(Core::Time) running;
	OPT(Core::Time) stopped;
	Core::TimeSpan stopDuration;
	OPT(Core::Time) terminated;
	OPT(Core::Time) killed;
	OPT(Core::Time) finished;
	QProcess *process;

	QTextEdit *stdOut{nullptr};
	QTextEdit *stdErr{nullptr};
	QTextEdit *processLog{nullptr};
	int maxStdOutChars{-1};
	int maxStdErrChars{-1};

	~Item() {
		delete process;
		process = nullptr;

		delete stdOut;
		stdOut = nullptr;

		delete stdErr;
		stdErr = nullptr;

		delete processLog;
		processLog = nullptr;
	}

	enum ItemState : std::int8_t {
		NotYetRunning,
		Starting,
		Running,
		Crashed,
		FailedToStart,
		Stopped,
		Terminated,
		Killed,
		ExitOnError,
		Success
	};

	[[nodiscard]]
	ItemState state() const {
		if ( process->state() == QProcess::Starting ) {
			return Starting;
		}

		if ( process->state() == QProcess::Running ) {
			return stopped ? Stopped : Running;
		}

		// QProcess::NotRunning

		if ( !started ) {
			return NotYetRunning;
		}

		if ( terminated ) {
			return Terminated;
		}

		if ( killed ) {
			return Killed;
		}

		if ( process->exitStatus() == QProcess::CrashExit ) {
			return Crashed;
		}

		if ( !running ) {
			return FailedToStart;
		}

		if ( process->exitCode() ) {
			return ExitOnError;
		}

		return Success;
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ProcessManager::Model : public QAbstractTableModel {
	public:
		enum Column : std::int8_t {
			ColState = 0,
			ColName = 1,
			ColStarted = 2,
			ColRuntime = 3,
			ColExitCode = 4,
			ColSIZE = 5
		};

		Model(QWidget *parent) : QAbstractTableModel(parent) {
			_statePixmaps[Item::NotYetRunning] = pixmap(parent, "process_asterisk");
			_statePixmaps[Item::Starting] = pixmap(parent, "process_continue");
			_statePixmaps[Item::Running] = pixmap(parent, "process_progress");
			_statePixmaps[Item::Crashed] = pixmap(parent, "process_bug");
			_statePixmaps[Item::FailedToStart] = pixmap(parent, "process_warning");
			_statePixmaps[Item::Stopped] = pixmap(parent, "process_pause");
			_statePixmaps[Item::Terminated] = pixmap(parent, "process_terminate");
			_statePixmaps[Item::Killed] = pixmap(parent, "process_kill");
			_statePixmaps[Item::ExitOnError] = pixmap(parent, "process_attention");
			_statePixmaps[Item::Success] = pixmap(parent, "process_done");
		}

		[[nodiscard]]
		int rowCount(const QModelIndex &parent = QModelIndex()) const override {
			Q_UNUSED(parent);
			return _rows.size();
		}

		[[nodiscard]]
		int columnCount(const QModelIndex &parent = QModelIndex()) const override {
			Q_UNUSED(parent);
			return ColSIZE;
		}

		[[nodiscard]]
		const Item *item(int row) const {
			if ( row < 0 || row >=_rows.size() ) {
				return {};
			}
			return _rows[row];
		}

		int row(const Item *item) const {
			return _rows.indexOf(item);
		}

		void addItem(const Item *item) {
			beginInsertRows(QModelIndex(), _rows.size(), _rows.size());
			_rows.append(item);
			endInsertRows();
		}

		void removeItem(const Item *item) {
			auto row = this->row(item);
			if ( row >= 0 ) {
				removeRow(row);
			}
		}

		void removeRow(int row) {
			if ( row < 0 || row >= _rows.size() ) {
				return;
			}

			beginRemoveRows(QModelIndex(), row, row);
			_rows.removeAt(row);
			endRemoveRows();
		}

		bool emitDataChanged(const QModelIndex &topLeft,
		                     const QModelIndex &bottomRight,
		                     const QVector<int> &roles = QVector<int>() ) {
			if ( topLeft.isValid() && bottomRight.isValid() ) {
				emit dataChanged(topLeft, bottomRight, roles);
				return true;
			}

			return false;
		}

		bool emitDataChanged(int row,
		                     const QVector<int> &roles = QVector<int>() ) {
			return emitDataChanged(createIndex(row, 0),
			                       createIndex(row, columnCount() - 1),
			                       roles);
		}

		bool emitDataChanged(int row, int col,
		                     const QVector<int> &roles = QVector<int>() ) {
			return emitDataChanged(createIndex(row, col), createIndex(row, col),
			                       roles);
		}

		[[nodiscard]]
		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const override {
			if ( orientation != Qt::Horizontal ) {
				return {};
			}

			switch ( role ) {
				case Qt::DisplayRole:
					switch ( section ) {
						case ColState:
							return "State";
						case ColName:
							return "Name";
						case ColStarted:
							return "Started";
						case ColRuntime:
							return "Runtime";
						case ColExitCode:
							return "Exit Code";
					}
					break;

				case Qt::ToolTipRole:
					switch ( section ) {
						case ColState:
							return "Process state";
						case ColRuntime:
							return "Runtime in seconds";
					}
					break;

				case Qt::TextAlignmentRole:
					switch ( section ) {
						case ColRuntime:
						case ColExitCode:
							return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);

						default:
							return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
					}
					break;
			}
			return {};
		}

		[[nodiscard]]
		QVariant data(const QModelIndex &index,
		              int role = Qt::DisplayRole) const override {
			const auto *item = _rows[index.row()];

			switch ( role ) {
				case Qt::DisplayRole:
					switch ( index.column() ) {
						case ColState:
							switch ( item->state() ) {
								case Item::Starting:
									return QString("Starting");
								case Item::Running:
									return QString("Running");
								case Item::NotYetRunning:
									return QString("Not yet started");
								case Item::Crashed:
									return QString("Crashed");
								case Item::FailedToStart:
									return QString("Failed to start");
								case Item::Stopped:
									return QString("Stopped");
								case Item::Terminated:
									return QString("Terminated");
								case Item::Killed:
									return QString("Killed");
								case Item::ExitOnError:
									return QString("Exit on error");
								case Item::Success:
									return QString("Finished");
							}
							return {};

						case ColName:
							return item->name;

						case ColStarted:
							if ( item->started ) {
								return QString::fromStdString(item->started->toString("%T"));
							}

						case ColRuntime:
							if ( !item->running ) {
								return {};
							}

							if ( item->finished ) {
								return runtime(*item->finished - *item->running - item->stopDuration);
							}

							if ( item->stopped ) {
								return runtime(*item->stopped - *item->running - item->stopDuration);
							}

							return runtime(Core::Time::UTC() - *item->running - item->stopDuration);

						case ColExitCode:
							if ( item->process->exitStatus() == QProcess::NormalExit ) {
								return item->process->exitCode();
							}
							return {};

					}
					return {};

				case Qt::DecorationRole:
					switch ( index.column() ) {
						case ColState:
							// progress activity
							if ( item->state() == Item::Running && !item->stopped ) {
								const auto &pm = _statePixmaps[Item::Running];
								auto pmSize = pm.size();
								auto pmLayoutSize = pmSize / pm.devicePixelRatioF();
								QPixmap rotatedPixmap(pmSize);
								rotatedPixmap.setDevicePixelRatio(pm.devicePixelRatioF());
								rotatedPixmap.fill(Qt::transparent);
								QPainter p(&rotatedPixmap);
								p.setRenderHint(QPainter::SmoothPixmapTransform);
								p.translate(0.5 * pmLayoutSize.width(), 0.5 * pmLayoutSize.height());
								p.rotate(_progressAngle);
								p.translate(-0.5 * pmLayoutSize.width(), -0.5 * pmLayoutSize.height());
								p.drawPixmap(0, 0, pm);
								p.end();
								return rotatedPixmap;
							}

							return _statePixmaps[item->state()];

						case ColName:
							return item->icon;
					}
					return {};

				case Qt::ToolTipRole:
					switch ( index.column() ) {
						case ColName:
							return item->description;

						case ColStarted:
							if ( item->started ) {
								return QString::fromStdString(item->started->iso());
							}
							break;

						case ColRuntime:
							if ( item->running ) {
								if ( item->finished ) {
									QString toolTip = QString("%1 - %2").arg(
									    QString::fromStdString(item->running->iso()),
									    QString::fromStdString(item->finished->iso()));
									if ( item->stopDuration.length() > 0 ) {
										toolTip += QString(" (stopped for %1s)")
										               .arg(item->stopDuration.length());
									}
									return toolTip;
								}

								if ( item->stopped ) {
									return QString("%1 - %2 (stopped for %3s)")
									    .arg(QString::fromStdString(item->running->iso()),
									         QString::fromStdString(item->stopped->iso()))
									    .arg((Core::Time::UTC() - *item->stopped +
									         item->stopDuration).length());
								}
							}
							break;

					}
					return {};

				case Qt::TextAlignmentRole:
					switch ( index.column() ) {
						case ColRuntime:
						case ColExitCode:
							return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);

						default:
							return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);

					}
					break;

				case Qt::UserRole:
					return {};

			}

			return {};
		}

		void setProgressAngle(qreal angle) {
			_progressAngle = angle;
		}

	protected:
		QVector<const Item*> _rows;
		QMap<Item::ItemState, QPixmap> _statePixmaps;
		qreal _progressAngle{0};
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ProcessManager::ProcessManager(QWidget *parent, Qt::WindowFlags f)
: QMainWindow(parent, f) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QProcess *ProcessManager::createProcess(QString name, QString description,
                                        QIcon icon, QVariant userData) {
	auto *process = new QProcess(this);
	process->setEnvironment(QProcess::systemEnvironment());
	process->setProcessChannelMode(QProcess::SeparateChannels);

	auto setupConsoleTextEdit = [](QTextEdit *edit) {
		edit->setReadOnly(true);
		edit->setAutoFillBackground(true);
		QPalette pal = edit->palette();
		pal.setColor(QPalette::Base, QColor(64,64,64));
		pal.setColor(QPalette::Text, Qt::white);
		edit->setPalette(pal);

		QFont mf = SCScheme.fonts.normal;
		mf.setFamily("Monospace");
		mf.setStyleHint(QFont::TypeWriter);
		edit->setFont(mf);
	};

	auto *item = new Item;
	item->name = std::move(name);
	item->description = std::move(description);
	item->icon = std::move(icon);
	item->userData = std::move(userData);
	item->process = process;
	item->created = Core::Time::UTC();
	item->stdOut = new QTextEdit;
	item->stdErr = new QTextEdit;
	item->processLog = new QTextEdit;

	setupConsoleTextEdit(item->stdOut);
	setupConsoleTextEdit(item->stdErr);
	setupConsoleTextEdit(item->processLog);

	item->stdOut->hide();
	item->stdErr->hide();
	item->processLog->hide();

	_ui.layoutStdout->addWidget(item->stdOut);
	_ui.layoutStderr->addWidget(item->stdErr);
	_ui.layoutLog->addWidget(item->processLog);

	connect(process, &QProcess::readyReadStandardOutput,
	        this, &ProcessManager::onProcessReadyReadStandardOutput);
	connect(process, &QProcess::readyReadStandardError,
	        this, &ProcessManager::onProcessReadyReadStandardError);
	connect(process, &QProcess::stateChanged,
			this, &ProcessManager::onProcessStateChanged);

	_items[process] = item;
	_model->addItem(item);

	// if the process manager is not the active window and no process is running select
	// the process just created
	if ( !isActiveWindow() && _running.empty() ) {
		auto index = proxyIndexForItem(item);
		if ( index.isValid() ) {
			_ui.table->selectRow(index.row());
		}
	}

	updateControls();

	addLog(item, item->created, "Created");

	emit stateChanged();

	return process;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ProcessManager::waitForStarted(QProcess *process, int timeout) {
	if ( process->waitForStarted(timeout) ) {
		return true;
	}

	QString detail;
	switch ( process->error() ) {
		case QProcess::FailedToStart:
			detail = "The process failed to start. Either the invoked program "
			         "is missing, or you may have insufficient permissions to "
			         "invoke the program.";
			break;
		case QProcess::Crashed:
			detail = "The process crashed some time after starting "
			         "successfully.";
			break;
		case QProcess::Timedout:
			detail = QString("Start timed out of %1ms exceeded.").arg(timeout);
			break;
		case QProcess::WriteError:
			detail = "An error occurred when attempting to write to the "
			         "process. For example, the process may not be running, or "
			         "it may have closed its input channel.";
			break;
		case QProcess::ReadError:
			detail = "An error occurred when attempting to read from the "
			         "process. For example, the process may not be running.";
			break;
		default:
			detail = "An unknown error occurred.";
	}

	auto msg = QString("Failed to start command %1: %2")
	           .arg(process->program(), detail);
	SEISCOMP_ERROR(msg.toStdString());

	auto *item = _items[process];
	// select item in table and show process manager
	if ( item ) {
		addLog(item, Core::Time::UTC(), msg);
		auto index = proxyIndexForItem(item);
		if ( index.isValid() ) {
			_ui.table->selectRow(index.row());
		}
		show();
	}
	// process not managed, show message box instead
	else {
		QMessageBox::warning(this, "Failed to start", msg);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ProcessManager::setConsoleLimits(QProcess *process, int charsOut,
                                      int charsErr) {
	auto *item = _items[process];
	if ( item ) {
		item->maxStdOutChars = charsOut;
		item->maxStdErrChars = charsErr;

		QString tt;

		if ( charsOut ) {
			tt = "Standard output of current process";
			if ( charsOut > 0 ) {
				tt = QString("%1 limited to %2 characters").arg(tt, charsOut);
			}
		}
		else {
			tt = "Capture of standard output disabled";
		}
		item->stdOut->setToolTip(tt);

		if ( charsErr ) {
			tt = "Error ouput of current process";
			if ( charsErr > 0 ) {
				tt = QString("%1 limited to %2 characters").arg(tt, charsErr);
			}
		}
		else {
			tt = "Capture of error output disabled";
		}
		item->stdErr->setToolTip(tt);

		addConsoleOutput(item->stdOut, "", charsOut);
		addConsoleOutput(item->stdErr, "", charsErr);

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::log(QProcess *process, const QString &message) {
	addLog(itemForProcess(process), Core::Time::UTC(), message);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ProcessManager::processCount() {
	return _items.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ProcessManager::runningCount() {
	return _running.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ProcessManager::erroneousCount() {
	return _erroneous.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ProcessManager::stop(QProcess *process) {
	auto *item = _items[process];
	if ( item && item->process->state() == QProcess::Running ) {
		if ( ::kill(item->process->processId(), SIGSTOP) == 0 ) {
			auto now = Core::Time::UTC();
			addLog(item, now, QString("Stop requested (SIGSTOP)"));
			if ( !item->stopped ) {
				item->stopped = now;
			}
			updateItemState(item);

			return true;
		}

		addLog(item, Core::Time::UTC(),
		       QString("Could not send stop signal (SIGSTOP): %1").arg(strerror(errno)));
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ProcessManager::continue_(QProcess *process) {
	auto *item = _items[process];
	if ( item && item->process->state() == QProcess::Running ) {
		if ( ::kill(item->process->processId(), SIGCONT) == 0 ) {
			auto now = Core::Time::UTC();
			if ( item->stopped ) {
				item->stopDuration += (now - *item->stopped);
				item->stopped.reset();
			}
			addLog(item, now, QString("Continue requested (SIGCONT)"));
			updateItemState(item);

			return true;
		}

		addLog(item, Core::Time::UTC(),
		       QString("Could not send continue signal (SIGCONT): %1")
		       .arg(strerror(errno)));
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ProcessManager::terminate(QProcess *process) {
	auto *item = _items[process];
	if ( item && item->process->state() != QProcess::NotRunning ) {
		item->process->terminate();
		item->terminated = Core::Time::UTC();
		if ( item->stopped ) {
			addLog(item, *item->terminated,
			       QString("Terminate requested on stopped process. Signal will not be "
			               "processed until process execution is continued."));
		}
		else {
			addLog(item, *item->terminated, QString("Terminate requested"));
		}
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ProcessManager::kill(QProcess *process) {
	auto *item = _items[process];
	if ( item && item->process->state() != QProcess::NotRunning ) {
		item->process->kill();
		item->killed = Core::Time::UTC();
		addLog(item, *item->killed, QString("Kill requested"));
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<QProcess*> ProcessManager::processes() {
	QList<QProcess*> processes;
	for ( auto *item : std::as_const(_items) ) {
		processes << item->process;
	}

	return processes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::activate() {
	if ( isHidden() ) {
		show();
	}
	if ( isMinimized() ) {
		showNormal();
	}
	if ( !isActiveWindow() ) {
		activateWindow();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onDataChanged(const QModelIndex &topLeft,
                                   const QModelIndex &bottomRight,
                                   const QVector<int> &roles) {
	if ( !roles.contains(Qt::DisplayRole) ) {
		return;
	}

	for ( int i = topLeft.column(); i <= bottomRight.column(); ++i ) {
		_ui.table->resizeColumnToContents(i);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onCurrentChanged(const QModelIndex &current,
                                      const QModelIndex &previous) {
	if ( previous.isValid() ) {
		const auto *item = itemForProxyIndex(previous);
		if ( item ) {
			item->stdOut->hide();
			item->stdErr->hide();
			item->processLog->hide();
		}
	}

	if ( current.isValid() ) {
		const auto *item = itemForProxyIndex(current);
		if ( item ) {
			item->stdOut->show();
			item->stdErr->show();
			item->processLog->show();
		}
	}

	updateControls();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onSelectionChanged(const QItemSelection &selected,
                                        const QItemSelection &deselected) {
	updateControls();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onProcessReadyReadStandardOutput() {
	auto *item = itemForProcessSender();
	if ( item ) {
		auto bytes = item->process->readAllStandardOutput();
		addConsoleOutput(item->stdOut, QString(bytes), item->maxStdOutChars);

		emit readStandardOutput(bytes, item->process, item->userData);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onProcessReadyReadStandardError() {
	auto *item = itemForProcessSender();
	if ( item ) {
		auto bytes = item->process->readAllStandardError();
		addConsoleOutput(item->stdErr, QString(bytes), item->maxStdErrChars);

		emit readStandardError(bytes, item->process, item->userData);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onProcessStateChanged() {
	auto *item = itemForProcessSender();
	if ( !item ) {
		return;
	}

	updateItemState(item);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onStopClicked() {
	auto *selectionModel = _ui.table->selectionModel();
	if ( selectionModel ) {
		for ( auto &index : selectionModel->selectedRows() ) {
			const auto *item = itemForProxyIndex(index);
			if ( item ) {
				stop(item->process);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onContinueClicked() {
	auto *selectionModel = _ui.table->selectionModel();
	if ( selectionModel ) {
		for ( auto &index : selectionModel->selectedRows() ) {
			const auto *item = itemForProxyIndex(index);
			if ( item ) {
				continue_(item->process);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onTerminateClicked() {
	auto *selectionModel = _ui.table->selectionModel();
	if ( selectionModel ) {
		for ( auto &index : selectionModel->selectedRows() ) {
			const auto *item = itemForProxyIndex(index);
			if ( item ) {
				terminate(item->process);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onKillClicked() {
	auto *selectionModel = _ui.table->selectionModel();
	if ( selectionModel ) {
		for ( auto &index : selectionModel->selectedRows() ) {
			const auto *item = itemForProxyIndex(index);
			if ( item ) {
				kill(item->process);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onRemoveClicked() {
	auto *selectionModel = _ui.table->selectionModel();
	if ( !selectionModel ) {
		return;
	}

	QList<const Item*> itemsToRemove;
	QList<int> rowsToRemove;
	for ( auto &index : selectionModel->selectedRows() ) {
		const auto *item = itemForProxyIndex(index);
		if ( !item || item->process->state() != QProcess::NotRunning ) {
			continue;
		}

		itemsToRemove.append(item);
		rowsToRemove.append(_model->row(item));
	}

	// Sort indexes in descending order to avoid changing the index
	std::sort(rowsToRemove.begin(), rowsToRemove.end(), std::greater<>());
	for ( auto row : rowsToRemove ) {
		_model->removeRow(row);
	}

	for ( const auto *item : itemsToRemove ) {
		_erroneous.remove(item->process);
		_running.remove(item->process);
		_items.remove(item->process);
		delete item;
	}

	updateControls();

	if ( !itemsToRemove.isEmpty() ) {
		emit stateChanged();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onClearClicked() {
	QList<const Item*> itemsToRemove;
	for ( int i = _model->rowCount()-1; i >= 0; --i ) {
		const auto *item = _model->item(i);
		if ( !item || item->process->state() != QProcess::NotRunning ) {
			continue;
		}

		itemsToRemove.append(item);
		_model->removeRow(i);
	}

	for ( const auto *item : itemsToRemove ) {
		_erroneous.remove(item->process);
		_running.remove(item->process);
		_items.remove(item->process);
		delete item;
	}

	updateControls();

	if ( !itemsToRemove.isEmpty() ) {
		emit stateChanged();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onProgressAnimationChanged(const QVariant &value) {
	_model->setProgressAngle(value.toDouble());
	for ( auto *process : std::as_const(_running) ) {
		auto *item = _items[process];
		if ( item ) {
			auto row = _model->row(item);
			_model->emitDataChanged(row, Model::ColState,
			                        { Qt::DecorationRole });
			_model->emitDataChanged(row, Model::ColRuntime,
			                        { Qt::DecorationRole, Qt::ToolTipRole });
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::onTableContextMenuRequested(const QPoint &pos) {
	QMenu menu;

	QAction *actionCopyCellClipboard = menu.addAction("Copy cell to clipboard");
	QAction *actionCopyToClipboard = menu.addAction("Copy selected rows to clipboard");

	QAction *result = menu.exec(_ui.table->mapToGlobal(pos));

	if ( result == actionCopyCellClipboard ) {
		QClipboard *cb = QApplication::clipboard();
		if ( cb ) {
			int column = _ui.table->columnAt(pos.x());
			int row = _ui.table->rowAt(pos.y());
			auto proxyIndex = _proxyModel->index(row, column);
			cb->setText(_proxyModel->data(proxyIndex).toString());
		}
	}
	else if ( result == actionCopyToClipboard ) {
		SCApp->copyToClipboard(_ui.table);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::init() {
	_ui.setupUi(this);

	_model = new Model(_ui.table);

	_proxyModel = new QSortFilterProxyModel(_ui.table);
	_proxyModel->setSourceModel(_model);
	_proxyModel->setDynamicSortFilter(true);
	_proxyModel->sort(Model::ColStarted, Qt::DescendingOrder);

	// table
	_ui.table->setModel(_proxyModel);

	_ui.table->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui.table->setFocus(Qt::TabFocusReason);
	_ui.table->setMouseTracking(true);
	_ui.table->setSelectionBehavior(QAbstractItemView::SelectRows);
	_ui.table->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_ui.table->resizeColumnToContents(Model::ColState);

	_ui.table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	_ui.table->horizontalHeader()->setSectionsMovable(true);
	_ui.table->horizontalHeader()->setSortIndicatorShown(true);
	_ui.table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	_ui.table->horizontalHeader()->setSortIndicator(Model::ColStarted, Qt::DescendingOrder);
	_ui.table->verticalHeader()->hide();

	connect(_ui.table->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
	        _ui.table, SLOT(sortByColumn(int,Qt::SortOrder)));
	connect(_ui.table->selectionModel(), &QItemSelectionModel::currentChanged,
	        this, &ProcessManager::onCurrentChanged);
	connect(_ui.table->selectionModel(), &QItemSelectionModel::selectionChanged,
	        this, &ProcessManager::onSelectionChanged);
	connect(_model, &QAbstractItemModel::dataChanged,
	        this, &ProcessManager::onDataChanged);
	connect(_ui.table, &QTableView::customContextMenuRequested,
	        this, &ProcessManager::onTableContextMenuRequested);

	// buttons
	_ui.btnStop->setIcon(icon("process_pause"));
	_ui.btnContinue->setIcon(icon("process_continue"));
	_ui.btnTerminate->setIcon(icon("process_terminate"));
	_ui.btnKill->setIcon(icon("process_kill"));
	_ui.btnRemove->setIcon(icon("process_remove"));
	_ui.btnClear->setIcon(icon("process_clean"));

	connect(_ui.btnStop, &QPushButton::clicked,
	        this, &ProcessManager::onStopClicked);
	connect(_ui.btnContinue, &QPushButton::clicked,
	        this, &ProcessManager::onContinueClicked);
	connect(_ui.btnTerminate, &QPushButton::clicked,
	        this, &ProcessManager::onTerminateClicked);
	connect(_ui.btnKill, &QPushButton::clicked,
	        this, &ProcessManager::onKillClicked);
	connect(_ui.btnRemove, &QPushButton::clicked,
	        this, &ProcessManager::onRemoveClicked);
	connect(_ui.btnClear, &QPushButton::clicked,
	        this, &ProcessManager::onClearClicked);

	// setup animation time for updating spinner icon an process runtime
	_progressAnimation.setStartValue(0.0);
	_progressAnimation.setEndValue(360.0);
	_progressAnimation.setLoopCount(-1);
	_progressAnimation.setDuration(1500);
	connect(&_progressAnimation, &QVariantAnimation::valueChanged,
	        this, &ProcessManager::onProgressAnimationChanged);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::updateControls() {
	_ui.btnStop->setEnabled(false);
	_ui.btnContinue->setEnabled(false);
	_ui.btnTerminate->setEnabled(false);
	_ui.btnKill->setEnabled(false);
	_ui.btnRemove->setEnabled(false);
	_ui.btnClear->setEnabled(_running.size() < _items.size());

	auto *selectionModel = _ui.table->selectionModel();
	if ( selectionModel ) {
		// stop/continue/terminate/kill buttons enabled if running process is found
		// Note: stop and continue buttons are not tested separately against
		// item->stopped since SIGSTOP and SIGCONT could have been sent
		// directly to the process outside the control of the process manager and the
		// user should get the option send the signals in this case.
		for ( auto &index : selectionModel->selectedRows() ) {
			const auto *item = itemForProxyIndex(index);
			if ( item && item->process->state() == QProcess::Running ) {
				_ui.btnStop->setEnabled(true);
				_ui.btnContinue->setEnabled(true);
				_ui.btnTerminate->setEnabled(true);
				_ui.btnKill->setEnabled(true);
				break;
			}
		}

		// remove button enabled if not running process is found
		for ( auto &index : selectionModel->selectedRows() ) {
			const auto *item = itemForProxyIndex(index);
			if ( item && item->process->state() != QProcess::Running ) {
				_ui.btnRemove->setEnabled(true);
				break;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::updateItemState(Item *item) {
	auto processCommand = [](const QProcess *process) {
		QString program = process->program();
		QStringList formattedArgs;
		auto args = process->arguments();
		for ( QString arg : std::as_const(args) ) {
			arg.replace("\"", "\\\"");
			if ( arg.contains(" ") || arg.contains("\"") ) {
				arg = "\"" + arg + "\"";
			}

			formattedArgs << arg;
		}

		if ( formattedArgs.empty() ) {
			return program;
		}

		return QString("%1 %2").arg(program, formattedArgs.join(" "));
	};

	bool erroneous = false;
	switch ( item->process->state() ) {
		case QProcess::Starting:
			item->started = Core::Time::UTC();
			addLog(item, *item->started,
			       QString("Starting: %1").arg(processCommand(item->process)));
			break;

		case QProcess::Running:
			if ( !item->running ) {
				item->running = Core::Time::UTC();
				addLog(item, *item->running,
				       QString("Started (PID: %1)").arg(item->process->processId()));
			}
			break;

		case QProcess::NotRunning:
			// not yet started
			if ( !item->started ) {
				break;
			}

			item->finished = Core::Time::UTC();
			erroneous = true;
			QString msg;
			switch ( item->state() ) {
				case Item::Crashed:
					msg = QString("Crashed");
					break;
				case Item::FailedToStart:
					msg = QString("Failed to start");
					break;
				case Item::Killed:
					msg = QString("Finished on exit code %1 after receiving "
					              "kill signal")
					      .arg(item->process->exitCode());
					break;
				case Item::Terminated:
					msg = QString("Finished on exit code %1 after receiving "
					              "terminate signal")
					      .arg(item->process->exitCode());
					break;
				case Item::ExitOnError:
					msg = QString("Exited on error code %1")
					      .arg(item->process->exitCode());
					break;
				case Item::Success:
					msg = QString("Finished successfully");
					erroneous = false;
					break;
				default:
					msg = QString("Unknown state: %1").arg(item->state());
			}

			addLog(item, *item->finished, msg);
	}

	// update set of running processes
	qsizetype stopped = 0;
	if ( item->process->state() == QProcess::Running ) {
		_running.insert(item->process);
		if ( item->stopped ) {
			++stopped;
		}
	}
	else {
		_running.remove(item->process);
	}

	// update set of erroneous processes
	if ( erroneous ) {
		_erroneous.insert(item->process);
	}
	else {
		_erroneous.remove(item->process);
	}

	// start/stop animation depending on running process set
	if ( _running.isEmpty() || _running.size() == stopped ) {
		_progressAnimation.stop();
	}
	else if ( _progressAnimation.state() != QAbstractAnimation::Running ) {
		_progressAnimation.start();
	}

	_model->emitDataChanged(_model->row(item));
	updateControls();

	emit stateChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::addConsoleOutput(QTextEdit *textEdit, const QString &text,
                                      int maxChars) {
	if ( !maxChars ) {
		textEdit->clear();
		return;
	}

	auto *scrollBar = textEdit->verticalScrollBar();
	bool isScrollMax = scrollBar->value() == scrollBar->maximum();

	// no char limits, just append
	if ( maxChars < 0 ) {
		textEdit->insertPlainText(text);
	}
	else {
		// new text exceeds limits, truncate new text
		if ( text.length() >= maxChars ) {
			textEdit->setPlainText(text.mid(text.length() - maxChars));
		}
		else {
			auto currentText = textEdit->toPlainText();
			int excessLength = currentText.length() + text.length() - maxChars;
			// new and current text exceed limits, truncate current text
			if ( excessLength > 0 ) {
				textEdit->setPlainText(currentText.mid(excessLength));
			}
			textEdit->insertPlainText(text);
		}
	}

	if ( isScrollMax ) {
		scrollBar->setValue(scrollBar->maximum());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessManager::addLog(const Item *item, const Core::Time &time,
                            const QString &message) {
	const char *lineTmpl = R"(<span title="%1">[%2]</span> %3<br>)";
	auto html = QString(lineTmpl)
	            .arg(QString::fromStdString(time.iso()),
	                 QString::fromStdString(time.toString("%T")),
	                 message.toHtmlEscaped());
	item->processLog->insertHtml(html);

	emit logEntryAdded(message, item->process, item->userData);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ProcessManager::Item *ProcessManager::itemForProcess(QProcess *process) const {
	if ( !_items.contains(process) ) {
		SEISCOMP_ERROR("Process not managed by this instance");
		return {};
	}

	return _items[process];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ProcessManager::Item *ProcessManager::itemForProcessSender() const {
	auto *process = dynamic_cast<QProcess*>(sender());
	if ( !process ) {
		SEISCOMP_ERROR("Sender not of Type QProcess");
		return {};
	}

	return itemForProcess(process);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ProcessManager::Item *
ProcessManager::itemForProxyIndex(const QModelIndex &index) const {
	return _model->item(_proxyModel->mapToSource(index).row());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QModelIndex ProcessManager::proxyIndexForItem(const Item *item) const {
	auto row = _model->row(item);
	QModelIndex sourceIndex = _model->index(row, 0);
	return _proxyModel->mapFromSource(sourceIndex);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ProcessStateLabel::ProcessStateLabel(ProcessManager *manager, QWidget *parent)
: SpinningLabel(parent), _manager(manager) {

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessStateLabel::mousePressEvent(QMouseEvent *event) {
	if ( event->button() == Qt::LeftButton ) {
		if ( _manager->isHidden() ) {
			_manager->show();
		}
		if ( _manager->isMinimized() ) {
			_manager->showNormal();
		}
		if ( !_manager->isActiveWindow() ) {
			_manager->activateWindow();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessStateLabel::onProcessStateChanged() {
	if ( !_manager ) {
		return;
	}

	if ( _manager->processCount() ) {
		show();
	}
	else {
		hide();
	}

	QString toolTip;
	int procCount = _manager->processCount();
	if ( _manager->runningCount() ) {
		toolTip = QString("%1 of %2 running").arg(_manager->runningCount())
		                                     .arg(procCount);
		if ( !_shouldRun ) {
			setPixmap(_progressPixmap);
			start();
		}
	}
	else {
		if ( procCount == 1 ) {
			toolTip = QString("1 process");
		}
		else {
			toolTip = QString("%1 processes").arg(procCount);
		}

		if ( _shouldRun ) {
			stop();
		}
		setPixmap(_manager->erroneousCount() ? _erroneousPixmap
		                                     : _defaultPixmap);
	}

	if ( _manager->erroneousCount() ) {
		toolTip = QString("%1, %2 erroneous").arg(toolTip)
		                                     .arg(_manager->erroneousCount());
	}
	setToolTip(toolTip);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ProcessStateLabel::init() {
	_defaultPixmap = Gui::pixmap(this, "process_manager");
	_progressPixmap = Gui::pixmap(this, "process_progress");
	_erroneousPixmap = Gui::pixmap(this, "process_attention");

	connect(_manager, &ProcessManager::stateChanged,
	        this, &ProcessStateLabel::onProcessStateChanged);

	onProcessStateChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace Seiscomp::Gui
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
