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


#ifndef SEISCOMP_PROCESSING_TIMEWINDOWPROCESSOR
#define SEISCOMP_PROCESSING_TIMEWINDOWPROCESSOR


#include <seiscomp/core/recordsequence.h>
#include <seiscomp/processing/waveformprocessor.h>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(TimeWindowProcessor);

class SC_SYSTEM_CLIENT_API TimeWindowProcessor : public WaveformProcessor {
	DECLARE_SC_CLASS(TimeWindowProcessor)

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		TimeWindowProcessor();

		//! D'tor
		~TimeWindowProcessor();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		void reset() override;

		//! Sets the time window for the data to be fed
		void setTimeWindow(const Core::TimeWindow &tw);
		const Core::TimeWindow &timeWindow() const;

		//! Returns the time window including the safety
		//! margin.
		const Core::TimeWindow &safetyTimeWindow() const;

		//! Sets the timewindow margin added to the timewindow
		//! when feeding the data into the processor. The default
		//! margin are 60 seconds.
		void setMargin(const Core::TimeSpan&);
		const Core::TimeSpan &margin() const;

		//! Derived classes should implement this method to
		//! compute their needed timewindow
		virtual void computeTimeWindow() {}

		//! Returns the continuous data for the requested timewindow
		const DoubleArray &continuousData() const;


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		void fill(size_t n, double *samples) override;
		bool store(const Record *rec) override;


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	protected:
		DoubleArray      _data;

	private:
		Core::TimeWindow _timeWindow;
		Core::TimeWindow _safetyTimeWindow;
		Core::TimeSpan   _safetyMargin;
};


}
}


#endif
