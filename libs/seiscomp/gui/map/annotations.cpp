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


#include <seiscomp/gui/core/application.h>
#include <QPainter>

#include "annotations.h"


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AnnotationStyle::AnnotationStyle(QObject *parent) : QObject(parent) {
	palette[0].borderPen = SCScheme.colors.map.annotations.normalBorder;
	palette[0].textPen = SCScheme.colors.map.annotations.normalText;
	palette[0].brush = SCScheme.colors.map.annotations.normalBackground;

	palette[1].borderPen = SCScheme.colors.map.annotations.highlightedBorder;
	palette[1].textPen = SCScheme.colors.map.annotations.highlightedText;
	palette[1].brush = SCScheme.colors.map.annotations.highlightedBackground;

	font.setPointSize(SCScheme.colors.map.annotations.textSize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnnotationStyle::draw(QPainter &painter, const AnnotationItem &item) {
	Palette &pal = palette[item.highlighted ? 1 : 0];

	painter.setFont(font);
	painter.setBrush(pal.brush);
	painter.setPen(pal.borderPen);
	painter.drawRoundedRect(item.labelRect, 3, 3);
	painter.setPen(pal.textPen);
	painter.drawText(item.labelRect, Qt::AlignCenter | Qt::TextSingleLine,
	                 item.text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRect AnnotationStyle::itemRect(QPainter &painter, const AnnotationItem &item,
                                const QPoint &pos) const {
	painter.setFont(font);

	if ( item.text.isEmpty() ) {
		return QRect();
	}

	QRect labelRect(painter.fontMetrics().boundingRect(item.text));
	int em = painter.fontMetrics().height();
	labelRect.moveBottomLeft(pos);
	labelRect.translate(-labelRect.width()/2, -em/4);
	labelRect.adjust(-em/4,-em/4,em/4,em/4);

	return labelRect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AnnotationItem::~AnnotationItem() {
	if ( pool ) {
		pool->take(this);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Annotations::Annotations(QObject *parent)
: QObject(parent) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Annotations::~Annotations() {
	clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AnnotationItem* Annotations::add(const QString &text) {
	AnnotationItem *item = new AnnotationItem(this, text);
	item->style = _style?_style:&_defaultStyle;

	_items.append(item);

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Annotations::clear() {
	while ( !_items.empty() ) {
		AnnotationItem *item = _items.front();
		item->pool = nullptr;
		delete item;
		_items.pop_front();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Annotations::remove(const AnnotationItem *item) {
	if ( take(item) ) {
		delete item;
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Annotations::take(const AnnotationItem *item) {
	AnnotationItem *ptr = const_cast<AnnotationItem*>(item);
	if ( !_items.removeOne(ptr) ) {
		return false;
	}

	ptr->pool = nullptr;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Map
} // ns Gui
} // ns Gempa
