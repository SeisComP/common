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


#ifndef SEISCOMP_GUI_PLOT_DATAXY_H
#define SEISCOMP_GUI_PLOT_DATAXY_H


#include <seiscomp/gui/plot/abstractdataset.h>
#include <QVector>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API DataXY : public AbstractDataSet {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief DataPoint represents a single point in a graph with a key
		 *        and a value.
		 */
		typedef QPointF DataPoint;

		/**
		 * @brief DataPoints represents the graph data as list of data points.
		 */
		typedef QVector<DataPoint> DataPoints;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		DataXY(QObject *parent = 0);


	// ----------------------------------------------------------------------
	//  AbstractGraph interface
	// ----------------------------------------------------------------------
	public:
		virtual int count() const;
		virtual Range getXRange() const;
		virtual Range getYRange() const;

		virtual void getBounds(Range &x, Range &y) const;

		virtual void clear();

		virtual void unproject(QPolygonF &poly,
		                       const Axis *keyAxis,
		                       const Axis *valueAxis) const;


	// ----------------------------------------------------------------------
	//  Public members
	// ----------------------------------------------------------------------
	public:
		DataPoints data;
};


}
}


#endif
