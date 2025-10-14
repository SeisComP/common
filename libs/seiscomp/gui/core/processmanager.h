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


#ifndef SEISCOMP_GUI_PROCESSMANAGER_H
#define SEISCOMP_GUI_PROCESSMANAGER_H


#include <seiscomp/gui/core/ui_processmanager.h>

#include <seiscomp/core/datetime.h>

#include <seiscomp/gui/core/spinninglabel.h>
#include <seiscomp/gui/qt.h>

#include <QAbstractTableModel>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QProcess>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTextEdit>
#include <QVariantAnimation>
#include <QVector>


namespace Seiscomp::Gui {

class ProcessStateLabel;


class SC_GUI_API ProcessManager : public QMainWindow {
	Q_OBJECT

	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	public:
		explicit ProcessManager(QWidget *parent = nullptr,
		                        Qt::WindowFlags f = Qt::WindowFlags());

	// ------------------------------------------------------------------
	// Protected Interface
	// ------------------------------------------------------------------
	public:
		/**
		 * @brief Creates a process with the given name, description and icon.
		 * The process is managed by this instance.
		 * @param name Name of the process
		 * @param description Description of the process shown as tool tip
		 * @param icon Icon to be shown next to the name
		 * @return QProcess instance mananged by this instance
		 */
		QProcess *createProcess(QString name, QString description={},
		                        QIcon icon={});

		/**
		 * @brief Wait for the process to start. If the start up fails logging
		 * entries are created, the corresponding item is selected in the
		 * process table and the process manager is shown if not already active.
		 * @param process The process to wait to start for
		 * @param timeout Start timeout in milliseconds
		 * @return True if the process could be launched
		 */
		bool waitForStarted(QProcess *process, int timeout = 5000);

		/**
		 * @brief Registers output streams to which the stdout and stderr data
		 * of the given process are written to in addition to the console tabs
		 * visible in the process manager.
		 * @param process The process to register the output streams for
		 * @param osOut The output stream used for stdout data
		 * @param osErr The output stream used for stderr data
		 * @return True if the process is managed by the process manager
		 */
		bool setOutputStreams(QProcess *process, std::ostream *osOut,
		                      std::ostream *osErr=nullptr);

		/**
		 * @brief Limits the data written to the console tabs visible in the
		 * process manager.
		 * @param process The process to implement limits for
		 * @param charsOut Maximum number of characters shown in the stdout tab.
		 * Use a negative number to remove any limit. Use 0 to disable any
		 * output to the stdout tab.
		 * @param charsErr Maximum number of characters shown in the stderr tab.
		 * Use a negative number to remove any limit. Use 0 to disable any
		 * output to the stderr tab.
		 * @return True if the process is managed by the process manager
		 */
		bool setConsoleLimits(QProcess *process, int charsOut,
		                      int charsErr = -1);

		/**
		 * @brief Create a log entry for the process.
		 * @param process The process to log a message for
		 * @param message The message to add to the process log
		 */
		void log(QProcess *process, const QString &message);

		/**
		 * @brief Return total number of processes.
		 * @return Total number of processes
		 */
		int processCount();

		/**
		 * @brief Return number of running processes.
		 * @return Number of running
		 */
		int runningCount();

		/**
		 * @brief Return number of erroneous processes. This includes processes
		 * that couldn't bestarted, that crashed or exited on error code other
		 * than 0.
		 * @return Number of erroneous processes
		 */
		int erroneousCount();

		/**
		 * @brief Stop execution of the process by sending the SIGSTOP (19)
 		 * signal. The process may be resumed via SIGCONT (18) later on.
		 * @param process Process to terminate
		 * @return True if the SIGSTOP signal could be sent
		 */
		bool stop(QProcess *process);

		/**
		 * @brief Continue execution of the process by sending the SIGCONT (18)
		 * signal. The process must have been stopped via SIGSTOP (19) before.
		 * @param process Process to continue
		 * @return True if the SIGCONT signal could be sent
		 */
		bool continue_(QProcess *process);

		/**
		 * @brief Terminate the process gracefully by sending the SIGTERM (9)
		 * signal.
		 * @param process Process to terminate
		 * @return True if the SIGTERM signal could be sent
		 */
		bool terminate(QProcess *process);

		/**
		 * @brief Forcefully kill the process by sending the SIGKILL (15)
		 * signal.
		 * @param process Process to kill
		 * @return True if the SIGKILL signal could be sent
		 */
		bool kill(QProcess *process);

		// friend class ProcessStateLabel;


	// ------------------------------------------------------------------
	// Signals
	// ------------------------------------------------------------------
	signals:
		// emitted if a process was added/removed or any of the managed
		// processes changed state
		void stateChanged();


	// ------------------------------------------------------------------
	// Protected Slots
	// ------------------------------------------------------------------
	protected slots:
		void onDataChanged(const QModelIndex &topLeft,
		                   const QModelIndex &bottomRight,
		                   const QVector<int> &roles = QVector<int>());
		void onCurrentChanged(const QModelIndex &current,
		                      const QModelIndex &previous);
		void onSelectionChanged(const QItemSelection &selected,
		                        const QItemSelection &deselected);
		void onProcessReadyReadStandardOutput();
		void onProcessReadyReadStandardError();
		void onProcessStateChanged();
		void onStopClicked();
		void onContinueClicked();
		void onTerminateClicked();
		void onKillClicked();
		void onRemoveClicked();
		void onClearClicked();
		void onProgressAnimationChanged(const QVariant &value);
		void onTableContextMenuRequested(const QPoint &pos);


	// ------------------------------------------------------------------
	// Protected Interface
	// ------------------------------------------------------------------
	protected:
		// forward declaration
		struct Item;
		class Model;

		void init();
		void updateControls();
		void updateItemState(Item *item);

		static void addConsoleOutput(QTextEdit *textEdit, const QString &text,
		                             int maxChars = -1);
		static void addLog(const Item *item, const Core::Time &time,
		            const QString &message);


	// ------------------------------------------------------------------
	// Protected data members
	// ------------------------------------------------------------------
	protected:
		Ui::ProcessManager      _ui;
		Model                  *_model{nullptr};
		QSortFilterProxyModel  *_proxyModel{nullptr};
		QMap<QProcess*, Item*>  _items;
		QSet<QProcess*>         _running;
		QSet<QProcess*>         _erroneous;
		QVariantAnimation       _progressAnimation;

	// ------------------------------------------------------------------
	// Private methods
	// ------------------------------------------------------------------
	private:
		Item *itemForProcess(QProcess *process) const;
		Item *itemForProcessSender() const;
		inline const Item *itemForProxyIndex(const QModelIndex &index) const;
		inline QModelIndex proxyIndexForItem(const Item *item) const;
};


class ProcessStateLabel : public SpinningLabel {
	Q_OBJECT

	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	public:
		ProcessStateLabel(ProcessManager *manager, QWidget *parent = nullptr);

	// ------------------------------------------------------------------
	// Protected Slots
	// ------------------------------------------------------------------
	protected slots:
		void mousePressEvent(QMouseEvent *event) override;
		void onProcessStateChanged();

	// ------------------------------------------------------------------
	// Protected Interface
	// ------------------------------------------------------------------
	protected:
		void init();

	// ------------------------------------------------------------------
	// Protected data members
	// ------------------------------------------------------------------
	protected:
		ProcessManager *_manager{nullptr};
		QPixmap _defaultPixmap;
		QPixmap _progressPixmap;
		QPixmap _erroneousPixmap;
};


} // namespace Seiscomp::Gui

#endif
