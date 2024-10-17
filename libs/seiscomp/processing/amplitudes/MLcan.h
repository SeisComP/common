/* ################################################################################
* #    Copyright (C) 2024 by IGN Spain                                           #
* #                                                                              #
* #    author: J. Barco, E. Suarez                                               #
* #    email:  jbarco@transportes.gob.es   ,  eadiaz@transportes.gob.es          #
* #    last modified: 2024-03-20                                                 #
* #                                                                              #
* #    This program is free software; you can redistribute it and/or modify      #
* #    it under the terms of the GNU General Public License as published by      #
* #    the Free Software Foundation; either version 2 of the License, or         #
* #    (at your option) any later version.                                       #
* #                                                                              #
* #    This program is distributed in the hope that it will be useful,           #
* #    but WITHOUT ANY WARRANTY; without even the implied warranty of            #
* #    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
* #    GNU General Public License for more details.                              #
* #                                                                              #
* #    You should have received a copy of the GNU General Public License         #
* #    along with this program; if not, write to the                             #
* #    Free Software Foundation, Inc.,                                           #
* #    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 #
* ################################################################################ */


#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_MLcan_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_MLcan_H


#include <seiscomp/processing/amplitudes/ML.h>


namespace Seiscomp {
namespace Processing {


//! Wrapper class that allows access to protected methods for
//! the proxy (see below).
class SC_SYSTEM_CLIENT_API AmplitudeProcessor_MLcan : public AbstractAmplitudeProcessor_ML {
	public:
		AmplitudeProcessor_MLcan();

	friend class AmplitudeProcessor_MLcan2h;
};


//! Proxy amplitude processor that holds two MLh processors to calculate
//! the amplitudes on both horizontals and then averages the result.
//! This class does not handle waveforms itself. It directs them to the
//! corresponding amplitude processors instead.
class SC_SYSTEM_CLIENT_API AmplitudeProcessor_MLcan2h : public AmplitudeProcessor {
	public:
		AmplitudeProcessor_MLcan2h();

	public:
		int capabilities() const override;
		IDList capabilityParameters(Capability cap) const override;
		bool setParameter(Capability cap, const std::string &value) override;
		std::string parameter(Capability cap) const override;

		bool setup(const Settings &settings) override;

		void setTrigger(const Core::Time& trigger) override;

		void setEnvironment(const DataModel::Origin *hypocenter,
		                    const DataModel::SensorLocation *receiver,
		                    const DataModel::Pick *pick) override;

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

		void reprocess(OPT(double) searchBegin, OPT(double) searchEnd) override;;


	private:
		void newAmplitude(const AmplitudeProcessor *proc,
		                  const AmplitudeProcessor::Result &res);


	private:
		struct ComponentResult {
			AmplitudeValue value;
			AmplitudeTime  time;
			double         snr;
			double         period;
		};

		enum CombinerProc {
			TakeMin,
			TakeMax,
			TakeAverage,
			TakeGeometricMean
		};

		mutable AmplitudeProcessor_MLcan _ampE, _ampN;
		CombinerProc                   _combiner;
		OPT(ComponentResult)           _results[2];
};


}
}


#endif
