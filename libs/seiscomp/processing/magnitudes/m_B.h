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


#ifndef SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_H
#define SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_H


#include <seiscomp//processing/magnitudeprocessor.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_mB : public MagnitudeProcessor {
	public:
		MagnitudeProcessor_mB();
		MagnitudeProcessor_mB(const std::string &type);

	public:
		void setDefaults() override;
		bool setup(const Settings &settings) override;

		Status computeMagnitude(double amplitude, const std::string &unit,
		                        double period, double snr,
		                        double delta, double depth,
		                        const DataModel::Origin *hypocenter,
		                        const DataModel::SensorLocation *receiver,
		                        const DataModel::Amplitude *,
		                        const Locale *,
		                        double &value) override;

		Status estimateMw(const Config::Config *config,
		                  double magnitude, double &Mw_estimate,
		                  double &Mw_stdError) override;
};


}
}


#endif
