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



#define SEISCOMP_COMPONENT AmplitudeMLv

#include <seiscomp/processing/amplitudes/MLv.h>


using namespace Seiscomp::Math;

namespace Seiscomp {
namespace Processing {


IMPLEMENT_SC_CLASS_DERIVED(AmplitudeProcessor_MLv, AbstractAmplitudeProcessor_ML, "AmplitudeProcessor_MLv");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_MLv, "MLv");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_MLv::AmplitudeProcessor_MLv()
: AbstractAmplitudeProcessor_ML("MLv") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeProcessor_MLv::AmplitudeProcessor_MLv(const Core::Time &trigger)
: AbstractAmplitudeProcessor_ML(trigger, "MLv") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeProcessor_MLv::computeAmplitude(const DoubleArray &data,
                                              size_t i1, size_t i2,
                                              size_t si1, size_t si2,
                                              double offset,
                                              AmplitudeIndex *dt, AmplitudeValue *amplitude,
                                              double *period, double *snr) {
	bool r = AbstractAmplitudeProcessor_ML::computeAmplitude(data, i1, i2, si1, si2,
	                                                         offset, dt, amplitude,
	                                                         period, snr);
	if ( !r ) return false;

	// Apply empirical correction for measuring ML on the vertical component.
	// Normally ML is measured on both horizontal components and the average
	// is taken.
	amplitude->value *= 2.0;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
