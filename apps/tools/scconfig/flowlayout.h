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

#ifndef __FLOWLAYOUT_H__
#define __FLOWLAYOUT_H__

#include <QLayout>
#include <QRect>
#include <QWidgetItem>
#include <QStyle>


class FlowLayout : public QLayout {
	public:
		FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
		FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
		~FlowLayout();


	public:
		void addItem(QLayoutItem *item);
		int horizontalSpacing() const;
		int verticalSpacing() const;
		Qt::Orientations expandingDirections() const;
		bool hasHeightForWidth() const;
		int heightForWidth(int) const;
		int count() const;
		QLayoutItem *itemAt(int index) const;
		QSize minimumSize() const;
		void setGeometry(const QRect &rect);
		QSize sizeHint() const;
		QLayoutItem *takeAt(int index);


	private:
		int doLayout(const QRect &rect, bool testOnly) const;
#if QT_VERSION >= 0x040300
		int smartSpacing(QStyle::PixelMetric pm) const;
#endif

		QVector<QLayoutItem *> itemList;
		int m_hSpace;
		int m_vSpace;
};

#endif
