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


#ifndef SEISCOMP_GUI_LOGMANAGER_H
#define SEISCOMP_GUI_LOGMANAGER_H


#include <seiscomp/gui/qt.h>

#ifndef Q_MOC_RUN
#include <seiscomp/logging/output.h>
#endif

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QWidget>


class QStandardItem;
class QStandardItemModel;


namespace Seiscomp::Gui {


class LogManagerPrivate;

class SC_GUI_API LogManager : public QMainWindow, public Seiscomp::Logging::Output {
	Q_OBJECT

	struct LogEntry;

	// ------------------------------------------------------------------
	// X'truction
	// ------------------------------------------------------------------
	public:
		LogManager(QWidget *parent = nullptr);
		~LogManager() override;


	// ------------------------------------------------------------------
	// Public interface
	// ------------------------------------------------------------------
	public:
		int level() const { return _level; };


	// ------------------------------------------------------------------
	//  Logging::Output interface
	// ------------------------------------------------------------------
	public:
		bool setup(const Seiscomp::Util::Url &url) override;
		void log(const char *channelName, Logging::LogLevel level, const char *msg,
		         time_t time, uint32_t microseconds) override;
		/**
		 * @brief Returns the widget representing the log data
		 * @return QWidget representing the data
		 */
		QWidget *dataView();


	// ------------------------------------------------------------------
	//  Logging::Output interface
	// ------------------------------------------------------------------
	protected:
		/**
		 * @brief restoreSettings Restores last session settings
		 */
		void restoreSettings();

		/**
		 * @brief saveSettings Save current session settings
		 *        e.g. Log level selection or sort order
		 */
		void saveSettings();

		/**
		 * @brief This method is called when the widget is shown. If
		 *        the widget is shown the first time we resize the columns
		 *        of the view to the available content
		 * @param event The show event
		 */
		void showEvent(QShowEvent *event) override;

		void setLevel(int level = Logging::LogLevel::LL_QUANTITY);
		bool eventFilter(QObject *obj, QEvent *ev) override;
		void subscribe();


	// ------------------------------------------------------------------
	//  Signals
	// ------------------------------------------------------------------
	signals:
		void levelChanged(int level);
		void newLog(const LogEntry &entry);


	// ------------------------------------------------------------------
	//  Slots
	// ------------------------------------------------------------------
	public slots:
		void activate();
		void clearView();

	protected slots:
		/**
		 * @brief This slot is called when the user selects/deselects a log level.
		 *  Applies the current selection as filter to the table.
		 */
		void logLevelSelectionChanged();

		/**
		 * @brief Copies the data of the current selected rows to clipboard.
		 */
		void copyLog();

		/**
		* @brief onContextMenuRequested Shows the table context menu
		* @param pos The mouse position
		*/
		void onContextMenuRequested(const QPoint &pos);

		/**
		 * @brief Filters the table based on the given filter
		 * @param filter The filter string
		 */
		void filterChanged(const QString &filter);


		/**
		 * @brief Enables/Disables the copy row action based on the current table
		 *        selection.
		 */
		void selectionChanged();

		void showSettings();

	private slots:
		void addLog(const LogEntry &entry);


	// ------------------------------------------------------------------
	//  Members
	// ------------------------------------------------------------------
	protected:
		int _level{Logging::LogLevel::LL_QUANTITY};

	private:
		LogManagerPrivate *_d_ptr;
};


class LogStateLabel : public QLabel {
	Q_OBJECT

	// ------------------------------------------------------------------
	// X'truction
	// ------------------------------------------------------------------
	public:
		LogStateLabel(LogManager *manager, QWidget *parent = nullptr);

	// ------------------------------------------------------------------
	// Protected Slots
	// ------------------------------------------------------------------
	protected slots:
		void mousePressEvent(QMouseEvent *event) override;
		void logStateChanged();

	// ------------------------------------------------------------------
	// Protected Interface
	// ------------------------------------------------------------------
	protected:
		void init();

	// ------------------------------------------------------------------
	// Protected data members
	// ------------------------------------------------------------------
	protected:
		LogManager *_manager{nullptr};
};


} // ns Seiscomp::Gui


#endif
