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


#ifndef SEISCOMP_GUI_ORIGINLOCATORVIEW_H
#define SEISCOMP_GUI_ORIGINLOCATORVIEW_H


#include <seiscomp/gui/core/diagramwidget.h>
#include <seiscomp/gui/datamodel/originlocatormap.h>
#include <seiscomp/gui/datamodel/pickerview.h>
#include <seiscomp/gui/map/mapwidget.h>
#ifndef Q_MOC_RUN
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/seismology/locatorinterface.h>
#include <seiscomp/core/baseobject.h>
#endif

#include <QAbstractItemModel>
#include <QStyledItemDelegate>

#include <set>


// Ui forward declarations
class QTreeWidgetItem;

namespace Ui {
	class OriginLocatorView;
	class FilterSettings;
}



namespace Seiscomp {

namespace DataModel {

DEFINE_SMARTPOINTER(Event);
DEFINE_SMARTPOINTER(Arrival);
DEFINE_SMARTPOINTER(Station);

}

namespace Gui {


class PickerView;
class RecordStreamThread;


class SC_GUI_API ArrivalModel : public QAbstractTableModel {
	Q_OBJECT

	public:
		struct Filter {
			virtual ~Filter() {}
			virtual bool accepts(int row, int idx, DataModel::Arrival *) const = 0;
		};

	public:
		ArrivalModel(DataModel::Origin* origin = nullptr, QObject *parent = 0);

	public:
		void setDisabledForeground(QColor c) { _disabledForeground = c; }

		void setOrigin(DataModel::Origin* origin);
		DataModel::Origin *origin() const;

		void setRowColor(int row, const QColor&);

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;

		QVariant data(const QModelIndex &index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const;

		Qt::ItemFlags flags(const QModelIndex &index) const;
		bool setData(const QModelIndex &index, const QVariant &value,
		             int role = Qt::EditRole);

		void setRowEnabled(int row, bool enabled);
		bool isRowEnabled(int row) const;

		void setDistance(int row, const QVariant &val);
		void setTakeOffAngle(int row, const QVariant &val);

		bool useNoArrivals() const;
		bool useArrival(int row) const;
		void setUseArrival(int row, DataModel::Arrival *arrival);

		bool backazimuthUsed(int row) const;
		void setBackazimuthUsed(int row, bool enabled);

		bool horizontalSlownessUsed(int row) const;
		void setHorizontalSlownessUsed(int row, bool enabled);

		bool timeUsed(int row) const;
		void setTimeUsed(int row, bool enabled);

		int  usedFlags(int row) const;

	private:
		static const int F_DISABLED = 1 << 31;

		DataModel::Origin *_origin;
		QVector<int>       _used;
		QVector<int>       _hoverState;
		QVector<QVariant>  _takeOffs;
		QVector<QVariant>  _distances;
		QVector<bool>      _enableState;
		QVector<QVariant>  _backgroundColors;
		QColor             _disabledForeground;
		QStringList        _header;
		std::string        _pickTimeFormat;
		std::string        _pickCTimeFormat;
};


class SC_GUI_API ArrivalDelegate : public QStyledItemDelegate {
	Q_OBJECT

	public:
		ArrivalDelegate(QWidget *parent = 0);

	public:
		void paint(QPainter * painter, const QStyleOptionViewItem & option,
		           const QModelIndex & index) const;

		bool editorEvent(QEvent *event, QAbstractItemModel *model,
		                 const QStyleOptionViewItem &option,
		                 const QModelIndex &index);

		QSize sizeHint(const QStyleOptionViewItem &option,
		               const QModelIndex &index) const;

	public slots:
		bool helpEvent(QHelpEvent *event, QAbstractItemView *view,
		               const QStyleOptionViewItem &option,
		               const QModelIndex &index);

	private:
		int         _flags[3];
		QString     _labels[3];
		int         _margin;
		int         _spacing;
		int         _statusRectWidth;
		mutable int _labelWidth;
};


class SC_GUI_API DiagramFilterSettingsDialog : public QDialog {
	Q_OBJECT

	public:
		struct Filter {
			virtual ~Filter() {};
			virtual bool accepts(DiagramWidget *w, int id) = 0;
		};


	public:
		DiagramFilterSettingsDialog(QWidget *parent = 0);
		~DiagramFilterSettingsDialog();

		Filter *createFilter() const;


	private slots:
		void filterChanged(int);

	private:
		::Ui::FilterSettings *_ui;
};


class SC_GUI_API OriginLocatorPlot : public DiagramWidget {
	Q_OBJECT

	public:
		OriginLocatorPlot(QWidget *parent = 0);


	signals:
		void focalMechanismCommitted(bool withMT = false,
		                             QPoint pos = QPoint(0, 0));


	protected slots:
		virtual void linkClicked();
		virtual void commitButtonClicked(bool);
		virtual void commitWithMTTriggered(bool);
};


class OriginLocatorViewPrivate;
class ProcessManager;


class SC_GUI_API OriginLocatorView : public QWidget {
	Q_OBJECT

	public:
		struct SC_GUI_API Config {
			double reductionVelocityP;
			bool   drawMapLines;
			bool   drawGridLines;
			bool   computeMissingTakeOffAngles;
			double defaultEventRadius;

			Config();
		};

		typedef std::vector<DataModel::PickPtr> PickList;
		typedef std::set<std::pair<DataModel::PickPtr, bool> > PickSet;
		typedef std::set<std::pair<DataModel::AmplitudePtr, bool> > AmplitudeSet;
		typedef std::set<std::string> StringSet;
		typedef std::map<std::string, DataModel::PickPtr> PickMap;


	public:
		OriginLocatorView(const MapsDesc &maps,
		                  const PickerView::Config &pickerConfig,
		                  QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
		OriginLocatorView(Map::ImageTree* mapTree,
		                  const PickerView::Config &pickerConfig,
		                  QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
		~OriginLocatorView();

	signals:
		void waveformsRequested();
		void eventListRequested();
		void locatorRequested();
		void computeMagnitudesRequested();

		void baseEventSet();
		void baseEventRejected();

		//! When a new origin has been set inside the view this signal
		//! is emitted. The bool flag signal whether a new origin has been
		//! created or not
		void newOriginSet(Seiscomp::DataModel::Origin *newOrigin,
		                  Seiscomp::DataModel::Event *event, bool localOrigin,
		                  bool relocated);
		void updatedOrigin(Seiscomp::DataModel::Origin* origin);
		void committedOrigin(Seiscomp::DataModel::Origin* origin,
		                     Seiscomp::DataModel::Event* baseEvent,
		                     const Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick>& changedPicks,
		                     const std::vector<Seiscomp::DataModel::AmplitudePtr>& newAmplitudes);
		void committedFocalMechanism(Seiscomp::DataModel::FocalMechanism *fm,
		                             Seiscomp::DataModel::Event *event,
		                             Seiscomp::DataModel::Origin *origin);

		void artificalOriginCreated(Seiscomp::DataModel::Origin*);

		void magnitudesAdded(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*);

		void undoStateChanged(bool undoState);
		void redoStateChanged(bool undoState);

		void requestRaise();

	public slots:
		void clear();

		void addObject(const QString& parentID, Seiscomp::DataModel::Object*);
		void updateObject(const QString& parentID, Seiscomp::DataModel::Object*);

		bool setOrigin(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*, bool = false);
		void addPick(Seiscomp::DataModel::Pick*);

		void setStationEnabled(const std::string& networkCode,
		                       const std::string& stationCode,
		                       bool state);

		void setMagnitudeCalculationEnabled(bool);
		void computeMagnitudes();
		void magnitudeRemoved(const QString &, Seiscomp::DataModel::Object*);
		void magnitudeSelected(const QString &, Seiscomp::DataModel::Magnitude*);

		void mergeOrigins(QList<Seiscomp::DataModel::Origin*>);
		void setLocalAmplitudes(Seiscomp::DataModel::Origin*, AmplitudeSet*, StringSet*);

		void drawStations(bool);
		void drawStationAnnotations(bool);

		bool undo();
		bool redo();

		void createArtificialOrigin();
		void createArtificialOrigin(const QPointF &epicenter,
		                            const QPoint &dialogPos);

	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		// Scripts
		void setScript0(const std::string&);
		void setScript1(const std::string&);

		// Configuration
		void setConfig(const Config&);
		const Config &config() const;

		/**
		 * @brief Adds a pick to the internal pick list which will be
		 *        sent along with the origin at commit.
		 * @param pick The pick to be managed with a smart pointer.
		 */
		void addLocalPick(Seiscomp::DataModel::Pick *pick);

		// Manual picker configuration
		void setPickerConfig(const PickerView::Config&);
		const PickerView::Config& pickerConfig() const;

		// Misc
		void setDatabase(Seiscomp::DataModel::DatabaseQuery*);
		void setPickerView(PickerView*);

		OriginLocatorMap* map() const;


	protected:
		void closeEvent(QCloseEvent *e);


	private slots:
		void editComment();
		void setOrigin(Seiscomp::DataModel::Origin*);
		void setCreatedOrigin(Seiscomp::DataModel::Origin*);

		void hoverArrival(int);
		void selectArrival(int);
		void selectStation(const std::string &, const std::string &);
		void residualsSelected();
		void adjustResidualsRect(QRectF&);

		void selectRow(const QModelIndex&, const QModelIndex&);

		void plotTabChanged(int);

		void zoomMap();

		void mapKeyPressed(QKeyEvent*);
		void objectDestroyed(QObject*);

		void importArrivals();
		void showWaveforms();
		void relocate();
		void commit(bool associate = true, bool ignoreDefaultEventType = false);
		void customCommit();
		void commitFocalMechanism(bool withMT = false, QPoint pos = QPoint(0, 0));
		void commitWithOptions();

		void tableArrivalsContextMenuRequested(const QPoint &pos);
		void tableArrivalsHeaderContextMenuRequested(const QPoint &pos);
		void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
		void changeArrival(int,bool);
		void changeArrivalEnableState(int,bool);
		void artificialOriginRequested(double lat, double lon, double depth, Seiscomp::Core::Time time);

		void updateBlinkState();

		void locatorProfileChanged(const QString &text);
		void locatorChanged(const QString &text);
		void configureLocator();

		void changePlotFilter();

		void runScript0();
		void runScript1();
		void commandStart();

		void evalResultAvailable(const QString &originID,
		                         const QString &className,
		                         const QString &script,
		                         const QString &result);

		void evalResultError(const QString &originID,
		                     const QString &className,
		                     const QString &script,
		                     int error);

		void evaluateOrigin(Seiscomp::DataModel::Origin *org,
		                    Seiscomp::DataModel::Event *event,
		                    bool localOrigin, bool relocated);

	private:
		struct PhasePickWithFlags {
			DataModel::PickPtr pick;
			std::string        phase;
			int                flags;
		};

		void init();

		void createArtificialOrigin(const QPointF &epicenter,
		                            double depth,
		                            Seiscomp::Core::Time time,
		                            const QPoint &dialogPos);

		void updateOrigin(Seiscomp::DataModel::Origin*);
		void updateContent();
		void addArrival(int idx, const DataModel::Arrival* arrival, const DataModel::Pick* pick, const QColor&);

		void readPicks(Seiscomp::DataModel::Origin*);

		bool merge(void *sourcePhases, void *targetPhases, bool checkDuplicates,
		           bool associateOnly, bool failOnNoNewPhases);

		void relocate(std::vector<PhasePickWithFlags>* additionalPicks,
		              bool associateOnly = false,
		              bool replaceExistingPhases = false);

		void relocate(DataModel::Origin *org,
		              std::vector<PhasePickWithFlags>* additionalPicks,
		              bool associateOnly = false,
		              bool replaceExistingPhases = false,
		              bool useArrivalTable = true);

		void applyNewOrigin(DataModel::Origin *org, bool relocated);

		void pushUndo();

		void startBlinking(QColor, QWidget *);
		void stopBlinking();

		void runScript(const QString &script, const QString &name);

		void selectArrivals(const ArrivalModel::Filter *f);
		void selectArrivals(const ArrivalModel::Filter &f);

		void setPlotFilter(DiagramFilterSettingsDialog::Filter *f);
		void applyPlotFilter();

		bool sendJournal(const std::string &objectID,
		                 const std::string &action,
		                 const std::string &params);

		DataModel::Notifier *
		createJournal(const std::string &objectID, const std::string &action,
		              const std::string &params);

		void setBaseEvent(DataModel::Event *e);
		void resetCustomLabels();

		void deleteSelectedArrivals();
		void activateSelectedArrivals(Seiscomp::Seismology::LocatorInterface::Flags flags,
		                              bool activate);
		void renameArrivals();

		void commitWithOptions(const void *options);

	private:
		OriginLocatorViewPrivate *_d_ptr;
};


inline DataModel::Origin *ArrivalModel::origin() const {
	return _origin;
}

inline int ArrivalModel::usedFlags(int row) const {
	return _used[row];
}


}
}

#endif
