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


#ifndef SEISCOMP_PROCESSING_FX_H
#define SEISCOMP_PROCESSING_FX_H


#include <seiscomp/processing/timewindowprocessor.h>
#include <boost/function.hpp>


namespace Seiscomp {

namespace DataModel {

class Pick;

}

namespace Processing {


DEFINE_SMARTPOINTER(FX);

/**
 * @brief Abstract (f)eature e(x)traction processor
 */
class SC_SYSTEM_CLIENT_API FX : public TimeWindowProcessor {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		//! Configuration structure to store processing dependent
		//! settings
		struct Config {
			double noiseBegin; /* default: -10 */
			double noiseEnd; /* default: -1 */
			double signalBegin; /* default: -1 */
			double signalEnd; /* default: 5 */
		};

		struct Environment {
			Environment(const DataModel::SensorLocation *sloc = 0,
			            const DataModel::Pick           *pick = 0);

			const DataModel::SensorLocation *receiver;
			const DataModel::Pick           *pick;
		};

		struct Result {
			StreamComponent component;
			const Record   *record;
			double          snr;
		};

		typedef boost::function<void (const FX*,
		                              const Result &)> PublishFunc;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		FX();
		FX(const Core::Time &trigger);


	// ----------------------------------------------------------------------
	//  Configuration Interface
	// ----------------------------------------------------------------------
	public:
		//! Set the start of the noise window relative to the trigger
		void setNoiseStart(double start) { _config.noiseBegin = start; }

		//! Set the end of the noise window relative to the trigger
		void setNoiseEnd(double end)  { _config.noiseEnd = end; }

		//! Set the start of the signal window relative to the trigger
		void setSignalStart(double start)  { _config.signalBegin = start; }

		//! Set the end of the signal window relative to the trigger
		void setSignalEnd(double end)  { _config.signalEnd = end; }

		//! Returns the current configuration
		const Config &config() const { return _config; }

		//! This method has to be called when all configuration
		//! settings have been set to calculate the timewindow
		void computeTimeWindow() override;

		/**
		 * @brief Sets the environment for the FX processor. Basically
		 *        it is the receiver and the pick made. The
		 *        pick time must correspond to the trigger time set.
		 * @param receiver The receiver
		 * @param pick The pick
		 */
		virtual void setEnvironment(const DataModel::SensorLocation *receiver,
		                            const DataModel::Pick *pick);

		const Environment &environment() const { return _environment; }


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the trigger used to compute the timewindow to calculate
		//! the amplitude
		//! Once a trigger has been set all succeeding calls will fail.
		void setTrigger(const Core::Time &trigger);

		Core::Time trigger() const;

		void setPublishFunction(const PublishFunc &func);

		/**
		 * @brief Allows to finalize a pick object as created by client code.
		 *
		 * This method will usually be called right before the pick will
		 * be stored or sent and inside the emit handler. It allows processors
		 * to set specific attributes or to add comments.
		 * The default implementation does nothing.
		 * @param pick The pick to be finalized
		 */
		virtual void finalizePick(DataModel::Pick *pick) const;


	// ----------------------------------------------------------------------
	//  Waveform processor interface
	// ----------------------------------------------------------------------
	public:
		void process(const Record *record, const DoubleArray &) override;


	// ----------------------------------------------------------------------
	//  Protected functions
	// ----------------------------------------------------------------------
	protected:
		void setDefault();

		//! This method gets called when an amplitude has to be published
		void emit(const Result &result);


	// ----------------------------------------------------------------------
	//  Protected Members
	// ----------------------------------------------------------------------
	protected:
		Core::Time  _trigger;
		Config      _config;
		Environment _environment;


	// ----------------------------------------------------------------------
	//  Private Members
	// ----------------------------------------------------------------------
	private:
		PublishFunc _func;
};


DEFINE_INTERFACE_FACTORY(FX);


inline Core::Time FX::trigger() const {
	return _trigger;
}


}
}


#define REGISTER_FXPROCESSOR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Processing::FX, Class> __##Class##InterfaceFactory__(Service)


#endif
