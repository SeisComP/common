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


#include <seiscomp/core/optional.h>
#include <seiscomp/core/datetime.h>

// Explicit template instantiation of some optional types
template class boost::optional<bool>;
template class boost::optional<int>;
template class boost::optional<float>;
template class boost::optional<double>;
template class boost::optional<Seiscomp::Core::Time>;


namespace Seiscomp {
namespace Core {


::boost::none_t const None = ::boost::none;

ValueError::ValueError() throw() {
}

ValueError::~ValueError() throw() {
}

const char* ValueError::what() const throw() {
	return "requested value has not been set";
}


}
}
