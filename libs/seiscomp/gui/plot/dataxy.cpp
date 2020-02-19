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


#include <seiscomp/gui/plot/dataxy.h>
#include <seiscomp/gui/plot/axis.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataXY::DataXY(QObject *parent)
: AbstractDataSet(parent) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DataXY::count() const {
	return data.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Range DataXY::getXRange() const {
	int cnt = data.count();
	Range r;

	if ( !cnt ) return r;

	r.lower = r.upper = data[0].x();
	for ( int i = 1; i < cnt; ++i ) {
		if ( data[i].x() < r.lower ) r.lower = data[i].x();
		else if ( data[i].x() > r.upper ) r.upper = data[i].x();
	}

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Range DataXY::getYRange() const {
	int cnt = data.count();
	Range r;

	if ( !cnt ) return r;

	r.lower = r.upper = data[0].y();
	for ( int i = 1; i < cnt; ++i ) {
		if ( data[i].y() < r.lower ) r.lower = data[i].y();
		else if ( data[i].y() > r.upper ) r.upper = data[i].y();
	}

	return r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataXY::getBounds(Range &x, Range &y) const {
	int cnt = data.count();

	if ( !cnt ) return;

	x.lower = x.upper = data[0].x();
	y.lower = y.upper = data[0].y();

	for ( int i = 1; i < cnt; ++i ) {
		if ( data[i].x() < x.lower ) x.lower = data[i].x();
		else if ( data[i].x() > x.upper ) x.upper = data[i].x();

		if ( data[i].y() < y.lower ) y.lower = data[i].y();
		else if ( data[i].y() > y.upper ) y.upper = data[i].y();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataXY::clear() {
	data.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DataXY::unproject(QPolygonF &poly, const Axis *keyAxis,
                       const Axis *valueAxis) const {
	int i = 0;

	// Find first "visible" data sample
	while ( i < data.count() ) {
		if ( data[i].x() >= keyAxis->range().lower ) {
			if ( i ) --i;
			break;
		}

		++i;
	}

	bool lastVisible = true;
	for ( ; i < data.count(); ++i ) {
		if ( data[i].x() > keyAxis->range().upper ) {
			if ( !lastVisible ) break;
			lastVisible = false;
		}
		double x0 = keyAxis->unproject(data[i].x());
		double y0 = -valueAxis->unproject(data[i].y());
		poly.append(QPointF(x0, y0));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
