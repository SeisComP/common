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


#ifndef SEISCOMP_PROCESSING_MAGNITUDES_UTILS_H
#define SEISCOMP_PROCESSING_MAGNITUDES_UTILS_H


#include <string>
#include <vector>


namespace Seiscomp {
namespace Processing {


template <typename T>
struct TableXY {
	using Item = std::pair<double, T>;
	using Items = std::vector<Item>;

	bool empty() const;
	bool set(const std::string &definition);
	bool set(const std::vector<std::string> &definition);

	// Throws out_of_range if x is out of range
	T at(double x) const;

	Items items;
};


using LogA0 = TableXY<double>;


}
}


#include "utils.ipp"


#endif
