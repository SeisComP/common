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


#ifndef SEISCOMP_GUI_QUICKFILTERWIDGET_H
#define SEISCOMP_GUI_QUICKFILTERWIDGET_H


#include <seiscomp/gui/qt.h>
#include <QWidget>


class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QTimer;


namespace Seiscomp {
namespace Gui {


/**
 * @brief Compact controls for a Butterworth filter which can be placed
 *        into a toolbar.
 *
 * Changing a value emits filterChanged with the corresponding filter
 * string, e.g. BW(3,1,8) or BW_HP(3,0.5), after a short delay so that
 * scrolling through values does not refilter the data on every step.
 */
class SC_GUI_API QuickFilterWidget : public QWidget {
	Q_OBJECT

	public:
		QuickFilterWidget(QWidget *parent = nullptr);

	public:
		//! Returns the filter string of the current settings
		QString filter() const;

		/**
		 * @brief Takes the settings from a single Butterworth filter string.
		 *        No signal is emitted.
		 * @return false if the string is not a single BW, BW_HP, BW_LP or
		 *         BW_BS filter, the settings are unchanged then.
		 */
		bool setFilter(const QString &filter);

		//! Returns whether the settings describe a usable filter
		bool isValid() const;


	signals:
		void filterChanged(const QString &filter);


	private slots:
		void typeChanged();
		void valueChanged();
		void emitFilter();


	private:
		QComboBox      *_type;
		QSpinBox       *_order;
		QDoubleSpinBox *_fmin;
		QDoubleSpinBox *_fmax;
		QTimer         *_timer;
};


}
}


#endif
