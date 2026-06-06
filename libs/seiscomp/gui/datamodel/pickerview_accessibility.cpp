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


#include "pickerview_accessibility.h"

#include <seiscomp/gui/core/recordwidget.h>
#include <seiscomp/gui/core/recordview.h>

#include <QDateTime>

// Define indices used in pickerview.cpp
#define ITEM_DISTANCE_INDEX  0
#define ITEM_RESIDUAL_INDEX  1
#define ITEM_AZIMUTH_INDEX  2

namespace Seiscomp {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidgetAccessible::RecordWidgetAccessible(RecordWidget *widget)
: QAccessibleWidget(widget, QAccessible::Client, "WaveformDisplay")
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidgetAccessible::~RecordWidgetAccessible()
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordWidgetAccessible::text(QAccessible::Text t) const
{
	RecordWidget *w = recordWidget();
	if ( !w ) return QString();

	switch ( t ) {
		case QAccessible::Name:
			return tr("Seismic Waveform Display");

		case QAccessible::Description: {
			QString desc;
			desc += tr("Waveform display showing seismic data");

			// Add time range
			Core::Time startTime = w->leftTime();
			Core::Time endTime = w->rightTime();
			desc += tr(". Time range: %1 to %2").arg(
				startTime.toString("%H:%M:%S").c_str(),
				endTime.toString("%H:%M:%S").c_str()
			);

			// Add trace count
			int traceCount = w->slotCount();
			desc += tr(". %1 trace%2").arg(traceCount).arg(traceCount > 1 ? "s" : "");

			// Add cursor position if active
			if ( !w->cursorText().isEmpty() ) {
				Core::Time cursorTime = w->cursorPos();
				desc += tr(". Cursor at %1 in %2 mode").arg(
					cursorTime.toString("%H:%M:%S.%f").c_str(),
					w->cursorText()
				);
			}

			return desc;
		}

		default:
			break;
	}

	return QString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessible::Role RecordWidgetAccessible::role() const
{
	return QAccessible::Client;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessible::State RecordWidgetAccessible::state() const
{
	QAccessible::State state;
	RecordWidget *w = recordWidget();

	if ( !w ) return state;

	state.active = w->isActive();
	state.focusable = true;
	state.focused = w->hasFocus();

	return state;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRect RecordWidgetAccessible::rect() const
{
	RecordWidget *w = recordWidget();
	return w ? w->geometry() : QRect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidgetAccessible::childCount() const
{
	// The waveform display itself doesn't have accessible children
	// Individual traces are handled by RecordViewItemAccessible
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *RecordWidgetAccessible::child(int /*index*/) const
{
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *RecordWidgetAccessible::parent() const
{
	return QAccessibleWidget::parent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *RecordWidgetAccessible::focusChild() const
{
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget *RecordWidgetAccessible::recordWidget() const
{
	return qobject_cast<RecordWidget*>(object());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItemAccessible::RecordViewItemAccessible(RecordViewItem *item)
: QAccessibleWidget(item->widget(), QAccessible::ListItem, "StationTrace")
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItemAccessible::~RecordViewItemAccessible()
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordViewItemAccessible::text(QAccessible::Text t) const
{
	RecordViewItem *item = recordViewItem();
	if ( !item ) return QString();

	switch ( t ) {
		case QAccessible::Name: {
			// Build station name from label
			QString stationCode = item->label()->text(0);
			return tr("Station %1").arg(stationCode);
		}

		case QAccessible::Description:
			return buildDescription();

		default:
			break;
	}

	return QString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessible::Role RecordViewItemAccessible::role() const
{
	return QAccessible::ListItem;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessible::State RecordViewItemAccessible::state() const
{
	QAccessible::State state;
	RecordViewItem *item = recordViewItem();

	if ( !item ) return state;

	RecordWidget *w = item->widget();
	if ( !w ) return state;

	state.active = item->isSelected();
	state.selected = item->isSelected();
	state.focusable = true;
	state.focused = w->hasFocus();

	return state;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRect RecordViewItemAccessible::rect() const
{
	RecordViewItem *item = recordViewItem();
	return item ? item->geometry() : QRect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordViewItemAccessible::childCount() const
{
	// Individual trace components could be children
	// For now, keep it simple
	return 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *RecordViewItemAccessible::child(int /*index*/) const
{
	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *RecordViewItemAccessible::parent() const
{
	// Return the parent RecordView's accessibility interface
	// We need to find the RecordView through the widget's parent hierarchy
	RecordViewItem *item = recordViewItem();
	if ( !item ) return nullptr;

	RecordWidget *w = item->widget();
	if ( !w ) return nullptr;

	// Try to find the RecordView parent
	QWidget *parentWidget = w->parentWidget();
	while ( parentWidget ) {
		if ( RecordView *view = qobject_cast<RecordView*>(parentWidget) ) {
			return QAccessible::queryAccessibleInterface(view);
		}
		parentWidget = parentWidget->parentWidget();
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem *RecordViewItemAccessible::recordViewItem() const
{
	return qobject_cast<RecordViewItem*>(object());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordViewItemAccessible::buildDescription() const
{
	RecordViewItem *item = recordViewItem();
	if ( !item ) return QString();

	QString desc;

	// Station code
	QString stationCode = item->label()->text(0);
	desc += tr("Station %1").arg(stationCode);

	// Channel info
	QString channel = item->label()->text(1);
	if ( !channel.isEmpty() ) {
		desc += tr(", channel %1").arg(channel);
	}

	// Distance
	QString distance = item->label()->text(ITEM_DISTANCE_INDEX);
	if ( !distance.isEmpty() ) {
		desc += tr(", distance %1").arg(distance);
	}

	// Azimuth
	QString azimuth = item->label()->text(ITEM_AZIMUTH_INDEX);
	if ( !azimuth.isEmpty() ) {
		desc += tr(", azimuth %1").arg(azimuth);
	}

	// Residual (if showing arrivals)
	QString residual = item->label()->text(ITEM_RESIDUAL_INDEX);
	if ( !residual.isEmpty() && residual != "-" ) {
		desc += tr(", residual %1").arg(residual);
	}

	// State
	RecordWidget *w = item->widget();
	if ( w ) {
		if ( !w->isEnabled() ) {
			desc += tr(" (disabled)");
		}
	}

	// Pick information
	if ( w ) {
		int pickCount = 0;
		QStringList phases;
		for ( int m = 0; m < w->markerCount(); ++m ) {
			RecordMarker *marker = w->marker(m);
			if ( marker ) {
				++pickCount;
				if ( !phases.contains(marker->text()) ) {
					phases.append(marker->text());
				}
			}
		}

		if ( pickCount > 0 ) {
			desc += tr(". %1 pick%2: %3").arg(pickCount)
				.arg(pickCount > 1 ? "s" : "")
				.arg(phases.join(", "));
		}
	}

	return desc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewAccessible::RecordViewAccessible(RecordView *view)
: QAccessibleWidget(view, QAccessible::List, "StationList")
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewAccessible::~RecordViewAccessible()
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordViewAccessible::text(QAccessible::Text t) const
{
	RecordView *view = recordView();
	if ( !view ) return QString();

	switch ( t ) {
		case QAccessible::Name:
			return tr("Seismic Station List");

		case QAccessible::Description: {
			QString desc;
			desc += tr("List of seismic stations with waveform traces");

			int count = view->rowCount();
			desc += tr(". %1 station%2").arg(count).arg(count > 1 ? "s" : "");

			// Count enabled stations
			int enabledCount = 0;
			for ( int i = 0; i < count; ++i ) {
				RecordViewItem *item = view->itemAt(i);
				if ( item && item->widget() && item->widget()->isEnabled() ) {
					++enabledCount;
				}
			}
			desc += tr(" (%1 enabled)").arg(enabledCount);

			return desc;
		}

		default:
			break;
	}

	return QString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessible::Role RecordViewAccessible::role() const
{
	return QAccessible::List;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessible::State RecordViewAccessible::state() const
{
	QAccessible::State state;
	RecordView *view = recordView();

	if ( !view ) return state;

	state.active = true;
	state.focusable = true;
	state.focused = view->hasFocus();
	state.multiSelectable = true;

	return state;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRect RecordViewAccessible::rect() const
{
	RecordView *view = recordView();
	return view ? view->geometry() : QRect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordViewAccessible::childCount() const
{
	RecordView *view = recordView();
	return view ? view->rowCount() : 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *RecordViewAccessible::child(int index) const
{
	RecordView *view = recordView();
	if ( !view || index < 0 || index >= view->rowCount() ) return nullptr;

	RecordViewItem *item = view->itemAt(index);
	if ( !item ) return nullptr;

	return QAccessible::queryAccessibleInterface(item);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *RecordViewAccessible::parent() const
{
	return QAccessibleWidget::parent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *RecordViewAccessible::focusChild() const
{
	RecordView *view = recordView();
	if ( !view ) return nullptr;

	RecordViewItem *currentItem = view->currentItem();
	if ( !currentItem ) return nullptr;

	return QAccessible::queryAccessibleInterface(currentItem);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordView *RecordViewAccessible::recordView() const
{
	return qobject_cast<RecordView*>(object());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QAccessibleInterface *pickerViewAccessibleFactory(const QString &key, QObject *object)
{
	if ( key == QLatin1String("RecordWidget") ) {
		if ( RecordWidget *widget = qobject_cast<RecordWidget*>(object) ) {
			return new RecordWidgetAccessible(widget);
		}
	}
	else if ( key == QLatin1String("RecordViewItem") ) {
		if ( RecordViewItem *item = qobject_cast<RecordViewItem*>(object) ) {
			return new RecordViewItemAccessible(item);
		}
	}
	else if ( key == QLatin1String("RecordView") ) {
		if ( RecordView *view = qobject_cast<RecordView*>(object) ) {
			return new RecordViewAccessible(view);
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Gui
} // namespace Seiscomp
