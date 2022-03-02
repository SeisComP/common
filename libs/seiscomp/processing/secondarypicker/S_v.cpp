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


#define SEISCOMP_COMPONENT VPicker

#include <seiscomp/logging/log.h>
#include <seiscomp/processing/operator/ncomps.h>
#include <seiscomp/math/filter.h>

#include "S_v.h"


using namespace std;

namespace Seiscomp {

namespace Processing {

REGISTER_SECONDARYPICKPROCESSOR(SVPicker, "S-V");


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SVPicker::SVPicker() : SAICPicker("V-AIC", Vertical) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SVPicker::~SVPicker() { }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SVPicker::setup(const Settings &settings) {
	if ( !SecondaryPicker::setup(settings) ) return false;

	try { setNoiseStart(settings.getDouble("spicker.V.noiseBegin")); }
	catch ( ... ) {}

	try { setSignalStart(settings.getDouble("spicker.V.signalBegin")); }
	catch ( ... ) {}

	try { setSignalEnd(settings.getDouble("spicker.V.signalEnd")); }
	catch ( ... ) {}

	AICConfig cfg = aicConfig();

	try { cfg.threshold = settings.getDouble("spicker.V.threshold"); }
	catch ( ... ) {}

	try { cfg.minSNR = settings.getDouble("spicker.V.minSNR"); }
	catch ( ... ) {}

	try { cfg.margin = settings.getDouble("spicker.V.marginAIC"); }
	catch ( ... ) {}

	try { cfg.timeCorr = settings.getDouble("spicker.V.timeCorr"); }
	catch ( ... ) {}

	try { cfg.filter = settings.getString("spicker.V.filter"); }
	catch ( ... ) { cfg.filter = "BW(4,0.3,1.0)"; }

	try { cfg.detecFilter = settings.getString("spicker.V.detecFilter"); }
	catch ( ... ) { cfg.detecFilter = "STALTA(1,10)"; }

	return setAicConfig(cfg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformOperator* SVPicker::createFilterOperator(Filter* compFilter)
{
	typedef Operator::FilterWrapper<double,1,Operator::NoOpWrapper<double,1>> FilterL2V;
	typedef NCompsOperator<double,1,FilterL2V> L2V;
	return new L2V(
			FilterL2V(compFilter, Operator::NoOpWrapper<double,1>(_streamConfig))
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
