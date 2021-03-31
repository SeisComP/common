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


#ifndef SEISCOMP_IO_RECORDFILTER_RESAMPLE_H
#define SEISCOMP_IO_RECORDFILTER_RESAMPLE_H


#include <seiscomp/io/recordfilter.h>
#include <seiscomp/core/genericrecord.h>

#include <mutex>
#include <deque>
#include <map>


namespace Seiscomp {
namespace IO {


//! @brief Base class for template class Resampler which defines
//! @brief common data structures.
class SC_SYSTEM_CORE_API RecordResamplerBase : public RecordFilterInterface {
	protected:
		RecordResamplerBase();
		virtual ~RecordResamplerBase();

	public:
		virtual Record *flush();
		virtual void reset();

	protected:
		typedef std::vector<double> Coefficients;
		typedef std::map<int, Coefficients*> CoefficientMap;

		static int                   _instanceCount;
		static CoefficientMap        _coefficients;
		static std::mutex            _coefficientMutex;

		double                       _currentRate;
		double                       _targetRate;
		double                       _fp;
		double                       _fs;
		int                          _maxN;
		int                          _lanczosKernelWidth;
		int                          _coeffScale;
};


//! @brief Resamples records to a given target rate.
//! @brief The output datatype is float.
//! This class does not demultiplexing and therefore handles only one stream
//! per instance or to put it into other words: subsequent fed records are not
//! checked for their stream ID and just taken as one contiuous stream.
//! Note: this class is only implemented for float and double outputs.
template <typename T>
class SC_SYSTEM_CORE_API RecordResampler : public RecordResamplerBase {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		RecordResampler(double targetFrequency, double fp = 0.7, double fs = 0.9,
		                double coeffScale = 10, int lanczosWidth = 3);

		//! D'tor
		virtual ~RecordResampler();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Feeds a record.
		//! @return A resampled record. May return nullptr if not enough data are
		//!         available to flush the record.
		virtual Record *feed(const Record *record);

		virtual void reset();

		RecordFilterInterface *clone() const;


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		struct Stage {
			double targetRate;

			// Fixed source sample rate derived from the first record
			// received.
			double sampleRate;
			double dt;

			int N;
			int N2;

			// The ring buffer that holds the last samples for downsampling.
			std::vector<T> buffer;

			// The number of samples still missing in the buffer before
			// filtering can be done
			size_t missingSamples;

			// The front index of the ring buffer
			size_t front;

			// Time of front of ring buffer
			Seiscomp::Core::Time startTime;

			// End time of last record
			Seiscomp::Core::Time lastEndTime;

			void reset() {
				missingSamples = buffer.size();
				front = 0;
				startTime = Seiscomp::Core::Time();
				lastEndTime = Seiscomp::Core::Time();
			}
		};

		struct DownsampleStage : Stage {
			DownsampleStage() : nextStage(nullptr) {}
			~DownsampleStage() { if ( nextStage ) delete nextStage; }

			// Flag that indicates that a streams is passed through
			// without resampling.
			bool passThrough;

			bool valid;

			size_t samplesToSkip;

			Coefficients *coefficients;

			DownsampleStage *nextStage;

			void reset() {
				Stage::reset();
				samplesToSkip = 0;
				if ( nextStage != nullptr ) nextStage->reset();
			}
		};

		struct UpsampleStage : Stage {
			double downRatio;
			int width;
		};

		void initCoefficients(DownsampleStage *stage);

		void init(DownsampleStage *stage, const Seiscomp::Record *rec, int upscale, int N);
		void init(UpsampleStage *stage, const Seiscomp::Record *rec, int N);

		Seiscomp::GenericRecord *resample(DownsampleStage *stage, const Seiscomp::Record *rec);
		Seiscomp::GenericRecord *resample(UpsampleStage *stage, const Seiscomp::Record *rec);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		DownsampleStage             *_downsampler;
		UpsampleStage               *_upsampler;
};


}
}


#endif
