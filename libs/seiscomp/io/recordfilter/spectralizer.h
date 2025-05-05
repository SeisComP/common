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


#ifndef SEISCOMP_IO_RECORDFILTER_SPECTRALIZER
#define SEISCOMP_IO_RECORDFILTER_SPECTRALIZER


#include <map>
#include <deque>

#include <seiscomp/math/filter.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/genericrecord.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(Spectrum);
class SC_SYSTEM_CORE_API Spectrum : public Core::BaseObject {
	public:
		Spectrum(const Core::Time &stime,
		         const Core::Time &etime,
		         const Core::TimeSpan &dt,
		         double freq, int sampleCount)
		: _startTime(stime), _endTime(etime), _dt(dt), _sampleCount(sampleCount)
		, _frequency(freq) {}

		void setData(ComplexDoubleArray *data) {
			_data = data;
		}

		bool isValid() const { return _data && _data->size() > 0; }

		const ComplexDoubleArray *data() const { return _data.get(); }
		ComplexDoubleArray *data() { return _data.get(); }

		const Core::Time &startTime() const { return _startTime; }
		const Core::Time &endTime() const { return _endTime; }
		const Core::TimeSpan &dt() const { return _dt; }

		Core::TimeSpan length() const { return _endTime - _startTime; }
		Core::Time center() const { return _startTime + Core::TimeSpan(double(length())*0.5); }

		double minimumFrequency() const { return 0; }
		double maximumFrequency() const { return _frequency; }


	private:
		Core::Time            _startTime;
		Core::Time            _endTime;
		Core::TimeSpan        _dt;
		int                   _sampleCount;
		double                _frequency;
		ComplexDoubleArrayPtr _data;
};



DEFINE_SMARTPOINTER(Spectralizer);
class SC_SYSTEM_CORE_API Spectralizer : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief The Options struct holds the parameters used to
		 * compute the spectrum.
		 */
		struct Options {
			Options();

			//! The window length in seconds used to compute the spectrum.
			double      windowLength;
			//! The window overlap for subsequent spectra. This value is
			//! defined as a fraction of windowLength and must be in [0,1).
			//! The effective time step is windowLength*windowOverlap.
			double      windowOverlap;
			//! The output spectrum samples. A value of -1 returns the full
			//! spectrum whereas values > 0 reduce the spectrum to the given
			//! number of samples using the maximum value of each bin with
			//! respect to magnitude of each spectrum sample.
			int         specSamples;
			//! An optional filter applied in advance to compute a spectrum.
			std::string filter;
			//! Disables aligning the processed time window with the given
			//! time step.
			bool        noalign;
			//! The taper width applied to either side of the processed time
			//! window given as fraction of windowLength, e.g. 0.05 for 5%.
			double      taperWidth;
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Spectralizer();
		virtual ~Spectralizer();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setOptions(const Options &opts);

		//! Push an record and compute spectra. Returns true if a records
		//! can be popped. The record id (net,sta,loc,cha) is not checked
		//! against the last record pushed. This filtering is left to
		//! the caller.
		bool push(const Record *rec);

		//! Pops an spectrum if available. Returns nullptr if more data are
		//! required.
		Spectrum *pop();

		//! Returns whether records are available
		bool canPop() const { return !_nextSpectra.empty(); }

		//! Clean up all associated resources
		void cleanup();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		using FilterPtr = Core::SmartPointer<Math::Filtering::InPlaceFilter<double>>;
		struct SpecBuffer {
			SpecBuffer() {}
			~SpecBuffer() {}

			FilterPtr filter;

			// Fixed source sample rate derived from the first record
			// received.
			double sampleRate;
			double dt;

			// The ring buffer that holds the last samples for fft.
			std::vector<double> buffer;
			DoubleArray tmp;
			int tmpOffset;

			size_t samplesToSkip;

			// The number of samples still missing in the buffer before
			// fft can be done
			size_t missingSamples;

			// The front index of the ring buffer
			size_t front;

			// Time of front of ring buffer
			OPT(Core::Time) startTime;

			// End time of last record
			OPT(Core::Time) lastEndTime;

			void reset(FilterPtr refFilter) {
				missingSamples = buffer.size();
				front = 0;
				samplesToSkip = 0;
				startTime = Core::None;;
				lastEndTime = Core::None;

				if ( refFilter ) {
					filter = refFilter->clone();
					filter->setSamplingFrequency(sampleRate);
				}
				else
					filter = nullptr;
			}
		};

		void init(const Record *rec);
		Record *fft(const Record *rec);

		double                        _windowLength;
		double                        _timeStep;
		bool                          _noalign;
		int                           _specSamples;
		double                        _taperWidth;
		FilterPtr                     _filter;
		SpecBuffer                   *_buffer;
		std::deque<Spectrum*>         _nextSpectra;
};


}
}


#endif
