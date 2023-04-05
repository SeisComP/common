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


#ifndef SEISCOMP_IO_RECORDFILTER_CROP
#define SEISCOMP_IO_RECORDFILTER_CROP


#include <seiscomp/io/recordfilter.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core/record.h>

#include <deque>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(Cropper);
class SC_SYSTEM_CORE_API Cropper : public RecordFilterInterface {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		~Cropper() override;


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool setWindowLength(double length);
		//! Must be less than 1
		bool setWindowOverlap(double overlap);
		bool setNoAlign(bool noalign);

		//! Push an record and compute spectra. Returns true if a records
		//! can be popped. The record id (net,sta,loc,cha) is not checked
		//! against the last record pushed. This filtering is left to
		//! the caller.
		Record *feed(const Record *rec) override;

		//! Requests to flush pending data. Flush should be called until
		//! nullptr is returned to flush all pending records.
		//! @return A copy of the flushed record
		Record *flush() override;

		//! Resets the record filter.
		void reset() override;

		RecordFilterInterface *clone() const override;


	protected:
		//! Clean up all associated resources
		void cleanup();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		struct CropBuffer {
			// Fixed source sample rate derived from the first record
			// received.
			double sampleRate;
			double dt;

			// The ring buffer that holds the last samples.
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
			Core::Time startTime;

			// End time of last record
			Core::Time lastEndTime;

			void reset() {
				missingSamples = buffer.size();
				front = 0;
				samplesToSkip = 0;
				startTime = Core::Time();
				lastEndTime = Core::Time();
			}
		};

		void init(const Record *rec);
		void crop(const Record *rec);

		double               _windowLength{20.0};
		double               _timeStep{10.0};
		bool                 _noalign{false};
		CropBuffer          *_buffer{nullptr};
		std::deque<Record*>  _nextRecords;
};


}
}


#endif
