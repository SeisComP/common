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


#include <seiscomp/qc/qcprocessor_latency.h>

#define SEISCOMP_COMPONENT SCQC
#include <seiscomp/logging/log.h>


namespace Seiscomp {
namespace Processing {

	
IMPLEMENT_SC_CLASS_DERIVED(QcProcessorLatency, QcProcessor, "QcProcessorLatency");


QcProcessorLatency::QcProcessorLatency() 
    : QcProcessor() {
	
	_lastRecordArrivalTime = Core::Time::UTC();

}




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcProcessorLatency::setState(const Record *record, const DoubleArray &data) {

	Core::Time now = Core::Time::UTC();

	_qcp->recordStartTime = now;
	_qcp->recordEndTime = now;
	_qcp->parameter = (double)(now - _lastRecordArrivalTime);
	_lastRecordArrivalTime = now;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double QcProcessorLatency::getLatency() {
	try {
		return boost::any_cast<double>(_qcp->parameter);
	}
	catch (const boost::bad_any_cast &) {
		throw Core::ValueException("no data");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
