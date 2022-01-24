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


#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_ML_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_ML_H


#include <seiscomp/processing/amplitudeprocessor.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API AbstractAmplitudeProcessor_ML : public AmplitudeProcessor {
	public:
		AbstractAmplitudeProcessor_ML(const std::string &type);
		AbstractAmplitudeProcessor_ML(const Core::Time &trigger, const std::string &type);

	public:
		void initFilter(double fsamp) override;

		int capabilities() const override;
		IDList capabilityParameters(Capability cap) const override;
		bool setParameter(Capability cap, const std::string &value) override;

		bool setup(const Settings &settings) override;


	protected:
		void setDefaultConfiguration();

		bool deconvolveData(Response *resp, DoubleArray &data, int numberOfIntegrations) override;

		/**
		 * Computes the zero-to-peak amplitude on the simulated Wood-Anderson
		 * trace.
		 */
		bool computeAmplitude(const DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt, AmplitudeValue *amplitude,
		                      double *period, double *snr) override;

		double timeWindowLength(double distance) const override;

	protected:
		enum AmplitudeMeasureType {
			AbsMax,
			MinMax,
			PeakTrough
		};

		AmplitudeMeasureType _amplitudeMeasureType;
		std::string          _preFilter;
		bool                 _applyWA{true};
};


}
}


#endif
