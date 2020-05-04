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


#ifndef SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_C_H
#define SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_C_H


#include <seiscomp//processing/magnitudes/m_B.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_mBc : public MagnitudeProcessor_mB {
	DECLARE_SC_CLASS(MagnitudeProcessor_mBc)

	public:
		MagnitudeProcessor_mBc();

		Status estimateMw(double magnitude, double &Mw_estimate,
		                  double &Mw_stdError) override;
};


}
}


#endif
