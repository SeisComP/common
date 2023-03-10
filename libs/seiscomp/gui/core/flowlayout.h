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

#ifndef SEISCOMP_GUI_FLOWLAYOUT_H
#define SEISCOMP_GUI_FLOWLAYOUT_H


#include <seiscomp/gui/qt.h>

#include <QLayout>
#include <QRect>
#include <QWidgetItem>
#include <QStyle>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API FlowLayout : public QLayout {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
		FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
		~FlowLayout();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		int horizontalSpacing() const;
		int verticalSpacing() const;


	// ------------------------------------------------------------------
	//  QLayout interface
	// ------------------------------------------------------------------
	public:
		void addItem(QLayoutItem *item) override;
		Qt::Orientations expandingDirections() const override;
		bool hasHeightForWidth() const override;
		int heightForWidth(int) const override;
		int count() const override;
		QLayoutItem *itemAt(int index) const override;
		QSize minimumSize() const override;
		void setGeometry(const QRect &rect) override;
		QSize sizeHint() const override;
		QLayoutItem *takeAt(int index) override;


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		virtual int doLayout(const QRect &rect, bool testOnly) const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
		int smartSpacing(QStyle::PixelMetric pm) const;

	protected:
		QVector<QLayoutItem *> _itemList;

		int                    _hSpace;
		int                    _vSpace;
};


}
}


#endif
