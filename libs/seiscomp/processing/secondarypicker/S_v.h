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


#ifndef SEISCOMP_PROCESSING_PICKER_S_V_H
#define SEISCOMP_PROCESSING_PICKER_S_V_H

#include "S_aic.h"

namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API SVPicker : public SAICPicker {
	public:
		//! C'tor
		SVPicker();

		//! D'tor
		~SVPicker();

		bool setup(const Settings &settings) override;

	protected:
		WaveformOperator* createFilterOperator(Filter* compFilter) override;
};


}
}


#endif
