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



#ifndef SEISCOMP_GUI_AMPLITUDEVIEW_H
#define SEISCOMP_GUI_AMPLITUDEVIEW_H

#include <seiscomp/gui/datamodel/ui_amplitudeview.h>
#include <seiscomp/gui/core/recordview.h>
#include <seiscomp/gui/core/connectionstatelabel.h>
#include <seiscomp/gui/core/utils.h>
#ifndef Q_MOC_RUN
#include <seiscomp/seismology/ttt.h>
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/amplitude.h>
#include <seiscomp/processing/amplitudeprocessor.h>
#include <seiscomp/processing/magnitudeprocessor.h>
#include <seiscomp/math/matrix3.h>
#endif

#include <QActionGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QMovie>
#include <QSet>
#include <QLabel>
#include <QLineEdit>


namespace Seiscomp {

namespace DataModel {

class SensorLocation;

}

namespace Gui {

class TimeScale;
class AmplitudeView;


namespace PrivateAmplitudeView {

class AmplitudeRecordLabel;

class SC_GUI_API ThreeComponentTrace : public QObject {
	Q_OBJECT

	public:
		ThreeComponentTrace() = default;
		~ThreeComponentTrace();

	public:
		void setTransformationEnabled(bool);
		void showProcessedData(bool);
		void setRecordWidget(RecordWidget *);
		void setFilter(RecordWidget::Filter *, const std::string &filterID);
		bool transform(int comp = -1, Record *rec = nullptr);

		bool setProcessedData(int comp,
		                      const std::string &networkCode,
		                      const std::string &stationCode,
		                      const std::string &locationCode,
		                      const Core::Time &startTime,
		                      double samplingFrequency,
		                      DoubleArrayPtr data);
		void removeProcessedData(int comp);

	private slots:
		void widgetDestroyed(QObject *obj);

	private:
		void transformedRecord(int comp, const Record *rec);

	public:
		// One component
		struct Component {
			std::string           channelCode;
			int                   recordSlot;
			RecordSequence       *raw{nullptr};
			RecordSequence       *transformed{nullptr};
			RecordSequence       *processed{nullptr};
			RecordWidget::Filter *filter{nullptr};
			RecordStreamThread   *thread{nullptr};
		};

		AmplitudeRecordLabel *label;
		Math::Matrix3d        transformation;
		Component             traces[3];
		std::string           filterID;
		RecordWidget         *widget{nullptr};
		bool                  enableTransformation{false};
		bool                  showProcessed{false};
};


class SC_GUI_API AmplitudeRecordLabel : public StandardRecordLabel {
	Q_OBJECT

	public:
		AmplitudeRecordLabel(int items=3, QWidget *parent=0, const char* name = 0);
		~AmplitudeRecordLabel();

	public:
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

		void updateProcessingInfo();

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

	public:
		double                            latitude;
		double                            longitude;
		const DataModel::SensorLocation  *location;

		Core::TimeWindow                  timeWindow;
		ThreeComponentTrace               data;

		Math::Matrix3f                    orientationZNE;
		Math::Matrix3f                    orientationZRT;

		Processing::AmplitudeProcessorPtr processor;
		Processing::MagnitudeProcessorPtr magnitudeProcessor;

		double                            initialMinSNR;

		QString                           infoText;
		bool                              isError;

		bool                              hasGotData;
		bool                              isEnabledByConfig;

	friend class Gui::AmplitudeView;
};


}


class AmplitudeViewPrivate;


class SC_GUI_API AmplitudeView : public QMainWindow {
	public:
		struct SC_GUI_API Config {
			typedef QPair<QString, QString> FilterEntry;
			typedef QVector<FilterEntry> FilterList;
			typedef QList<QString> StringList;

			QString recordURL;
			FilterList filters;

			bool   showAllComponents;
			bool   loadStrongMotionData;
			double allComponentsMaximumStationDistance;
			double defaultAddStationsDistance;

			bool ignoreDisabledStations;
			bool hideStationsWithoutData;

			QColor timingQualityLow;
			QColor timingQualityMedium;
			QColor timingQualityHigh;

			Core::TimeSpan preOffset;
			Core::TimeSpan postOffset;

			Config();

			void addFilter(const QString &f, const QString &n) {
				filters.push_back(QPair<QString, QString>(f, n));
			}
		};


	Q_OBJECT

	public:
		//! Default c'tor
		AmplitudeView(QWidget *parent = 0, Qt::WindowFlags f = 0);
		~AmplitudeView();

	public:
		bool setConfig(const Config &c, QString *error = nullptr);

		void setDatabase(Seiscomp::DataModel::DatabaseQuery*);

		//! Sets an origin and inserts the traces for each arrival
		//! in the view.
		bool setOrigin(Seiscomp::DataModel::Origin*, const std::string &magType);

		bool hasModifiedAmplitudes() const;
		void getChangedAmplitudes(ObjectChangeList<DataModel::Amplitude> &list) const;

		void stop();

		void selectTrace(const Seiscomp::DataModel::WaveformStreamID &wid);
		void selectTrace(const std::string &code);

		const std::string &currentMagnitudeType() const;

		void setStrongMotionCodes(const std::vector<std::string> &codes);


	public slots:
		void setDefaultDisplay();
		void applyAmplitudes();
		void changeFilter(int);
		void setArrivalState(int arrivalId, bool state);

		void setStationEnabled(const std::string& networkCode,
		                       const std::string& stationCode,
		                       bool state);

		void setCurrentStation(const std::string& networkCode,
		                       const std::string& stationCode);


	signals:
		void magnitudeCreated(Seiscomp::DataModel::Magnitude*);
		void amplitudesConfirmed(Seiscomp::DataModel::Origin*, QList<Seiscomp::DataModel::AmplitudePtr>);


	private slots:
		void receivedRecord(Seiscomp::Record*);

		void updateTraceInfo(RecordViewItem*, const Seiscomp::Record*);
		void onAddedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*);
		void onSelectedTime(Seiscomp::Core::Time);
		void onSelectedTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time);
		void onChangingTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time);
		void onSelectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time);
		void setAlignment(Seiscomp::Core::Time);
		void acquisitionFinished();
		void commit();
		void itemSelected(RecordViewItem*, RecordViewItem*);
		void updateMainCursor(RecordWidget*,int);
		void updateSubCursor(RecordWidget*,int);
		void updateItemLabel(RecordViewItem*, char);
		void updateItemRecordState(const Seiscomp::Record*);
		void updateRecordValue(Seiscomp::Core::Time);
		void showTraceScaleToggled(bool);
		void showTheoreticalArrivals(bool);

		void limitFilterToZoomTrace(bool);

		void toggleFilter();
		void addNewFilter(const QString&);

		void scaleVisibleAmplitudes();

		void changeScale(double, double);
		void changeTimeRange(double, double);

		void sortAlphabetically();
		void sortByDistance();

		void showZComponent();
		void showNComponent();
		void showEComponent();

		void alignOnOriginTime();
		void alignOnPArrivals();

		void pickNone(bool);
		void pickAmplitudes(bool);

		void scaleAmplUp();
		void scaleAmplDown();
		void scaleTimeUp();
		void scaleTimeDown();

		void scaleReset();

		void scrollLeft();
		void scrollFineLeft();
		void scrollRight();
		void scrollFineRight();

		void createAmplitude();
		void setAmplitude();
		void confirmAmplitude();
		void deleteAmplitude();

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

		void recalculateAmplitude();
		void recalculateAmplitudes();

		void enableAutoScale();
		void disableAutoScale();

		void zoomSelectionHandleMoved(int,double,Qt::KeyboardModifiers);
		void zoomSelectionHandleMoveFinished();

		void selectionHandleMoved(int,double,Qt::KeyboardModifiers);
		void selectionHandleMoveFinished();

		void addStations();

		void searchStation();
		void search(const QString&);
		void nextSearch();
		void abortSearchStation();

		void openConnectionInfo(const QPoint &);

		void ttInterfaceChanged(QString);
		void ttTableChanged(QString);


	protected:
		void showEvent(QShowEvent* event);

		RecordLabel* createLabel(RecordViewItem*) const;


	private:
		void figureOutTravelTimeTable();

		void init();

		RecordViewItem* addStream(const DataModel::SensorLocation *,
		                          const DataModel::WaveformStreamID& streamID,
		                          const Core::Time &referenceTime,
		                          bool showDisabled);

		RecordViewItem* addRawStream(const DataModel::SensorLocation *,
		                             const DataModel::WaveformStreamID& streamID,
		                             const Core::Time &referenceTime);

		void queueStream(const DataModel::WaveformStreamID& streamID, int component);

		void setupItem(const char comps[3], RecordViewItem*);
		bool addTheoreticalArrivals(RecordViewItem*,
		                            const std::string& netCode,
		                            const std::string& staCode,
		                            const std::string& locCode);
		bool fillTheoreticalArrivals();

		void resetState();

		void updateOriginInformation();

		void loadNextStations(float distance);

		void setCursorText(const QString&);
		void setCursorPos(const Seiscomp::Core::Time&, bool always = false);
		void setTimeRange(float, float);

		void acquireStreams();

		bool applyFilter(RecordViewItem *item = nullptr);


		//! Makes sure that the time range [tmin, tmax] is visible.
		//! When the interval is larger than the visible area
		//! the time range will be left aligned.
		void ensureVisibility(double &tmin, double &tmax);
		void ensureVisibility(const Seiscomp::Core::Time &time, int pixelMargin);

		RecordMarker *updatePhaseMarker(Seiscomp::Gui::RecordViewItem*,
		                                const Processing::AmplitudeProcessor *proc,
		                                const Processing::AmplitudeProcessor::Result &res);

		void setPhaseMarker(Seiscomp::Gui::RecordWidget*, const Seiscomp::Core::Time&);

		void updateCurrentRowState();
		void setMarkerState(Seiscomp::Gui::RecordWidget*, bool);

		bool setArrivalState(Seiscomp::Gui::RecordWidget* w, int arrivalId, bool state);
		void resetAmplitude(RecordViewItem *item, const QString &text, bool enable);

		void fetchManualAmplitudes(std::vector<RecordMarker*>* marker = nullptr) const;

		void showComponent(char componentCode);
		void fetchComponent(char componentCode);

		void addAmplitude(Gui::RecordViewItem*,
		                  DataModel::Amplitude*,
		                  DataModel::Pick*, Core::Time reference, int id);

		void addFilter(const QString& name, const QString& filter);

		void changeFilter(int, bool force);

		void closeThreads();

		char currentComponent() const;

		void searchByText(const QString &text);

		void newAmplitudeAvailable(const Processing::AmplitudeProcessor*,
		                           const Processing::AmplitudeProcessor::Result &);

	private:
		AmplitudeViewPrivate *_d_ptr;
};


}
}


#endif
