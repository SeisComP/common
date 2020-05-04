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


#ifndef SEISCOMP_PROCESSING_DETECTOR_H
#define SEISCOMP_PROCESSING_DETECTOR_H


#include <seiscomp/processing/waveformprocessor.h>
#include <boost/function.hpp>
#include <seiscomp/client.h>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(Detector);

class SC_SYSTEM_CLIENT_API Detector : public WaveformProcessor {
	DECLARE_SC_CLASS(Detector)

	public:
		typedef boost::function<void (const Detector*, const Record*, const Core::Time&)> PublishFunc;

	public:
		Detector(double initTime);

	public:
		void setPublishFunction(const PublishFunc& func);

		void setOffset(const Core::TimeSpan& ofs);
		Core::TimeSpan offset() const;

		void setDeadTimeAfterPick(const Core::TimeSpan &dt);
		Core::TimeSpan deadTimeAfterPick() const;

		virtual const std::string &methodID() const = 0;

		virtual void reset() override;

	protected:
		virtual bool emitPick(const Record* rec, const Core::Time& t);

	private:
		PublishFunc    _func;
		Core::Time     _lastPick;
		Core::TimeSpan _offset;
		Core::TimeSpan _deadTimeAfterPick;
};



DEFINE_SMARTPOINTER(SimpleDetector);

class SC_SYSTEM_CLIENT_API SimpleDetector : public Detector {
	DECLARE_SC_CLASS(SimpleDetector)

	public:
		SimpleDetector(double deadTime = 0.0);
		SimpleDetector(double on, double off, double deadTime);

	public:
		void setDeadTime(double deadTime) { _initTime = deadTime; }
		void setThresholds(double on, double off);

		//! Reimplemented method to analyse the filtered datastream.
		//! It just checks whether a sample in filteredData exceeds the
		//! 'on' threshold and calls validateOn or goes below the 'off'
		//! threshold and calls validateOff.
		void process(const Record *record, const DoubleArray &filteredData) override;

		void reset() override;

	protected:
		bool isOn() const { return _triggered; }
		bool isOff() const { return !_triggered; }

		//! This methods returns whether a pick has been emitted after
		//! setting the triggering state to 'on' or not
		//! This value is only valid after the trigger state has
		//! switched from 'off' to 'on'.
		bool emittedPick() const { return _pickEmitted; }

		//! This method gets called whenever the 'on' threshold is reached.
		//! A derived class can return false for some reason to disable
		//! setting the trigger state to true.
		//!
		//! When false is returned the processing of the current record
		//! will be finished immediatly.
		virtual bool validateOn(const Record *record, size_t &i, const DoubleArray &filteredData);

		//! This method gets called whenever a filtered sample falls below
		//! the 'off' threshold. A derived class can return false to disable
		//! setting the trigger state to false. When false is returned the
		//! processing of the current record will be finished immediatly.
		virtual bool validateOff(const Record *record, size_t i, const DoubleArray &filteredData);

		virtual const std::string &methodID() const override;


	private:
		bool          _triggered;
		bool          _pickEmitted;
		double        _thresholdOn;
		double        _thresholdOff;
};


}
}


#endif
