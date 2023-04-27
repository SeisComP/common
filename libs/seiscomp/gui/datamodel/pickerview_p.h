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


#ifndef SEISCOMP_GUI_PICKERVIEW_PRIVATE_H
#define SEISCOMP_GUI_PICKERVIEW_PRIVATE_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the SeisComP API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include "pickerview.h"

#include <seiscomp/gui/datamodel/ui_pickerview.h>


// Ui forward declaration
namespace Ui {
	class PickerView;
}


namespace Seiscomp {
namespace Gui {


class PickerViewPrivate {
	private:
		struct WaveformRequest {
			WaveformRequest(double dist, const Core::TimeWindow &tw,
			                const DataModel::WaveformStreamID &sid,
			                char c)
			: distance(dist), timeWindow(tw), streamID(sid), component(c) {}

			bool operator<(const WaveformRequest &other) const {
				return distance < other.distance;
			}

			double                      distance;
			Core::TimeWindow            timeWindow;
			DataModel::WaveformStreamID streamID;
			char                        component;
		};

		struct SpectrogramOptions {
			double minRange;
			double maxRange;
			double tw;
		};

		typedef std::list<WaveformRequest> WaveformStreamList;
		typedef std::map<std::string, PrivatePickerView::PickerRecordLabel*> RecordItemMap;


	private:
		Seiscomp::DataModel::DatabaseQuery *reader;
		QSet<QString>                       stations;

		QComboBox                          *comboFilter;
		QComboBox                          *comboRotation;
		QComboBox                          *comboUnit;
		QComboBox                          *comboTTT;
		QComboBox                          *comboTTTables;
		QDoubleSpinBox                     *spinDistance;
		QComboBox                          *comboPicker;

		QLineEdit                          *searchStation;
		QLabel                             *searchLabel;

		static QSize                        defaultSpectrumWidgetSize;
		static QByteArray                   spectrumWidgetGeometry;
		PickerView::Config::UncertaintyList uncertainties;

		//QScrollArea* _zoomTrace;
		ConnectionStateLabel               *connectionState;
		RecordView                         *recordView;
		RecordWidget                       *currentRecord;
		TimeScale                          *timeScale;
		Seiscomp::DataModel::OriginPtr      origin;

		Core::TimeWindow                    timeWindowOfInterest;

		QActionGroup                       *actionsUncertainty;
		QActionGroup                       *actionsPickGroupPhases;
		QActionGroup                       *actionsPickFavourites;

		QActionGroup                       *actionsAlignOnFavourites;
		QActionGroup                       *actionsAlignOnGroupPhases;

		QList<QMenu*>                       menusPickGroups;
		QList<QMenu*>                       menusAlignGroups;

		QList<QString>                      phases;
		QList<QString>                      showPhases;
		float                               minTime, maxTime;
		Core::TimeWindow                    timeWindow;
		float                               zoom;
		float                               currentAmplScale;
		QString                             currentPhase;
		QString                             lastRecordURL;
		TravelTimeTableInterfacePtr         ttTable;
		bool                                centerSelection;
		bool                                checkVisibility;
		bool                                acquireNextStations;
		bool                                componentFollowsMouse{false};
		int                                 lastFilterIndex;
		bool                                autoScaleZoomTrace;
		bool                                loadedPicks;
		int                                 currentSlot;
		bool                                alignedOnOT;
		RecordWidget::Filter               *currentFilter;
		QString                             currentFilterID;

		QWidget                            *pickInfoList;

		double                              tmpLowerUncertainty;
		double                              tmpUpperUncertainty;

		int                                 currentRotationMode;
		int                                 currentUnitMode;
		int                                 lastFoundRow;
		QColor                              searchBase, searchError;

		std::vector<std::string>            broadBandCodes;
		std::vector<std::string>            strongMotionCodes;

		WaveformStreamList                  nextStreams;
		WaveformStreamList                  allStreams;

		RecordItemMap                       recordItemLabels;

		mutable ObjectChangeList<DataModel::Pick> changedPicks;
		std::vector<DataModel::PickPtr>     picksInTime;

		QVector<RecordStreamThread*>        acquisitionThreads;
		QList<PickerMarkerActionPlugin*>    markerPlugins;

		PickerView::Config                  config;
		SpectrogramOptions                  specOpts;

		QWidget                            *spectrumView;

		::Ui::PickerView                    ui;
		bool                                settingsRestored;

		static std::string                  ttInterface;
		static std::string                  ttTableName;

	friend class PickerView;
};


QSize PickerViewPrivate::defaultSpectrumWidgetSize = QSize(500,400);
QByteArray PickerViewPrivate::spectrumWidgetGeometry;

std::string PickerViewPrivate::ttInterface = "libtau";
std::string PickerViewPrivate::ttTableName = "iasp91";


}
}


#endif
