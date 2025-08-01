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


#ifndef SEISCOMP_GUI_PICKERVIEW_H
#define SEISCOMP_GUI_PICKERVIEW_H


#include <seiscomp/gui/core/recordview.h>
#include <seiscomp/gui/core/connectionstatelabel.h>
#include <seiscomp/gui/core/utils.h>
#ifndef Q_MOC_RUN
#include <seiscomp/io/recordfilter/iirfilter.h>
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/seismology/ttt.h>
#include <seiscomp/math/matrix3.h>
#include <seiscomp/processing/picker.h>
#endif
#include <QActionGroup>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QMovie>
#include <QSet>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>


namespace Seiscomp {

namespace DataModel {

class SensorLocation;

}

namespace Processing {

DEFINE_SMARTPOINTER(AmplitudeProcessor);

}

namespace Gui {

class TimeScale;
class PickerView;
class SpectrumWidget;


namespace PrivatePickerView {


class SC_GUI_API ThreeComponentTrace : public QObject {
	Q_OBJECT

	public:
		ThreeComponentTrace() = default;
		~ThreeComponentTrace();

	public:
		void setTransformationEnabled(bool enable);
		void setL2Horizontals(bool);
		void setPassThrough(int component, bool enable);
		void setRecordWidget(RecordWidget *);
		void reset();
		void setFilter(RecordWidget::Filter *);
		bool transform(int comp = -1, Seiscomp::Record *rec = nullptr);

	private slots:
		void widgetDestroyed(QObject *obj);

	public:
		typedef IO::RecordIIRFilter<double> Filter;

		// One component
		struct Component {
			std::string         channelCode;
			RecordSequence     *raw{nullptr};
			RecordSequence     *transformed{nullptr};
			Filter              filter{nullptr};
			RecordStreamThread *thread{nullptr};
			bool                passthrough{false};
		};

		Math::Matrix3d  transformation;
		Component       traces[3];
		RecordWidget   *widget{nullptr};
		bool            enableTransformation{false};
		bool            enableL2Horizontals{false};
};


class SC_GUI_API PickerRecordLabel : public StandardRecordLabel {
	Q_OBJECT

	public:
		PickerRecordLabel(int items=3, QWidget *parent=0, const char* name = 0);
		~PickerRecordLabel();


	public:
		void setConfigState(bool);

		void setControlledItem(RecordViewItem *controlledItem);
		RecordViewItem *controlledItem() const;

		void setLinkedItem(bool sm);

		void enabledExpandButton(RecordViewItem *controlledItem);
		void disableExpandButton();
		void unlink();

		bool isLinkedItem() const;
		bool isExpanded() const;

		void setLabelColor(QColor);
		void removeLabelColor();


	protected:
		void visibilityChanged(bool);
		void resizeEvent(QResizeEvent *e);
		void paintEvent(QPaintEvent *e);


	public slots:
		void extentButtonPressed();

	private slots:
		void enableExpandable(const Seiscomp::Record*);


	private:
		bool            _isLinkedItem;
		bool            _isExpanded;
		QPushButton    *_btnExpand;
		RecordViewItem *_linkedItem;
		bool            _hasLabelColor;
		QColor          _labelColor;

	private:
		double               latitude;
		double               longitude;
		double               elevation;
		int                  unit;
		QString              gainUnit[3];
		double               gainToSI[3];
		ThreeComponentTrace  data;
		Math::Matrix3d       orientationZNE;
		Math::Matrix3d       orientationZRT;
		Math::Matrix3d       orientationLQT;

		bool                 hasGotData;
		bool                 isEnabledByConfig;

	friend class Gui::PickerView;
};


}


class SC_GUI_API PickerMarkerActionPlugin : public QObject {
	public:
		virtual ~PickerMarkerActionPlugin() {}

	public:
		//! Returns the action title as added to the context menu
		virtual QString title() const = 0;

		virtual bool init(const DataModel::WaveformStreamID &wid,
		                  const Core::Time &time) = 0;

		//! Feed a record of the current stream (including all available
		//! components)
		virtual void setRecords(RecordSequence *seqZ, RecordSequence *seq1, RecordSequence *seq2) = 0;

		//! Finalize the action, no more data will be fed a this point
		virtual void finalize() = 0;
};


DEFINE_INTERFACE_FACTORY(PickerMarkerActionPlugin);


class SpectrumViewBase : public QWidget {
	Q_OBJECT

	public:
		SpectrumViewBase(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
		: QWidget(parent, f) {}

	protected slots:
		virtual void modeChanged(int) = 0;
		virtual void windowFuncChanged(int) = 0;
		virtual void windowWidthChanged(double) = 0;
};


class PickerViewPrivate;


class SC_GUI_API PickerView : public QMainWindow {
	public:
		struct SC_GUI_API Config {
			typedef QPair<QString, QString> FilterEntry;
			typedef QVector<FilterEntry> FilterList;
			typedef QList<QString> StringList;
			typedef StringList PhaseList;

			MAKEENUM(
				RotationType,
				EVALUES(
					RT_123,
					RT_ZNE,
					RT_ZRT,
					RT_LQT,
					RT_ZH
				),
				ENAMES(
					"123",
					"ZNE",
					"ZRT",
					"LQT",
					"ZH(L2)"
				)
			);

			MAKEENUM(
				UnitType,
				EVALUES(
					UT_RAW,
					UT_ACC,
					UT_VEL,
					UT_DISP
				),
				ENAMES(
					"Sensor",
					"Acceleration",
					"Velocity",
					"Displacement"
				)
			);

			struct PhaseGroup {
				QString name;
				QList<PhaseGroup> childs;
			};

			typedef QList<PhaseGroup> GroupList;
			typedef QPair<float, float> Uncertainty;
			typedef QVector<Uncertainty> UncertaintyList;
			typedef QMap<QString, UncertaintyList> UncertaintyProfiles;
			typedef QPair<QString, QString> ChannelMapItem;
			typedef QMultiMap<QString, ChannelMapItem> ChannelMap;

			QString recordURL;
			ChannelMap channelMap;

			FilterList filters;

			QString integrationFilter;
			bool onlyApplyIntegrationFilterOnce{true};

			GroupList phaseGroups;
			PhaseList favouritePhases;

			PhaseList showPhases;

			UncertaintyProfiles uncertaintyProfiles;
			QString uncertaintyProfile;

			bool showCrossHair{false};

			bool ignoreUnconfiguredStations{false};
			bool ignoreDisabledStations{true};
			bool loadAllComponents{true};
			bool loadAllPicks{true};
			bool loadStrongMotionData{false};
			bool usePerStreamTimeWindows{false};
			bool limitStations{false};
			bool showAllComponents{false};
			bool hideStationsWithoutData{false};
			bool hideDisabledStations{false};
			bool showDataInSensorUnit{false};
			bool limitFilterToZoomTrace{false};

			RotationType initialRotation{RT_123};
			UnitType     initialUnit{UT_RAW};

			int    limitStationCount{10};
			double allComponentsMaximumStationDistance{10.0};
			double defaultAddStationsDistance{15.0};
			bool   loadStationsWithinDistanceInitially{false};

			double defaultDepth{10.0};

			bool removeAutomaticStationPicks{false};
			bool removeAutomaticPicks{false};

			Core::TimeSpan preOffset{60, 0};
			Core::TimeSpan postOffset{120, 0};
			Core::TimeSpan minimumTimeWindow{1800, 0};

			double alignmentPosition{0.5};

			QColor timingQualityLow{Qt::darkRed};
			QColor timingQualityMedium{Qt::yellow};
			QColor timingQualityHigh{Qt::darkGreen};

			OPT(double) repickerSignalStart;
			OPT(double) repickerSignalEnd;

			void addFilter(const QString &f, const QString &n) {
				filters.push_back(QPair<QString, QString>(f, n));
			}

			void addShowPhase(const QString &ph) {
				showPhases.push_back(ph);
			}

			void getPickPhases(StringList &phases) const;
			void getPickPhases(StringList &phases, const QList<PhaseGroup> &groups) const;
		};


	Q_OBJECT

	public:
		//! Default c'tor
		//! The mode defaults to ringbuffer with a buffer
		//! size of 30 minutes
		PickerView(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

		//! Creates a RecordView using a time window
		PickerView(const Seiscomp::Core::TimeWindow&,
		           QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

		//! Creates a RecordView using a timespan and
		//! a ringbuffer
		PickerView(const Seiscomp::Core::TimeSpan&,
		           QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

		~PickerView();

	public:
		bool setConfig(const Config &c, QString *error = nullptr);

		void setDatabase(Seiscomp::DataModel::DatabaseQuery*);
		void activateFilter(int index);

		void setBroadBandCodes(const std::vector<std::string> &codes);
		void setStrongMotionCodes(const std::vector<std::string> &codes);

		void setAuxiliaryChannels(const std::vector<std::string> &patterns,
		                          double minimumDistance, double maximumDistance);

		//! Sets an origin an inserts the traces for each arrival
		//! in the view.
		bool setOrigin(Seiscomp::DataModel::Origin*,
		               double relTimeWindowStart,
		               double relTimeWindowEnd);

		//! Sets an origin and keeps all available traces
		bool setOrigin(Seiscomp::DataModel::Origin*);

		bool hasModifiedPicks() const;
		void getChangedPicks(ObjectChangeList<DataModel::Pick> &list) const;

		void stop();

		void selectTrace(const std::string &, const std::string &);
		void selectTrace(const Seiscomp::DataModel::WaveformStreamID &wid);


	signals:
		void requestArtificialOrigin(double lat, double lon, double depth, Seiscomp::Core::Time time);
		void originCreated(Seiscomp::DataModel::Origin*);
		void arrivalChanged(int id, bool state);
		void arrivalEnableStateChanged(int id, bool state);


	public slots:
		void setDefaultDisplay();
		void applyPicks();
		void changeFilter(int);
		void changeRotation(int);
		void changeUnit(int);
		void setArrivalState(int arrivalId, bool state);
		void addPick(Seiscomp::DataModel::Pick* pick);

		void setStationEnabled(const std::string& networkCode,
		                       const std::string& stationCode,
		                       bool state);


	private slots:
		void receivedRecord(Seiscomp::Record*);

		void updateTraceInfo(RecordViewItem*, const Seiscomp::Record*);
		void currentMarkerChanged(Seiscomp::Gui::RecordMarker*);
		void apply(QAction*);
		void setPickPhase(QAction*);
		void alignOnPhase(QAction*);
		void onAddedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*);
		void onSelectedTime(Seiscomp::Core::Time);
		void onSelectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time);
		void setAlignment(Seiscomp::Core::Time);
		void acquisitionFinished();
		void handleAcquisitionError(const QString &msg);
		void relocate();
		void modifyOrigin();
		void updateTheoreticalArrivals();
		void itemSelected(RecordViewItem*, RecordViewItem*);
		void updateMainCursor(RecordWidget*,int);
		void updateSubCursor(RecordWidget*,int);
		void updateItemLabel(RecordViewItem*, char);
		void updateItemRecordState(const Seiscomp::Record*);
		void updateRecordValue(Seiscomp::Core::Time);
		void showTraceScaleToggled(bool);

		void specLogToggled(bool);
		void specSmoothToggled(bool);
		void specMinValue(double);
		void specMaxValue(double);
		void specTimeWindow(double);
		void specApply();

		void limitFilterToZoomTrace(bool);

		void showTheoreticalArrivals(bool);
		void showUnassociatedPicks(bool);
		void showSpectrogram(bool);
		void showSpectrum();

		void toggleFilter();
		void nextFilter();
		void previousFilter();
		void addNewFilter(const QString&);

		void scaleVisibleAmplitudes();

		void zoomSelectionHandleMoved(int, double, Qt::KeyboardModifiers);
		void zoomSelectionHandleMoveFinished();

		void changeScale(double, double);
		void changeTimeRange(double, double);

		void sortAlphabetically();
		void sortByDistance();
		void sortByAzimuth();
		void sortByResidual();
		void sortByPhase(const QString&);

		void showAllComponents(bool);
		void showZComponent();
		void showNComponent();
		void showEComponent();

		void alignOnOriginTime();
		void alignOnPArrivals();
		void alignOnSArrivals();

		void pickNone(bool);
		void pickP(bool);
		void pickS(bool);

		void scaleAmplUp();
		void scaleAmplDown();
		void scaleTimeUp();
		void scaleTimeDown();

		void scaleReset();

		void scrollLeft();
		void scrollFineLeft();
		void scrollRight();
		void scrollFineRight();

		void automaticRepick();
		void gotoNextMarker();
		void gotoPreviousMarker();

		void createPick();
		void setPick();
		void confirmPick();
		void resetPick();
		void deletePick();

		void setCurrentRowEnabled(bool);
		void setCurrentRowDisabled(bool);

		void loadNextStations();
		void showUsedStations(bool);

		void moveTraces(double offset);
		void move(double offset);
		void zoom(float factor);
		void applyTimeRange(double,double);

		void sortByState();
		void alignByState();
		void componentByState();
		void updateLayoutFromState();

		void firstConnectionEstablished();
		void lastConnectionClosed();

		void beginWaitForRecords();
		void doWaitForRecords(int value);
		void endWaitForRecords();

		void showFullscreen(bool);

		void enableAutoScale();
		void disableAutoScale();

		void addStations();

		void searchStation();
		void search(const QString&);
		void nextSearch();
		void abortSearchStation();

		void setPickPolarity();
		void setPickOnset();
		void setPickUncertainty();

		void openContextMenu(const QPoint &p);
		void openRecordContextMenu(const QPoint &p);

		void previewUncertainty(QAction *);
		void previewUncertainty(double lower, double upper);

		void openConnectionInfo(const QPoint &);
		void destroyedSpectrumWidget(QObject *);

		void ttInterfaceChanged(int);
		void ttTableChanged(int);


	protected:
		void showEvent(QShowEvent* event);

		RecordLabel* createLabel(RecordViewItem*) const;


	private:
		void announceAmplitude();
		void figureOutTravelTimeTable();
		void updateTransformations(PrivatePickerView::PickerRecordLabel *label);

		void init();
		void initPhases();
		bool fillTheoreticalArrivals();
		bool fillRawPicks();

		int loadPicks();

		const TravelTime* findPhase(const TravelTimeList &list, const QString &phase, double delta);

		RecordViewItem* addStream(const DataModel::SensorLocation *,
		                          const DataModel::WaveformStreamID& streamID,
		                          double distance,
		                          const std::string& text,
		                          bool showDisabled,
		                          bool addTheoreticalArrivals,
		                          const DataModel::Stream *base = nullptr);

		RecordViewItem* addRawStream(const DataModel::SensorLocation *,
		                             const DataModel::WaveformStreamID& streamID,
		                             double distance,
		                             const std::string& text,
		                             bool addTheoreticalArrivals,
		                             const DataModel::Stream *base = nullptr);

		void queueStream(double dist, const DataModel::WaveformStreamID& streamID, char component);

		void setupItem(const char comps[3], RecordViewItem*);
		bool addTheoreticalArrivals(RecordViewItem*,
		                            const std::string& netCode,
		                            const std::string& staCode,
		                            const std::string& locCode);

		bool addRawPick(Seiscomp::DataModel::Pick*);

		void resetState();
		void alignOnPhase(const QString&, bool theoretical);

		void diffStreamState(Seiscomp::DataModel::Origin* oldOrigin,
		                     Seiscomp::DataModel::Origin* newOrigin);

		void updateOriginInformation();

		void loadNextStations(float distance);

		void setCursorText(const QString&);
		void setCursorPos(const Seiscomp::Core::Time&, bool always = false);
		void setTimeRange(double, double);

		void acquireStreams();

		bool applyFilter(RecordViewItem *item = nullptr);
		bool applyRotation(RecordViewItem *item, int type);
		void updateRecordAxisLabel(RecordViewItem *item);


		//! Makes sure that the time range [tmin, tmax] is visible.
		//! When the interval is larger than the visible area
		//! the time range will be left aligned.
		void ensureVisibility(double &tmin, double &tmax);
		void ensureVisibility(const Seiscomp::Core::Time &time, int pixelMargin);

		void updatePhaseMarker(Seiscomp::Gui::RecordWidget*, const Seiscomp::Core::Time&);
		void declareArrival(Seiscomp::Gui::RecordMarker *m, const QString &phase, bool);
		void updateUncertaintyHandles(RecordMarker *marker);

		void updateCurrentRowState();
		void setMarkerState(Seiscomp::Gui::RecordWidget*, bool);

		bool setArrivalState(Seiscomp::Gui::RecordWidget* w, int arrivalId, bool state);

		void fetchManualPicks(std::vector<RecordMarker*>* marker = nullptr) const;

		void showComponent(char componentCode);
		void fetchComponent(char componentCode);

		void addArrival(Seiscomp::Gui::RecordWidget*, Seiscomp::DataModel::Arrival*, int id);
		void addFilter(const QString& name, const QString& filter);

		void changeFilter(int, bool force);

		void closeThreads();

		char currentComponent() const;

		void searchByText(const QString &text);

		void emitPick(const Processing::Picker *, const Processing::Picker::Result &res);


	private:
		PickerViewPrivate *_d_ptr;
};


}
}


#define REGISTER_PICKER_MARKER_ACTION(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Gui::PickerMarkerActionPlugin, Class> __##Class##InterfaceFactory__(Service)


#endif
