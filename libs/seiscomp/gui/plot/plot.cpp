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


#include <seiscomp/gui/plot/axis.h>
#include <seiscomp/gui/plot/graph.h>
#include <seiscomp/gui/plot/plot.h>
#include <seiscomp/gui/plot/abstractlegend.h>

#include <QPainter>
#include <iostream>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Plot::Plot(QObject *parent) : QObject(parent), _legend(nullptr) {
	xAxis = new Axis(this);
	xAxis->setPosition(Axis::Bottom);
	xAxis->setGrid(true);

	yAxis = new Axis(this);
	yAxis->setPosition(Axis::Left);
	yAxis->setGrid(true);

	xAxis2 = new Axis(this);
	xAxis2->setPosition(Axis::Top);
	xAxis2->setVisible(false);

	yAxis2 = new Axis(this);
	yAxis2->setPosition(Axis::Right);
	yAxis2->setVisible(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Graph *Plot::addGraph(Axis *keyAxis, Axis *valueAxis) {
	Graph *graph = new Graph(keyAxis == nullptr ? xAxis : keyAxis,
	                         valueAxis == nullptr ? yAxis : valueAxis,
	                         this);
	_graphs.append(graph);
	return graph;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plot::addGraph(Graph *graph) {
	graph->setParent(this);
	_graphs.append(graph);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Axis *Plot::addAxis(Axis::AxisPosition pos) {
	Axis *axis = nullptr;

	switch ( pos ) {
		case Axis::Bottom:
			axis = new Axis(this);
			axis->setPosition(pos);
			_extraXAxis1.append(axis);
			break;
		case Axis::Top:
			axis = new Axis(this);
			axis->setPosition(pos);
			_extraXAxis2.append(axis);
			break;
		case Axis::Left:
			axis = new Axis(this);
			axis->setPosition(pos);
			_extraYAxis1.append(axis);
			break;
		case Axis::Right:
			axis = new Axis(this);
			axis->setPosition(pos);
			_extraYAxis2.append(axis);
			break;
		default:
			break;
	}
	return axis;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plot::setLegend(AbstractLegend *legend) {
	if ( _legend == legend ) return;

	if ( _legend != nullptr )
		delete _legend;

	_legend = legend;

	if ( _legend != nullptr )
		_legend->setParent(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plot::updateRanges() {
	// Reset ranges
	xAxis->setRange(Range());
	yAxis->setRange(Range());
	xAxis2->setRange(Range());
	yAxis2->setRange(Range());

	for ( int i = 0; i < _extraXAxis1.count(); ++i )
		_extraXAxis1[i]->setRange(Range());
	for ( int i = 0; i < _extraXAxis2.count(); ++i )
		_extraXAxis2[i]->setRange(Range());
	for ( int i = 0; i < _extraYAxis1.count(); ++i )
		_extraYAxis1[i]->setRange(Range());
	for ( int i = 0; i < _extraYAxis2.count(); ++i )
		_extraYAxis2[i]->setRange(Range());

	foreach ( Graph *graph, _graphs ) {
		Range key, value;

		if ( graph->isEmpty() ) continue;
		graph->getBounds(key, value);

		graph->keyAxis()->extendRange(key);
		graph->valueAxis()->extendRange(value);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plot::draw(QPainter &p, const QRect &rect) {
#define AXIS_SPACING 6
	int xAxisHeight = xAxis->isVisible() ? xAxis->sizeHint(p) : 0;
	int xAxis2Height = xAxis2->isVisible() ? xAxis2->sizeHint(p) : 0;

	_plotRect = rect;
	_plotRect.adjust(0,xAxis2Height,0,-xAxisHeight);

	p.save();

	if ( p.device()->devicePixelRatioF() > 1.0 ) {
		p.setRenderHint(QPainter::Antialiasing, true);
		p.translate(0.5, 0.5);
	}

	// Draw axis
	for ( int i = _extraYAxis1.count()-1; i >= 0; --i ) {
		if ( _extraYAxis1[i]->isVisible() ) {
			QRect yAxisRect(_plotRect.left(),_plotRect.top(),0,_plotRect.height());
			_extraYAxis1[i]->updateLayout(p, yAxisRect);
			_extraYAxis1[i]->draw(p, yAxisRect);
			_plotRect.adjust(yAxisRect.width()+AXIS_SPACING-1,0,0,0);
		}
	}

	if ( yAxis->isVisible() ) {
		QRect yAxisRect(_plotRect.left(),_plotRect.top(),0,_plotRect.height());
		yAxis->updateLayout(p, yAxisRect);
		yAxis->draw(p, yAxisRect);
		_plotRect.adjust(yAxisRect.width()-1,0,0,0);
	}

	for ( int i = _extraYAxis2.count()-1; i >= 0; --i ) {
		if ( !_extraYAxis2[i]->isVisible() ) continue;
		QRect yAxis2Rect(_plotRect.right(),_plotRect.top(),0,_plotRect.height());
		_extraYAxis2[i]->updateLayout(p, yAxis2Rect);
		_extraYAxis2[i]->draw(p, yAxis2Rect);
		_plotRect.adjust(0,0,-yAxis2Rect.width()-AXIS_SPACING-1,0);
	}

	if ( yAxis2->isVisible() ) {
		QRect yAxis2Rect(_plotRect.right(),_plotRect.top(),0,_plotRect.height());
		yAxis2->updateLayout(p, yAxis2Rect);
		yAxis2->draw(p, yAxis2Rect);
		_plotRect.adjust(0,0,-(yAxis2Rect.width()+1),0);
	}

	if ( xAxis->isVisible() ) {
		QRect xAxisRect(_plotRect.left(),rect.bottom(),_plotRect.width(),0);
		xAxis->updateLayout(p, xAxisRect);
		xAxis->draw(p, xAxisRect);
	}

	if ( xAxis2->isVisible() ) {
		QRect xAxis2Rect(_plotRect.left(),rect.top(),_plotRect.width(),0);
		xAxis2->updateLayout(p, xAxis2Rect);
		xAxis2->draw(p, xAxis2Rect);
	}

	// Setup clipping
	if ( yAxis->isVisible() ) _plotRect.adjust( yAxis->pen().width()+1, 0, 0, 0);
	for ( int i = 0; i < _extraYAxis1.count(); ++i ) {
		if ( _extraYAxis1[i]->isVisible() )
			_plotRect.adjust( _extraYAxis1[i]->pen().width()+1, 0, 0, 0);
	}

	if ( xAxis2->isVisible() ) _plotRect.adjust( 0, xAxis2->pen().width()+1, 0, 0);

	if ( yAxis2->isVisible() ) _plotRect.adjust( 0, 0,-yAxis2->pen().width()-1, 0);
	for ( int i = 0; i < _extraYAxis2.count(); ++i ) {
		if ( _extraYAxis2[i]->isVisible() )
			_plotRect.adjust( 0, 0,-_extraYAxis2[i]->pen().width()-1, 0);
	}

	if ( xAxis->isVisible() ) _plotRect.adjust( 0, 0, 0, -xAxis->pen().width()-1);

	p.setClipRect(_plotRect);

	if ( xAxis->isVisible() && xAxis->hasGrid() ) xAxis->drawGrid(p, _plotRect, false, true);
	if ( yAxis->isVisible() && yAxis->hasGrid() ) yAxis->drawGrid(p, _plotRect, false, true);
	for ( int i = 0; i < _extraYAxis1.count(); ++i )
		if ( _extraYAxis1[i]->isVisible() && _extraYAxis1[i]->hasGrid() ) _extraYAxis1[i]->drawGrid(p, _plotRect, false, true);
	if ( xAxis2->isVisible() && xAxis2->hasGrid() ) xAxis2->drawGrid(p, _plotRect, false, true);
	if ( yAxis2->isVisible() && yAxis2->hasGrid() ) yAxis2->drawGrid(p, _plotRect, false, true);
	for ( int i = 0; i < _extraYAxis2.count(); ++i )
		if ( _extraYAxis2[i]->isVisible() && _extraYAxis2[i]->hasGrid() ) _extraYAxis2[i]->drawGrid(p, _plotRect, false, true);

	if ( xAxis->isVisible() && xAxis->hasGrid() ) xAxis->drawGrid(p, _plotRect, true, false);
	if ( yAxis->isVisible() && yAxis->hasGrid() ) yAxis->drawGrid(p, _plotRect, true, false);
	for ( int i = 0; i < _extraYAxis1.count(); ++i )
		if ( _extraYAxis1[i]->isVisible() && _extraYAxis1[i]->hasGrid() ) _extraYAxis1[i]->drawGrid(p, _plotRect, true, false);
	if ( xAxis2->isVisible() && xAxis2->hasGrid() ) xAxis2->drawGrid(p, _plotRect, true, false);
	if ( yAxis2->isVisible() && yAxis2->hasGrid() ) yAxis2->drawGrid(p, _plotRect, true, false);
	for ( int i = 0; i < _extraYAxis2.count(); ++i )
		if ( _extraYAxis2[i]->isVisible() && _extraYAxis2[i]->hasGrid() ) _extraYAxis2[i]->drawGrid(p, _plotRect, true, false);

	p.translate(_plotRect.left(), _plotRect.bottom()+1);
	foreach ( Graph *graph, _graphs ) {
		if ( graph->isEmpty() || !graph->isVisible() )
			continue;

		if ( !graph->keyAxis() || !graph->keyAxis()->isVisible()
		  || !graph->valueAxis() || !graph->valueAxis()->isVisible() )
			continue;

		graph->draw(p);
	}

	if ( (_legend != nullptr) && _legend->isVisible() ) {
		p.translate(-_plotRect.left(), -_plotRect.bottom()-1);
		_legend->draw(p, _plotRect, _graphs);
	}

	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
