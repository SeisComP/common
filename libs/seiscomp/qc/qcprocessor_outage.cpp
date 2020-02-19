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


#define SEISCOMP_COMPONENT SCQC
#include <seiscomp/logging/log.h>
#include <seiscomp/qc/qcprocessor_outage.h>


namespace Seiscomp {
namespace Processing {

IMPLEMENT_SC_CLASS_DERIVED(QcProcessorOutage, QcProcessor, "QcProcessorOutage");


QcProcessorOutage::QcProcessorOutage() : QcProcessor(), _threshold(1800) {}

void QcProcessorOutage::setThreshold(int threshold) {
	_threshold = threshold;
}

bool QcProcessorOutage::setState(const Record *record, const DoubleArray &data) {
	if (_stream.lastRecord) {
		try {
			Core::Time lastRecEnd = _stream.lastRecord->endTime();
			Core::Time curRecStart = record->startTime();
			double diff = 0.0;

			/* to handle out-of-order records */
			if (_recent < lastRecEnd ) {
				diff = (double)(curRecStart - lastRecEnd);
				_recent = lastRecEnd;
			}
			else {
				SEISCOMP_DEBUG("QcProcessorOutage::setState() for %s.%s.%s.%s -> recent: %s lastRecEnd: %s curRecStart: %s",
				               record->networkCode().c_str(),record->stationCode().c_str(),record->locationCode().c_str(),
				               record->channelCode().c_str(),_recent.iso().c_str(),lastRecEnd.iso().c_str(),curRecStart.iso().c_str());

				if (_recent < curRecStart)
					diff = (double)(curRecStart - _recent);
			}

			if ( diff >= _threshold ) {
				_qcp->parameter = diff;
				return true;
			}
		}
		catch (Core::ValueException) {}
	}

	return false;
}

double QcProcessorOutage::getOutage() {
	try {
		return boost::any_cast<double>(_qcp->parameter);
	}
	catch (const boost::bad_any_cast &) {
		throw Core::ValueException("no data");
	}
}


}
}

