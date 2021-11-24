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



#ifndef SEISCOMP_PROCESSING_SECONDARYPICKER_H
#define SEISCOMP_PROCESSING_SECONDARYPICKER_H


#include <seiscomp/core/interfacefactory.h>
#include <seiscomp/processing/timewindowprocessor.h>
#include <boost/function.hpp>



namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(SecondaryPicker);


class SC_SYSTEM_CLIENT_API SecondaryPicker : public TimeWindowProcessor {
	DECLARE_SC_CLASS(SecondaryPicker)

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		//! Configuration structure to store processing dependent
		//! settings
		struct Config {
			double noiseBegin;  // start of the noise window to initialize filters
			double signalBegin; // start time relative to P pick
			double signalEnd;   // end time relative to P pick
		};

		struct Result {
			const Record *record;
			double        snr;
			Core::Time    time;
			double        timeLowerUncertainty;
			double        timeUpperUncertainty;
			std::string   phaseCode;
			std::string   filterID;
			OPT(double)   slowness;
			OPT(double)   backAzimuth;
		};

		struct Trigger {
			Trigger()
			: onsetLowerUncertainty(-1), onsetUpperUncertainty(-1), snr(-1) {}

			Core::Time    onset;
			double        onsetLowerUncertainty;
			double        onsetUpperUncertainty;
			OPT(double)   slowness;
			OPT(double)   backAzimuth;
			double        snr;
		};


		typedef boost::function<void (const SecondaryPicker*,
		                              const Result &)> PublishFunc;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SecondaryPicker();

		//! D'tor
		~SecondaryPicker();


	// ----------------------------------------------------------------------
	//  Configuration Interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the methodID of the method used to determine
		//! the secondary pick.
		virtual const std::string &methodID() const = 0;

		//! Returns the filter used by concrete implementations
		virtual const std::string &filterID() const = 0;

		//! Set the start of the noise window relative to the trigger
		void setNoiseStart(double start) { _config.noiseBegin = start; }

		//! Set the start of the signal window relative to the trigger.
		void setSignalStart(double start)  { _config.signalBegin = start; }

		//! Set the end of the signal window relative to the trigger.
		void setSignalEnd(double end)  { _config.signalEnd = end; }

		//! Returns the current configuration
		const Config &config() const { return _config; }

		//! This method has to be called when all configuration
		//! settings has been set to calculate the timewindow.
		void computeTimeWindow() override;

		void reset() override;


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the trigger used to compute the timewindow to calculate
		//! the amplitude.
		//! Once a trigger has been set all succeeding calls will fail.
		void setTrigger(const Trigger& trigger);
		const Trigger &trigger() const { return _trigger; }

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

		void setReferencingPickID(const std::string&);
		const std::string& referencingPickID() const;


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		//! Zero gap tolerance!
		//! The default implementation does not tolerate gaps and does not
		//! handle them.
		bool handleGap(Filter *filter, const Core::TimeSpan& span,
		               double lastSample, double nextSample,
		               size_t missingSamples) override;


		//! This method is called when a pick has to be published
		void emitPick(const Result &result);


	// ----------------------------------------------------------------------
	//  Protected Members
	// ----------------------------------------------------------------------
	protected:
		Trigger _trigger;

		// config
		Config _config;


	// ----------------------------------------------------------------------
	//  Private Members
	// ----------------------------------------------------------------------
	private:
		PublishFunc _func;
		std::string _pickID;
};


DEFINE_INTERFACE_FACTORY(SecondaryPicker);


inline Core::TimeWindow SecondaryPicker::noiseWindow() const {
	return Core::TimeWindow(
		_trigger.onset + Core::TimeSpan(_config.noiseBegin),
		_trigger.onset + Core::TimeSpan(_config.signalBegin)
	);
}

inline Core::TimeWindow SecondaryPicker::signalWindow() const {
	return Core::TimeWindow(
		_trigger.onset + Core::TimeSpan(_config.signalBegin),
		_trigger.onset + Core::TimeSpan(_config.signalEnd)
	);
}


}
}


#define REGISTER_SECONDARYPICKPROCESSOR_VAR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Processing::SecondaryPicker, Class> __##Class##InterfaceFactory__(Service)

#define REGISTER_SECONDARYPICKPROCESSOR(Class, Service) \
static REGISTER_SECONDARYPICKPROCESSOR_VAR(Class, Service)


#endif
