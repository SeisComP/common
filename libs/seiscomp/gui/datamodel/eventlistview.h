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



#ifndef SEISCOMP_GUI_ORIGINLISTVIEW_H
#define SEISCOMP_GUI_ORIGINLISTVIEW_H

#include <seiscomp/gui/core/connectiondialog.h>
#include <seiscomp/gui/core/utils.h>
#include <seiscomp/gui/qt.h>
#ifndef Q_MOC_RUN
#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/timewindow.h>
#include <seiscomp/geo/boundingbox.h>
#endif

#include <QWidget>


// Ui forward declarations
class QTreeWidget;
class QTreeWidgetItem;

namespace Ui {
	class EventListView;
	class EventListViewRegionFilterDialog;
}


namespace Seiscomp {

namespace DataModel {


DEFINE_SMARTPOINTER(Event);
DEFINE_SMARTPOINTER(Origin);
DEFINE_SMARTPOINTER(FocalMechanism);
DEFINE_SMARTPOINTER(Pick);
DEFINE_SMARTPOINTER(Station);
DEFINE_SMARTPOINTER(Amplitude);
class OriginReference;
class DatabaseQuery;
class Notifier;

}

namespace Client {

DEFINE_SMARTPOINTER(Connection);

}

namespace Geo {

DEFINE_SMARTPOINTER(GeoFeature);

}

namespace Gui {


namespace Private {

class EventTreeItem;
class OriginTreeItem;
class FocalMechanismTreeItem;
class EventFilterWidget;

}


class CommandMessage;


class SC_GUI_API EventListView : public QWidget {
	Q_OBJECT

	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef QMap<QString, DataModel::StationPtr> StationMap;

		struct Filter {
			Filter(const Seiscomp::Core::TimeWindow& tw = Seiscomp::Core::TimeWindow())
			   : startTime(tw.startTime()), endTime(tw.endTime()) {}
			Seiscomp::Core::Time     startTime;
			Seiscomp::Core::Time     endTime;
			OPT(float)               minLatitude, maxLatitude;
			OPT(float)               minLongitude, maxLongitude;
			OPT(float)               minDepth, maxDepth;
			OPT(float)               minMagnitude, maxMagnitude;
			std::string              eventID;

			bool isNull() const;
		};


	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		EventListView(Seiscomp::DataModel::DatabaseQuery* reader,
		              bool withOrigins = true, bool withFocalMechanisms = false,
		              QWidget * parent = 0, Qt::WindowFlags f = 0);
		~EventListView();


	public:
		void setRelativeMinimumEventTime(const Seiscomp::Core::TimeSpan&);
		void add(Seiscomp::DataModel::Event* event,
		         Seiscomp::DataModel::Origin* origin);

		//! Sets the usage of messaging to notify about new objects or updates.
		//! When disabled a local EventParameters instance is searched for and
		//! all updates and adds are applied to this instance.
		void setMessagingEnabled(bool);
		void setEventModificationsEnabled(bool);
		void setSortingEnabled(bool);
		void moveSection(int, int);

		void setControlsHidden(bool);
		void setCustomControls(QWidget*) const;
		void setFMLinkEnabled(bool);

		QList<Seiscomp::DataModel::Event*> selectedEvents();

		QTreeWidget *eventTree() { return _treeWidget; }
		Seiscomp::DataModel::Event *eventFromTreeItem(QTreeWidgetItem *item) const;

		int eventCount() const;

		//! \since 15.0.0
		int visibleEventCount() const;


	signals:
		void originAdded();
		void focalMechanismAdded();

		void originSelected(Seiscomp::DataModel::Origin*,
		                    Seiscomp::DataModel::Event* = nullptr);
		void originUpdated(Seiscomp::DataModel::Origin*);

		void focalMechanismSelected(Seiscomp::DataModel::FocalMechanism*,
		                            Seiscomp::DataModel::Event* = nullptr);
		void focalMechanismUpdated(Seiscomp::DataModel::FocalMechanism*);

		void eventSelected(Seiscomp::DataModel::Event*);
		void eventFMSelected(Seiscomp::DataModel::Event*);
		void originReferenceAdded(const std::string &, Seiscomp::DataModel::OriginReference*);

		//! Emitted when the event list is cleared
		void reset();
		void eventAddedToList(Seiscomp::DataModel::Event*, bool fromNotification);
		void eventUpdatedInList(Seiscomp::DataModel::Event*);
		void eventRemovedFromList(Seiscomp::DataModel::Event*);

		//! \since 15.0.0
		void visibleEventCountChanged();

		//! Emitted when a bigger update process has finished, such as show/hide
		//! of a subset of events.
		void eventsUpdated();


	public slots:
		void setAutoSelect(bool);
		void onCommand(Seiscomp::Gui::CommandMessage* cmsg);
		void messageAvailable(Seiscomp::Core::Message*, Seiscomp::Client::Packet*);
		void notifierAvailable(Seiscomp::DataModel::Notifier*);

		void updateOrigin(Seiscomp::DataModel::Origin* origin);
		void insertOrigin(Seiscomp::DataModel::Origin* origin,
		                  Seiscomp::DataModel::Event* baseEvent,
		                  const Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick>&,
		                  const std::vector<Seiscomp::DataModel::AmplitudePtr>&);

		void updateFocalMechanism(Seiscomp::DataModel::FocalMechanism *fm);
		void insertFocalMechanism(Seiscomp::DataModel::FocalMechanism *fm,
		                          Seiscomp::DataModel::Event *event,
		                          Seiscomp::DataModel::Origin *origin = 0);

		void setInterval(const Seiscomp::Core::TimeWindow&);

		void selectFirstEnabledEvent();
		void selectEvent(int index);
		void selectEventID(const std::string& publicID);

		/**
		 * @brief Activates the previous visible event.
		 * This is similar to selecting the event before (visually below) the
		 * current event in the list.
		 */
		void setPreviousEvent();

		/**
		 * @brief Activates the next visible event.
		 * This is similar to selecting the event after (visually above) the
		 * current event in the list.
		 */
		void setNextEvent();

		void readFromDatabase();
		void readFromDatabase(const Seiscomp::Gui::EventListView::Filter&);
		void clear();
		void clearDatabaseFilter();

		void selectEventFM(const QString &);

	protected slots:
		void itemSelected(QTreeWidgetItem*,int);
		void itemPressed(QTreeWidgetItem*,int);
		void copyRowToClipboard();

		void readLastDays();
		void readInterval();

		void onShowOtherEvents(int checked);
		void onShowForeignEvents(int checked);
		void onHideOutsideRegion(int checked);
		void onFilterRegionModeChanged(int mode);

		void updateAgencyState();

	private slots:
		void sortItems(int);
		void headerContextMenuRequested(const QPoint &);
		void waitDialogDestroyed(QObject *o);

		void regionSelectionChanged(int index);
		void changeRegion();

		void itemEntered(QTreeWidgetItem *item, int);
		void itemExpanded(QTreeWidgetItem * item);
		void currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
		void indicatorResized(const QSize &size);

		void evalResultAvailable(const QString &publicID,
		                         const QString &className,
		                         const QString &script,
		                         const QString &result);

		void evalResultError(const QString &publicID,
		                     const QString &className,
		                     const QString &script,
		                     int error);


	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		void initTree();

		Private::EventTreeItem* addEvent(Seiscomp::DataModel::Event*, bool fromNotification);
		Private::OriginTreeItem* addOrigin(Seiscomp::DataModel::Origin*, QTreeWidgetItem* parent, bool highPriority);
		Private::FocalMechanismTreeItem* addFocalMechanism(Seiscomp::DataModel::FocalMechanism*, QTreeWidgetItem* parent);

		Private::EventTreeItem* findEvent(const std::string&);
		Private::OriginTreeItem* findOrigin(const std::string&);
		Private::FocalMechanismTreeItem* findFocalMechanism(const std::string&);

		void removeExpiredEvents();
		void updateHideState();
		bool updateHideState(QTreeWidgetItem *item);

		void updateOriginProcessColumns(QTreeWidgetItem *item, bool highPriority);
		void updateEventProcessColumns(QTreeWidgetItem *item, bool highPriority);

		bool sendJournalAndWait(const std::string &objectID,
		                        const std::string &action,
		                        const std::string &params, const char *group);

		void loadItem(QTreeWidgetItem*);


	public:
		struct ProcessColumn {
			int     pos;
			QString script;
		};

		struct ItemConfig {
			ItemConfig() : createFMLink(false) {}

			QColor                    disabledColor;

			bool                      createFMLink;
			QStringList               header;
			QVector<int>              columnMap;
			int                       customColumn;
			std::string               originCommentID;
			std::string               eventCommentID;
			QString                   customDefaultText;
			QMap<std::string, QColor> customColorMap;
			QVector<ProcessColumn>    originScriptColumns;
			QVector<ProcessColumn>    eventScriptColumns;
			QSet<int>                 eventScriptPositions;
			QHash<QString, int>       originScriptColumnMap;
			QHash<QString, int>       eventScriptColumnMap;
			QSet<int>                 hiddenEventTypes;
			QSet<QString>             preferredAgencies;
		};

		struct Region {
			QString                name;
			Geo::GeoBoundingBox    bbox;
			const Geo::GeoFeature *poly{nullptr};
		};

		typedef QList<Region> FilterRegions;

	private:
		::Ui::EventListView                *_ui;
		Private::EventFilterWidget         *_filterWidget;
		ItemConfig                          _itemConfig;
		FilterRegions                       _filterRegions;
		QTreeWidget                        *_treeWidget;
		QTreeWidgetItem                    *_unassociatedEventItem;
		QWidget                            *_commandWaitDialog;
		QMovie                             *_busyIndicator;
		QLabel                             *_busyIndicatorLabel;
		//StationMap                        _associatedStations;
		Seiscomp::DataModel::DatabaseQuery *_reader;
		Seiscomp::Core::TimeSpan            _timeAgo;
		Filter                              _filter;
		bool                                _autoSelect;
		bool                                _withOrigins;
		bool                                _withFocalMechanisms;
		bool                                _updateLocalEPInstance;
		//bool                              _withComments;
		bool                                _blockSelection;
		bool                                _blockRemovingOfExpiredEvents;
		bool                                _blockCountSignal;
		bool                                _hideOtherEvents;
		bool                                _hideForeignEvents;
		bool                                _hideOutsideRegion;
		bool                                _checkEventAgency;
		bool                                _showOnlyLatestPerAgency;
		int                                 _regionIndex;
		mutable int                         _visibleEventCount;
};


class SC_GUI_API EventListViewRegionFilterDialog : public QDialog {
	Q_OBJECT

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		EventListViewRegionFilterDialog(QWidget *parent, EventListView::Region *target,
		                                EventListView::FilterRegions *regionList);
		~EventListViewRegionFilterDialog();


	// ------------------------------------------------------------------
	//  QDialog interface
	// ------------------------------------------------------------------
	public:
		virtual void accept();


	// ------------------------------------------------------------------
	//  Slots
	// ------------------------------------------------------------------
	private slots:
		void regionSelectionChanged(const QString &);
		void showError(const QString &);


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		::Ui::EventListViewRegionFilterDialog *_ui;
		EventListView::Region                 *_target;
		EventListView::FilterRegions          *_regionList;
};


}
}


#endif
