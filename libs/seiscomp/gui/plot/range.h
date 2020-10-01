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


#ifndef SEISCOMP_GUI_PLOT_RANGE_H
#define SEISCOMP_GUI_PLOT_RANGE_H


#include <seiscomp/gui/qt.h>


namespace Seiscomp {
namespace Gui {


struct SC_GUI_API Range {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	//! Constructs an invalid range
	Range();

	//! Constructs a range with lower and upper bounds which will be normalized
	Range(double lower, double upper);


	// ----------------------------------------------------------------------
	//  Pulic interface
	// ----------------------------------------------------------------------
	double length() const;
	double center() const;
	bool contains(double value) const;
	void normalize();
	void extend(const Range &other);
	void translate(double delta);
	bool isValid() const;


	// ----------------------------------------------------------------------
	//  Attributes
	// ----------------------------------------------------------------------
	double lower;
	double upper;
};




inline Range::Range() : lower(1), upper(-1) {}
inline Range::Range(double lower, double upper) : lower(lower), upper(upper) {
	normalize();
}

inline double Range::length() const { return upper - lower; }

inline double Range::center() const { return (lower+upper)*0.5; }

inline bool Range::contains(double value) const {
	return (value >= lower) && (value <= upper);
}

inline bool Range::isValid() const { return lower <= upper; }

inline void Range::translate(double delta) { lower += delta; upper += delta; }


}
}


#endif
