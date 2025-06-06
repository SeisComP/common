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


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
: QLayout(parent), _hSpace(hSpacing), _vSpace(vSpacing) {
	setContentsMargins(margin, margin, margin, margin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
: _hSpace(hSpacing), _vSpace(vSpacing) {
	setContentsMargins(margin, margin, margin, margin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FlowLayout::~FlowLayout() {
	QLayoutItem *item;
	while ( (item = takeAt(0)) ) {
		delete item;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FlowLayout::addItem(QLayoutItem *item) {
	_itemList.append(item);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FlowLayout::horizontalSpacing() const {
	if ( _hSpace >= 0 ) {
		return _hSpace;
	}
	else {
		return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FlowLayout::verticalSpacing() const {
	if ( _vSpace >= 0 ) {
		return _vSpace;
	}
	else {
		return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FlowLayout::count() const {
	return _itemList.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QLayoutItem *FlowLayout::itemAt(int index) const {
	return _itemList.value(index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QLayoutItem *FlowLayout::takeAt(int index) {
	if ( index >= 0 && index < _itemList.size() ) {
		QLayoutItem *item = _itemList[index];
		_itemList.remove(index);
		return item;
	}
	else {
		return 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::Orientations FlowLayout::expandingDirections() const {
	return Qt::Orientations();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FlowLayout::hasHeightForWidth() const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FlowLayout::heightForWidth(int width) const {
	int height = doLayout(QRect(0, 0, width, 0), true);
	return height;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FlowLayout::setGeometry(const QRect &rect) {
	QLayout::setGeometry(rect);
	doLayout(rect, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QSize FlowLayout::sizeHint() const {
	return minimumSize();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QSize FlowLayout::minimumSize() const {
	QSize size;
	QLayoutItem *item;

	foreach (item, _itemList) {
		size = size.expandedTo(item->minimumSize());
	}

	size += QSize(contentsMargins().left() + contentsMargins().right(),
	              contentsMargins().top() + contentsMargins().bottom());
	return size;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FlowLayout::doLayout(const QRect &rect, bool testOnly) const {
	int left, top, right, bottom;
	left = contentsMargins().left();
	right = contentsMargins().right();
	top = contentsMargins().top();
	bottom = contentsMargins().bottom();
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
	for ( int i = 0; i < _itemList.size(); ++i ) {
		minSize = minSize.expandedTo(_itemList[i]->minimumSize());
	}

	int startIdx = 0;
	while ( true ) {
		int numItems = 0;
		for ( int i = startIdx; i < _itemList.size(); ++i, ++numItems ) {
			item = _itemList[i];

			itemWidth = qMax(minSize.width(), item->sizeHint().width());
			if ( usedItemWidth >= 0 && itemWidth > usedItemWidth ) {
				itemWidth = usedItemWidth;
			}

			int nextX = x + itemWidth + spaceX;

			// Finished a line
			if ( nextX - spaceX > effectiveRect.right()+1 && lineHeight > 0 ) {
				break;
			}

			x = nextX;
			lineHeight = qMax(lineHeight, item->sizeHint().height());
		}

		if ( numItems <= 0 ) {
			break;
		}

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


		x = effectiveRect.x();
		lineHeight = 0;
		for ( int i = startIdx; i < startIdx+maxItems; ++i ) {
			item = _itemList[i];

			int itemHeight;
			if ( item->hasHeightForWidth() ) {
				itemHeight = item->heightForWidth(usedItemWidth);
			}
			else {
				itemHeight = item->sizeHint().height();
			}

			if ( !testOnly ) {
				item->setGeometry(QRect(QPoint(x, y),
				                  QSize(usedItemWidth, itemHeight)));
			}

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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const {
	QObject *parent = this->parent();
	if ( !parent ) {
		return -1;
	}
	else if ( parent->isWidgetType() ) {
		QWidget *pw = static_cast<QWidget *>(parent);
		return pw->style()->pixelMetric(pm, 0, pw);
	}
	else {
		return static_cast<QLayout *>(parent)->spacing();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
