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


#ifndef SEISCOMP_GUI_AMPLITUDEVIEW_PRIVATE_H
#define SEISCOMP_GUI_AMPLITUDEVIEW_PRIVATE_H


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


#include "amplitudeview.h"

#include <seiscomp/gui/datamodel/ui_amplitudeview.h>


// Ui forward declaration
namespace Ui {
	class AmplitudeView;
}


namespace Seiscomp {
namespace Gui {


class AmplitudeViewPrivate {
	private:
		struct WaveformRequest {
			WaveformRequest(const Core::TimeWindow &tw,
			                const DataModel::WaveformStreamID &sid,
			                char c)
			: timeWindow(tw), streamID(sid), component(c) {}

			Core::TimeWindow            timeWindow;
			DataModel::WaveformStreamID streamID;
			int                         component;
		};

		using RecordItemMap = std::map<std::string, PrivateAmplitudeView::AmplitudeRecordLabel*>;
		using WaveformStreamList = std::list<WaveformRequest>;

	private:
		Seiscomp::DataModel::DatabaseQuery *reader;
		QSet<QString>                       stations;
		QComboBox                          *comboFilter;
		QComboBox                          *comboTTT;
		QComboBox                          *comboTTTables;
		QLabel                             *labelAmpType;
		QComboBox                          *comboAmpType;
		QLabel                             *labelAmpCombiner;
		QComboBox                          *comboAmpCombiner;
		QDoubleSpinBox                     *spinDistance;
		QCheckBox                          *checkOverrideSNR;
		QDoubleSpinBox                     *spinSNR;

		QLineEdit                          *searchStation;
		QLabel                             *searchLabel;

		ConnectionStateLabel               *connectionState;
		RecordView                         *recordView;
		RecordWidget                       *currentRecord;
		TimeScale                          *timeScale;
		DataModel::OriginPtr                origin;
		DataModel::MagnitudePtr             magnitude;
		std::string                         magnitudeType;
		std::string                         amplitudeType;

		QList<QString>                      phases;
		double                              minTime, maxTime;
		double                              minDist;
		double                              maxDist;

		float                               zoom;
		float                               currentAmplScale;
		QString                             lastRecordURL;
		TravelTimeTableInterfacePtr         ttTable;
		bool                                centerSelection;
		bool                                checkVisibility;
		bool                                acquireNextStations;
		int                                 lastFilterIndex;
		bool                                autoScaleZoomTrace;
		bool                                showProcessedData;
		int                                 currentSlot;
		RecordWidget::Filter               *currentFilter;
		std::string                         currentFilterStr;

		int                                 lastFoundRow;
		QColor                              searchBase, searchError;

		WaveformStreamList                  nextStreams;
		WaveformStreamList                  allStreams;

		std::vector<std::string>            broadBandCodes;

		RecordItemMap                       recordItemLabels;

		mutable ObjectChangeList<DataModel::Amplitude> changedAmplitudes;
		std::vector<DataModel::AmplitudePtr> amplitudesInTime;

		QVector<RecordStreamThread*>        acquisitionThreads;

		std::vector<std::string>            strongMotionCodes;

		RecordWidgetDecorator              *zoomDecorator;
		RecordWidgetDecorator              *generalDecorator;

		AmplitudeView::Config               config;

		::Ui::AmplitudeView                 ui;
		bool                                settingsRestored;

		int                                 componentMap[3];
		int                                 slotCount;

		static std::string                  ttInterface;
		static std::string                  ttTableName;

	friend class AmplitudeView;
};


std::string AmplitudeViewPrivate::ttInterface = "libtau";
std::string AmplitudeViewPrivate::ttTableName = "iasp91";


}
}


#endif
