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


#ifndef SEISCOMP_PROCESSING_WAVEFORMPROCESSOR_H
#define SEISCOMP_PROCESSING_WAVEFORMPROCESSOR_H


#include <seiscomp/core/record.h>
#include <seiscomp/core/recordsequence.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/enumeration.h>
#include <seiscomp/math/filter.h>
#include <seiscomp/processing/processor.h>
#include <seiscomp/processing/stream.h>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(WaveformOperator);
DEFINE_SMARTPOINTER(WaveformProcessor);


class SC_SYSTEM_CLIENT_API WaveformProcessor : public Processor {
	DECLARE_SC_CLASS(WaveformProcessor)

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		enum Component {
			VerticalComponent         = 0,  /* usually Z */
			FirstHorizontalComponent  = 1,  /* usually N */
			SecondHorizontalComponent = 2   /* usually E */
		};

		enum StreamComponent {
			Vertical            = 0,  /* usually Z */
			FirstHorizontal     = 1,  /* usually N */
			SecondHorizontal    = 2,  /* usually E */
			Horizontal,               /* usually N + E */
			Any                       /* all available components */
		};

		typedef Math::Filtering::InPlaceFilter<double> Filter;

		MAKEENUM(
			SignalUnit,
			EVALUES(
				//! Displacement
				Meter,
				//! Velocity
				MeterPerSecond,
				//! Acceleration
				MeterPerSecondSquared
			),
			// SEED names: see SEEDManual, ch.5 / 34, p.47
			ENAMES(
				"M",
				"M/S",
				"M/S**2"
			)
		);


		MAKEENUM(
			Status,
			EVALUES(
				//! No associated value
				WaitingForData,
				//! Associated value is progress in [0,100]
				InProgress,
				//! Associated value is 100
				Finished,
				//! Associated value is the last status
				Terminated,
				//! Associated value is the failed snr value
				LowSNR,
				//! No associated value yet
				QCError,
				//! Data is clipped
				DataClipped,
				//! Error during deconvolution
				DeconvolutionFailed,
				//! Distance hint is out of range to continue processing
				DistanceOutOfRange,
				//! Depth hint is out of range to continue processing
				DepthOutOfRange,
				//! Unit is not supported, e.g. m/s <> m/s**2
				IncompatibleUnit,
				//! Orientation missing
				MissingOrientation,
				//! Gain missing
				MissingGain,
				//! Response missing
				MissingResponse,
				//! Sampling frequency does not match. Either records of
				//! one trace have different sampling frequencies or
				//! the sampling frequencies of different channels do not match.
				InvalidSamplingFreq,
				//! No associated value yet (error code?)
				Error,
				// -- The following enumerations were added with API 12 ---
				//! No distance hint set
				MissingDistance,
				//! No depth hint set
				MissingDepth,
				//! No time hint set
				MissingTime,
				//! No hypocenter (Origin) given
				MissingHypocenter,
				//! No receiver (SensorLocation) given
				MissingReceiver,
				//! No pick (Pick) given
				MissingPick,
				//! Metadata is incomplete, e.g. a particualr stream attribute
				//! is not set or empty
				IncompleteMetadata,
				//! The epicentre is out of supported regions
				EpicenterOutOfRegions,
				//! The receiver is out of supported regions
				ReceiverOutOfRegions,
				//! The entire raypath does not lie entirely in the supported
				//! regions
				RayPathOutOfRegions,
				//! Travel time table lookup failed
				TravelTimeEstimateFailed,
				//! Arrival has not been found
				ArrivalNotFound,
				//! Configuration error
				ConfigurationError,
				// -- The following enumerations were added with API 17 ---
				//! Period out of range
				PeriodOutOfRange
			),
			ENAMES(
				"waiting for data",
				"in progress",
				"finished",
				"terminated",
				"low SNR",
				"QC error",
				"data clipped",
				"deconvolution failed",
				"distance out of range",
				"depth out of range",
				"incompatible unit",
				"missing orientation",
				"missing gain",
				"missing response",
				"invalid sampling frequency",
				"error",
				"missing distance hint",
				"missing depth hint",
				"missing time hint",
				"missing hypocenter",
				"missing receiver",
				"missing pick",
				"incomplete metadata",
				"epicenter out of regions",
				"receiver out of regions",
				"ray path out of regions",
				"travel time estimate failed",
				"arrival not found",
				"configuration error",
				"period out of range"
			)
		);


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		WaveformProcessor(const Core::TimeSpan &initTime = Core::TimeSpan(0, 0),
		                  const Core::TimeSpan &gapThreshold = Core::TimeSpan(0, 100000));

		//! D'tor
		virtual ~WaveformProcessor();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Call this method in derived classes to make sure
		//! the filters samplingFrequency will be set correctly
		//! The feed method filters the data according to the
		//! set filter and calls process.
		//! NOTE: This method does not distinguish between different
		//!       channelcodes. Its just feeds what it gets into the
		//!       data array. If multiple channels need to be combined
		//!       this method must be reimplemented.
		virtual bool feed(const Record *record);

		//! Convenience function to feed a whole record sequence into
		//! a WaveformProcessor.
		//! feed(const Record *) is used to feed a single record
		//! @return The number of records successfully fed
		int feedSequence(const RecordSequence *sequence);

		//! Sets a userdata pointer that is managed by the processor
		//! (stored inside a SmartPointer).
		void setUserData(Core::BaseObject *obj) const;

		//! Returns the set userdata object. This object must not be
		//! deleted by the caller. It is managed by the processor and
		//! dereferenced (and deleted) automatically. But the
		//! caller may hold a SmartPointer to that object.
		Core::BaseObject *userData() const;

		//! Sets up the waveform processor. By default it reads whether
		//! to check for data saturation or not
		virtual bool setup(const Settings &settings) override;

		//! Initialize the filter for the given sampling frequency
		virtual void initFilter(double fsamp);

		//! Sets the filter to apply
		virtual void setFilter(Filter *filter);

		//! Sets a operator for all fed records. An operator sits between
		//! feed and store.
		void setOperator(WaveformOperator *pipe);
		WaveformOperator *getOperator() const;

		//! The gain is input unit -> sensor unit
		Stream &streamConfig(Component c);
		const Stream &streamConfig(Component c) const;

		//! Sets the component to use to calculate the amplitude for
		void setUsedComponent(StreamComponent c) { _usedComponent = c; }

		//! Returns the used component
		StreamComponent usedComponent() const { return _usedComponent; }

		//! Sets the maximal gap length in seconds for that
		//! missing samples are handled or tolerated. Default: no tolerance
		void setGapTolerance(const Core::TimeSpan &length);
		const Core::TimeSpan &gapTolerance() const;

		//! Enables/disables the linear interpolation of missing samples
		//! inside a set gap tolerance
		void setGapInterpolationEnabled(bool enable);
		bool isGapInterpolationEnabled() const;

		//! Enables saturation check of absolute values of incoming samples and
		//! sets the status to DataClipped if checked positive. The data
		//! is checked in the fill method. If derived classes reimplement
		//! this method without calling this implementation, the check is
		//! not performed.
		void setSaturationCheckEnabled(bool enable);

		//! Sets the saturation threshold. The default is -1
		void setSaturationThreshold(double t);

		//! Returns whether saturation check is enabled
		bool isSaturationCheckEnabled() const;

		//! Returns the saturation threshold that is applied if saturation
		//! check is enabled.
		double saturationThreshold() const { return _saturationThreshold; }

		/**
		 * @brief Resets the processing state
		 *
		 * This method resets the current processing states back to as if
		 * wouldn't have received any data yet. It not *not* meant to reset
		 * the processors configuration to its defaults. This method is going
		 * to be called when e.g. gaps are detected.
		 */
		virtual void reset();

		//! Returns the data's sampling frequency
		double samplingFrequency() const;

		void setEnabled(bool e);
		bool isEnabled() const { return _enabled; }

		//! Returns the timewindow of already processed data
		Core::TimeWindow dataTimeWindow() const;

		//! Returns the last fed record
		const Record *lastRecord() const;

		//! Returns the current status of the processor
		Status status() const;

		//! Returns the value associated with the status
		double statusValue() const;

		//! Terminates the processor ignoring its current state
		void terminate();

		//! Default implementation returns if the status if greater
		//! than InProgress.
		virtual bool isFinished() const;

		//! Closes the processor meaning that no more records
		//! are going to be fed in. The processing has been finished.
		virtual void close() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		//! Callback methods to react on changes of the
		//! enable state
		virtual void disabled() {}
		virtual void enabled() {}

		//! Virtual method that must be used in derived classes to analyse
		//! a datastream.
		//! It gives the raw record and the filtered data array as parameter.
		virtual void process(const Record *record,
		                     const DoubleArray &filteredData) = 0;

		//! Handles gaps. Returns whether the gap has been handled or not.
		virtual bool handleGap(Filter *filter, const Core::TimeSpan&,
		                       double lastSample, double nextSample,
		                       size_t missingSamples);

		virtual void fill(size_t n, double *samples);
		virtual bool store(const Record *rec);

		void setStatus(Status status, double value);

		bool parseSaturationThreshold(const Settings &settings,
		                              const std::string &optionName);

		void setupStream(double fsamp);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	protected:
		//! Describes the current state of a component (e.g. Z or N)
		struct StreamState {
			StreamState();
			~StreamState();

			//! Value of the last sample
			double            lastSample;

			//! Number of samples required to finish initialization
			size_t            neededSamples;
			//! Number of samples already received
			size_t            receivedSamples;
			//! Initialization state
			bool              initialized;

			//! The last received record on this component
			RecordCPtr        lastRecord;
			//! The complete processed data time window so far
			Core::TimeWindow  dataTimeWindow;

			//! The sampling frequency of the component
			double            fsamp;
			//! The filter (if used)
			Filter           *filter;
		};

		bool                        _enabled;

		Core::TimeSpan              _initTime;

		//! threshold to recognize a gap
		Core::TimeSpan              _gapThreshold;
		//! gap length to tolerate
		Core::TimeSpan              _gapTolerance;

		//! default: false
		bool                        _enableSaturationCheck;
		double                      _saturationThreshold;

		//! default: false
		bool                        _enableGapInterpolation;

		StreamState                 _stream;

		//! default: vertical
		StreamComponent             _usedComponent;
		Stream                      _streamConfig[3];


	private:
		Status                      _status;
		double                      _statusValue;
		WaveformOperatorPtr         _operator;

		mutable Core::BaseObjectPtr _userData;
};


inline WaveformOperator *WaveformProcessor::getOperator() const {
	return _operator.get();
}


}
}


#endif
