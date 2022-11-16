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


#include "math.h"
#include <cstdint>


namespace Seiscomp {
namespace Math {


namespace {


template <typename T>
inline int getFixedDigits10(T val) {
	T e = 1;
	T p = std::log10(val < 0 ? -val : val);

	if ( p < 0 ) {
		p = std::floor(-p);
	}
	else {
		p = 0;
	}

	val *= std::pow(10, p);

	while ( std::abs(round(val * e) - val * e) > 1E-6 ) {
		e *= 10;
		++p;
	}

	return p;
}


template <typename T>
inline int getScientificDigits10(T val) {
	T e = 1;
	T p = std::log10(val < 0 ? -val : val);
	T n = 0;

	if ( p < 0 ) {
		p = std::floor(-p);
	}
	else {
		n = std::floor(p) + 1;
		p = 0;
	}

	val *= std::pow(10, p);

	p = n;

	while ( std::abs(round(val * e) - val * e) > 1E-6 ) {
		e *= 10;
		++p;
	}

	return p;
}


}


double round(double val) {
	return static_cast<int64_t>(val + 0.5);
}


int significantFixedDigits10(float val) {
	return getFixedDigits10(val);
}


int significantFixedDigits10(double val) {
	return getFixedDigits10(val);
}

int significantScientificDigits10(float val) {
	return getScientificDigits10(val);
}


int significantScientificDigits10(double val) {
	return getScientificDigits10(val);
}


} // namespace Math
} // namespace Seiscomp
