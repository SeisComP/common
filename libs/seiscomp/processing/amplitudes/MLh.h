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


#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_MLH_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_MLH_H


#include <seiscomp/processing/amplitudes/ML.h>


namespace Seiscomp {
namespace Processing {


//! Wrapper class that allows access to protected methods for
//! the proxy (see below).
class SC_SYSTEM_CLIENT_API AmplitudeProcessor_MLh : public AbstractAmplitudeProcessor_ML {
	public:
		AmplitudeProcessor_MLh();

	friend class AmplitudeProcessor_ML2h;
};


//! Proxy amplitude processor that holds two MLh processors to calculate
//! the amplitudes on both horizontals and then averages the result.
//! This class does not handle waveforms itself. It directs them to the
//! corresponding amplitude processors instead.
class SC_SYSTEM_CLIENT_API AmplitudeProcessor_ML2h : public AmplitudeProcessor {
	DECLARE_SC_CLASS(AmplitudeProcessor_ML2h)

	public:
		AmplitudeProcessor_ML2h();
		AmplitudeProcessor_ML2h(const Core::Time &trigger);

	public:
		int capabilities() const override;
		IDList capabilityParameters(Capability cap) const override;
		bool setParameter(Capability cap, const std::string &value) override;

		bool setup(const Settings &settings) override;

		void setTrigger(const Core::Time& trigger) override;

		void computeTimeWindow() override;

		void reset() override;
		void close() const override;

		bool feed(const Record *record) override;

		bool computeAmplitude(const DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt, AmplitudeValue *amplitude,
		                      double *period, double *snr) override;

		const AmplitudeProcessor *componentProcessor(Component comp) const override;
		const DoubleArray *processedData(Component comp) const override;

		void reprocess(OPT(double) searchBegin, OPT(double) searchEnd) override;


	protected:
		double timeWindowLength(double distance) const override;


	private:
		void newAmplitude(const AmplitudeProcessor *proc,
		                  const AmplitudeProcessor::Result &res);


	private:
		struct ComponentResult {
			AmplitudeValue value;
			AmplitudeTime  time;
			double         snr;
		};

		enum CombinerProc {
			TakeMin,
			TakeMax,
			TakeAverage,
			TakeGeometricMean
		};

		mutable AmplitudeProcessor_MLh _ampE, _ampN;
		CombinerProc                   _combiner;
		OPT(ComponentResult)           _results[2];
};


}
}


#endif
