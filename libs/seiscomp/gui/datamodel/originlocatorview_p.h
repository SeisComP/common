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


#ifndef SEISCOMP_GUI_ORIGINLOCATORVIEW_PRIVATE_H
#define SEISCOMP_GUI_ORIGINLOCATORVIEW_PRIVATE_H


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


#include "originlocatorview.h"

#include <seiscomp/gui/datamodel/ui_originlocatorview.h>
#include <seiscomp/seismology/firstmotion.h>


namespace Seiscomp::Gui {

class OriginLocatorViewPrivate {
	private:
		OriginLocatorViewPrivate(const PickerView::Config &config)
		: pickerConfig(config) {}

		using ScriptLabel = QPair<QLabel*,QLabel*>;
		using ScriptLabelMap = QHash<QString, ScriptLabel>;
		using PickList = OriginLocatorView::PickList;
		using PickSet = OriginLocatorView::PickSet;
		using PickMap = OriginLocatorView::PickMap;
		using AmplitudeSet = OriginLocatorView::AmplitudeSet;

		struct OriginMemento {
			OriginMemento() {}
			OriginMemento(DataModel::Origin* o)
			 : origin(o) {}
			OriginMemento(DataModel::Origin* o, const PickSet &ps,
			              const AmplitudeSet &as, bool newOne)
			 : origin(o), newPicks(ps), newAmplitudes(as), newOrigin(newOne) {}

			DataModel::OriginPtr origin;
			PickSet              newPicks;
			AmplitudeSet         newAmplitudes;
			bool                 newOrigin;
		};

		Seiscomp::DataModel::DatabaseQuery   *reader{nullptr};
		Map::ImageTreePtr                     maptree;
		::Ui::OriginLocatorView               ui;
		QTabBar                              *plotTab{nullptr};
		OriginLocatorMap                     *map{nullptr};
		OriginLocatorMap                     *toolMap{nullptr};
		PickerView                           *recordView{nullptr};
		DiagramWidget                        *residuals{nullptr};
		ArrivalModel                          modelArrivals;
		QSortFilterProxyModel                *modelArrivalsProxy{nullptr};
		DataModel::EventPtr                   baseEvent;
		DataModel::OriginPtr                  currentOrigin;
		DataModel::OriginPtr                  baseOrigin;
		DataModel::EvaluationStatus           newOriginStatus;
		std::string                           preferredFocMech;
		bool                                  localOrigin;

		OPT(DataModel::EventType)             defaultEventType;

		QStack<OriginMemento>                 undoList;
		QStack<OriginMemento>                 redoList;

		QTreeWidgetItem                      *unassociatedEventItem{nullptr};
		PickMap                               associatedPicks;
		PickList                              originPicks;
		PickSet                               changedPicks;
		AmplitudeSet                          changedAmplitudes;
		double                                minimumDepth;
		Seismology::LocatorInterfacePtr       locator;
		std::string                           defaultEarthModel;
		TravelTimeTable                       ttTable;

		ScriptLabelMap                        scriptLabelMap;

		bool                                  blockReadPicks;

		// Manual picker configuration attributes
		OriginLocatorView::Config             config;
		PickerView::Config                    pickerConfig;
		std::string                           script0;
		std::string                           script1;

		std::string                           displayCommentID;
		std::string                           displayCommentDefault;
		bool                                  displayComment;

		int                                   blinkCounter;
		char                                  blinker;
		QColor                                blinkColor;
		QWidget                              *blinkWidget{nullptr};
		QTimer                                blinkTimer;
		QMenu                                *commitMenu{nullptr};
		QAction                              *actionCommitOptions{nullptr};

		DiagramFilterSettingsDialog          *plotFilterSettings{nullptr};
		DiagramFilterSettingsDialog::Filter  *plotFilter{nullptr};

		QPushButton                          *btnCommandMenu{nullptr};

		// Last auto-inversion result (used to populate FM fields on commit)
		bool                                  hasAutoFMSolution{false};
		Seismology::FMSolution                lastFMSolution;

	friend class OriginLocatorView;
};


} // namespace Seiscomp::Gui


#endif
