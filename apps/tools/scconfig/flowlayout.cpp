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

#include <QWidget>
#include <iostream>

#include "flowlayout.h"


FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
: QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing) {
	setMargin(margin);
}


FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
: m_hSpace(hSpacing), m_vSpace(vSpacing) {
	setMargin(margin);
}


FlowLayout::~FlowLayout() {
	QLayoutItem *item;
	while ( (item = takeAt(0)) )
		delete item;
}


void FlowLayout::addItem(QLayoutItem *item) {
	itemList.append(item);
}


int FlowLayout::horizontalSpacing() const {
	if ( m_hSpace >= 0 )
		return m_hSpace;
	else
#if QT_VERSION >= 0x040300
		return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
#else
		return spacing();
#endif
}


int FlowLayout::verticalSpacing() const {
	if ( m_vSpace >= 0 )
		return m_vSpace;
	else
#if QT_VERSION >= 0x040300
		return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
#else
		return spacing();
#endif
}


int FlowLayout::count() const {
	return itemList.size();
}


QLayoutItem *FlowLayout::itemAt(int index) const {
	return itemList.value(index);
}


QLayoutItem *FlowLayout::takeAt(int index) {
	if ( index >= 0 && index < itemList.size() ) {
		QLayoutItem *item = itemList[index];
		itemList.remove(index);
		return item;
	}
	else
		return 0;
}


Qt::Orientations FlowLayout::expandingDirections() const {
	return 0;
}


bool FlowLayout::hasHeightForWidth() const {
	return true;
}


int FlowLayout::heightForWidth(int width) const {
	int height = doLayout(QRect(0, 0, width, 0), true);
	return height;
}


void FlowLayout::setGeometry(const QRect &rect) {
	QLayout::setGeometry(rect);
	doLayout(rect, false);
}


QSize FlowLayout::sizeHint() const {
	return minimumSize();
}


QSize FlowLayout::minimumSize() const {
	QSize size;
	QLayoutItem *item;
	foreach (item, itemList)
		size = size.expandedTo(item->minimumSize());

	size += QSize(2*margin(), 2*margin());
	return size;
}


int FlowLayout::doLayout(const QRect &rect, bool testOnly) const {
	int left, top, right, bottom;
	left = top = right = bottom = margin();
	QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
	int x = effectiveRect.x();
	int y = effectiveRect.y();
	int lineHeight = 0;
	int itemWidth;
	int usedItemWidth = -1;
	int itemsPerRow = -1;

	int spaceX = qMax(0,horizontalSpacing());
	int spaceY = qMax(0,verticalSpacing());

	QSize minSize;
	QLayoutItem *item;
	for ( int i = 0; i < itemList.size(); ++i )
		minSize = minSize.expandedTo(itemList[i]->minimumSize());

	//itemsPerRow = 2;
	//usedItemWidth = (effectiveRect.width()-(itemsPerRow-1)*spaceX) / itemsPerRow;

	int startIdx = 0;
	while ( true ) {
		int numItems = 0;
		for ( int i = startIdx; i < itemList.size(); ++i, ++numItems ) {
			item = itemList[i];

			itemWidth = qMax(minSize.width(), item->sizeHint().width());
			if ( usedItemWidth >= 0 && itemWidth > usedItemWidth )
				itemWidth = usedItemWidth;

			int nextX = x + itemWidth + spaceX;

			// Finished a line
			if ( nextX - spaceX > effectiveRect.right()+1 && lineHeight > 0 )
				break;

			x = nextX;
			lineHeight = qMax(lineHeight, item->sizeHint().height());
		}

		if ( numItems <= 0 ) break;
		//if ( numItems > 2 ) numItems = 2;

		if ( itemsPerRow < 0 ) {
			itemsPerRow = (effectiveRect.width() - spaceX) /
			               (minSize.width() + spaceX);
			if ( itemsPerRow > numItems ) itemsPerRow = numItems;
			else if ( itemsPerRow < 1 ) itemsPerRow = 1;
			usedItemWidth = (effectiveRect.width()-(itemsPerRow-1)*spaceX) / itemsPerRow;
		}

		int maxItems = itemsPerRow;
		if ( maxItems > numItems ) maxItems = numItems;
		if ( maxItems <= 0 ) break;

		//std::cout << "idx:" << startIdx << ",y:" << y << ",cnt:" << numItems << std::endl;

		x = effectiveRect.x();
		lineHeight = 0;
		for ( int i = startIdx; i < startIdx+maxItems; ++i ) {
			item = itemList[i];

			int itemHeight;
			if ( item->hasHeightForWidth() )
				itemHeight = item->heightForWidth(usedItemWidth);
			else
				itemHeight = item->sizeHint().height();

			if ( !testOnly )
				item->setGeometry(QRect(QPoint(x, y),
				                  QSize(usedItemWidth, itemHeight)));
			x += usedItemWidth + spaceX;
			lineHeight = qMax(lineHeight, itemHeight);
		}

		x = effectiveRect.x();
		y += lineHeight + spaceY;
		lineHeight = 0;

		startIdx += maxItems;
	}

	return y + lineHeight - rect.y() + bottom;
}
#if QT_VERSION >= 0x040300


int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const {
	QObject *parent = this->parent();
	if ( !parent )
		return -1;
	else if ( parent->isWidgetType() ) {
		QWidget *pw = static_cast<QWidget *>(parent);
		return pw->style()->pixelMetric(pm, 0, pw);
	}
	else
		return static_cast<QLayout *>(parent)->spacing();
}
#endif
