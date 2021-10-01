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


#include "annotationlayer.h"

#include <seiscomp/gui/map/canvas.h>

#include <QPainter>


namespace Seiscomp {
namespace Gui {
namespace Map {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AnnotationLayer::AnnotationLayer(QObject *parent, Annotations *annotations)
: Layer(parent) {
	setAnnotations(annotations);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AnnotationLayer::draw(const Canvas *canvas, QPainter &painter) {
	if ( !_annotations ) return;

	painter.save();

	int width = canvas->width();
	int height = canvas->height();
	int fontHeight = painter.fontMetrics().height();

	Grid grid(height / fontHeight + 1);

	for ( const AnnotationItem *item : *_annotations ) {
		if ( !item->visible ) continue;

		int centerX = item->labelRect.center().x();

		if ( centerX < 0 || centerX >= width ) continue;

		int minGridY = item->labelRect.top() / fontHeight;
		int maxGridY = item->labelRect.bottom() / fontHeight;

		if ( minGridY >= grid.count() ) continue;
		if ( maxGridY < 0 ) continue;

		if ( minGridY < 0 ) minGridY = 0;
		if ( maxGridY >= grid.count() ) maxGridY = grid.count() - 1;

		bool foundPlace = true;

		for ( int gridY = minGridY; (gridY <= maxGridY) && foundPlace; ++gridY ) {
			QList<QRect> &gridRow = grid[gridY];

			for ( const QRect &rect : gridRow ) {
				if ( rect.intersects(item->labelRect) ) {
					foundPlace = false;
					break;
				}
			}
		}

		if ( !foundPlace ) continue;

		for ( int gridY = minGridY; (gridY <= maxGridY) && foundPlace; ++gridY ) {
			grid[gridY].append(item->labelRect);
		}

		item->style->draw(painter, *item);
	}

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Map
} // ns Gui
} // ns Seiscomp
