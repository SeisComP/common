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


#ifndef SEISCOMP_GUI_PLOT_DATAY_H
#define SEISCOMP_GUI_PLOT_DATAY_H


#include <seiscomp/gui/plot/abstractdataset.h>
#include <QVector>


namespace Seiscomp {
namespace Gui {


/**
 * @brief The DataY class describes a data set with an array of values and
 *        a procedural key space which is linearily interpolated for each
 *        value between index 0 and N-1. By definition the lower key corresponds
 *        with value at index 0 and the upper key with value at index N-1.
 *        That means the distance between two subsequent keys is range.length / (N-1)
 *        which requires at least two values to render a valid key space.
 */
class SC_GUI_API DataY : public AbstractDataSet {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief DataPoint represents a single value in a graph. The key values
		 *        will be generated.
		 */
		typedef double DataPoint;

		/**
		 * @brief DataPoints represents the graph data as list of data points.
		 */
		typedef QVector<DataPoint> DataPoints;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		DataY(QObject *parent = 0);


	// ----------------------------------------------------------------------
	//  AbstractGraph interface
	// ----------------------------------------------------------------------
	public:
		virtual int count() const;
		virtual Range getXRange() const;
		virtual Range getYRange() const;

		virtual void clear();

		virtual void unproject(QPolygonF &poly,
		                       const Axis *keyAxis,
		                       const Axis *valueAxis) const;


	// ----------------------------------------------------------------------
	//  Public members
	// ----------------------------------------------------------------------
	public:
		//! The x range
		Range      x;
		//! The y data points
		DataPoints y;
};


}
}


#endif
