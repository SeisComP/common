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


#ifndef SEISCOMP_PROCESSING_QCPROCESSOR_H
#define SEISCOMP_PROCESSING_QCPROCESSOR_H

#include <deque>

#include <seiscomp/core/datetime.h>
#include <seiscomp/processing/waveformprocessor.h>
#include <seiscomp/core/baseobject.h>

#include <boost/any.hpp>

using boost::any_cast;

namespace Seiscomp {
namespace Processing {




DEFINE_SMARTPOINTER(QcProcessorObserver);

class QcProcessorObserver: public Core::BaseObject {
	DECLARE_SC_CLASS(QcProcessorObserver);

	public:
		QcProcessorObserver();
		virtual void update() = 0;
};



DEFINE_SMARTPOINTER(QcParameter);

//! This class represents the QC processing result.
class SC_SYSTEM_CLIENT_API QcParameter : public Core::BaseObject {
	DECLARE_SC_CLASS(QcParameter);

	public:
		QcParameter() {}

		Core::Time recordStartTime, recordEndTime;
		float recordSamplingFrequency;
		boost::any parameter;
};



DEFINE_SMARTPOINTER(QcProcessor);

class SC_SYSTEM_CLIENT_API QcProcessor : public WaveformProcessor {
	DECLARE_SC_CLASS(QcProcessor);

	public:
		//! Constructor
		QcProcessor(const Core::TimeSpan& deadTime=0.0, const Core::TimeSpan &gapThreshold=300.0);

		//! Destructor
		virtual ~QcProcessor();

		//! To register in QC processing interested observers
		bool subscribe(QcProcessorObserver *obs);

		//! To unregister in QC processing uninterested observers
		bool unsubscribe(QcProcessorObserver *obs);

		//! Returns the result of QC processing
		QcParameter* getState() const;

		//! Calculates the specific result in derived classes
		virtual bool setState(const Record* record, const DoubleArray& data) = 0;

		//! Returns true if QC processing result is successfully initialized; false otherwise
		bool isSet() const;

		//! Returns true in case of a valid value in QC processing result; false otherwise
		bool isValid() const;

	protected:
		//! Implements the inherited method
		//! Notifies registered observers
		virtual void process(const Record* record, const DoubleArray& data);

		QcParameterPtr _qcp;
    
	private:
		std::deque<QcProcessorObserver *> _observers;
		bool _setFlag;
		bool _validFlag;
};


}
}

#endif
