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



#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_mB_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_mB_H

#include <seiscomp/processing/amplitudeprocessor.h>


namespace Seiscomp {

namespace Processing {


class SC_SYSTEM_CLIENT_API AmplitudeProcessor_mB : public AmplitudeProcessor {
	public:
		AmplitudeProcessor_mB();
		AmplitudeProcessor_mB(const Core::Time& trigger);

	public:
		void finalizeAmplitude(DataModel::Amplitude *amplitude) const override;

	protected:
		double timeWindowLength(double distance) const override;
		bool computeAmplitude(const DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt,
		                      AmplitudeValue *amplitude,
		                      double *period, double *snr) override;
};


}

}


#endif
