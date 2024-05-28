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


#ifndef SEISCOMP_GUI_EVENTLISTVIEW_PRIVATE_H
#define SEISCOMP_GUI_EVENTLISTVIEW_PRIVATE_H


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


#include "eventlistview.h"

#include <seiscomp/gui/datamodel/ui_eventlistview.h>


namespace Seiscomp {
namespace Gui {


class EventListViewPrivate {
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

	public:
		EventListViewPrivate();
		~EventListViewPrivate();

	private:
		::Ui::EventListView                *_ui;
		Private::EventFilterWidget         *_filterWidget;
		QTreeWidget                        *_treeWidget;
		QTreeWidgetItem                    *_unassociatedEventItem;
		QWidget                            *_commandWaitDialog;
		QMovie                             *_busyIndicator;
		QLabel                             *_busyIndicatorLabel;
		//StationMap                        _associatedStations;
		Seiscomp::DataModel::DatabaseQuery *_reader;
		Seiscomp::Core::TimeSpan            _timeAgo;
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
		bool                                _hideFinalRejectedEvents;
		bool                                _hideNewEvents;
		bool                                _checkEventAgency;
		bool                                _showOnlyLatestPerAgency;
		int                                 _regionIndex;
		EventListView::Filter               _filter;
		ItemConfig                          _itemConfig;
		EventListView::FilterRegions        _filterRegions;
		mutable int                         _visibleEventCount;

	friend class EventListView;
};


}
}


#endif
