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


#ifndef SEISCOMP_GUI_PICKERVIEW_ACCESSIBILITY_H
#define SEISCOMP_GUI_PICKERVIEW_ACCESSIBILITY_H


#include <QAccessible>
#include <QAccessibleInterface>
#include <QAccessibleWidget>
#include <QObject>
#include <QString>


namespace Seiscomp {
namespace Gui {


class RecordWidget;
class RecordViewItem;
class RecordView;


// Forward declarations
class RecordWidgetAccessible;
class RecordViewItemAccessible;
class RecordViewAccessible;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * Custom accessibility interface for RecordWidget (waveform display)
 * 
 * Provides screen reader access to:
 * - Current time range
 * - Number of traces
 * - Current cursor position
 * - Active picks/markers
 */
class RecordWidgetAccessible : public QAccessibleWidget {
public:
	explicit RecordWidgetAccessible(RecordWidget *widget);

	// QAccessibleInterface methods
	QString text(QAccessible::Text t) const override;
	QAccessible::Role role() const override;
	QAccessible::State state() const override;
	QRect rect() const override;

	// Hierarchy navigation
	int childCount() const override;
	QAccessibleInterface *child(int index) const override;
	QAccessibleInterface *parent() const override;

	// Selection support
	QAccessibleInterface *focusChild() const override;

protected:
	~RecordWidgetAccessible() override;

private:
	RecordWidget *recordWidget() const;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * Custom accessibility interface for RecordViewItem (station trace)
 * 
 * Provides screen reader access to:
 * - Station code
 * - Channel code
 * - Distance and azimuth
 * - Pick information
 * - Trace state (enabled/disabled)
 */
class RecordViewItemAccessible : public QAccessibleWidget {
public:
	explicit RecordViewItemAccessible(RecordViewItem *item);

	// QAccessibleInterface methods
	QString text(QAccessible::Text t) const override;
	QAccessible::Role role() const override;
	QAccessible::State state() const override;
	QRect rect() const override;

	// Hierarchy navigation
	int childCount() const override;
	QAccessibleInterface *child(int index) const override;
	QAccessibleInterface *parent() const override;

protected:
	~RecordViewItemAccessible() override;

private:
	RecordViewItem *recordViewItem() const;
	QString buildDescription() const;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * Custom accessibility interface for RecordView (list of traces)
 * 
 * Provides screen reader access to:
 * - List of stations
 * - Current selection
 * - Total count
 * - Navigation hints
 */
class RecordViewAccessible : public QAccessibleWidget {
public:
	explicit RecordViewAccessible(RecordView *view);

	// QAccessibleInterface methods
	QString text(QAccessible::Text t) const override;
	QAccessible::Role role() const override;
	QAccessible::State state() const override;
	QRect rect() const override;

	// Hierarchy navigation
	int childCount() const override;
	QAccessibleInterface *child(int index) const override;
	QAccessibleInterface *parent() const override;

	// Selection support
	QAccessibleInterface *focusChild() const override;

protected:
	~RecordViewAccessible() override;

private:
	RecordView *recordView() const;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * Accessibility factory function
 */
QAccessibleInterface *pickerViewAccessibleFactory(const QString &key, QObject *object);


} // namespace Gui
} // namespace Seiscomp


#endif // SEISCOMP_GUI_PICKERVIEW_ACCESSIBILITY_H
