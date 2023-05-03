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


#ifndef SEISCOMP_UTILS_TABVALUES_H
#define SEISCOMP_UTILS_TABVALUES_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core.h>

#include <vector>
#include <string>


namespace Seiscomp {
namespace Util {


DEFINE_SMARTPOINTER(TabValues);

/**
 * @brief The TabValues class manages a table of values which can be
 *        extracted with interpolation.
 *
 * A table consists of an X and a Y axis each with a set of given nodes.
 * The table format on disc is compatible to the phase tables of LOCSAT.
 *
 * \code
 * n # Something
 *  yn # Number of y samples
 *    y1, y2, y3, ..., yn
 *  xn # Number of x samples
 *    x1, x2, x3, ..., xn
 *  xn samples for y1
 *  xn samples for y2
 *  ...
 *  xn samples for yn
 * \endcode
 *
 * Samples are separated with spaces.
 */
class SC_SYSTEM_CORE_API TabValues : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		using ValueList = std::vector<double>;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		TabValues();


	public:
		/**
		 * @brief Reads a value table in LOCSAT format.
		 * @param filename The path to the file to be read.
		 * @return Success flag
		 */
		bool read(const std::string &filename);

		bool validValue(size_t ix, size_t iy) const;
		double value(size_t ix, size_t iy) const;

		bool interpolate(double &interpolatedValue,
		                 bool extrapolation, bool inHole, bool returnYDerivs,
		                 double xPos, double yPos,
		                 double *x1stDeriv = 0, double *y1stDeriv = 0,
		                 double *x2ndDeriv = 0, double *y2ndDeriv = 0,
		                 int *error = 0) const;

		bool interpolate(double &interpolatedValue,
		                 bool extrapolation, bool returnYDerivs,
		                 double xPos, double yPos,
		                 double *x1stDeriv = 0, double *y1stDeriv = 0,
		                 double *x2ndDeriv = 0, double *y2ndDeriv = 0,
		                 int *error = 0) const;


	// ----------------------------------------------------------------------
	//  Public members
	// ----------------------------------------------------------------------
	public:
		// X,Y value range
		std::string            header;
		ValueList              x, y;
		OPT(double)            lowerUndefinedX, upperUndefinedX;
		std::vector<ValueList> values; // y-major
};


inline bool TabValues::validValue(size_t ix, size_t iy) const {
	return values[iy][ix] != -1;
}

inline double TabValues::value(size_t ix, size_t iy) const {
	return values[iy][ix];
}


}
}

#endif
