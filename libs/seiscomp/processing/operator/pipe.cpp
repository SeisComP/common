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



#include <seiscomp/processing/operator/pipe.h>


namespace Seiscomp {
namespace Processing {


PipeOperator::PipeOperator(WaveformOperator *op1, WaveformOperator *op2)
: _op1(op1), _op2(op2) {
	WaveformOperator::connect(op1, op2);
}


WaveformProcessor::Status PipeOperator::feed(const Record *rec) {
	return _op1->feed(rec);
}


void PipeOperator::reset() {
	if ( _op1 ) _op1->reset();
	if ( _op2 ) _op2->reset();
}


}
}
