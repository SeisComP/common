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


#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/plot/legend.h>
#include <seiscomp/gui/plot/graph.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Legend::Legend(Qt::Alignment align, QObject *parent)
: AbstractLegend(parent), _alignment(align) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::draw(QPainter &p, const QRect &plotRect,
                  const QList<Graph*> &graphs) {
	int width = 0;
	int height = 0;

	QFontMetrics fm = p.fontMetrics();
	int fontHeight = fm.height();
	int halfFontHeight = fontHeight / 2;

	foreach ( Graph *g, graphs ) {
		if ( !g->isVisible() ) continue;
		if ( g->isEmpty() ) continue;
		if ( g->name().isEmpty() ) continue;

		width = qMax(width, QT_FM_WIDTH(fm, g->name()));
		height += fontHeight;
	}

	QRect r(0,0,width+5*fontHeight/2,height+fontHeight);
	if ( _alignment & Qt::AlignLeft )
		r.moveLeft(plotRect.left());
	else
		r.moveRight(plotRect.right()-1);

	if ( _alignment & Qt::AlignBottom )
		r.moveBottom(plotRect.bottom()-1);
	else
		r.moveTop(plotRect.top());

	p.setRenderHint(QPainter::Antialiasing, false);
	p.setPen(QColor(192,192,192));
	p.setBrush(QColor(255,255,255,192));
	p.drawRect(r);

	int lx = r.left() + halfFontHeight;
	int tx = r.left() + 2*fontHeight;
	int y = r.top() + halfFontHeight;

	QRect symbolRect(lx, y, fontHeight, fontHeight);

	foreach ( Graph *g, graphs ) {
		if ( !g->isVisible() ) continue;
		if ( g->isEmpty() ) continue;
		if ( g->name().isEmpty() ) continue;

		g->drawSymbol(p, symbolRect);
		p.setPen(Qt::black);
		p.drawText(tx, y, width, fontHeight, Qt::AlignLeft | Qt::AlignVCenter, g->name());

		y += fontHeight;
		symbolRect.moveTop(y);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
