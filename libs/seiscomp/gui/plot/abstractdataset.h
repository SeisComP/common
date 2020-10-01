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


#ifndef SEISCOMP_GUI_PLOT_ABSTRACTDATASET_H
#define SEISCOMP_GUI_PLOT_ABSTRACTDATASET_H


#include <seiscomp/gui/plot/range.h>
#include <QObject>
#include <QRectF>


class QPolygonF;


namespace Seiscomp {
namespace Gui {


class Axis;


class SC_GUI_API AbstractDataSet : public QObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		explicit AbstractDataSet(QObject *parent=0);


	// ----------------------------------------------------------------------
	//  Query interface
	// ----------------------------------------------------------------------
	public:
		bool isEmpty() const { return this->count() == 0; }

		virtual int count() const = 0;
		virtual Range getXRange() const = 0;
		virtual Range getYRange() const = 0;

		virtual void getBounds(Range &x, Range &y) const;

		/**
		 * @brief Convenience function that returns a QRectF instead of two
		 *        single ranges.
		 * @return The bounding rectangle of the graph data
		 */
		QRectF getBounds() const;

		virtual void clear() = 0;

		/**
		 * @brief Unprojects the data from axis space to pixel space
		 * @param poly The target polygon that holds the projected and possibly
		 *             clipped data. Note that the number of points in the
		 *             polygon does not necessarily match the number of points
		 *             in the graph due to clipping.
		 * @param keyAxis The axis to project keys to
		 * @param valueAxis The axis to project values to
		 */
		virtual void unproject(QPolygonF &poly,
		                       const Axis *keyAxis,
		                       const Axis *valueAxis) const = 0;
};


}
}


#endif
