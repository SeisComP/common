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


#ifndef SEISCOMP_CORE_ARRAYFACTORY_H
#define SEISCOMP_CORE_ARRAYFACTORY_H

#include <seiscomp/core/array.h>


namespace Seiscomp {

/**
 * Factory class for the different array classes.
 */
class SC_SYSTEM_CORE_API ArrayFactory {
 public:
	//! Creates an array object specified by the given data type
	static Array* Create(Array::DataType toCreate, Array::DataType caller, int size, const void *data);
	static Array* Create(Array::DataType toCreate, const Array *source);
};

}

#endif
