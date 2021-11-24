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


#ifndef SEISCOMP_PROCESSING_PICKER_H
#define SEISCOMP_PROCESSING_PICKER_H


#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/processing/timewindowprocessor.h>
#include <seiscomp/client.h>
#include <boost/function.hpp>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(Picker);


class SC_SYSTEM_CLIENT_API Picker : public TimeWindowProcessor {
	DECLARE_SC_CLASS(Picker)

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		//! Configuration structure to store processing dependent
		//! settings
		struct Config {
			double noiseBegin;  // start of the noise window to initialize filters
			double signalBegin; // default: -30, relative to _trigger
			double signalEnd;   // default: +10, relative to _trigger
			double snrMin;      // default: 3
		};

		enum Polarity {
			POSITIVE,
			NEGATIVE,
			UNDECIDABLE
		};

		struct Result {
			const Record *record;
			double        snr;
			Core::Time    time;
			double        timeLowerUncertainty;
			double        timeUpperUncertainty;
			double        timeWindowBegin;
			double        timeWindowEnd;
			OPT(double)   slowness;
			OPT(double)   backAzimuth;
			OPT(Polarity) polarity;
		};

		typedef boost::function<void (const Picker*,
		                              const Result &)> PublishFunc;

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Picker();
		Picker(const Core::Time& trigger);

		//! D'tor
		~Picker();


	// ----------------------------------------------------------------------
	//  Configuration Interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the name of the method used by concrete implementations
		virtual const std::string &methodID() const = 0;

		//! Returns the filter used by concrete implementations
		virtual const std::string &filterID() const = 0;

		//! Set the start of the noise window relative to the trigger
		void setNoiseStart(double start) { _config.noiseBegin = start; }

		//! Set the start of the signal window relative to the trigger
		void setSignalStart(double start)  { _config.signalBegin = start; }

		//! Set the end of the signal window relative to the trigger
		void setSignalEnd(double end)  { _config.signalEnd = end; }

		void setMinSNR(double snr) { _config.snrMin = snr; }

		//! Returns the current configuration
		const Config &config() const { return _config; }

		//! This method has to be called when all configuration
		//! settings has been set to calculate the timewindow
		void computeTimeWindow() override;

		void reset() override;


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the trigger used to compute the timewindow to calculate
		//! the amplitude
		//! Once a trigger has been set all succeeding calls will fail.
		void setTrigger(const Core::Time &trigger);
		Core::Time trigger() const;

		/**
		 * @brief Returns the currently set noise window
		 *
		 * This is being computed as trigger + noiseBegin and
		 * trigger + signalBegin
		 * @return The time window of the noise
		 */
		Core::TimeWindow noiseWindow() const;

		/**
		 * @brief Returns the currently set signal window
		 *
		 * This is being computed as trigger + signalBegin and
		 * trigger + signalEnd
		 * @return The time window of the signal
		 */
		Core::TimeWindow signalWindow() const;

		void setPublishFunction(const PublishFunc& func);

		//! Dumps the record data into an ASCII file
		void writeData();


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		//! Implement this method to compute a pick.
		//! The default implementation returns false and does nothing.
		//! The returned uncertainty measures are in samples.
		//! Negative uncertainties are invalid and not recognized further.
		virtual bool calculatePick(int n, const double *data,
		                           int signalStartIdx, int signalEndIdx,
		                           int &triggerIdx, int &lowerUncertainty,
		                           int &upperUncertainty, double &snr,
		                           OPT(Polarity) &polarity) = 0;

		void process(const Record *record, const DoubleArray &filteredData) override;
		bool handleGap(Filter *filter, const Core::TimeSpan& span,
		               double lastSample, double nextSample,
		               size_t missingSamples) override;

		//! This method is called when a pick has to be published
		void emitPick(const Result &result);


	private:
		void init();


	// ----------------------------------------------------------------------
	//  Protected Members
	// ----------------------------------------------------------------------
	protected:
		Core::Time _trigger;

		// config
		Config _config;


	// ----------------------------------------------------------------------
	//  Private Members
	// ----------------------------------------------------------------------
	private:
		PublishFunc _func;
};


DEFINE_INTERFACE_FACTORY(Picker);


inline Core::Time Picker::trigger() const {
	return _trigger;
}

inline Core::TimeWindow Picker::noiseWindow() const {
	return Core::TimeWindow(
		_trigger + Core::TimeSpan(_config.noiseBegin),
		_trigger + Core::TimeSpan(_config.signalBegin)
	);
}

inline Core::TimeWindow Picker::signalWindow() const {
	return Core::TimeWindow(
		_trigger + Core::TimeSpan(_config.signalBegin),
		_trigger + Core::TimeSpan(_config.signalEnd)
	);
}


}
}


#define REGISTER_POSTPICKPROCESSOR_VAR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Processing::Picker, Class> __##Class##InterfaceFactory__(Service)

#define REGISTER_POSTPICKPROCESSOR(Class, Service) \
static REGISTER_POSTPICKPROCESSOR_VAR(Class, Service)


#endif
